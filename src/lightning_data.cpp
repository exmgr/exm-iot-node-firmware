#include "lightning_data.h"

namespace LightningData
{
	/** 
	 * Store for lIGHTNING sensor data
	 */
    DataStore<LightningData::Entry> store(LIGHTNING_DATA_PATH, LIGHTNING_DATA_ENTRIES_PER_SUBMIT_REQ);

    /******************************************************************************
    * Add Lightning data to tore
    ******************************************************************************/
    RetResult add(LightningData::Entry *data)
    {
		RetResult ret = store.add(data);

		// Commit on every add
		store.commit();

        return ret;
    }   

    /******************************************************************************
    * Get pointer to store (for use with reader)
    ******************************************************************************/
    DataStore<LightningData::Entry>* get_store()
    {
        return &store;
    }

    /********************************************************************************
	 * Print all data from a Lightning data strcut
	 * @param data Lightning data structure
	 *******************************************************************************/
	void print(const LightningData::Entry *data)
	{
		debug_print(F("Timestamp: "));
		debug_println(data->timestamp);

		debug_print(F("Distance (km): "));
		debug_println(data->distance);

		debug_print(F("Energy: "));
		debug_println(data->energy);
	}
} // namespace LightningData
