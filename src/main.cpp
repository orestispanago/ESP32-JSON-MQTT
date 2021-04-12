#include <Arduino.h>
#include <connection.h>

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // config WiFi as client
}

void loop()
{
  if (connected())
  {
    String json = "{\"temp_in\":\"" + String(random(10, 20)) + "\"}";
    char *payload = &json[0]; // converts String to char*
    upload(payload);
  }
}