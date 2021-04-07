#include "sdi12_sensor.h"
#include "app_config.h"
#include "sdi12_log.h"

/******************************************************************************
 * Default constructor (private)
 *****************************************************************************/

/******************************************************************************
 * Constructor
 *****************************************************************************/
Sdi12Sensor::Sdi12Sensor(gpio_num_t data_pin) : _sdi12(SDI12_UART_NUM, data_pin)
{

}

/******************************************************************************
* Read whole response (until \r) into buffer. Times out if no response.
* Removes \r charcater and replaces with \0
* @return Number of bytes read or 0 on timeout or failure
******************************************************************************/
size_t Sdi12Sensor::read_response()
{
    size_t bytes = _sdi12.readBytesUntil('\r', _buff, sizeof(_buff));

    // Put string termination at the end of the response
    if(bytes > sizeof(_buff) - 1)
    {
        _buff[sizeof(_buff) - 1] = '\0';
    }
    else
    {
        _buff[bytes] = '\0';
    }

    #ifdef DEBUG
        debug_print(F("SDI12 response: "));
        if(bytes == 0)
        {
            debug_println(F("No data"));
        }
        else
        {
            debug_println(_buff);
        }
    #endif

	// Log comms if enabled in config
	if(FLAGS.LOG_RAW_SDI12_COMMS)
	{
		SDI12Log::add(_buff);
	}	


    return bytes;
}

/******************************************************************************
* Write command
******************************************************************************/
size_t Sdi12Sensor::write_command(char *cmd)
{
    #ifdef DEBUG
        debug_print(F("SDI12 writing command: "));
        debug_println(cmd);
    #endif

	// Log comms if enabled in config
	if(FLAGS.LOG_RAW_SDI12_COMMS)
	{
		SDI12Log::add(cmd);
	}	

    return _sdi12.write_command(cmd);
}

/******************************************************************************
 * Send measure command with CRC (aMC!)
 * Sensor responds with time to wait until measurement results are ready
 *****************************************************************************/
RetResult Sdi12Sensor::measure(uint16_t *secs_to_wait, uint8_t *measurement_vals)
{
    set_last_error(ERROR_NONE);

	// Address parsed from response
	// // All responses contain the SDI12 device address as the first char
	int addr = 0;
	// // Number of vals parsed from response
	int vals = 0;

	// // Request measurement start
    write_command("0MC!");
	delay(SDI12_COMMAND_WAIT_TIME_MS);
	
    if(read_response() == 0)
    {
        set_last_error(ERROR_NO_RESPONSE);
        return RET_ERROR;
    }

	// // Parse response.
	// Response format must be ABBBC
	// BBB: seconds to wait
	// C: number of measurements to be returned
	int response_secs = 0, response_vals = 0;

	vals = sscanf(_buff, "%1d%3d%1d", &addr, &response_secs, &response_vals);

	// // Must be exactly 3 vals
	if(vals != 3)
	{
		debug_println(F("Invalid response returned."));

		set_last_error(ERROR_INVALID_RESPONSE);

		return RET_ERROR;
	}

	// // Validate vals
	// // Check address
	if(addr != 0)
	{
		debug_print("Invalid address value, expected: ");
		debug_println(addr);

		set_last_error(ERROR_INVALID_RESPONSE);

		return RET_ERROR;
	}

	*secs_to_wait = response_secs;
	*measurement_vals = response_vals;

	return RET_OK;
}

/******************************************************************************
 * Request measurement results from sensor (aDx!), check CRC and parse into variables
 * @param adapter       SDI12Adapter to use
 * @param batch         Sensor returns measurement data in batches of 3. 0 indexed
 *                      batch number (0-9)
 * @param r1, r2, r3    Vars to receive the results. Unused should be set to nullptr
 * @return RET_ERROR if no data received, could not parse or crc failure
 *****************************************************************************/
RetResult Sdi12Sensor::read_measurement_data(uint8_t batch, float *o1, float *o2, float *o3)
{
	set_last_error(ERROR_NONE);

	// Expected response format
	// aadr+val1+val2+val3
	const char *parse_format = "%1d%f%f%f";
	// Parsed device address
	uint8_t addr = 0;
	// Parsed vals
	float d1 = 0, d2 = 0, d3 = 0;

	// Number of vars to expect
	// If less than this count parsed, abort
	uint8_t var_count = 0;
	if(o1 != nullptr) var_count++;
	if(o2 != nullptr) var_count++;
	if(o3 != nullptr) var_count++;

	// At least one output var must be provided
	if(var_count < 1)
		return RET_ERROR;

	debug_printf("Getting batch %d - Vars: %d\n", batch, var_count);

	//
	// Request data
	//
	char cmd[10] = "";

	snprintf(cmd, sizeof(cmd), "0D%d!", batch);

	write_command(cmd);
	delay(SDI12_COMMAND_WAIT_TIME_MS);
	read_response();

	if(!_sdi12.check_crc(_buff))
	{
		debug_println("Response failed CRC check.");

		set_last_error(ERROR_CRC_FAIL);

		return RET_ERROR;
	}

	// Parse data
	uint8_t vals = sscanf(_buff, parse_format, &addr, &d1, &d2, &d3);

	// // Check response dev address and var count. If not expected amount of vars, abort
	if(addr != 0 || vals != (var_count + 1))
	{
		debug_println("Could not parse response.");

		set_last_error(ERROR_INVALID_RESPONSE);
		
		return RET_ERROR;
	}


	// debug_print(F("Parsed: "));
	// if(o1 != nullptr)
	// 	debug_printf("%f - ", d1);
	// if(o2 != nullptr)
	// 	debug_printf("%f - ", d2);
	// if(o3 != nullptr)
	// 	debug_printf("%f", d3);

	// debug_println();

	// All ok
	if(o1 != nullptr) *o1 = d1;
	if(o2 != nullptr) *o2 = d2;
	if(o3 != nullptr) *o3 = d3;

	return RET_OK;
}

/******************************************************************************
 * Get last received data from adater (for debug)
 *****************************************************************************/
const char* Sdi12Sensor::get_buffer()
{
	// return _adapter->get_data();
    return _buff;
}

/******************************************************************************
 * Get last occurred error
 *****************************************************************************/
Sdi12Sensor::ErrorCode Sdi12Sensor::get_last_error()
{
	return _last_error;	
}

void Sdi12Sensor::set_last_error(ErrorCode error)
{
	_last_error = error;
}
