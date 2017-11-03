#ifndef GLOBALS_H
#define GLOBALS_H

const int TEMP_SENSOR_PIN = 15;
const int TRIGGER_PIN = 7;

#include <chrono>
#define SAMPLE_INTERVAL std::chrono::seconds(10)

#if DEBUG
const char MQTT_HOST[] = "hexadecimal";
const int MQTT_PORT = 1884;
#else
const char MQTT_HOST[] = "207.162.8.230";
const int MQTT_PORT = 8080;
#endif

const int MQTT_KEEPALIVE = 0;

#endif /* Guard */
