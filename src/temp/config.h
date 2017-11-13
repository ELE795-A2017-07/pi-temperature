#ifndef GLOBALS_H
#define GLOBALS_H


const int TEMP_SENSOR_PIN = 15;
const int TRIGGER_PIN = 7;


#include <chrono>
#define SAMPLE_INTERVAL std::chrono::minutes(10)


#if DEBUG
const char MQTT_HOST[] = "hexadecimal";
const int MQTT_PORT = 1884;
#else
const char MQTT_HOST[] = "207.162.8.230";
const int MQTT_PORT = 8080;
#endif

const int MQTT_KEEPALIVE = 0;


#include "lora_defines.h"
/* Set this to the right regulation from lora_defines.h */
#define LORA_REGULATION FCC_US_REGULATION

// set to true if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
#define PABOOST false

/* Set this to the right band from lora_defines.h */
#define LORA_BAND BAND868

#define LORA_MODE  1
#define node_addr 8
#define DEFAULT_DEST_ADDR 1


#endif /* Guard */
