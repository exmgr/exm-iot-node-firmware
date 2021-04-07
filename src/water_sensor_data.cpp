#include "water_sensor_data.h"
#include "CRC32.h"
#include "utils.h"
#include "common.h"

namespace WaterSensorData
{
	/** 
	 * Store for water sensor data
	 * Number of entries per file is the same as the number of entries in a request packet.
	 * This way if a request succeeds, a whole file can be deleted, if not the file remains
	 * to be resent at a later time
	 */
    DataStore<WaterSensorData::Entry> store(WATER_SENSOR_DATA_PATH, WATER_SENSOR_DATA_ENTRIES_PER_SUBMIT_REQ);

    /******************************************************************************
    * Add water sensor data to storage
	* No packet is added without having its CRC updated first
    ******************************************************************************/
    RetResult add(WaterSensorData::Entry *data)
    {
		RetResult ret = store.add(data);

		// Commit on every add
		store.commit();

        return ret;
    }   

    /******************************************************************************
    * Get pointer to store (for use with reader)
    ******************************************************************************/
    DataStore<WaterSensorData::Entry>* get_store()
    {
        return &store;
    }

    /********************************************************************************
	 * Print all data from a water sensor data strcut
	 * @param data Water sensor data structure
	 *******************************************************************************/
	void print(const WaterSensorData::Entry *data)
	{
		debug_print(F("Timestamp: "));
		debug_println(data->timestamp);

		debug_print(F("Temperature: "));
		debug_println(data->temperature);

		debug_print(F("Diss. oxygen: "));
		debug_println(data->dissolved_oxygen);

		debug_print(F("Conductivity: "));
		debug_println(data->conductivity);

		debug_print(F("PH: "));
		debug_println(data->ph);

		debug_print(F("ORP: "));
		debug_println(data->orp);

		debug_print(F("Pressure: "));
		debug_println(data->pressure);

		debug_print(F("Depth (cm): "));
		debug_println(data->depth_cm);

		debug_print(F("Depth (ft): "));
		debug_println(data->depth_ft);

		debug_print(F("Water level: "));
		debug_println(data->water_level);
	}
}