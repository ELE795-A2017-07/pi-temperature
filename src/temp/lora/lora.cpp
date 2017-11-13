#include "lora.h"

void LoRa::init(int loraMode, int channel, int node_addr, bool paboost) {
	int e;

	sx1272.ON();
	e = sx1272.setMode(loraMode);
	sx1272._enableCarrierSense = true;
	e = sx1272.setChannel(channel);
	sx1272._needPABOOST = paboost;
	e = sx1272.setPowerDBM((uint8_t)MAX_DBM);
	e = sx1272.setNodeAddress(node_addr);
	delay(500);
}


void LoRa::setup_exchange(void) {
	sx1272.CarrierSense();
	sx1272.setPacketType(PKT_TYPE_DATA);
}

int LoRa::exchange(int dest_addr) {
	int e;
	uint8_t r_size;
	uint8_t message[100];

	r_size = sprintf((char*)message, "Ping");
	e = sx1272.sendPacketTimeoutACK(dest_addr, message, r_size);
	return e;
}
