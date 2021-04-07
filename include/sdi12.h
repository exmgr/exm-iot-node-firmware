#ifndef SDI12_H
#define SDI12_H

#include <Arduino.h>
#include <HardwareSerial.h>

class Sdi12
{
public:
    enum State
    {
        STATE_LISTENING = 1,
        STATE_TX
    };

    Sdi12(int uart_num, gpio_num_t data_pin);

    size_t write_command(char *cmd);

    int available(void);
    int read();
    size_t readBytesUntil(char terminator, char *buffer, size_t length);

    static bool check_crc(const char *buff);
private:
    /** Default constructor privade, useΟr must initialize serial on construct */
    Sdi12();

    /** UART number to use */
    int _uart_num = 0;

    /** Data pin for TX/RX */
    gpio_num_t _data_pin;

    /** HW serial instance, must be initiated and provided by externally */
    HardwareSerial _serial;

    /** Current state */
    State _state;

    void switch_to_tx();
    void switch_to_rx();

    //
    // Constants
    //
    /** SDI12 standard baud rade */
    const int BAUD_RATE = 1200;

    const int UART_TIMEOUT_MS = 3000;
};

#endif