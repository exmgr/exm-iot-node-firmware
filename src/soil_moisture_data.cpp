#include "soil_moisture_data.h"
#include "CRC32.h"
#include "utils.h"
#include "common.h"

namespace SoilMoistureData
{
	/** 
	 * Store for moisture sensor data
	 * Number of entries per file is the same as the number of entries in a request packet.
	 * This way if a request succeeds, a whole file can be deleted, if not the file remains
	 * to be resent at a later time
	 */
    DataStore<SoilMoistureData::Entry> store(SOIL_MOISTURE_DATA_PATH, SOIL_MOISTURE_DATA_ENTRIES_PER_SUBMIT_REQ);

    /******************************************************************************
    * Add water sensor data to storage
	* No packet is added without having its CRC updated first
    ******************************************************************************/
    RetResult add(SoilMoistureData::Entry *data)
    {
		RetResult ret = store.add(data);

		// Commit on every add
		store.commit();

        return ret;
    }   

    /******************************************************************************
    * Get pointer to store (for use with reader)
    ******************************************************************************/
    DataStore<SoilMoistureData::Entry>* get_store()
    {
        return &store;
    }

    /********************************************************************************
	 * Print all data from a water sensor data strcut
	 * @param data Water sensor data structure
	 *******************************************************************************/
	void print(const SoilMoistureData::Entry *data)
	{
		debug_print(F("Timestamp: "));
		debug_println(data->timestamp);

		debug_print(F("VWC: "));
		debug_println(data->vwc);

		debug_print(F("Temperature: "));
		debug_println(data->temperature);

		debug_print(F("Conductivity: "));
		debug_println(data->conductivity);
	}
}