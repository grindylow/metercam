/* MeterCam by Martin Grill
 *  ......
 * License: ...
 */

/* https://github.com/khoih-prog/ESP_DoubleResetDetector/  -  EEPROM doesn't work, LITTLEFS crashes! */
//#include <ESP_DoubleResetDetector.h>  

/* https://github.com/tzapu/WiFiManager */
#include <WiFiManager.h>  

///* https://github.com/plapointe6/EspMQTTClient */
//#include "EspMQTTClient.h"


#include <ArduinoJson.h>

#include <PubSubClient.h>

#include <ESPAsyncWebServer.h>

/* we use LittleFS for storing settings */
#include <LittleFS.h>

/* This project does not share any code with Rui Santos's esp32-cam project. Nevertheless,
 * Rui's contribution to the esp32-cam universe is hereby gratefully acknowledged.
 * Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-display-web-server/
 */

// our own submodules
#include "mywebserver.h"
#include "camoperator.h"

WiFiManager myWiFiManager;
AsyncWebServer server(80);
CamOperator myCamOperator;

const char *mqtt_server = "192.168.88.203";
WiFiClient espClient;
PubSubClient myPubSubClient(espClient);

/** After taking a picture, we stay awake a little in case someone wants
  * to talk to us over MQTT. */
unsigned long whenShouldIGoToSleep = ULONG_MAX;

/** We will tell the uC to sleep this many intervals - can be reconfigured
  * by sending us an MQTT message. */
uint16_t desired_sleep_intervals = 0;  // Special value 0: "do not set new value before going to sleep"

/** a new MQTT message arrived */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Deserialize the JSON document
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  desired_sleep_intervals = doc["sleep_intervals"];
  Serial.printf("desired_sleep_intervals = %u\n", desired_sleep_intervals);
}

void onConnectionEstablished() {
/*
  myMQTTClient.subscribe("mytopic/test", [] (const String &payload)  {
    Serial.println(payload);
  });

  myMQTTClient.publish("mytopic/test", "This is a message");
  */
}

void onNewPicture(int _size, const uint8_t* _buf)
{
  Serial.printf("onNewPicture(size=%d,buf=%d)\n", _size, _buf);
  if(_size>UINT16_MAX)
  {
    Serial.println("- too large for our MQTT implementation :-(. not transmitting.");
    return;
  }
  boolean r;
  r = myPubSubClient.publish("metercam/DEADBEEF/jpg", _buf, _size);
  Serial.printf("publish result: %d\n", (int)r);
  whenShouldIGoToSleep = millis() + 2000;
  //myMQTTClient.publish("mytopic/img", std::string(_buf, _buf+_size));
}

void setup() {
  boolean r;
  Serial.begin(115200);  // "Serial" is for debugging output
  Serial.println("entered setup()");

  Serial.println("** Normal boot **");
  myWiFiManager.autoConnect("METERCAM", "metercam");

  // start web server
  init_webserver(server, &myCamOperator);
  server.begin();

  // Configure MQTT connection
  myPubSubClient.setServer(mqtt_server, 1883);
  myPubSubClient.setCallback(callback);
  r = myPubSubClient.setBufferSize(60000);
  Serial.printf("myPubSubClient.setBufferSize() = %d\n", (int)r);
    
  // initialise camera
  myCamOperator.registerNewPictureCallback(onNewPicture);
  myCamOperator.init();

  // whenever we boot, we take a picture right away
  myCamOperator.requestPicture();

  // comms to/from µC
#define RXD2 15 // 16
#define TXD2 14 // 17
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  whenShouldIGoToSleep = 0;
  Serial.println("setup() finished");
}

void reconnectPubSubClient() {
  // Loop until we're reconnected
  while (!myPubSubClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (myPubSubClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, subscribe to inbox...
      myPubSubClient.subscribe("metercam/DEADBEEF/inbox");
      // ...and announce ourselves.
      myPubSubClient.publish("metercam/DEADBEEF/info", "about to take a picture");
    } else {
      Serial.print("failed, rc=");
      Serial.print(myPubSubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
  unsigned long myTime;
  if(Serial2.available()) {
    Serial.print("Got something: ");
    char c=Serial2.read();
    Serial.printf("0x%02x\n", c);
  }
  //delay(1000);
  reconnectPubSubClient();  // will only reconnect if not alread connected
  myPubSubClient.loop();
  myCamOperator.loop();
  if(millis()>whenShouldIGoToSleep)
  {
    if(desired_sleep_intervals)
    {
      Serial.printf("About to set new number of desired sleep intervals to %u...\n", desired_sleep_intervals);
      Serial2.write('A');
      Serial2.write((uint8_t)((desired_sleep_intervals>>8)&0xff));
      Serial2.write((uint8_t)(desired_sleep_intervals&0xff));
      desired_sleep_intervals = 0;
      whenShouldIGoToSleep += 500;  // give us a chance to also see the reply
    }
    else
    {
      Serial.print("About to go to sleep.\n");
      Serial2.print("S"); // "go to sleep now!"
      delay(1000);
      Serial.print("If you can see this, the 'S' command wasn't processed for some reason.\n");
    }
  }
}
