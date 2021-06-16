#include "SPIFFS.h"
#include "log.h"
#include "const.h"
#include "rtc.h"
#include "utils.h"
#include "common.h"

namespace Log
{
	/** Log data store */
	DataStore<Log::Entry> store(LOG_DATA_PATH, LOG_ENTRIES_PER_SUBMIT_REQ);

	/**
	 * Timestamp of the last time a log has been recorded (log() called)
	 * Logs are submitted to TB and TB uses the timestamp as a key, so if multiple values
	 * have the same mS timestamp, only the last one is stored. RTC tracks time in seconds so
	 * if log() is called more than once a second, the result is multiple logs with the same key for TB.
	 * A quick workaround is to add 1mS every time log() is called within the same second.
	 * This works only if RTC works and tracks time correctly and there are no logs stored
	 * with a future timestamp because RTC was reset.
	 */
	uint32_t _last_log_tstamp = 0;
	int _last_log_tstamp_counter = 0;

	/**
	 * When disabled, all logs are ignored until enabled again.
	 */
	bool _enabled = true;

	/******************************************************************************
	* Create log entry with current timestamp.
	* @param code Error code
	* @param meta1 Metadata field 1
	* @param meta2 Metadata field 2
	******************************************************************************/
	bool log(Log::Code code, uint32_t meta1, uint32_t meta2)
	{
		Entry entry;

		entry.code = code;
		entry.meta1 = meta1;
		entry.meta2 = meta2;

		// debug_print_i(F("Log: "));
		// debug_print(entry.code, DEC);
		// debug_print(F(" ( "));
		// debug_print(meta1);
		// debug_print(F(" , "));
		// debug_print(meta2);
		// debug_println(F(" )"));

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


		if(!_enabled)
		{
			Utils::serial_style(STYLE_RED);
			debug_print(F("Logging disabled, ignoring log: "));
			print(&entry);
			Utils::serial_style(STYLE_RESET);
			return RET_ERROR;
		}

		store.add(&entry);
		store.commit();

		return RET_OK;
	}

	/******************************************************************************
	* Get pointer to store (for use with reader)
	******************************************************************************/
	DataStore<Entry>* get_store()
	{
		return &store;
	}

	/******************************************************************************
	* Commit log data store to flash
	******************************************************************************/
	RetResult commit()
	{
		return store.commit();
	}

	/********************************************************************************
	 * Print all data from a log entry struct
	 * @param entry Pointer to log entry struct
	 *******************************************************************************/
	void print(const Log::Entry *entry)
	{
		debug_print(F("Code: "));
		debug_print(entry->code);

		debug_print(F(" -- Meta 1: "));
		debug_print(entry->meta1);

		debug_print(F(" -- Meta 2: "));
		debug_println(entry->meta2);

		debug_printf("Timestamp: %lld\n", entry->timestamp);
		debug_print(" (");

		time_t tstamp_sec = entry->timestamp / 1000;
		debug_print(ctime(&tstamp_sec));
		debug_println(" )");
	}

	/******************************************************************************
	 * Enable/disable logging
	 *****************************************************************************/
	void set_enabled(bool enabled)
	{
		_enabled = enabled;
	}
}