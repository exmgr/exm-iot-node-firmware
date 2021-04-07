#ifndef SDI12_SENSOR_H
#define SDI12_SENSOR_H
#include "sdi12.h"
#include "const.h"
#include "common.h"

class Sdi12Sensor
{
public:
	enum ErrorCode
	{
		ERROR_NONE,
        ERROR_NO_RESPONSE,
		ERROR_INVALID_RESPONSE,
		ERROR_CRC_FAIL
	};

	Sdi12Sensor(gpio_num_t data_pin);

    size_t read_response();
    size_t write_command(char *cmd);

	RetResult measure(uint16_t *secs_to_wait, uint8_t *measurement_vals);
	RetResult read_measurement_data(uint8_t batch, float *o1, float *o2, float *o3);

    bool check_crc();

	ErrorCode get_last_error();

	const char *get_buffer();
private:
    /** Default constructor is private, user must provide extra data for init */
    Sdi12Sensor();

    /** SDI12 object used for comms */
    Sdi12 _sdi12;

	void set_last_error(ErrorCode error);
	ErrorCode _last_error = ERROR_NONE;

    /** Received data buffer */
	char _buff[SDI12_RECV_BUFF_SIZE] = "";
};

#endif