#ifndef connection_h
#define connection_h

#include <WiFi.h>
#include <MQTTClient.h> // MQTT Client from Joël Gaehwiler https://github.com/256dpi/arduino-mqtt   keepalive manually to 15s


boolean connected();
void upload(char* payload);

#endif