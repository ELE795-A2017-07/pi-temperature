#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <array>

#include "globals.h"
#include "mqtt.h"

using namespace std;

void init(void) {
	//OneWire::init();
	Mqtt::init();
}

int main (void) {
	uint64_t rom_code;
	array<uint8_t, 9> scratchpad;
	int temp_ready = -2;
	Mqtt mqtt;

	cout << "Temp started" << endl;
	init();

	//int major, minor, revision;
	//mosquitto_lib_version(&major, &minor, &revision);
	//cout << "Using mosquitto v" << major << "." << minor << "." << revision << endl;
	mqtt = Mqtt(MQTT_CLIENT_ID, nullptr, true);
	mqtt.set_credentials("", "");
	mqtt.connect(MQTT_HOST, MQTT_PORT, MQTT_KEEPALIVE);

	oscope_trigger();
	rom_code = read_rom_code();

	if (rom_code == 0) {
		cout << "Sensor didn't respond" << endl;
	} else {
		cout << "Sensor responded" << endl << "rom code is " << hex << rom_code << endl;
		int ret;
		int32_t mid;
		//Hex code is two characters per byte + '0x' + '\0'
		const size_t PAYLOAD_LEN = (sizeof (rom_code)) * 2 + 3;
		uint8_t payload[PAYLOAD_LEN] = {0};
		snprintf(((char*)payload), PAYLOAD_LEN, "0x%llx", rom_code);
		ret = mqtt.publish(&mid, MQTT_CLIENT_ID "/temperature", PAYLOAD_LEN, payload, 0, false);
		cout << "MQTT publish returned " << dec << ret << " and its ID is " << mid << " PAYLOAD_LEN is " << PAYLOAD_LEN << endl;

		uint16_t last_temp = E_INVALID_SCRATCH;
		uint16_t temp_val;
		while (false) {
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
