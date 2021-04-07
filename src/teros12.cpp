#include "utils.h"
#include "rtc.h"
#include "log.h"
#include "sdi12_sensor.h"
#include "common.h"
#include "teros12.h"
#include "power_control.h"
#include "water_sensors.h"

namespace Teros12
{
    /******************************************************************************
    * Private functions
    ******************************************************************************/
   RetResult measure_data(SoilMoistureData::Entry *data);

    /******************************************************************************
     * Initialization
     ******************************************************************************/
    RetResult init()
    {
        return RET_OK;
    }

    /******************************************************************************
    * Measure soil moisture. A wrapper for measure_data, makes sure power is off
    * before returning
    ******************************************************************************/
    RetResult measure(SoilMoistureData::Entry *data)
    {
        debug_println("Measuring Teros12");

        WaterSensors::on();

        RetResult ret = measure_data(data);

        WaterSensors::off();

        return ret;
    }
    
    /******************************************************************************
     * Send measure command to the sensor and fill data structure
     * Aquatroll is configured to return 5 measurements (in this order):
     * RDO - Dissolved oxygen (concentration) - mg/L
     * RDO - Dissolved oxygen (%saturation) - %Sat
     * RDO - Temperature - C
     * Cond - Specific Conductivity - uS/cm
     * pH/ORP
     * @param data Output structure
     ******************************************************************************/
    RetResult measure_data(SoilMoistureData::Entry *data)
    {
               // Zero structure
        memset(data, 0, sizeof(SoilMoistureData::Entry));

        // I2C slave SDI12 adapter
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
                debug_println(F("Invalid response returned."));

                Log::log(Log::SOIL_MOISTURE_INVALID_RESPONSE);

                return RET_ERROR;
            }

            return RET_ERROR;
        }

        // Check if seconds within range. Range values are chosen empirically.
        // If values not within range, something is wrong with the response or with the
        // configuration of Aquatroll
        if(secs_to_wait > TEROS12_MEASURE_WAIT_SEC_MAX)
        {
            debug_println(F("Invalid number of seconds to wait for measurements."));
            return RET_ERROR;
        }
        // The exact number of measured values is known and configured into Teros12
        if(measured_values != TEROS12_NUMBER_OF_MEASUREMENTS)
        {
            debug_print(F("Invalid number of measured values. Expected: "));
            debug_println(TEROS12_NUMBER_OF_MEASUREMENTS, DEC);
            return RET_ERROR;
        }

        debug_print(F("Waiting (sec): "));
        debug_println(secs_to_wait + SDI12_MEASURE_EXTRA_WAIT_SECS);
        delay((secs_to_wait + SDI12_MEASURE_EXTRA_WAIT_SECS) * 1000);

        //
        // Request measurement data
        //
        SoilMoistureData::Entry sensor_data = {0};

        // Vars to receive all the measurement data
        // Data will be copied to output structure only after successfull measurement
        float d1 = 0, d2 = 0, d3 = 0, d4 = 0, d5 = 0;

        //
        // Get measurements
        //
        int tries = 3;
        RetResult ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(0, &sensor_data.vwc, &sensor_data.temperature, &sensor_data.conductivity) != RET_OK)
            {
                debug_println(F("Could not get measurement results."));

                Log::log(Log::SOIL_MOISTURE_MEASUREMENT_DATA_REQ_FAILED, 0);

                if(tries > 0)
                {
                    debug_println(F("Retrying"));
                }
                else
                {
                    ret = RET_ERROR;
                }
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

        //
        // All data received successfully, fill strcture
        //
        memcpy(data, &sensor_data, sizeof(sensor_data));

        // If all fields 0 return error even if CRC is OK
        if(data->conductivity == 0 && data->temperature == 0 && data->vwc == 0)
        {
            debug_println(F("All measured vals are 0, aborting."));

            Log::log(Log::SOIL_MOISTURE_ZERO_VALS);
            return RET_ERROR;
        }

        Utils::serial_style(STYLE_GREEN);
        debug_println(F("All Teros12 data is received successfully."));
        Utils::serial_style(STYLE_RESET);

        return RET_OK;
    }

    /******************************************************************************
	* Read soil moisture sensor and log data in memory
	* @return RET_ERROR Failed to read
	******************************************************************************/
	RetResult log()
	{
		SoilMoistureData::Entry data = {0};

        Log::log(Log::SOIL_MOISTURE_SENSOR_MEASUREMENT_LOG);

		//
		// Read soil moisture sensor
        // 
        if(Teros12::measure(&data) != RET_OK)
        {
            Serial.println(F("Failed reading soil moisture"));
            Log::log(Log::SOIL_MOISTURE_MEASUREMENT_FAILED);
            return RET_ERROR;
        }

		//
		// Set timestamp and save
		//
		data.timestamp = RTC::get_timestamp();

		debug_println(F("Soil moisture data"));
		SoilMoistureData::print(&data);

		SoilMoistureData::add(&data);
		SoilMoistureData::get_store()->commit();

		return RET_OK;
	}
}