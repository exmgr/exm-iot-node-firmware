#include "fo_data.h"
#include "log.h"

namespace FoData
{
	/**
	 * Private vars
	 */
	DataStore<StoreEntry> store(FO_DATA_STORE_PATH, FO_DATA_STORE_ENTRIES_PER_SUBMIT_REQ);

    /** FO wakeup count */
    int _wakeup_count = 0;

    /******************************************************************************
    * Add entry to store
    ******************************************************************************/
    RetResult add(FoData::StoreEntry *data)
    {
        data->wakeups = _wakeup_count;

		RetResult ret = store.add(data);

		// Commit on every add
		store.commit();

        Log::log(Log::FO_WAKEUPS, _wakeup_count);

        _wakeup_count = 0;

        return ret;
    }   

    /******************************************************************************
    * Get pointer to store (for use with reader)
    ******************************************************************************/
    DataStore<StoreEntry>* get_store()
    {
        return &store;
    }

    /******************************************************************************
    * Increase wake up count
    ******************************************************************************/
    void inc_wakeup_count()
    {
        _wakeup_count++;
    }

    /********************************************************************************
	* Print all data from a struct
	* @param data Data structure
	*******************************************************************************/
	void print(const FoData::StoreEntry *data)
	{
		Utils::print_separator(F("FO Data"));

        Serial.println(F("FO data print not implemented"));

		Utils::print_separator(NULL);
	}
}