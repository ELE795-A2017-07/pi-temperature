#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <wiringPi.h>

using namespace std;

const int TEMP_SENSOR_PIN = 15;

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
	digitalWrite(TEMP_SENSOR_PIN, LOW);
	busywait(500);
}

bool read_presence(bool waitUntilLineFree) {
	bool b_responded = false;

	//Sensor waits 15us to 60 us then pulls low for 60us to 240us
	auto t1 = get_time_point(60 + 250);

	pinMode(TEMP_SENSOR_PIN, INPUT);
	//Wait for the line to go up after the reset pulse
	while (digitalRead(TEMP_SENSOR_PIN)) {}

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

int main (void) {
	bool b_present;
	int bits[64] = { 0 };

	cout << "Temp started" << endl;
	init();

	oscope_trigger();
	reset_pulse();
	b_present = read_presence(true);

	if (b_present) {
		uint64_t rom_code = 0;
		write_cmd(READ_ROM);
		for (int i = 0; i < 64; i++) {
			bits[i] = read_bit();
			rom_code |= (((uint64_t)bits[i]) << i);
		}
		cout << "Sensor responded" << endl << "rom code is " << hex << rom_code << endl;
		#if 0
		for (int i = 0; i < 64; i++) {
			cout << "bits[" << i << "] = " << bits[i] << endl;
		}
		#endif
	} else {
		cout << "Sensor didn't respond" << endl;
	}

	pinMode(TEMP_SENSOR_PIN, INPUT);
	cout << "Exiting....." << endl;
	return 0;
}
