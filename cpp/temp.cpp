#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <wiringPi.h>

using namespace std;

const int TEMP_SENSOR_PIN = 15;

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
int16_t read_scratchpad(uint64_t *p_rom_code, int nb_bits = 12) {
	int16_t val = 0;

	if (!init_seq()) {
		return -1;
	}
	if (p_rom_code == nullptr) {
		skip_rom();
	} else {
		match_rom(*p_rom_code);
	}

	write_cmd(READ_SCRATCH);
	//Add one bit for CRC
	for (int i = 0; i < nb_bits + 1; i++) {
		int bit = read_bit();
		val |= bit << i;
	}

	return val;
}

int main (void) {
	uint64_t rom_code;
	int16_t temp_val;
	int temp_ready = -2;

	cout << "Temp started" << endl;
	init();

	oscope_trigger();
	rom_code = read_rom_code();

	if (rom_code == 0) {
		cout << "Sensor didn't respond" << endl;
	} else {
		cout << "Sensor responded" << endl << "rom code is " << hex << rom_code << endl;
		int16_t last_temp = -1;
		while (true) {
			temp_ready = convert_t(nullptr);
			temp_val = read_scratchpad(nullptr);

			//cout << "Temperature is ready? " << dec << temp_ready << endl;
			if (temp_ready > 0 && temp_val != last_temp) {
				last_temp = temp_val;
				float temp = ((temp_val & 0xfff) >> 4) + float(temp_val & 0xf) / 16;
				cout << "Temp = " << hex << temp_val << endl;
				cout << "Temperature is around " << dec << temp << endl;
			}
		}
	}

	pinMode(TEMP_SENSOR_PIN, INPUT);
	cout << "Exiting....." << endl;
	return 0;
}
