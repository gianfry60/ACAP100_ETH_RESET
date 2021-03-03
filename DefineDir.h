

#ifndef DefineDir_h
#define DefineDir_h

#define debug
//#define NDEBUG // Comment this line to enable serial debugging

#define Ver "1.7.7" 
#define Beta
// Only one if the following to be true
#define Mensa           false   // Set reader Type refectory
#define Presenze        false   // Set reader Type Attendance detection
#define CA              true    // Set reader Type Access Control
#define PMensa          false   // Set reader Type refectory take away

// Only one if the following to be true
#define USE_W5500       true    // Use W5500 Shield
#define USE_ENC28J60    false   // Use ENC28J60 Shield


#define NoWiFi // Comment this line to enable WiFi Not now impruvment jet

#define rele2 7       // define rele2 pin
#define rele1 5       // define rele1 pin
#define ResetETH 4    // define reset W5500 pin
#define beeper 8      // define beeper pin

#define TFT_GREY 0x5AEB
#define STRING_LEN 128
#define STRING_LEN_LONG 256

#endif // DefineDir_h
