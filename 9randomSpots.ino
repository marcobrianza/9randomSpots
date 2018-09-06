// board= Wemos D1 mini PRO

String softwareName = "9randomSpots";
String softwareVersion = "1.0.1"; //
String software = "";

#define MAX_PARAM 40
#include "credentials.h"

//boot Count
#include <EEPROM.h>

//Wi-Fi
#include <ESP8266WiFi.h>  // ESP8266 core 2.4.1 
WiFiClient  wifi;

//Wi-FiManager
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // Version 0.12.0
#include "FS.h"


#define MQTT_MAX MQTT_MAX_PACKET_SIZE

char THING_ID[MAX_PARAM];
String thingId = "";
String appId = "9RS_";

#define THING_NAME_DEFAULT ""
char THING_NAME[MAX_PARAM] = THING_NAME_DEFAULT;
String s_thingName = "";
String s_mqttServer = "";

// name, prompt, default, length
WiFiManagerParameter wfm_thingName("thingName", "Thing Name", THING_NAME, sizeof(THING_NAME));
WiFiManagerParameter wfm_mqttServer("mqttServer", "MQTT Server", MQTT_SERVER, sizeof(MQTT_SERVER));

//OTA
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
const char* OTA_PASSWORD = "12345678";

//http update
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>


String MD5_URL = "http://iot.marcobrianza.it/art/9randomSpots_md5.txt";
String FW_URL = "http://iot.marcobrianza.it/art/9randomSpots.ino.d1_mini.bin";

//MQTT
#include <PubSubClient.h> // version 2.6.0 //in PubSubClient.h change #define MQTT_MAX_PACKET_SIZE 512 
#include <ArduinoJson.h> // version 5.13.1
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
int MQTT_PORT = 1883;
String mqttRoot =   "9randomSpots";
//
String mqtt_randomColor = "randomColor";
String mqtt_beat = "beat";
String mqtt_config = "config";
String mqtt_brightness = "brightness";
String mqtt_cpm = "cpm";

String mqtt_color = "color";

String mqttPublish_randomColor = "";
String mqttPublish_beat = "";
String mqttPublish_cpm = "";

String mqttSubscribe_color = "";
String mqttSubscribe_config = "";
String mqttSubscribe_brightness = "";


//random color
const byte SPOTS = 9;
const byte LEDS_SPOT = 4;

const unsigned int MAX_H = 32; // number of variaitons per each color
const unsigned int MAX_S = 16;
const unsigned int MAX_V = 16;

const unsigned int MAX_CCC = MAX_H * MAX_S * MAX_V;
const unsigned  int MAX_NUM = MAX_CCC * SPOTS ;

const unsigned int NUM_LEDS = SPOTS * LEDS_SPOT;

//LED
#include <FastLED.h> // version 3.1.6
#define LED_DATA_PIN D3
int GLOBAL_BRIGHTNESS = 255;
struct COLOR {
  byte h;
  byte s;
  byte v;
};


COLOR spots[SPOTS];
CRGB leds[NUM_LEDS];


// test
#define BOOT_TEST_LIGHT 2
#define BOOT_RESET 3
#define BOOT_TEST_BOARD 4
#define TEST_TIME 30000


//beat
#define BEAT_INTERVAL 900000 //900000 60000


// geiger counter data -----------------------------
#include "RadiationWatch.h" //0.6.1

#define  GEIGER_SIG_PIN  D1
#define  GEIGER_NS_PIN  D2
RadiationWatch radiationWatch(GEIGER_SIG_PIN, GEIGER_NS_PIN);
unsigned long loops;
unsigned long last_random = 0;
int last_spot = 0;


void setup() {

  FastLED.setBrightness(GLOBAL_BRIGHTNESS);
  FastLED.addLeds<WS2812B, LED_DATA_PIN, GRB>(leds, NUM_LEDS);
  memset(leds, 0, NUM_LEDS * 3);
  FastLED.show();


  Serial.begin(115200);  Serial.println();
  software = softwareName + " - " + softwareVersion + " - " + ESP.getCoreVersion() + " - " + ESP.getSketchMD5();// + " - " + String (__DATE__) + " - " + String(__TIME__);;
  Serial.println(software);

  getTHING_ID();

  byte c = bootCount();
  Serial.print("\nboot count=");
  Serial.println(c);

  if (c == BOOT_TEST_LIGHT) {
    Serial.println("Test mode");
    testDevice();
  }

  if (c == BOOT_RESET) {
    WiFi.disconnect();
    writeAttribute("thingName", "");
    writeAttribute("mqttServer", "");
  }


  if (c == BOOT_TEST_BOARD) {
    connectWifi();
  } else {
    connectWifi_or_AP(c);
  }

  autoUpdate();

  s_thingName = readAttribute("thingName");
  if (s_thingName == "") {
    s_thingName = thingId;
  }

  Serial.println ("thingName=" + s_thingName);
  s_thingName.toCharArray(THING_NAME, MAX_PARAM);
  WiFi.hostname(s_thingName);

  s_mqttServer = readAttribute("mqttServer");
  if (s_mqttServer != "") {
    s_mqttServer.toCharArray(MQTT_SERVER, MAX_PARAM);
  }

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqtt_callback);
  mqttSubscribe_color = mqttRoot + "/" + thingId + "/" + mqtt_color;
  mqttSubscribe_config = mqttRoot + "/" + thingId + "/" + mqtt_config;
  mqttSubscribe_brightness =  mqttRoot + "/" + thingId + "/" + mqtt_brightness;

  mqttPublish_randomColor = mqttRoot + "/" + thingId + "/" + mqtt_randomColor;
  mqttPublish_beat = mqttRoot + "/" + thingId + "/" + mqtt_beat;
  mqttPublish_cpm = mqttRoot + "/" + thingId + "/" + mqtt_cpm;;

  setupOTA();

  //MDNS discovery
  MDNS.addServiceTxt("arduino", "tcp", "thingId", thingId);
  MDNS.addServiceTxt("arduino", "tcp", "thingName", s_thingName);
  MDNS.addServiceTxt("arduino", "tcp", "software", software);
  MDNS.update();

  WiFi.setAutoReconnect(true);

  radiationWatch.setup();
  radiationWatch.registerRadiationCallback(&onRadiationPulse);
}

void loop() {

  if (!mqttClient.connected())  reconnectMQTT();
  mqttClient.loop();

  publishBeat(false);

  ArduinoOTA.handle();
  radiationWatch.loop();
  loops++;
}



void testDevice() {

  const int T = 3000;
  const byte L = 255;

  Serial.println(softwareVersion);
  Serial.println(F("Test Start"));

  showAllLeds(L, 0, 0);
  delay(T);
  showAllLeds(0, L, 0);
  delay(T);
  showAllLeds(0, 0, L);
  delay(T);
  showAllLeds(0, 0, 0);

  Serial.println(F("Test End"));

}




