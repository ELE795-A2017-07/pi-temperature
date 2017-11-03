#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <array>

#include "globals.h"
#include "mqtt.h"
#include "onewire.h"

using namespace std;

void init(void) {
	OneWire::init(TEMP_SENSOR_PIN, TRIGGER_PIN);
	Mqtt::init();
}

int32_t send_temperature(Mqtt& mqtt, float temp) {
	int ret;
	int32_t mid;
	//Hex code is two characters per byte + '0x' + '\0'
	//const size_t PAYLOAD_LEN = (sizeof (rom_code)) * 2 + 3;

	//sign (1), integral part (3), decimal separator (1), fractional part (3), end-of-string (1) = 9
	const size_t PAYLOAD_LEN = 9;
	uint8_t payload[PAYLOAD_LEN] = {0};
	snprintf(((char*)payload), PAYLOAD_LEN, "%f", temp);
	ret = mqtt.publish(&mid, MQTT_CLIENT_ID "/temperature", PAYLOAD_LEN, payload, 0, false);
	cout << "MQTT publish returned " << dec << ret << " and its ID is " << mid << " PAYLOAD_LEN is " << PAYLOAD_LEN << endl;
	if (ret != 0) {
		return -1;
	}

	return mid;
}


int main (void) {
	uint64_t rom_code;
	array<uint8_t, 9> scratchpad;
	int temp_ready = -2;
	Mqtt mqtt = Mqtt(MQTT_CLIENT_ID, nullptr, true);

	cout << "Temp started" << endl;
	init();

	//int major, minor, revision;
	//mosquitto_lib_version(&major, &minor, &revision);
	//cout << "Using mosquitto v" << major << "." << minor << "." << revision << endl;
	mqtt.set_credentials("", "");
	mqtt.connect(MQTT_HOST, MQTT_PORT, MQTT_KEEPALIVE);

	OneWire::oscope_trigger();
	rom_code = OneWire::read_rom_code();

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

		uint16_t last_temp = OneWire::E_INVALID_SCRATCH;
		uint16_t temp_val;
		for (int i = 0; i < 10; ) {
			bool do_print = true;
			bool is_valid = false;
			temp_ready = OneWire::convert_t(nullptr);
			scratchpad = OneWire::read_scratchpad(nullptr);
			temp_val = (scratchpad[1] << 8) | scratchpad[0];

			if (temp_ready < 1) {
				do_print = false;
			} else if (!OneWire::scratch_crc_check(scratchpad)) {
				cout << "next is bogus:" << endl;
				//digitalWrite(TRIGGER_PIN, HIGH);
			} else {
				//digitalWrite(TRIGGER_PIN, LOW);
				if (temp_val == last_temp) {
					//do_print = false;
				} else {
					is_valid = true;
				}
			}
			//cout << "Temperature is ready? " << dec << temp_ready << endl;
			if (do_print) {
				i++;
				last_temp = temp_val;
				float temp = ((temp_val & 0xff0) >> 4) + float(temp_val & 0xf) / 16;
				cout << "Scratch = 0x";
				for (int i = 8; i >= 0; i--) {
					printf("%02x", scratchpad[i]);
				}
				cout << endl;
				cout << "Temperature is around " << dec << temp << endl;
				if (is_valid) {
					send_temperature(mqtt, temp);
					std::this_thread::sleep_for(std::chrono::seconds(5));
				}
			}
		}
	}

	//pinMode(TEMP_SENSOR_PIN, INPUT);
	cout << "Exiting....." << endl;
	return 0;
}
