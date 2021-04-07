#include "atmos41.h"
#include <math.h>
#include "utils.h"
#include "rtc.h"
#include "log.h"
#include "sdi12_sensor.h"
#include "atmos41_data.h"
#include "common.h"

namespace Atmos41
{
	/******************************************************************************
     * Initialization
     ******************************************************************************/
    RetResult init()
    {
        return RET_OK;
    }

    /******************************************************************************
     * Turn weather station ON
     * TODO: Same power source as the water sensors, so turning on/off affects them
     * as well. To be moved to own power source and switch.
     ******************************************************************************/
	RetResult on()
	{
		debug_println(F("Atmos41 ON."));

		#ifdef TCALL_H
			// Turn IP5306 power boost OFF to reduce idle current
			Utils::ip5306_set_power_boost_state(true);
		#endif

		digitalWrite(PIN_WATER_SENSORS_PWR, 1);
		delay(WATER_SENSORS_POWER_ON_DELAY_MS);

        return RET_OK;
	}

    /******************************************************************************
     * Turn weather station OFF
     * TODO: Same power source as the water sensors, so turning on/off affects them
     * as well. To be moved to own power source and switch.
     *****************************************************************************/
	RetResult off()
	{
		debug_println(F("Atmos41 OFF."));

		digitalWrite(PIN_WATER_SENSORS_PWR, 0);
		delay(100);

		#ifdef TCALL_H
			// Turn IP5306 power boost OFF to reduce idle current
			Utils::ip5306_set_power_boost_state(false);
		#endif

        return RET_OK;
	}

    /******************************************************************************
     * Send measure command to the sensor and fill data structure
     * @param data Output structure
     ******************************************************************************/
    RetResult measure(Atmos41Data::Entry *data)
    {
        debug_println("Measuring Atmos41.");

        // Return dummy values switch
        if(FLAGS.MEASURE_DUMMY_WEATHER)
        {
            return measure_dummy(data);
        }

        // Zero structure
        memset(data, 0, sizeof(Atmos41Data::Entry));

        Sdi12Sensor sensor(PIN_SDI12_DATA);

        // Output variables
        // Seconds to wait
        uint16_t secs_to_wait = 0;
        // Vals to be measured
        uint8_t measured_values = 0;

        if(sensor.measure(&secs_to_wait, &measured_values) != RET_OK)
        {
            Sdi12Sensor::ErrorCode error = sensor.get_last_error();
            if(error == Sdi12Sensor::ERROR_INVALID_RESPONSE)
            {
                Log::log(Log::WEATHER_STATION_INVALID_RESPONSE, strlen(sensor.get_buffer()), 0);

                return RET_ERROR;
            }

            return RET_ERROR;
        }

        // Check if seconds within range. Range values are chosen empirically.
        // If values not within range, something is wrong with the response or with the
        // configuration of sensor
        if(secs_to_wait > WEATHER_STATION_MEASURE_WAIT_SEC_MAX)
        {
            debug_println(F("Invalid number of seconds to wait for measurements."));
            return RET_ERROR;
        }
        // The exact number of measured values is constant
        if(measured_values != WEATHER_STATION_NUMBER_OF_MEASUREMENTS)
        {
            debug_print(F("Invalid number of measured values. Expected: "));
            debug_println(WEATHER_STATION_NUMBER_OF_MEASUREMENTS, DEC);
            return RET_ERROR;
        }


        // TODO: 
        // When sleeping, pins go low and sensor is disabled.
        // Use RTC GPIO so power pin  can stay high and ESP32 can sleep instead of waiting
        debug_print(F("Waiting (sec): "));
        debug_println(secs_to_wait + SDI12_MEASURE_EXTRA_WAIT_SECS);
        delay((secs_to_wait + SDI12_MEASURE_EXTRA_WAIT_SECS) * 1000);

        //
        // Request measurement data
        // 
        Atmos41Data::Entry weather_data;

        // Vars to receive all the measurement data
        // Data will be copied to output structure only after successfull measurement
        float d1 = 0, d2 = 0, d3 = 0;

        // Tries requesting data before aborting
        int tries = 3;

        //
        // Batch 0
        //
        tries = 3;
        uint8_t batch = 0;
        RetResult ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(batch, &d1, &d2, &d3) != RET_OK)
            {
                debug_print(F("Could not get measurement results for batch: "));
                debug_println(batch, DEC);

                Log::log(Log::WEATHER_STATION_MEASUREMENT_DATA_REQ_FAILED, batch);

                if(tries > 0)
                {
                    debug_println(F("Retrying"));
                }
                else
                {
                    ret = RET_ERROR;
                }

                delay(500);
            }
            else
            {
                ret = RET_OK;
                break;
            }
        }

        if(ret != RET_OK)
        {

            debug_println(F("Aborting"));
            return RET_ERROR;
        }

		weather_data.solar = d1;
		weather_data.precipitation = d2;
		weather_data.strikes = d3;

        //
        // Batch 1
        //
        tries = 3;
        ret = RET_ERROR;
        batch = 1;

        while(tries--)
        {
            if(sensor.read_measurement_data(batch, &d1, &d2, &d3) != RET_OK)
            {
                debug_print(F("Could not get measurement results for batch: "));
                debug_println(batch, DEC);

                Log::log(Log::WEATHER_STATION_MEASUREMENT_DATA_REQ_FAILED, batch);

                if(tries > 0)
                {
                    debug_println(F("Retrying"));
                }
                else
                {
                    ret = RET_ERROR;
                }

                delay(500);
            }
            else
            {
                ret = RET_OK;
                break;
            }
        }

        if(ret != RET_OK)
        {

            debug_println(F("Aborting"));
            return RET_ERROR;
        }

    	weather_data.wind_speed = d1;
		weather_data.wind_dir = d2;
		weather_data.wind_gust_speed = d3;

		//
        // Batch 2
        //
        tries = 3;
        ret = RET_ERROR;
        batch = 2;

        while(tries--)
        {
            if(sensor.read_measurement_data(batch, &d1, &d2, &d3) != RET_OK)
            {
                debug_print(F("Could not get measurement results for batch: "));
                debug_println(batch, DEC);

                Log::log(Log::WEATHER_STATION_MEASUREMENT_DATA_REQ_FAILED, batch);

                if(tries > 0)
                {
                    debug_println(F("Retrying"));
                }
                else
                {
                    ret = RET_ERROR;
                }

                delay(500);
            }
            else
            {
                ret = RET_OK;
                break;
            }
        }

        if(ret != RET_OK)
        {

            debug_println(F("Aborting"));
            return RET_ERROR;
        }

		weather_data.air_temp = d1;
		weather_data.vapor_pressure = d2;
		weather_data.atm_pressure = d3;

        // Convert pressure to hPa from kPa
        weather_data.atm_pressure *= 10;

        //
        // Calculate relative humidity from air temp and vapor pressure
        //
        weather_data.rel_humidity = 100 * (weather_data.vapor_pressure / (0.611 * exp((17.502 * weather_data.air_temp) / (240.97 + weather_data.air_temp))));
       
        //
        // Calculate dew point
        //
        const float a = 17.625;
        const float b = 243.04;
        weather_data.dew_point = (b * (log(weather_data.rel_humidity / 100) + (a * weather_data.air_temp) / (b + weather_data.air_temp))) /
                                 (a - log(weather_data.rel_humidity / 100) - (a * weather_data.air_temp) / (b + weather_data.air_temp));

        // Copy structure to output
        memcpy(data, &weather_data, sizeof(weather_data));

        Utils::serial_style(STYLE_GREEN);
        debug_println(F("All Weather data is received successfully."));
        Utils::serial_style(STYLE_RESET);

        Atmos41Data::print(data);

        return RET_OK;
    }

    /******************************************************************************
	* Read weather station and log data in memory
	******************************************************************************/
	RetResult measure_log()
	{
        Atmos41Data::Entry data;

        Log::log(Log::WEATHER_STATION_MEASUREMENT_LOG);

        if(Atmos41::on() != RET_OK)
        {
            debug_println(F("Could not turn weather station ON."));
            Atmos41::off();
            return RET_ERROR;
        }

        RetResult ret = Atmos41::measure(&data);

        Atmos41::off();

        if(ret == RET_ERROR)
        {
            debug_println(F("Could not measure weather station."));
            Log::log(Log::WEATHER_STATION_MEASUREMENT_FAILED);
            return RET_ERROR;
        }

        data.timestamp = RTC::get_timestamp();

        debug_println(F("Weather data:"));
		Atmos41Data::print(&data);

		Atmos41Data::add(&data);
		Atmos41Data::get_store()->commit();

        return ret;
    }

    /******************************************************************************
     * Fill weather data struct with dummy data
     * Used for debugging only
     * @param data Output structure
     *****************************************************************************/
    RetResult measure_dummy(Atmos41Data::Entry *data)
    {
        debug_println(F("Returning dummy values"));
        static Atmos41Data::Entry dummy = {
            0, 5, 20, 3, 4, 120, 7, 25, 100, 50, 52           
        };

        // Add/subtract random offset
        dummy.solar += random(-10, 10);
        dummy.precipitation += random(-5, 5);
        dummy.strikes += random(-2, 2);
        
        dummy.wind_speed += (float)random(-5, 5);
        if(dummy.wind_speed < 0)
            dummy.wind_speed = 31 - dummy.wind_speed;
        else if(dummy.wind_speed >= 30)
            dummy.wind_speed -= 30;

        dummy.wind_dir += random(-10, 10);
        if(dummy.wind_dir < 0)
            dummy.wind_dir = 360 - dummy.wind_dir;
        else if(dummy.wind_dir >= 360)
            dummy.wind_dir -= 360;

        dummy.wind_gust_speed += (float)random(-5, 5);
        if(dummy.wind_gust_speed < 0)
            dummy.wind_gust_speed = 31 - dummy.wind_gust_speed;
        else if(dummy.wind_gust_speed >= 30)
            dummy.wind_gust_speed -= 30;

        dummy.air_temp += random(-2, 2);
        if(dummy.air_temp >= 38)
            dummy.air_temp = 38;
        else if(dummy.air_temp <= -10)
            dummy.air_temp = -10;

        dummy.vapor_pressure = (float)random(-25, 25) / 100;
        dummy.atm_pressure = (float)random(-2, 2);
        dummy.rel_humidity = (float)random(-2, 2);        
        dummy.dew_point = (float)random(-2, 2);

        memcpy(data, &dummy, sizeof(dummy));

        // Emulate waiting time
        delay(1000);

        return RET_OK;
    }
}