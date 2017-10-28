#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <array>
#include <wiringPi.h>
#include <mosquitto.h>

using namespace std;

//Invalid value, bits 15 to 11 should all be sign bits
const uint16_t E_INVALID_SCRATCH = 0x8000;

const int TEMP_SENSOR_PIN = 15;
const char MQTT_HOST[] = "207.162.8.230";
const int MQTT_PORT = 8080;
const int MQTT_KEEPALIVE = 0;

typedef enum {
	SEARCH_ROM    = 0xF0,
	READ_ROM      = 0x33,
	MATCH_ROM     = 0x55,
	SKIP_ROM      = 0xCC,
	ALARM_SEARCH  = 0xEC
} ROM_CMDS_t;

typedef enum {
	CONVERT_T     = 0x44,
	WRITE_SCRATCH = 0x4E,
	READ_SCRATCH  = 0xBE,
	COPY_SCRATCH  = 0x48,
	RECALL_E2     = 0xB8,
	READ_PWR_SUP  = 0xB4
} FCT_CMDS_t;

chrono::time_point<chrono::system_clock> get_time_point(int us_delay) {
	return chrono::system_clock::now() + chrono::microseconds(us_delay);
}

void busywait_until(chrono::time_point<chrono::system_clock> end_tp) {
	while (chrono::system_clock::now() < end_tp) {
		//Busywait, do nothing
	}
}

void busywait(int us_delay) {
	busywait_until(get_time_point(us_delay));
}

void init(void) {
	wiringPiSetup();
	pinMode(TEMP_SENSOR_PIN, OUTPUT);
	pullUpDnControl(TEMP_SENSOR_PIN, PUD_UP);

	mosquitto_lib_init();
}

void oscope_trigger(void) {
	pinMode(TEMP_SENSOR_PIN, OUTPUT);

	//This sends a recognizable pattern for the oscilloscope to trigger on
	digitalWrite(TEMP_SENSOR_PIN, LOW);
	busywait(25);
	digitalWrite(TEMP_SENSOR_PIN, HIGH);
	busywait(25);
	digitalWrite(TEMP_SENSOR_PIN, LOW);
	busywait(25);
	digitalWrite(TEMP_SENSOR_PIN, HIGH);
	busywait(25);
	digitalWrite(TEMP_SENSOR_PIN, LOW);
	busywait(25);
	digitalWrite(TEMP_SENSOR_PIN, HIGH);
	busywait(25);
}

void reset_pulse(void) {
	pinMode(TEMP_SENSOR_PIN, OUTPUT);
	digitalWrite(TEMP_SENSOR_PIN, LOW);
	busywait(500);
}

bool read_presence(bool waitUntilLineFree = true) {
	bool b_responded = false;

	//Sensor waits 15us to 60 us then pulls low for 60us to 240us
	auto t1 = get_time_point(60 + 250);

	pinMode(TEMP_SENSOR_PIN, INPUT);
	//Wait for the line to go up after the reset pulse
	while (!digitalRead(TEMP_SENSOR_PIN)) {}

	while (!b_responded && chrono::system_clock::now() < t1) {
		if (!digitalRead(TEMP_SENSOR_PIN)) {
			b_responded = true;

			if (waitUntilLineFree) {
				while (!digitalRead(TEMP_SENSOR_PIN)) {}
				busywait(2);
			}
		}
	}

	return b_responded;
}

void write_bit(int bit) {
	pinMode(TEMP_SENSOR_PIN, OUTPUT);
	digitalWrite(TEMP_SENSOR_PIN, LOW);

	if (bit) {
		busywait(10);
		pinMode(TEMP_SENSOR_PIN, INPUT);
		busywait(50);
	} else {
		busywait(60);
		pinMode(TEMP_SENSOR_PIN, INPUT);
	}
}

void write_slot_recovery(void) {
	pinMode(TEMP_SENSOR_PIN, INPUT);
	busywait(2);
}

//Stuff is transmitted LSb first
void write_cmd(int8_t cmd) {
	for (int i = 0; i < 8; i++) {
		int b = !!(cmd & (1 << i));
		write_bit(b);
		write_slot_recovery();
	}
}

int read_bit(void) {
	//Indicate a read time slot by pulling the line for 1us
	pinMode(TEMP_SENSOR_PIN, OUTPUT);
	digitalWrite(TEMP_SENSOR_PIN, LOW);
	busywait(1);

	//Read the line within 15us
	pinMode(TEMP_SENSOR_PIN, INPUT);
	busywait(2);
	int bit = digitalRead(TEMP_SENSOR_PIN);
	busywait(60);


	//Wait for the line to go back to the pullup state
	while (!digitalRead(TEMP_SENSOR_PIN)) {}

	return bit;
}

int init_seq(void) {
	reset_pulse();
	return read_presence(true);
}

uint64_t read_rom_code(void) {
	uint64_t rom_code = 0;
	int bit;

	int b_present = init_seq();
	if (b_present) {
		write_cmd(READ_ROM);
		for (int i = 0; i < 64; i++) {
			bit = read_bit();
			rom_code |= (uint64_t(bit) << i);
		}
	}
	return rom_code;
}

void skip_rom(void) {
	write_cmd(SKIP_ROM);
}

void match_rom(uint64_t rom_code) {
	if (0) { //STUB
		write_cmd(MATCH_ROM);
	}
	return;
}

int convert_t(uint64_t *p_rom_code, bool b_wait=true) {
	int bit;

	do {
		if (!init_seq()) {
			return -1;
		}
		if (p_rom_code == nullptr) {
			skip_rom();
		} else {
			match_rom(*p_rom_code);
		}

		write_cmd(CONVERT_T);
		bit = read_bit();
	} while (!bit && b_wait);

	return bit;
}


//Returns the (nb_bits + 1) bits of data (and CRC) in the lsbs or <0 on errors
array<uint8_t, 9> read_scratchpad(uint64_t *p_rom_code, int nb_bits = 9*8) {
	array<uint8_t, 9> data = { 0 };

	if (!init_seq()) {
		return { {E_INVALID_SCRATCH & 0xFF, E_INVALID_SCRATCH >> 8} };
	}
	if (p_rom_code == nullptr) {
		skip_rom();
	} else {
		match_rom(*p_rom_code);
	}

	write_cmd(READ_SCRATCH);
	//Add one bit for CRC
	for (int i = 0; i < nb_bits; i++) {
		int bit = read_bit();
		data[i/8] |= bit << (i%8);
	}

	return data;
}

int crc_check(uint16_t data, int nb_data_bits) {
	uint16_t crc_bit_mask = 1 << nb_data_bits;
	uint16_t data_mask = crc_bit_mask - 1;
	uint16_t val = data & data_mask;
	int crc_bit = !!(data & crc_bit_mask);
}

int main (void) {
	uint64_t rom_code;
	array<uint8_t, 9> scratchpad;
	int temp_ready = -2;
	struct mosquitto *mosq_client;

	cout << "Temp started" << endl;
	init();
	mosq_client = mosquitto_new("dioo-test", nullptr);
	mosquitto_username_pw_set(mosq_client, "", "");
	mosquitto_connect(mosq_client, MQTT_HOST, MQTT_PORT, MQTT_KEEPALIVE, true);

	oscope_trigger();
	rom_code = read_rom_code();

	if (rom_code == 0) {
		cout << "Sensor didn't respond" << endl;
	} else {
		cout << "Sensor responded" << endl << "rom code is " << hex << rom_code << endl;

		uint16_t last_temp = E_INVALID_SCRATCH;
		uint16_t temp_val;
		while (true) {
			temp_ready = convert_t(nullptr);
			scratchpad = read_scratchpad(nullptr);
			temp_val = (scratchpad[1] << 8) | scratchpad[0];

			//cout << "Temperature is ready? " << dec << temp_ready << endl;
			if (temp_ready > 0 && temp_val != last_temp) {
				last_temp = temp_val;
				float temp = ((temp_val & 0xff0) >> 4) + float(temp_val & 0xf) / 16;
				cout << "Scratch = 0x";
				for (int i = 8; i >= 0; i--) {
					printf("%02x", scratchpad[i]);
				}
				cout << endl;
				cout << "Temperature is around " << dec << temp << endl;
			}
		}
	}

	mosquitto_disconnect(mosq_client);
	mosquitto_destroy(mosq_client);
	pinMode(TEMP_SENSOR_PIN, INPUT);
	cout << "Exiting....." << endl;
	return 0;
}
