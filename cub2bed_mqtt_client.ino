/*
  Project Name:   cub2bed_client
  Developer:      Eric Klein Jr. (temp2@ericklein.com)
  Description:    client for cub2bed solution

  See README.md for target information, revision history, feature requests, etc.
*/

// Conditionals (hardware functionality)
#define DEBUG         // Output to the serial port
#define LED           // output to button LED
//#define NEOPIXEL    // output to neopixel(s)
//#define RJ45    // use Ethernet to send data to cloud services
#define WIFI      // use WiFi to send data to cloud services
//#define ADAFRUITIO

bool waitingForServerAction = false;

//button
#include <buttonhandler.h>
#define sendMessageButtonPin  5
#define longButtonPressDelay  3000
ButtonHandler buttonSendMessage(sendMessageButtonPin,longButtonPressDelay);
// globals related to buttons
enum { BTN_NOPRESS = 0, BTN_SHORTPRESS, BTN_LONGPRESS };

#ifdef LED
  #define ledPin  2
#endif

#ifdef NEOPIXEL
  #include <Adafruit_NeoPixel.h>
  #define neoPixelPin   10
  #define ledCount      1
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(ledCount, neoPixelPin);
#endif

// Adafruit IO and network device setup
#include "secrets.h"

#ifdef WIFI
  #if defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(ARDUINO_AVR_UNO_WIFI_REV2)
    #include <WiFiNINA.h>
  #elif defined(ARDUINO_SAMD_MKR1000)
    #include <WiFi101.h>
  #elif defined(ARDUINO_ESP8266_ESP12)
    #include <ESP8266WiFi.h>
  #endif

  WiFiClient client;

  //use WiFiClientSecure for SSL
  //WiFiClientSecure client;
#endif

#ifdef RJ45
  // Set MAC address. If unknown, be careful for duplicate addresses across projects.
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  #include <SPI.h>
  #include <Ethernet.h>
  //#include <EthernetClient.h>  // Adafruit says required, not so in my experience?
  EthernetClient client;
#endif

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
Adafruit_MQTT_Client mqtt(&client, MQTT_BROKER, MQTT_PORT, MQTT_USER, MQTT_PASS);
#ifdef ADAFRUITIO
  Adafruit_MQTT_Publish statusLightPub = Adafruit_MQTT_Publish(&mqtt, MQTT_USER "/feeds/status-light");
  Adafruit_MQTT_Publish cub2bedPub = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USER "/feeds/XXX");
  Adafruit_MQTT_Subscribe cub2bedSub = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USER "/feeds/XXX");
#else
  Adafruit_MQTT_Publish statusLightPub = Adafruit_MQTT_Publish(&mqtt, MQTT_PUB_TOPIC1);
  Adafruit_MQTT_Publish cub2bedPub = Adafruit_MQTT_Publish(&mqtt, MQTT_PUB_TOPIC2);
  Adafruit_MQTT_Subscribe cub2bedSub = Adafruit_MQTT_Subscribe(&mqtt, MQTT_SUB_TOPIC);
#endif

void setup() 
{
  #ifdef DEBUG
    Serial.begin(115200);
    while (!Serial) 
      {
        // wait for serial port
      }
      Serial.println("cub2bed client started");
      #ifdef LED
        Serial.print("LED");
      #endif
      #ifdef NEOPIXEL
        Serial.print("NEOPIXEL");
      #endif
      Serial.println(" code path(s) enabled");
  #endif

  #ifdef WIFI
    // Connect to WiFi access point.
    #ifdef DEBUG
      Serial.print("connecting to ");
      Serial.println(WIFI_SSID);
    #endif

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) 
    {
      #ifdef DEBUG
        Serial.print(".");
      #endif
      delay(500);
    }

    #ifdef DEBUG
      Serial.println();  // finishes the status dots print
      Serial.print("WiFi connected, IP address: ");
      Serial.println(WiFi.localIP());
    #endif
  #endif

  #ifdef RJ45
    // Configure Ethernet CS pin, not needed if using default D10
    //Ethernet.init(10);  // Most Arduino shields
    //Ethernet.init(5);   // MKR ETH shield
    //Ethernet.init(0);   // Teensy 2.0
    //Ethernet.init(20);  // Teensy++ 2.0
    //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
    //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

    // Initialize Ethernet and UDP
    if (Ethernet.begin(mac) == 0)
    {
      #ifdef DEBUG
        Serial.println("Failed to configure Ethernet using DHCP");
      #endif
      // Check for Ethernet hardware present
      if (Ethernet.hardwareStatus() == EthernetNoHardware)
      {
        #ifdef DEBUG
          Serial.println("Ethernet hardware not found");
        #endif
      } 
      else if (Ethernet.linkStatus() == LinkOFF) 
      {
        #ifdef DEBUG
          Serial.println("Ethernet cable is not connected.");
        #endif
      }
      while (1);
    }
    #ifdef DEBUG
      Serial.print("IP number assigned by DHCP is ");
      Serial.println(Ethernet.localIP());
    #endif
  #endif

  mqtt.subscribe(&cub2bedSub);

  // Setup push button
  buttonSendMessage.init();

  #ifdef LED
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    // visually validate correct wiring
    #ifdef DEBUG
      for (int i=0;i<9;i++)
      {
        digitalWrite(ledPin,HIGH);
        delay(250);
        digitalWrite(ledPin,LOW);
      }
    #endif
  #endif

  #ifdef NEOPIXEL
    strip.begin();
    strip.setPixelColor(0,0,255,0);
    strip.setBrightness(50);
    strip.show();
  #endif
}

void loop() 
{
  MQTT_connect();
  // if we're waiting for the second message
  if (waitingForServerAction)
  {
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(5000))) 
    {
      if (subscription == &cub2bedSub)
      {
        if (strcmp((char *)cub2bedSub.lastread, "ontheway") == 0)
        {
          // visually indicate that request was successful
          #ifdef LED
            for(int i=0;i<3;i++)
            {
              digitalWrite(ledPin, HIGH);
              delay(500);
              digitalWrite(ledPin, LOW);
              delay(500);   
            }
          #endif
          #ifdef NEOPIXEL
            for (int i=0;i<10;i++)
            {
              strip.setPixelColor(0,0,255,0); // green
              strip.show();
              delay(100);
              strip.setPixelColor(0,0,0,0);
              strip.show();
              delay(100);
            }
            // return to normal state indicator
            strip.setPixelColor(0,0,255,0);
            strip.show();
          #endif
          #ifdef DEBUG
            Serial.println("On the way message processed");
          #endif
        }
        if (strcmp((char *)cub2bedSub.lastread, "needtowork") == 0) 
        {
          #ifdef LED
            for(int i=0;i<10;i++)
            {
              digitalWrite(ledPin, HIGH);
              delay(150);
              digitalWrite(ledPin, LOW);
              delay(150);   
            }
          #endif
          #ifdef NEOPIXEL
            for (int i=0;i<10;i++)
            {
              strip.setPixelColor(0,255,0,0); // red
              strip.show();
              delay(100);
              strip.setPixelColor(0,0,0,0);
              strip.show();
              delay(100);
            }
            // return to normal status indicator
            strip.setPixelColor(0,0,255,0);
            strip.show();
          #endif
          #ifdef DEBUG
            Serial.println("Need to work message processed");
          #endif
        }
      waitingForServerAction = false;  
      }
    }
  }
  resolveButtons();
}

void resolveButtons()
{
  switch (buttonSendMessage.handle()) 
  {
    case BTN_SHORTPRESS:
      // only handle button press if we aren't already processing a request
      if (!waitingForServerAction)
      {
        // Send message to status-light
        #ifdef DEBUG
            Serial.print("On sent via MQTT publish to: ");
            Serial.print(MQTT_PUB_TOPIC1);
        #endif
        if (!statusLightPub.publish("On"))
        {
          #ifdef DEBUG
            Serial.println(" failed");
          #endif
        }
        else 
        {
          #ifdef DEBUG
            Serial.println(" successful");
          #endif
        }
        // Send message to cub2bed
        #ifdef DEBUG
          Serial.print("sub2bed sent via MQTT publish to: ");
          Serial.print(MQTT_PUB_TOPIC2);
        #endif
        if (!cub2bedPub.publish("cub2bed"))
        {
          #ifdef DEBUG
            Serial.println(" failed");
          #endif
        }
        else 
        {
          #ifdef DEBUG
            Serial.println(" successful");
          #endif
        }
        #ifdef LED
          digitalWrite(ledPin,HIGH);
        #endif
        #ifdef NEOPIXEL
          strip.setPixelColor(0, 255, 255, 0); // yellow
          strip.show();
        #endif
        waitingForServerAction = true;
      }
      else
      {
        Serial.println("button press ignored");
      }
    break;
    case BTN_LONGPRESS:
      #ifdef DEBUG
        Serial.println("button long press -> tbd");
      #endif
    break;
  }
  #ifdef DEBUG
    //Serial.println("No button press detected");
  #endif
}

void MQTT_connect()
// Connects and reconnects to MQTT broker, call from loop() to maintain connection
{
  // Stop if already connected
  if (mqtt.connected()) 
  {
    return;
  }
  #ifdef DEBUG
    Serial.println("connecting to MQTT broker");
  #endif

  uint8_t ret;
  uint8_t retries = 3;
  while (mqtt.connect() != 0)
  {
    #ifdef DEBUG
      Serial.println(mqtt.connectErrorString(ret));
      Serial.println("retrying MQTT connection in 5 seconds");
    #endif
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) 
    {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  #ifdef DEBUG
    Serial.println("connected to MQTT broker");
  #endif
}