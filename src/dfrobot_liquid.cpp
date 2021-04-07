#include "dfrobot_liquid.h"

namespace DFRobotLiquid
{
	/******************************************************************************
	 * Init
	 *****************************************************************************/	
	RetResult init()
	{
		analogReadResolution(12);
	}

	/******************************************************************************
	 * Read water level sensor and populate SensorData entry structure
	 * Use appropriate function depending on the WATER_LEVEL_INPUT_CHANNEL switch
	 *****************************************************************************/
	RetResult measure(WaterSensorData::Entry *data)
	{
        debug_println(F("Measuring water level (analog)"));

        // Return dummy values switch
        if(FLAGS.MEASURE_DUMMY_WATER_LEVEL)
        {
            return measure_dummy(data);
        }

		// X measurements, calc average
		int level_raw = 0;
		for(int i = 0; i < WATER_LEVEL_MEASUREMENTS_COUNT; i++)
		{
			level_raw += analogRead(PIN_WATER_LEVEL_ANALOG);
			delay(WATER_LEVEL_DELAY_BETWEEN_MEAS_MS);
		}
		level_raw /= WATER_LEVEL_MEASUREMENTS_COUNT;

		// Convert to mV
		int mv = ((float)3600 / 4096) * level_raw;

		// Convert to CM
		int cm = (mv / WATER_LEVEL_MV_PER_MM) * 10;

		data->water_level = cm;

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

		data->water_level = 64;
        data->water_level += (float)random(-400, 400) / 100;

		// Emulate waiting time
        delay(1000);

        return RET_OK;
    }
}