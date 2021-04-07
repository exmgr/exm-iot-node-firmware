#include "sdi12.h"
#include "common.h"

/******************************************************************************
* Constructor
* Set hw serial and rx/tx pin on construct
* @param serial HW serial instance to use. Must be initiated externally
*               and provided by user
* @param data_pin Pin to use for data comm pin
******************************************************************************/
Sdi12::Sdi12(int uart_num, gpio_num_t data_pin) : _serial(uart_num)
{
	_data_pin = data_pin;

	switch_to_rx();
}

/******************************************************************************
* Switch serial to TX mode
******************************************************************************/
void Sdi12::switch_to_tx()
{
    _state = STATE_TX;

    // Restart UART with data pin set to TX and a dummy pin for RX
    gpio_set_direction(_data_pin, gpio_mode_t::GPIO_MODE_DISABLE);
    _serial.begin(BAUD_RATE, SERIAL_7E1, 999, _data_pin, true, UART_TIMEOUT_MS);
}

/******************************************************************************
* Switch serial to RX mode
******************************************************************************/
void Sdi12::switch_to_rx()
{
    _state = STATE_LISTENING;

    // Restart UART with data pin set to RX and a dummy pin for TX
    gpio_set_direction(_data_pin, gpio_mode_t::GPIO_MODE_DISABLE);
    _serial.begin(BAUD_RATE, SERIAL_7E1, _data_pin, 999, true, UART_TIMEOUT_MS);
}

/******************************************************************************
* Check for available bytes in buffer
* @returns Number of available bytes
******************************************************************************/
int Sdi12::available(void)
{
	return _serial.available();
}

/******************************************************************************
* Read a byte from the rx buffer
* @returns Number of bytes read, -1 if no data available
******************************************************************************/
int Sdi12::read()
{
	if(_state != STATE_LISTENING)
		switch_to_rx();

	return _serial.read();
}

/******************************************************************************
* Read bytes from the rx buffer until a delimiter is found
* @returns Number of bytes read
******************************************************************************/
size_t Sdi12::readBytesUntil(char terminator, char *buffer, size_t length)
{
	if(_state != STATE_LISTENING)
		switch_to_rx();

	return _serial.readBytesUntil(terminator, buffer, length);
}

/******************************************************************************
* Write an SDI12 command
* @returns Number of bytes written
******************************************************************************/
size_t Sdi12::write_command(char *cmd)
{
	pinMode(_data_pin, OUTPUT);
	digitalWrite(_data_pin, 1);
	delay(20);
	digitalWrite(_data_pin, 0);
	delay(15);

	switch_to_tx();

	size_t ret = _serial.write(cmd);
	_serial.flush();
	_serial.end();

	switch_to_rx();

	return ret;
}

/******************************************************************************
* Run CRC check on data. It is assumed data contains CRC in the last 3 chars
* as per the SDI12 standard
* @param buff Data to check
* @returns True if check passed
******************************************************************************/
bool Sdi12::check_crc(const char *buff)
{
	int data_length = strlen(buff);

    debug_print(F("LEN: "));
    debug_println(data_length, DEC);

    debug_print(F("Checking CRC for: "));
    debug_println(buff);

	// No point in calculating CRC if there's no data
	if(data_length < 4)
		return false;

	// Extract crc from string
	char input_crc[4] = "";
	strncpy(input_crc, (const char*)&buff[data_length-3], 3);
	input_crc[3] = '\0';

    debug_print(F("CRC: "));
    debug_println(input_crc);

	// Calculate
	uint16_t calced_crc = 0;

	for (int i = 0; i < data_length - 3; i++)
	{
		calced_crc = buff[i] ^ calced_crc;

		for (int j = 1; j <= 8; j++)
		{
			if (calced_crc & 0x01)
			{
				calced_crc = calced_crc >> 1;
				calced_crc = 0xA001 ^ calced_crc;
			}
			else
			{
				calced_crc = calced_crc >> 1;
			}
		}
	}

	char calced_crc_str[4] = "";
	calced_crc_str[0] = 0x40 | (calced_crc >> 12);
	calced_crc_str[1] = 0x40 | ((calced_crc >> 6) & 0x3F);
	calced_crc_str[2] = 0x40 | (calced_crc & 0x3F);

	return strcmp(input_crc, calced_crc_str) == 0;
}