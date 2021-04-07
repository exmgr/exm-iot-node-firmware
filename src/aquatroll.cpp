#include "aquatroll.h"
#include "utils.h"
#include "rtc.h"
#include "log.h"
#include "sdi12_sensor.h"
#include "common.h"

namespace Aquatroll
{
    /******************************************************************************
     * Initialization
     ******************************************************************************/
    RetResult init()
    {
        return RET_OK;
    }

    /******************************************************************************
    * Measure correct Aquatroll model depending on config
    ******************************************************************************/
    RetResult measure(WaterSensorData::Entry *data)
    {
        switch (AQUATROLL_MODEL)
        {
        case AQUATROLL_MODEL_400:
            return measure_aquatroll400(data);
            break;
        case AQUATROLL_MODEL_500:
            return measure_aquatroll500(data);
            break;
        case AQUATROLL_MODEL_600:
            return measure_aquatroll600(data);
            break;
        default:
            debug_println_e(F("Invalid Aquatroll model"));
            break;
        }

        return RET_ERROR;
    }

    /******************************************************************************
     * Send measure command to the sensor and fill data structure
     * Aquatroll is configured to return the following measurements (in this order):
     * RDO - Dissolved oxygen (concentration) - mg/L
     * RDO - Dissolved oxygen (%saturation) - %Sat
     * RDO - Temperature - C
     * Cond - Specific Conductivity - uS/cm
     * pH/ORP - pH
     * pH/ORP - Oxidation Reduction Potential (ORP) - mV
     * Pres(A) 250ft - Pressure mBar
     * Pres(A) 250ft - Depth - cm
     * @param data Output structure
     ******************************************************************************/
    RetResult measure_aquatroll400(WaterSensorData::Entry *data)
    {
        debug_println("Measuring water quality.");

        // Return dummy values switch
        if(FLAGS.MEASURE_DUMMY_WATER_QUALITY)
        {
            return measure_dummy(data);
        }

        // Zero structure
        memset(data, 0, sizeof(WaterSensorData::Entry));

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

                Log::log(Log::WATER_QUALITY_INVALID_RESPONSE);

                return RET_ERROR;
            }

            return RET_ERROR;
        }

        // Check if seconds within range. Range values are chosen empirically.
        // If values not within range, something is wrong with the response or with the
        // configuration of Aquatroll
        if(secs_to_wait > AQUATROLL_MEASURE_WAIT_SEC_MAX)
        {
            debug_println(F("Invalid number of seconds to wait for measurements."));
            Serial.print(F("Seconds to wait: "));
            Serial.println(secs_to_wait, DEC);
            return RET_ERROR;
        }
        // The exact number of measured values is known and configured into Aquatroll
        if(measured_values != AQUATROLL400_NUMBER_OF_MEASUREMENTS)
        {
            debug_print(F("Invalid number of measured values. Expected: "));
            debug_println(AQUATROLL400_NUMBER_OF_MEASUREMENTS, DEC);
            Serial.print(F("Returned: "));
            Serial.println(measured_values, DEC);
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
        WaterSensorData::Entry sensor_data = {0};

        // Vars to receive all the measurement data
        // Data will be copied to output structure only after successfull measurement
        float d1 = 0, d2 = 0, d3 = 0, d4 = 0, d5 = 0, d6 = 0, d7 = 0, d8 = 0;

        //
        // Batch 0
        //
        int tries = 3;
        RetResult ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(0, &d1, &d2, &d3) != RET_OK)
            {
                debug_println(F("Could not get measurement results for batch 0."));

                Log::log(Log::WATER_QUALITY_MEASUREMENT_DATA_REQ_FAILED, 0);

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
        // Batch 1
        //
        tries = 3;
        ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(1, &d4, &d5, &d6) != RET_OK)
            {
                debug_println(F("Could not get measurement results for batch 1."));
                Log::log(Log::WATER_QUALITY_MEASUREMENT_DATA_REQ_FAILED, 1);

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
        // Batch 2
        //
        tries = 3;
        ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(2, &d7, &d8, nullptr) != RET_OK)
            {
                debug_println(F("Could not get measurement results for batch 2."));
                Log::log(Log::WATER_QUALITY_MEASUREMENT_DATA_REQ_FAILED, 2);

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
        data->dissolved_oxygen = d1;
        data->temperature = d3;
        data->conductivity = d4;
        data->ph = d5;
        data->orp = d6;
        data->pressure = d7;
        data->depth_cm = d8;
        data->depth_ft = data->depth_cm * 0.032808399;

        // If all fields 0 return error even if CRC is OK
        if(data->dissolved_oxygen == 0 && data->temperature == 0 && data->conductivity == 0 && data->ph == 0 && 
        data->orp == 0 && data->pressure == 0 && data->depth_cm == 0 && data->depth_ft == 0)
        {
            debug_println(F("All measured vals are 0, aborting."));
            Log::log(Log::WATER_QUALITY_ZERO_VALS);
            return RET_ERROR;
        }

        Utils::serial_style(STYLE_GREEN);
        debug_println(F("All water quality data is received successfully."));
        Utils::serial_style(STYLE_RESET);

        return RET_OK;
    }

    /******************************************************************************
     * Send measure command to the sensor and fill data structure
     * Aquatroll is configured to return the following measurements (in this order):
     * RDO - Dissolved Oxygen (concentration) - mg/L
     * RDO - Dissolved Oxygen (%saturation) - %Sat
     * Cond - Temperature - C
     * Cond - Specific Conductivity - uS/cm
     * pH/ORP - ph -pH
     * pH/ORP - Oxidation Reductino Potential (ORP) - mV
     * Pres 30ft - Pressure - PSI
     * Pres 30ft - Depth - ft
     * @param data Output structure
     ******************************************************************************/
    RetResult measure_aquatroll500(WaterSensorData::Entry *data)
    {
        debug_println("Measuring water quality.");

        // Return dummy values switch
        if(FLAGS.MEASURE_DUMMY_WATER_QUALITY)
        {
            return measure_dummy(data);
        }

        // Zero structure
        memset(data, 0, sizeof(WaterSensorData::Entry));

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

                Log::log(Log::WATER_QUALITY_INVALID_RESPONSE);

                return RET_ERROR;
            }

            return RET_ERROR;
        }

        // Check if seconds within range. Range values are chosen empirically.
        // If values not within range, something is wrong with the response or with the
        // configuration of Aquatroll
        if(secs_to_wait > AQUATROLL_MEASURE_WAIT_SEC_MAX)
        {
            debug_println(F("Invalid number of seconds to wait for measurements."));
            Serial.print(F("Seconds to wait: "));
            Serial.println(secs_to_wait, DEC);
            return RET_ERROR;
        }
        // The exact number of measured values is known and configured into Aquatroll
        if(measured_values != AQUATROLL500_NUMBER_OF_MEASUREMENTS)
        {
            debug_print(F("Invalid number of measured values. Expected: "));
            debug_println(AQUATROLL500_NUMBER_OF_MEASUREMENTS, DEC);
            Serial.print(F("Returned: "));
            Serial.println(measured_values, DEC);
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
        WaterSensorData::Entry sensor_data = {0};

        // Vars to receive all the measurement data
        // Data will be copied to output structure only after successfull measurement
        float d1 = 0, d2 = 0, d3 = 0, d4 = 0, d5 = 0, d6 = 0, d7 = 0, d8 = 0;

        //
        // Batch 0
        //
        int tries = 3;
        RetResult ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(0, &d1, &d2, &d3) != RET_OK)
            {
                debug_println(F("Could not get measurement results for batch 0."));

                Log::log(Log::WATER_QUALITY_MEASUREMENT_DATA_REQ_FAILED, 0);

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
        // Batch 1
        //
        tries = 3;
        ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(1, &d4, &d5, &d6) != RET_OK)
            {
                debug_println(F("Could not get measurement results for batch 1."));
                Log::log(Log::WATER_QUALITY_MEASUREMENT_DATA_REQ_FAILED, 1);

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
        // Batch 2
        //
        tries = 3;
        ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(2, &d7, &d8, nullptr) != RET_OK)
            {
                debug_println(F("Could not get measurement results for batch 2."));
                Log::log(Log::WATER_QUALITY_MEASUREMENT_DATA_REQ_FAILED, 2);

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
        data->dissolved_oxygen = d1;
        data->temperature = d3;
        data->conductivity = d4;
        data->ph = d5;
        data->orp = d6;
        data->pressure = d7;
        data->depth_ft = d8;
        data->depth_cm = data->depth_ft * 30.48f;

        // If all fields 0 return error even if CRC is OK
        if(data->dissolved_oxygen == 0 && data->temperature == 0 && data->conductivity == 0 && data->ph == 0 && 
        data->orp == 0 && data->pressure == 0 && data->depth_cm == 0 && data->depth_ft == 0)
        {
            debug_println(F("All measured vals are 0, aborting."));
            Log::log(Log::WATER_QUALITY_ZERO_VALS);
            return RET_ERROR;
        }

        Utils::serial_style(STYLE_GREEN);
        debug_println(F("All water quality data is received successfully."));
        Utils::serial_style(STYLE_RESET);

        return RET_OK;
    }

    /******************************************************************************
     * Send measure command to the sensor and fill data structure
     * Aquatroll is configured to return the following measurements (in this order):
     * RDO - Dissolved Oxygen (concentration) - mg/L
     * RDO - Dissolved Oxygen (%saturation) - %Sat
     * Cond - Temperature - C
     * Cond - Specific Conductivity - uS/cm
     * Pres 30ft - Pressure - PSI
     * Pres 30ft - Depth - ft
     * Turb - Total suspended solids
     * @param data Output structure
     ******************************************************************************/
    RetResult measure_aquatroll600(WaterSensorData::Entry *data)
    {
        debug_println("Measuring water quality.");

        // Return dummy values switch
        if(FLAGS.MEASURE_DUMMY_WATER_QUALITY)
        {
            return measure_dummy(data);
        }

        // Zero structure
        memset(data, 0, sizeof(WaterSensorData::Entry));

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

                Log::log(Log::WATER_QUALITY_INVALID_RESPONSE);

                return RET_ERROR;
            }

            return RET_ERROR;
        }

        // Check if seconds within range. Range values are chosen empirically.
        // If values not within range, something is wrong with the response or with the
        // configuration of Aquatroll
        if(secs_to_wait > AQUATROLL_MEASURE_WAIT_SEC_MAX)
        {
            debug_println(F("Invalid number of seconds to wait for measurements."));
            Serial.print(F("Seconds to wait: "));
            Serial.println(secs_to_wait, DEC);
            return RET_ERROR;
        }
        // The exact number of measured values is known and configured into Aquatroll
        if(measured_values != AQUATROLL600_NUMBER_OF_MEASUREMENTS)
        {
            debug_print(F("Invalid number of measured values. Expected: "));
            debug_println(AQUATROLL600_NUMBER_OF_MEASUREMENTS, DEC);
            Serial.print(F("Returned: "));
            Serial.println(measured_values, DEC);
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
        WaterSensorData::Entry sensor_data = {0};

        // Vars to receive all the measurement data
        // Data will be copied to output structure only after successfull measurement
        float d1 = 0, d2 = 0, d3 = 0, d4 = 0, d5 = 0, d6 = 0, d7 = 0;

        //
        // Batch 0
        //
        int tries = 3;
        RetResult ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(0, &d1, &d2, &d3) != RET_OK)
            {
                debug_println(F("Could not get measurement results for batch 0."));

                Log::log(Log::WATER_QUALITY_MEASUREMENT_DATA_REQ_FAILED, 0);

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
        // Batch 1
        //
        tries = 3;
        ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(1, &d4, &d5, &d6) != RET_OK)
            {
                debug_println(F("Could not get measurement results for batch 1."));
                Log::log(Log::WATER_QUALITY_MEASUREMENT_DATA_REQ_FAILED, 1);

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
        // Batch 2
        //
        tries = 3;
        ret = RET_ERROR;
        while(tries--)
        {
            if(sensor.read_measurement_data(2, &d7, nullptr, nullptr) != RET_OK)
            {
                debug_println(F("Could not get measurement results for batch 2."));
                Log::log(Log::WATER_QUALITY_MEASUREMENT_DATA_REQ_FAILED, 2);

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
        data->dissolved_oxygen = d1;
        data->temperature = d3;
        data->conductivity = d4;
        data->pressure = d5;
        data->depth_ft = d6;
        data->tss = d7;
        data->depth_cm = data->depth_ft * 30.48f;

        // If all fields 0 return error even if CRC is OK
        if(data->dissolved_oxygen == 0 && data->temperature == 0 && data->conductivity == 0 && data->pressure == 0 && 
        data->pressure == 0 && data->depth_cm == 0 && data->depth_ft == 0 && data->tss == 0)
        {
            debug_println(F("All measured vals are 0, aborting."));
            Log::log(Log::WATER_QUALITY_ZERO_VALS);
            return RET_ERROR;
        }

        Utils::serial_style(STYLE_GREEN);
        debug_println(F("All water quality data is received successfully."));
        Utils::serial_style(STYLE_RESET);

        return RET_OK;
    }

    /******************************************************************************
     * Fill water quality struct with dummy data
     * Used for debugging only
     * @param data Output structure
     *****************************************************************************/
    RetResult measure_dummy(WaterSensorData::Entry *data)
    {
        debug_println(F("Returning dummy values"));
        static WaterSensorData::Entry dummy = {
            24, 20, 50, 7, 200, 64
        };

        // Add/subtract random offset
        dummy.conductivity += (float)random(-400, 400) / 100;
        dummy.dissolved_oxygen += (float)random(-400, 400) / 100;
        dummy.ph += (float)random(-200, 200) / 100;
        dummy.temperature += (float)random(-400, 400) / 100;

        memcpy(data, &dummy, sizeof(dummy));

        // Emulate waiting time
        delay(1000);

        return RET_OK;
    }
}