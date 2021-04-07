#include "wifi_serial.h"
#include "utils.h"
#include <HardwareSerial.h>
#include "const.h"

#include "wifi_modem.h"
#include "common.h"

#if WIFI_DEBUG_CONSOLE

/******************************************************************************
* Extends HardwareSerial class to intercept serial console messages and forward
* them to HTTP logging service after logging them to regular serial console.
* When Wifi logging is enabled, all debug_print messages are forwarded to 
* WifiSerial as well. Using debug_print messages in this class will result in
* infinite recursion
******************************************************************************/

/** Global serial object */
WifiSerial WifiDebugSerial(0);

/******************************************************************************
* Constructor
******************************************************************************/
WifiSerial::WifiSerial(int uart_nr) : HardwareSerial(uart_nr)
{
    // Set cert
	_wifi_secure_client.setCACert(WIFI_ROOT_CA_CERTIFICATE);   
}

/******************************************************************************
* Write character
******************************************************************************/
size_t WifiSerial::write(uint8_t c)
{
    Serial.write(c);

    append_char(c);

    // Utils::serial_style(STYLE_MAGENTA);
    // Serial.print(F("Wifi write char: "));
    // Serial.println(c);
    // Utils::serial_style(STYLE_RESET);
    return 1;
}

/******************************************************************************
* Write buffer
* Forward via wifi before redirecting back to serial
******************************************************************************/
size_t WifiSerial::write(const uint8_t *buffer, size_t size)
{
    Serial.write(buffer, size);

    for (size_t i = 0; i < size; i++)
    {
        append_char(buffer[i]);
    }

    // Utils::serial_style(STYLE_MAGENTA);
    // Serial.print(F("Wifi write buff: "));
    // Serial.println((char*)buffer);
    // Utils::serial_style(STYLE_RESET);
    return size;
}

/******************************************************************************
* Append character to buffer, flush when full or newline detected
******************************************************************************/
void WifiSerial::append_char(char c)
{
    if(_buff_len >= WIFI_SERIAL_BUFFER_SIZE - 1)
    {
        flush();
    }

    // Newlines mark the end of the log message so buffer is flushed and they
    // are ignored
    if(c == '\n')
    {
        // Ignore newlines at the beginning of a buffer (eg. logs that are just newlines)
        if(_buff_len != 0)
            flush();
    }
    else
    {
        _buff[_buff_len] = c;
        _buff_len++;
    }
}

/******************************************************************************
* Flush buffer to HTTP service
******************************************************************************/
void WifiSerial::flush()
{
    _buff[_buff_len] = '\0';

    int real_pos = 0;
    for (size_t i = 0; i < _buff_len; i++)
    {
        if(_buff[i] != '\n' && _buff[i] != '\r')
        {
            _buff[real_pos++] = _buff[i];

            if(_buff[i] == '\0')
            {
                break;
            }
        }
    }
    _buff[real_pos] = '\0';
    
    // Connect to wifi
    if(!WifiModem::is_connected())
    {
        Serial.println(F("WifiSerial: Connecting to wifi"));
        WifiModem::connect();
    }
    
    //
    // HTTP request
    //
    _http_client.begin(_wifi_secure_client, TIMBER_API_URL);

    // Add headers
    _http_client.addHeader("Content-Type", "application/json");
	_http_client.addHeader("Authorization", TIMBER_AUTH_HEADER);

    // Prepare message
    char req_buff[WIFI_SERIAL_BUFFER_SIZE + 300] = "";
    snprintf(req_buff, sizeof(req_buff), "{\"message\": \"%s\"}", _buff);

    // Serial.print(F("Msg: "));
    // Serial.println(req_buff);

    int code = _http_client.POST((uint8_t*)req_buff, strlen(req_buff));

	if(code != 202)
	{
        Serial.print(F("Could not submit Wifi log, code: "));
        Serial.println(code, DEC);
	}

    // Reset
    _buff_len = 0;
    _buff[0] = '\0';
}

#endif