/* standalone device with MQTT function
 *  
 * The requirements are as follows:
 * - it should run its main tasks regardless of the availability of WiFi/MQTT
 * - it should recover lost WiFi/MQTT without interrupting the main tasks
 * - unless other examples on the net it does not start the WiFi/MQTT connection in the setup() section
 *   since the device should start with its main functions immediately and do the connection setup later
 * 
 * Other features
 * - OTA over WiFi using the Arduino IDE
 * - MQTT over TLS for encrypted data transfer
 * - configuration parameters for device name, SSID, WAP-PW, MQTT broker username and password are stored in Preferences
 * 
 *  Feedback to improve this code is welcome
 *  Urs Eppenberger
 */
#include <Arduino.h>
#include <WiFi.h>       // needed for the WiFi communication
#include <MQTTClient.h> // MQTT Client from Joël Gaehwiler https://github.com/256dpi/arduino-mqtt   keepalive manually to 15s
#include <ArduinoJson.h>

const char *WiFi_SSID = "YourWiFiSSID";
const char *WiFi_PW = "YourWiFiPassword";
const char *mqtt_broker = "YourMQTTBrokerIP";
const char *mqtt_user = "YourMQTTBrokerUsername";
const char *mqtt_pw = "YourMQTTBrokerPassword";
const char *input_topic = "YourTopic";
String clientId = "ESP32Client-"; // Necessary for user-pass auth

unsigned long waitCount; // counter
uint8_t conn_stat;       // Connection status for WiFi and MQTT:
                         //
                         // status |   WiFi   |    MQTT
                         // -------+----------+------------
                         //      0 |   down   |    down
                         //      1 | starting |    down
                         //      2 |    up    |    down
                         //      3 |    up    |  starting
                         //      4 |    up    | finalising
                         //      5 |    up    |     up

unsigned long lastStatus; // counter in example code for conn_stat == 5
unsigned long lastTask;   // counter in example code for conn_stat <> 5

WiFiClient espClient;
MQTTClient mqttClient(512);

unsigned long uploadInterval = 10000;
unsigned long currentMillis;
unsigned long lastUploadMillis;

StaticJsonDocument<200> jsonDoc;
char payload[256];

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // config WiFi as client
}

boolean connected()
{
  // with current code runs roughly 400 times per second
  // start of non-blocking connection setup section
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
    mqttClient.publish(topic, "Up and running!");
    conn_stat = 5;
    break;
  }
  return conn_stat == 5;
}

void loop()
{
  // end of non-blocking connection setup section

  // start section with tasks where WiFi/MQTT is required
  if (connected())
  {
    currentMillis = millis();
    if (currentMillis - lastUploadMillis > uploadInterval)
    {
      lastUploadMillis = currentMillis;
      jsonDoc["temp_in"] = random(10, 20);
      serializeJson(jsonDoc, payload);
      mqttClient.publish(topic, payload);
      mqttClient.loop(); //      give control to MQTT to send message to broker
      Serial.print("FreeHeap: ");
      Serial.println(ESP.getFreeHeap());
    }
    mqttClient.loop(); // internal household function for MQTT
  }
  // end of section for tasks where WiFi/MQTT are required

  // start section for tasks which should run regardless of WiFi/MQTT
  // if (millis() - lastTask > 1000) {                                 // Print message every second (just as an example)
  //   Serial.println("print this every second");
  //   lastTask = millis();
  // }
  // delay(100);
  // end of section for tasks which should run regardless of WiFi/MQTT
}
