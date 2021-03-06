#ifndef CONFIG_H
#define CONFIG_H

#include "lora_defines.h"

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Set this to the right regulation from lora.h */
#define LORA_REGULATION FCC_US_REGULATION
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// set to true if your radio is an HopeRF RFM92W, HopeRF RFM95W, Modtronix inAir9B, NiceRF1276
// or you known from the circuit diagram that output use the PABOOST line instead of the RFO line
#define PABOOST false
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// IMPORTANT
///////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Set this to the right band from lora.h */
#define LORA_BAND BAND868
///////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif /* Guard */
