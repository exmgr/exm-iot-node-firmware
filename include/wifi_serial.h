#ifndef WIFI_SERIAL_H
#define WIFI_SERIAL_H
#include <HardwareSerial.h>
#include "const.h"

#include <HTTPClient.h>

/******************************************************************************
* Subclasses Hardware serial to intercept all printed messages and forward them
* via WiFi to HTTP logging service
* Uses ESP32 HTTPClient class.
* Note: HTTPClient is ESP32 native library while HttpClient (camelcase) is a
* 3rd party arduino library
******************************************************************************/

class WifiSerial : public HardwareSerial
{
public:
    WifiSerial(int uart_nr);

    size_t write(uint8_t c);
    size_t write(const uint8_t *buffer, size_t size);

    void flush();
    void append_char(char c);
private:
    /** Buffer for incoming messages. Messages are stored here until buffer is full
     * or newline is found */
    char _buff[WIFI_SERIAL_BUFFER_SIZE] = "";

    /** Current buffer length */
    int _buff_len = 0;

    /** HTTP client object used for all requests */
    HTTPClient _http_client;

    WiFiClientSecure _wifi_secure_client;
};

extern WifiSerial WifiDebugSerial;

#endif