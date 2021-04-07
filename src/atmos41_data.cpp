#include "atmos41_data.h"
#include "CRC32.h"
#include "utils.h"
#include "common.h"

#include "data_store.h"

namespace Atmos41Data
{
	/** 
	 * Store for weather data
	 * Number of entries per file is the same as the number of entries in a request packet.
	 * This way if a request succeeds, a whole file can be deleted, if not the file remains
	 * to be resent at a later time
	 */
    DataStore<Atmos41Data::Entry> store(ATMOS41_DATA_PATH, ATMOS41_DATA_ENTRIES_PER_SUBMIT_REQ);

    /******************************************************************************
    * Add water weather data to storage
	* No packet is added without having its CRC updated first
    ******************************************************************************/
    RetResult add(Atmos41Data::Entry *data)
    {
		RetResult ret = store.add(data);

		// Commit on every add
		store.commit();

        return ret;
    }   

    /******************************************************************************
    * Get pointer to store (for use with reader)
    ******************************************************************************/
    DataStore<Atmos41Data::Entry>* get_store()
    {
        return &store;
    }

    /********************************************************************************
	 * Print all data from a struct
	 * @param data Data structure
	 *******************************************************************************/
	void print(const Atmos41Data::Entry *data)
	{
		Utils::print_separator(F("Atmos41 Data"));

		debug_print(F("Timestamp: "));
		debug_println(data->timestamp);

		debug_print(F("Solar: "));
		debug_println(data->solar);

		debug_print(F("Precipitation: "));
		debug_println(data->precipitation);

		debug_print(F("Strikes: "));
		debug_println(data->strikes);

		debug_print(F("Wind speed: "));
		debug_println(data->wind_speed);

		debug_print(F("Wind dir: "));
		debug_println(data->wind_dir, DEC);

		debug_print(F("Wind gust speed: "));
		debug_println(data->wind_gust_speed, DEC);

		debug_print(F("Air temp: "));
		debug_println(data->air_temp, DEC);

		debug_print(F("Vapor press: "));
		debug_println(data->vapor_pressure, DEC);

		debug_print(F("Atm press: "));
		debug_println(data->atm_pressure, DEC);

		debug_print(F("Relative humidity: "));
		debug_println(data->rel_humidity, DEC);

		debug_print(F("Dew point: "));
		debug_println(data->dew_point, DEC);

		Utils::print_separator(NULL);
	}
}