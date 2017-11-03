#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <array>
#include <cstdint>

namespace OneWire {

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

void init(void);

void oscope_trigger(void);

void reset_pulse(void);

bool read_presence(bool waitUntilLineFree = true);

void write_bit(int bit);

void write_slot_recovery(void);

void write_cmd(int8_t cmd);

int read_bit(void);

int init_seq(void);

uint64_t read_rom_code(void);

void skip_rom(void);

void match_rom(uint64_t rom_code);

int convert_t(uint64_t *p_rom_code, bool b_wait=true);

std::array<uint8_t, 9> read_scratchpad(uint64_t *p_rom_code, int nb_bits = 9*8);

bool scratch_crc_check(std::array<uint8_t, 9> scratchpad);

}

#endif /* Guard */
