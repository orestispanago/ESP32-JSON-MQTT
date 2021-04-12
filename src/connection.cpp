#include <connection.h>
#include <config.h>

String clientId;
unsigned long lastUploadMillis;

unsigned long waitCount;
uint8_t conn_stat;
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

WiFiClient espClient;       // TCP client object, uses SSL/TLS
MQTTClient mqttClient(512); // MQTT client object with a buffer size of 512 (depends on your message size)

boolean connected()
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
        mqttClient.begin(mqtt_broker, 1883, espClient);          //   config MQTT Server, use port 8883 for secure connection
        clientId = "ESP32Client-" + String(random(0xffff), HEX); // Create a random client ID
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
    return conn_stat == 5;
}

void upload(char *payload)
{
    if (millis() - lastUploadMillis > uploadInterval)
    {
        mqttClient.publish(input_topic, payload);
        mqttClient.loop(); //      give control to MQTT to send message to broker
        lastUploadMillis = millis();
    }
    mqttClient.loop();
}