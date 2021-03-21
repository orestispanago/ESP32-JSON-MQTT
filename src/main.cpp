#include <Arduino.h>
#include "WiFi.h"
#include <PubSubClient.h>

const char *ssid = "YourWiFiSSID";
const char *password = "YourWiFiPassword";
const char *mqttServer = "YourMQTTServerIP";
const int mqttPort = 1883;
const char *mqttUser = "YourMQTTBrokerUsername";
const char *mqttPassword = "YourMQTTBrokerPassword";
const char *topic = "YourTopic";

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  pinMode(2, OUTPUT);
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  client.subscribe(topic);
}


void loop()
{
  String json = "{\"temp_in\":\"" + String(random(10, 20)) +  "\"}";
  char* payload = &json[0]; // converts String to char*
  client.publish(topic, payload);
  delay(5000);
}
