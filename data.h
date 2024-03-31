/*
  Project:      mqtt_button
  Description:  data pair configuration
*/

#ifndef DATA_H
#define DATA_H

// Define tag keys used to store important device attributes in
// InfluxDB.  Will be used as InfluxDB tag keys for the associated
// attribute.  Only relevant to InfluxDB (not MQTT).  Maps directly
// to the important data tag values set in config.h, which are
// expected to be customized there on a per installation basis.  These,
// however, should not be changed.
#define TAG_KEY_DEVICE        "device"     // Maps to DEVICE in secrets.h
// #define TAG_KEY_DEVICE_ID  "device_id"  // Maps to DEVICE_ID in secrets.h
#define TAG_KEY_SITE          "site"       // Maps to SITE in secrets.h
#define TAG_KEY_LOCATION      "location"   // Maps to LOCATION in secrets.h
#define TAG_KEY_ROOM          "room"       // Maps to ROOM in secrets.h

// Defines value keys (strings) used for representing stored 
// sensor and device data as key/value pairs in InfluxDB and
// as parts of MQTT topics.  Must be unique, and must conform
// to syntax required for field keys by InfluxDB and MQTT.

#define VALUE_KEY_LIGHT           "light"
#define VALUE_KEY_RSSI            "rssi"

#endif  // #ifdef DATA_H