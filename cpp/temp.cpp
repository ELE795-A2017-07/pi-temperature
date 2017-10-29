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
const int TRIGGER_PIN = 7;

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

	pinMode(TRIGGER_PIN, OUTPUT);
	digitalWrite(TRIGGER_PIN, LOW);

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
	busywait(2);

	//Read the line within 15us
	pinMode(TEMP_SENSOR_PIN, INPUT);
	busywait(3);
	int bit = digitalRead(TEMP_SENSOR_PIN);
	busywait(60);

	//Wait for the line to go back to the pullup state
	//while (!digitalRead(TEMP_SENSOR_PIN)) {}

	write_slot_recovery();

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

bool scratch_crc_check(array<uint8_t, 9> scratchpad) {
	const int CRC_TABLE[] = {
			  0,  94, 188, 226,  97,  63, 221, 131, 194, 156, 126,  32, 163, 253,  31,  65,
			157, 195,  33, 127, 252, 162,  64,  30,  95,   1, 227, 189,  62,  96, 130, 220,
			 35, 125, 159, 193,  66,  28, 254, 160, 225, 191,  93,   3, 128, 222,  60,  98,
			190, 224,   2,  92, 223, 129,  99,  61, 124,  34, 192, 158,  29,  67, 161, 255,
			 70,  24, 250, 164,  39, 121, 155, 197, 132, 218,  56, 102, 229, 187,  89,   7,
			219, 133, 103,  57, 186, 228,   6,  88,  25,  71, 165, 251, 120,  38, 196, 154,
			101,  59, 217, 135,   4,  90, 184, 230, 167, 249,  27,  69, 198, 152, 122,  36,
			248, 166,  68,  26, 153, 199,  37, 123,  58, 100, 134, 216,  91,   5, 231, 185,
			140, 210,  48, 110, 237, 179,  81,  15,  78,  16, 242, 172,  47, 113, 147, 205,
			 17,  79, 173, 243, 112,  46, 204, 146, 211, 141, 111,  49, 178, 236,  14,  80,
			175, 241,  19,  77, 206, 144, 114,  44, 109,  51, 209, 143,  12,  82, 176, 238,
			 50, 108, 142, 208,  83,  13, 239, 177, 240, 174,  76,  18, 145, 207,  45, 115,
			202, 148, 118,  40, 171, 245,  23,  73,   8,  86, 180, 234, 105,  55, 213, 139,
			 87,   9, 235, 181,  54, 104, 138, 212, 149, 203,  41, 119, 244, 170,  72,  22,
			233, 183,  85,  11, 136, 214,  52, 106,  43, 117, 151, 201,  74,  20, 246, 168,
			116,  42, 200, 150,  21,  75, 169, 247, 182, 232,  10,  84, 215, 137, 107, 53 };

	int r = 0;
	for (int i = 0; i < 9; i++) {
		r = CRC_TABLE[r ^ scratchpad[i]];
	}

	return !r;
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
		int ret;
		uint16_t mid;
		//Hex code is two characters per byte + '0x' + '\0'
		const size_t PAYLOAD_LEN = (sizeof (rom_code)) * 2 + 3;
		uint8_t payload[PAYLOAD_LEN] = {0};
		snprintf(((char*)payload), PAYLOAD_LEN, "0x%llx", rom_code);
		ret = mosquitto_publish(mosq_client, &mid, MQTT_CLIENT_ID "/temperature", PAYLOAD_LEN, payload, 0, false);
		cout << "MQTT publish returned " << dec << ret << " and its ID is " << mid << " PAYLOAD_LEN is " << PAYLOAD_LEN << endl;

		uint16_t last_temp = E_INVALID_SCRATCH;
		uint16_t temp_val;
		while (true) {
			bool do_print;
			do_print = true;
			temp_ready = convert_t(nullptr);
			scratchpad = read_scratchpad(nullptr);
			temp_val = (scratchpad[1] << 8) | scratchpad[0];

			if (temp_ready < 1) {
				do_print = false;
			} else if (!scratch_crc_check(scratchpad)) {
				cout << "next is bogus:" << endl;
				digitalWrite(TRIGGER_PIN, HIGH);
			} else {
				digitalWrite(TRIGGER_PIN, LOW);
				if (temp_val == last_temp) {
					do_print = false;
				}
			}
			//cout << "Temperature is ready? " << dec << temp_ready << endl;
			if (do_print) {
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
