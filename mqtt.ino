

//--------------MQTT--------------
void reconnectMQTT() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("\nAttempting MQTT connection...");
    // Attempt to connect
    wifiClient = WiFiClient(); // workaround to fix reconnection
    if (mqttClient.connect(THING_ID, THING_ID, MQTT_PASSWORD)) {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("connected\n");
      // ... and resubscribe
      char MQTT_SUBSCRIBE[MQTT_MAX_PACKET_SIZE];
#define QOS_AT_LEAST_1 1

      mqttSubscribe_color.toCharArray(MQTT_SUBSCRIBE, mqttSubscribe_color.length() + 1);
      mqttClient.subscribe(MQTT_SUBSCRIBE, QOS_AT_LEAST_1);
      Serial.print("Subscibed to: ");
      Serial.println(MQTT_SUBSCRIBE);

      mqttSubscribe_config.toCharArray(MQTT_SUBSCRIBE, mqttSubscribe_config.length() + 1);
      mqttClient.subscribe(MQTT_SUBSCRIBE, QOS_AT_LEAST_1);
      Serial.print("Subscibed to: ");
      Serial.println(MQTT_SUBSCRIBE);

      mqttSubscribe_brightness.toCharArray(MQTT_SUBSCRIBE, mqttSubscribe_brightness.length() + 1);
      mqttClient.subscribe(MQTT_SUBSCRIBE, QOS_AT_LEAST_1);
      Serial.print("Subscibed to: ");
      Serial.println(MQTT_SUBSCRIBE);

      publishBeat(true);

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void publishRandomColor() {
  StaticJsonBuffer<MQTT_MAX> jsonBufferMQTT;
  JsonObject& jsonMsg = jsonBufferMQTT.createObject();

  jsonMsg["p"] = last_spot;
  jsonMsg["h"] = spots[last_spot].h;
  jsonMsg["s"] = spots[last_spot].s;
  jsonMsg["v"] = spots[last_spot].v;

  jsonMsg["random"] = last_random;

  jsonMsg["count"] = radiationWatch.radiationCount();
  jsonMsg["cpm"] =  radiationWatch.cpm();
  jsonMsg["uSv/h"] = radiationWatch.uSvh();
  jsonMsg["uSv/h Error"] = radiationWatch.uSvhError();


  String jsonU;

  char mqttData[MQTT_MAX];
  jsonMsg.printTo(mqttData);

  char mqttTopicPub[MQTT_MAX];
  mqttPublish_randomColor.toCharArray(mqttTopicPub, mqttPublish_randomColor.length() + 1);

  int ret = mqttClient.publish(mqttTopicPub, mqttData);

  Serial.print("MQTT message sent: ");
  Serial.print(mqttTopicPub);
  Serial.print(" ");
  Serial.print(mqttData);
  Serial.print(" result: ");
  Serial.println(ret);
}

void publishBeat(bool force) {
  static unsigned long last_beat = 0;
  unsigned long now = millis();

  if ((now - last_beat > BEAT_INTERVAL) || force ) {
    last_beat = now;
    static unsigned long b = 0;
    String bb = String(b);
    b++;

    StaticJsonBuffer<MQTT_MAX> jsonBufferMQTT;
    JsonObject& jsonMsg = jsonBufferMQTT.createObject();

    jsonMsg["count"] = bb;

    jsonMsg["softwareName"] = softwareName;
    jsonMsg["softwareVersion"] = softwareVersion;

    //jsonMsg["MD5"] = ESP.getSketchMD5();

    jsonMsg["thingName"] = THING_NAME;

    String jsonU;
    char mqttData[MQTT_MAX];
    jsonMsg.printTo(mqttData);

    char mqttTopicPub[MQTT_MAX];

    mqttPublish_beat.toCharArray(mqttTopicPub, mqttPublish_beat.length() + 1);
    int ret = mqttClient.publish(mqttTopicPub, mqttData);

    Serial.print("MQTT message sent: ");
    Serial.print(mqttTopicPub);
    Serial.print(" ");
    Serial.print(mqttData);
    Serial.print(" result: ");
    Serial.println(ret);
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String Topic = String(topic);
  Serial.println("MQTT message arrived: " + Topic);

  int p1 = mqttRoot.length() + 1;
  int p2 = Topic.indexOf("/", p1);

  // String topic_root = String(topic).substring(0, p1-1);
  String topic_id = String(topic).substring(p1, p2);
  String topic_leaf = String(topic).substring(p2 + 1);

  //Serial.println(topic_root);
  Serial.println(topic_id);
  Serial.println(topic_leaf);

  DynamicJsonBuffer jsonBuffer(MQTT_MAX);

  if ((topic_id = thingId) ) {

    if (topic_leaf == mqtt_color) {

      /*
        {
        "p": 1,
        "h": 128,
        "s": 128,
        "v": 255
        }
      */

      JsonObject& root = jsonBuffer.parseObject(payload);

      int p = root["p"];
      spots[p].h = root["h"];
      spots[p].s = root["s"];
      spots[p].v = root["v"];

      Serial.print("spot "); Serial.print(p); Serial.print(" ");
      Serial.print("hsv ");
      Serial.print(spots[p].h); Serial.print(" ");
      Serial.print(spots[p].s); Serial.print(" ");
      Serial.print(spots[p].v); Serial.print(" ");

      Serial.println();

      drawDots();

    }

    if ( (topic_leaf == mqtt_config)) {
      JsonObject& root = jsonBuffer.parseObject(payload);
      /*
          {
          "command":"update",
          "option":"http://iot.marcobrianza.it/art/9randomSpots.ino.d1_mini.bin"
          }
      */

      String command = root["command"];
      String option = root["option"];

      Serial.println(command);
      Serial.println(option);

      if (command = "update") {
        showAllLeds(64, 0, 0);
        int u = httpUpdate(option);
        if (u != HTTP_UPDATE_OK) showAllLeds(64, 64, 0);
      }

      Serial.println(command);
      Serial.println(option);
    }

    if ( (topic_leaf == mqtt_brightness)) {
      payload[length] = '\0';
      String s = String((char*)payload);
      int i = s.toInt();
      i = constrain(i, 0, 255);
      Serial.println(i);
      FastLED.setBrightness(i);
      FastLED.show();

    }

  }
}

