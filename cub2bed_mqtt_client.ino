/*
  Project Name:   cub2bed_client
  Developer:      Eric Klein Jr. (temp2@ericklein.com)
  Description:    client for cub2bed solution

  See README.md for target information, revision history, feature requests, etc.
*/

// Conditionals (hardware functionality)
//#define DEBUG       // Output to the serial port
#define BUTTON_LED    // output to button LED
//#define NEOPIXEL    // output to neopixel(s)
//#define RJ45        // use Ethernet to send data to cloud services
#define WIFI          // use WiFi to send data to cloud services

// Gloval variables
unsigned long previousMQTTPingTime = 0;
bool waitingForServerAction = false;

// for future use with DEBUG
#define onboardLED 2

//button
#include <buttonhandler.h>
#define sendMessageButtonPin  5
#define longButtonPressDelay  3000
ButtonHandler buttonSendMessage(sendMessageButtonPin,longButtonPressDelay);
// globals related to buttons
enum { BTN_NOPRESS = 0, BTN_SHORTPRESS, BTN_LONGPRESS };

#ifdef BUTTON_LED
  #define buttonLEDPin  16
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
  //WiFiClientSecure client; // for SSL
#endif

#ifdef RJ45
  // Set MAC address. If unknown, be careful for duplicate addresses across projects.
  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
  #include <SPI.h>
  #include <Ethernet.h>
  EthernetClient client;
#endif

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
Adafruit_MQTT_Client mqtt(&client, MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);
Adafruit_MQTT_Publish statusLightPub = Adafruit_MQTT_Publish(&mqtt, MQTT_PUB_TOPIC1);
Adafruit_MQTT_Publish cub2bedPub = Adafruit_MQTT_Publish(&mqtt, MQTT_PUB_TOPIC2);
Adafruit_MQTT_Subscribe cub2bedSub = Adafruit_MQTT_Subscribe(&mqtt, MQTT_SUB_TOPIC);

void setup() 
{
  // used for fatal error messaging
  pinMode(LED_BUILTIN, OUTPUT);

  #ifdef DEBUG
    Serial.begin(115200);
    while (!Serial) 
      {
        // wait for serial port
      }
      Serial.println("cub2bed client started");
      #ifdef BUTTON_LED
        Serial.print("LED");
      #endif
      #ifdef NEOPIXEL
        Serial.print("NEOPIXEL");
      #endif
      Serial.println(" code path(s) enabled");
      // Use the built-in blue LED for debug info
      pinMode(onboardLED, OUTPUT);
      digitalWrite(onboardLED, LOW);
  #endif

  #ifdef WIFI
    uint8_t tries = 1;
    #define MAXTRIES 11

    // Connect to WiFi access point
    #ifdef DEBUG
      Serial.print("Connecting to WiFI AP ");
      Serial.println(WIFI_SSID);
    #endif

    // hostname has to come before WiFi.begin
    WiFi.hostname(MQTT_CLIENT_ID);
    // WiFi.setHostname(MQTT_CLIENT_ID); //for WiFiNINA
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) 
    {
      // Error handler - WiFi does not initially connect
      #ifdef DEBUG
        Serial.print("WiFi AP connect attempt ");
        Serial.print(tries);
        Serial.print("of");
        Serial.print(MAXTRIES);
        Serial.print(" in ");
        Serial.print(tries*10);
        Serial.println(" seconds");
      #endif
      delay(tries*10000); // using delay because this is not loop()

      tries++;

      // FATAL ERROR 03 - WiFi doesn't connect
      if (tries == MAXTRIES)
      {
        #ifdef DEBUG
          Serial.print("FATAL ERROR 03; can not connect to WiFi after ");
          Serial.print(MAXTRIES);
          Serial.println(" attempts");
        #endif
        while (1)
        {
          // endless loop communicating fatal error 03
          #ifdef BUTTON_LED
            digitalWrite(buttonLEDPin, HIGH);
          #endif
          digitalWrite(LED_BUILTIN, HIGH);
          delay(3000);
          #ifdef BUTTON_LED
            digitalWrite(buttonLEDPin, LOW);
          #endif
          digitalWrite(LED_BUILTIN, LOW);
          delay(3000);
        }
      }
    }
    #ifdef DEBUG
      Serial.println();  // finishes the status dots print
      Serial.print("WiFi IP address is: ");
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
      // FATAL ERROR 02 - Ethernet doesn't connect
      
      // generic error
      #ifdef DEBUG
        Serial.println("FATAL ERROR 02; Failed to configure Ethernet using DHCP");
      #endif

      // identified errors
      if (Ethernet.hardwareStatus() == EthernetNoHardware)
      {
        #ifdef DEBUG
          Serial.println("FATAL ERROR 02; Ethernet hardware not found");
        #endif
      } 
      else if (Ethernet.linkStatus() == LinkOFF) 
      {
        #ifdef DEBUG
          Serial.println("FATAL ERROR 02; Ethernet cable is not connected");
        #endif
      }
      while (1)
      {
        // endless loop communicating fatal error 02
        #ifdef BUTTON_LED
          digitalWrite(buttonLEDPin, HIGH);
        #endif
        digitalWrite(LED_BUILTIN, HIGH);
        delay(2000);
        #ifdef BUTTON_LED
          digitalWrite(buttonLEDPin, LOW);
        #endif
        digitalWrite(LED_BUILTIN, LOW);
        delay(2000);
      }
    }
    #ifdef DEBUG
      Serial.print("Ethernet IP address is: ");
      Serial.println(Ethernet.localIP());
    #endif
  #endif

  mqtt.subscribe(&cub2bedSub);

  // Setup push button
  buttonSendMessage.init();

  #ifdef BUTTON_LED
    pinMode(buttonLEDPin, OUTPUT);
    digitalWrite(buttonLEDPin, LOW);
    
    // visually validate correct wiring
    #ifdef DEBUG
      for (int i=0;i<9;i++)
      {
        digitalWrite(buttonLEDPin,HIGH);
        delay(250);
        digitalWrite(buttonLEDPin,LOW);
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
    while ((subscription = mqtt.readSubscription(1000))) 
    {
      if (subscription == &cub2bedSub)
      {
        #ifdef DEBUG
          Serial.print("Received: ");
          Serial.print((char *)cub2bedSub.lastread);
          Serial.print(" from: ");
          Serial.println(MQTT_SUB_TOPIC);
        #endif
        if (strcmp((char *)cub2bedSub.lastread, "ontheway") == 0)
        {
          // visually indicate that request was successful
          #ifdef BUTTON_LED
            for(int i=0;i<3;i++)
            {
              digitalWrite(buttonLEDPin, HIGH);
              delay(500);
              digitalWrite(buttonLEDPin, LOW);
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
            Serial.println("on the way message processed");
          #endif
          waitingForServerAction = false;  
        }
        if (strcmp((char *)cub2bedSub.lastread, "needtowork") == 0) 
        {
          #ifdef BUTTON_LED
            for(int i=0;i<10;i++)
            {
              digitalWrite(buttonLEDPin, HIGH);
              delay(150);
              digitalWrite(buttonLEDPin, LOW);
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
            Serial.println("need to work message processed");
          #endif
          waitingForServerAction = false;
        }
      }
    }
  }
  resolveButtons();
  // ping the broker to keep the connection alive
  if((millis() - previousMQTTPingTime) > MQTT_KEEP_ALIVE * 1000)
  {
    previousMQTTPingTime = millis();   
    if(! mqtt.ping())
    {
      mqtt.disconnect();
      #ifdef DEBUG
        Serial.println("disconnected from broker because ping says no connection");
      #endif
    }
    #ifdef DEBUG
      Serial.println("pinged the broker to maintain connection");
    #endif
  }
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
          Serial.print("cub2bed sent via MQTT publish to: ");
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
        #ifdef BUTTON_LED
          digitalWrite(buttonLEDPin,HIGH);
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
  uint8_t mqttErr;
  uint8_t tries = 1;
  #define MAXTRIES 3

  // exit if already connected
  if (mqtt.connected()) 
  {
    return;
  }
  #ifdef DEBUG
    Serial.print("connecting to broker: ");
    Serial.println(MQTT_BROKER);
  #endif

  while (mqtt.connect() != 0)
  {
    // Error handler - can not connect to MQTT broker
    #ifdef DEBUG
      Serial.println(mqtt.connectErrorString(mqttErr));
      Serial.print("MQTT broker connect attempt ");
      Serial.print(tries);
      Serial.print("of");
      Serial.print(MAXTRIES);
      Serial.print(" in ");
      Serial.print(tries*10);
      Serial.println(" seconds");
    #endif
    mqtt.disconnect();
    delay(tries*10000);
    tries++;

    // FATAL ERROR 01 - Can not connect to MQTT broker
    if (tries == MAXTRIES)
    {
      #ifdef DEBUG
        Serial.print("FATAL ERROR 01; can not connect to MQTT broker after");
        Serial.print(MAXTRIES);
        Serial.println(" attempts");
      #endif
      while (1)
      {
        // endless loop communicating fatal error 01
        #ifdef BUTTON_LED
          digitalWrite(buttonLEDPin, HIGH);
        #endif
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        #ifdef BUTTON_LED
          digitalWrite(buttonLEDPin, LOW);
        #endif
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
      }
    }
  }
  #ifdef DEBUG
    Serial.println("reconnected to MQTT broker");
  #endif
}