#include <Arduino.h>
#include <WiFi.h>
#include <MQTTClient.h> // MQTT Client from Joël Gaehwiler https://github.com/256dpi/arduino-mqtt   keepalive manually to 15s

const char *WiFi_SSID = "YourWiFiSSID";
const char *WiFi_PW = "YourWiFiPassword";
const char *mqtt_broker = "YourMQTTBrokerIP";
const char *mqtt_user = "YourMQTTBrokerUsername";
const char *mqtt_pw = "YourMQTTBrokerPassword";
const char *input_topic = "YourTopic";
String clientId = "ESP32Client-"; // Necessary for user-pass auth

unsigned long waitCount = 0;
uint8_t conn_stat = 0;
// Connection status for WiFi and MQTT:
//
// status |   WiFi   |    MQTT
// -------+----------+------------
//      0 |   down   |    down
//      1 | starting |    down
//      2 |    up    |    down
//      3 |    up    |  starting
//      4 |    up    | finalising
//      5 |    up    |     up

unsigned long uploadInterval = 10000;
unsigned long lastUploadMillis;

WiFiClient espClient;       // TCP client object, uses SSL/TLS
MQTTClient mqttClient(512); // MQTT client object with a buffer size of 512 (depends on your message size)

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // config WiFi as client
}

void loop()
{
  if ((WiFi.status() != WL_CONNECTED) && (conn_stat != 1))
  {
    conn_stat = 0;
  }
  if ((WiFi.status() == WL_CONNECTED) && !mqttClient.connected() && (conn_stat != 3))
  {
    conn_stat = 2;
  }
  if ((WiFi.status() == WL_CONNECTED) && mqttClient.connected() && (conn_stat != 5))
  {
    conn_stat = 4;
  }
  switch (conn_stat)
  {
  case 0: // MQTT and WiFi down: start WiFi
    Serial.println("MQTT and WiFi down: start WiFi");
    WiFi.begin(WiFi_SSID, WiFi_PW);
    conn_stat = 1;
    break;
  case 1: // WiFi starting, do nothing here
    Serial.println("WiFi starting, wait : " + String(waitCount));
    waitCount++;
    break;
  case 2: // WiFi up, MQTT down: start MQTT
    Serial.println("WiFi up, MQTT down: start MQTT");
    mqttClient.begin(mqtt_broker, 1883, espClient); //   config MQTT Server, use port 8883 for secure connection
    clientId += String(random(0xffff), HEX);        // Create a random client ID
    mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pw);
    conn_stat = 3;
    waitCount = 0;
    break;
  case 3: // WiFi up, MQTT starting, do nothing here
    Serial.println("WiFi up, MQTT starting, wait : " + String(waitCount));
    mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pw);
    waitCount++;
    break;
  case 4: // WiFi up, MQTT up: finish MQTT configuration
    Serial.println("WiFi up, MQTT up: finish MQTT configuration");
    mqttClient.publish(input_topic, "{\"Status\":\"up and running!\"}");
    conn_stat = 5;
    break;
  }

  if (conn_stat == 5)
  {
    if (millis() - lastUploadMillis > uploadInterval)
    {
      String json = "{\"temp_in\":\"" + String(random(10, 20)) + "\"}";
      char *payload = &json[0]; // converts String to char*
      mqttClient.publish(input_topic, payload);
      mqttClient.loop();           //      give control to MQTT to send message to broker
      lastUploadMillis = millis();
    }
    mqttClient.loop();
  }
}
