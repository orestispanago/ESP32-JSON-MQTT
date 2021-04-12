#include <Arduino.h>
#include <connection.h>

unsigned long uploadInterval = 10000;
unsigned long lastUploadMillis;

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // config WiFi as client
}

void loop()
{
  if (connected())
  {
    if (millis() - lastUploadMillis > uploadInterval)
    {
      String json = "{\"temp_in\":\"" + String(random(10, 20)) + "\"}";
      char *payload = &json[0]; // converts String to char*
      mqttClient.publish(input_topic, payload);
      mqttClient.loop(); //      give control to MQTT to send message to broker
      lastUploadMillis = millis();
    }
    mqttClient.loop();
  }
}
