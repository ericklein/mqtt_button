/*
  Project Name:   mqtt_button
  Description:    public (non-secret) configuration data
*/

// Configuration Step 1: Create and/or configure secrets.h. Use secrets_template.h as guide to create secrets.h

// Configuration Step 2: Set debug parameters
// comment out to turn off; 1 = summary, 2 = verbose
// #define DEBUG 2

// Configuration variables that change rarely

// Network
// max connection attempts to network services
const uint8_t networkConnectAttemptLimit = 3;
// seconds between network service connect attempts
const uint8_t networkConnectAttemptInterval = 10;

const uint16_t networkMQTTKeepAliveInterval = 300; // seconds

// Hardware

// Button
const uint8_t buttonPin = 5; // initially LOW
const uint8_t buttonLEDPin =  16;
const int buttonDebounceDelay = 50; // time in milliseconds to debounce button