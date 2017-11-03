#ifndef GLOBALS_H
#define GLOBALS_H

//Invalid value, bits 15 to 11 should all be sign bits
const uint16_t E_INVALID_SCRATCH = 0x8000;

const int TEMP_SENSOR_PIN = 15;
const int TRIGGER_PIN = 7;

#define MQTT_CLIENT_ID "dioo-test"
#if 0
const char MQTT_HOST[] = "207.162.8.230";
const int MQTT_PORT = 8080;
#else
const char MQTT_HOST[] = "hexadecimal";
const int MQTT_PORT = 1884;
#endif

const int MQTT_KEEPALIVE = 0;

#endif /* Guard */