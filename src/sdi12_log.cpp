#include "sdi12_log.h"
#include "utils.h"
#include "rtc.h"
#include "common.h"

/******************************************************************************
 * Logs raw SDI12 communication data
 *****************************************************************************/
namespace SDI12Log
{
	DataStore<SDI12Log::Entry> store("/sdi12", 8);

	/******************************************************************************
	 * Add entry
	 *****************************************************************************/
	RetResult add(char *response)
	{
		// To prevent duplicate timestamps (as in Log)
		static uint32_t _last_log_tstamp = 0;
		static int _last_log_tstamp_counter = 0;

		Entry entry = {0};

		uint32_t cur_tstamp = RTC::get_timestamp();
		entry.timestamp = cur_tstamp * 1000LL;	

		// If its not the first log within this second, add +1 mS to make sure TB doesn't overwrite
		// its value (since TB uses the timestamp as the primary key for each record.)
		if(cur_tstamp == _last_log_tstamp)
		{
			_last_log_tstamp_counter++;
			entry.timestamp += _last_log_tstamp_counter;
		}
		else
		{
			_last_log_tstamp_counter = 0;
		}

		_last_log_tstamp = cur_tstamp;	

		// Populate response
		strncpy(entry.response, response, sizeof(entry.response));

		RetResult ret = store.add(&entry);
		store.commit();

		return ret;
	}

    DataStore<Entry>* get_store()
	{
		return &store;
	}
	
	/******************************************************************************
	 * Print all data from a struct
	 *****************************************************************************/
	void print(const Entry *entry)
	{
		Utils::print_separator(F("Weather Data"));

		debug_print(F("Response: "));
		debug_println(entry->response);

		debug_printf("Timestamp: %lld\n", entry->timestamp);
		debug_print(" (");

		time_t tstamp_sec = entry->timestamp / 1000;
		debug_print(ctime(&tstamp_sec));
		debug_println(" )");

		Utils::print_separator(NULL);
	}
}