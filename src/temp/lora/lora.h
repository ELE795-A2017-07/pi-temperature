#ifndef LORA_H
#define LORA_H

#include "config.h"
#include "SX1272.h"

#if LORA_REGULATION == ETSI_EUROPE_REGULATION
#define MAX_DBM 14
#elif LORA_REGULATION == SENEGAL_REGULATION
#define MAX_DBM 10
#elif LORA_REGULATION == FCC_US_REGULATION
#define MAX_DBM 14
#endif

#if LORA_BAND == BAND868
#if LORA_REGULATION == SENEGAL_REGULATION
const uint32_t DEFAULT_CHANNEL=CH_04_868;
#else
const uint32_t DEFAULT_CHANNEL=CH_10_868;
#endif
#elif LORA_BAND == BAND900
const uint32_t DEFAULT_CHANNEL=CH_05_900;
#elif LORA_BAND == BAND433
const uint32_t DEFAULT_CHANNEL=CH_00_433;
#endif

namespace LoRa {
	void init(int loraMode, int channel, int node_addr, bool paboost = true);
	void setup_exchange(void);
	int exchange(int dest_addr);
};

#endif /* Guard */
