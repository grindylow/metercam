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

#include <PubSubClient.h>

#include <ESPAsyncWebServer.h>

/* we use LittleFS for storing settings */
#include <LITTLEFS.h>

/* This project does not share any code with Rui Santos's esp32-cam project. Nevertheless,
 * Rui's contribution to the esp32-cam universe is hereby gratefully acknowledged.
 * Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-display-web-server/
 */

// our own submodules
#include "webserver.h"
#include "camoperator.h"

WiFiManager myWiFiManager;
AsyncWebServer server(80);
CamOperator myCamOperator;

const char *mqtt_server = "192.168.88.24";
WiFiClient espClient;
PubSubClient myPubSubClient(espClient);


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
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
  Serial.printf("onNewPicture(size=%d,buf=%d)", _size, _buf);
  if(_size>UINT16_MAX)
  {
    Serial.println("- too large for our MQTT implementation :-(. not transmitting.");
    return;
  }
  boolean r;
  r = myPubSubClient.publish("outTopic", _buf, _size);
  Serial.printf("publish result: %d", (int)r);
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
      // Once connected, publish an announcement...
      myPubSubClient.publish("outTopic", "hello world");
      // ... and resubscribe
      myPubSubClient.subscribe("inTopic");
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
  Serial.println("Transmitting 'A'...");
  Serial2.print('A');
  if(Serial2.available()) {
    Serial.print("Got something: ");
    Serial.print(char(Serial2.read()));
    Serial.print("\n");
  }
  delay(1000);
  //reconnectPubSubClient();  // will only reconnect if not alread connected
  //myPubSubClient.loop();
  //myCamOperator.loop();
}
