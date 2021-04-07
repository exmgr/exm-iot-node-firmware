#ifndef MQTT_H
#define MQTT_H

#include "gsm.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Adafruit_MQTT_FONA.h"

class MQTT
{
public:
    MQTT(Adafruit_FONA *fona, const char *broker_url, uint16_t port, const char *username, const char *pass);
    ~MQTT();

    uint8_t connect();
    uint8_t disconnect();
    bool is_connected();
    uint8_t publish(char *topic, char *msg);

private:
    uint8_t set_credentials(const char *broker_url, uint16_t port, const char *user_id, const char *username, const char *password);
    Adafruit_MQTT_FONA _mqtt_client;

    // Broker credentials
    const char *_broker_url = NULL;
    const char *_username = NULL;
    const char *_password = NULL;
    const char *_userid = NULL;
    uint16_t _port = 0;
};

#endif