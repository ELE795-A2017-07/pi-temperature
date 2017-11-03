#include <array>
#include <wiringPi.h>

#include "onewire.h"

namespace OneWire {

void init(void) {
	wiringPiSetup();
	pinMode(TEMP_SENSOR_PIN, OUTPUT);
	pullUpDnControl(TEMP_SENSOR_PIN, PUD_UP);

	pinMode(TRIGGER_PIN, OUTPUT);
	digitalWrite(TRIGGER_PIN, LOW);
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
std::array<uint8_t, 9> read_scratchpad(uint64_t *p_rom_code, int nb_bits = 9*8) {
	std::array<uint8_t, 9> data = { 0 };

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

bool scratch_crc_check(std::array<uint8_t, 9> scratchpad) {
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

}
