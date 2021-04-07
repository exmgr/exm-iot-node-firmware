#include "int_env_sensor.h"
#include "const.h"
#include "struct.h"
#include "SparkFunBME280.h"
#include "utils.h"
#include "log.h"
#include "rtc.h"
#include "common.h"

namespace IntEnvSensor
{
//
// Private vars
//
/** Sensor object */
BME280 sensor;

/******************************************************************************
* Init fuel gauge
******************************************************************************/
RetResult init()
{
	sensor.setI2CAddress(BME280_I2C_ADDR1);

	if (sensor.beginI2C(Wire) == false)
	{
		Utils::serial_style(STYLE_RED);
		debug_println(F("Could not communicate with BME280 on addr1"));
		Utils::serial_style(STYLE_RESET);

		sensor.setI2CAddress(BME280_I2C_ADDR2); // TODO check if can be reused
		if (sensor.beginI2C(Wire) == false)
		{
			Utils::serial_style(STYLE_RED);
			debug_println(F("Could not communicate with BME280 on addr2"));
			Utils::serial_style(STYLE_RESET);
			return RET_ERROR;
		}
	}

	return RET_OK;
}

/******************************************************************************
* Wake up, measure and go back to sleep
* Pass NULL to ignore a measurement
* @param temp Temperature output
* @param hum Humidity output
* @param alt Altitude output
* @param press Pressure output
******************************************************************************/
RetResult read(float *temp = NULL, float *hum = NULL, int *press = NULL, int *alt = NULL)
{
	// Wake up
	sensor.setMode(1);

	// Try to read mode back to see if the device is alive
	// There is no other way to check if the sensor actually is there with this lib
	if (sensor.getMode() != 1)
	{
		return RET_ERROR;
	}

	if (temp != NULL)
	{
		*temp = sensor.readTempC();
	}

	if (hum != NULL)
	{
		*hum = sensor.readFloatHumidity();
	}

	if (press != NULL)
	{
		*press = (int)(sensor.readFloatPressure() / 100);
	}

	if (alt != NULL)
	{
		*alt = (int)sensor.readFloatAltitudeMeters();
		//I (manolis) think this requires prior calibration for pressure at sea level or smt
		//TODO confirm its working else remove
	}

	// Go back to sleep
	sensor.setMode(00);

	return RET_OK;
}

/******************************************************************************
* Store measurements in log
******************************************************************************/
RetResult log()
{
	float temp = 0, hum = 0;
	int press = 0, alt = 0;

	RetResult ret = RET_ERROR;
	ret = read(&temp, &hum, &press, &alt);

	if(ret == RET_OK)
	{
		Log::log(Log::INT_ENV_SENSOR1, temp, hum);
		Log::log(Log::INT_ENV_SENSOR2, press, alt);

		Utils::serial_style(STYLE_BLUE);
		debug_printf("Temp: %4.2fC | Hum: %4.2f%% | Press: %dhPa | Alt: %dm\n",
					temp, hum, press, alt);
		Utils::serial_style(STYLE_RESET);
	}
	else
	{
		debug_println(F("Could not read int. env. sensor."));
	}

	// Log RTC tmp
	if(FLAGS.EXTERNAL_RTC_ENABLED)
	{
		Log::log(Log::RTC_TEMPERATURE, RTC::get_external_rtc_temp());
		debug_printf("RTC Temp: %4.2f\n", RTC::get_external_rtc_temp());
	}

	return ret;
}

} // namespace IntEnvSensor