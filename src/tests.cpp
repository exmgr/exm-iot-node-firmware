#include "tests.h"
#include <Preferences.h>
#include "CRC32.h"
#include "SPIFFS.h"
#include "const.h"
#include "app_config.h"
#include "gsm.h"
#include "flash.h"
#include "rtc.h"
#include "utils.h"
#include "data_store.h"
#include "data_store_reader.h"
#include "water_sensor_data.h"
#include "sleep_scheduler.h"
#include "device_config.h"
#include "limits.h"
#include "remote_control.h"
#include "device_config.h"
#include "common.h"

namespace Tests
{
	/******************************************************************************
	 * Privates
	 ******************************************************************************/
	enum TestResult
	{
		SUCCESS,
		FAILURE,

		// Test didn't run
		IGNORED = -1
	};

	/** Pointers to test functions mapped to their type */
	RetResult (*test_funcs[])() = {
		[RTC_FROM_GSM] = rtc_from_gsm,
		[DATA_STORE] = data_store,
		[WAKEUP_TIMES] = wakeup_times,
		[DEVICE_CONFIG] = device_config
	};

	/** Test names mapped to their type */
	const char *test_names[] = {
		[RTC_FROM_GSM] = "RTC from GSM",
		[DATA_STORE] = "Buffered data store",
		[WAKEUP_TIMES] = "Wake-up times",
		[DEVICE_CONFIG] = "Device configuration store"
	};

	/******************************************************************************
	 * Test specific config
	******************************************************************************/
	
	//
	// Data store
	//
	// Total packets to write
	const int DATA_STORE_ELEMENTS_TO_WRITE = 150;

	// Path of test data store
	const char *DATA_STORE_PATH = "/test";

	// Entries to write in a file before creating a new one
	const char DATA_STORE_ENTRIES_PER_FILE = 12;

	// Size of a single entry in the data store (header + body)
	const int DATA_STORE_ENTRY_SIZE = sizeof(DataStore<WaterSensorData::Entry>::Entry);

	// Size in bytes of a file that doesn't fit any more entries
	const int DATA_STORE_FULL_FILE_SIZE = DATA_STORE_ENTRY_SIZE *  DATA_STORE_ENTRIES_PER_FILE;

	//
	// Wakeup times
	//
	// How many wake up "times" to calculate starting from now
	const int WAKEUP_TIMES_SERIES_LEN = 100;


	/******************************************************************************
	 * Set dummy date in RTC, ask GSM module to update time from NTP and see if
	 * it has changed
	******************************************************************************/
	RetResult rtc_from_gsm()
	{
		// todo: fix unimplemented non trivial designated initializers then remove
		return RET_ERROR;
		//
		// Update RTC
		//
		debug_println(F("Updating RTC time"));
		RetResult success = RET_ERROR;
		GSM::init();
		if(GSM::on() == RET_OK)
		{
			if(GSM::connect_persist() == RET_OK)
			{
				if(RTC::sync() != RET_OK)
				{
					debug_println("Could not update time from GSM.");
				}
				else
				{
					success = RET_OK;
				}
			}

			GSM::off();
		}
		if(!success)
		{
			debug_println(F("Could not update RTC time."));
		}
		else
		{
			debug_println(F("RTC updated: "));
			RTC::print_time();
		}

		return success;   
	}

	/******************************************************************************
	 * Buffered data store
	 * Write dummy data, read back, check values
	******************************************************************************/
	RetResult data_store()
	{
		//
		// Mount
		//
		Utils::serial_style(STYLE_BLUE);
		debug_println(F("# Mounting"));
		Utils::serial_style(STYLE_RESET);
		if(Flash::mount() != RET_OK)
		{
			debug_println(F("# Could not begin SPIFFS."));
			return RET_ERROR;
		}

		//
		// Format
		//
		Utils::serial_style(STYLE_BLUE);
		debug_println(F("# Formatting"));
		Utils::serial_style(STYLE_RESET);
		if(!SPIFFS.format())
		{
			debug_println(F("# Format failed."));
			return RET_ERROR;
		}
		Utils::serial_style(STYLE_BLUE);
		debug_println(F("# Format done"));
		Utils::serial_style(STYLE_RESET);

		//
		// Create data store 
		//
		Utils::serial_style(STYLE_BLUE);
		debug_println(F("# Creating dummy entries"));
		Utils::serial_style(STYLE_RESET);
		DataStore<WaterSensorData::Entry> store(DATA_STORE_PATH, DATA_STORE_ENTRIES_PER_FILE);
		WaterSensorData::Entry dummy_data_entries[DATA_STORE_ELEMENTS_TO_WRITE] = {0};

		//
		// Build dummy data, add to store and check after every entry if data written is expected
		//

		// Counter of total bytes written
		int expected_bytes_written = 0;

		for(int i = 0; i < DATA_STORE_ELEMENTS_TO_WRITE; i++)
		{
			// Build new entry dummy data
			WaterSensorData::Entry new_entry = {0};
			new_entry.timestamp = i; // This entry will be later searched for by this id
			new_entry.temperature = random(1000000);
			new_entry.dissolved_oxygen = random(1000000);
			new_entry.conductivity = random(1000000);
			new_entry.ph = random(1000000);
			new_entry.water_level = random(1000000);

			// Write to store
			store.add(&new_entry);
			if(store.commit() != RET_OK)
			{
				debug_println(F("Could not commit data."));
				return RET_ERROR;
			}

			expected_bytes_written += DATA_STORE_ENTRY_SIZE;

			// Add to dummy array to confirm later
			memcpy(&dummy_data_entries[i], &new_entry, sizeof(new_entry));

			// Expected number of files with size DATA_STORE_ENTRY_SIZE * DATA_STORE_ENTRIES_PER_FILE
			int expected_full_files = ((i+1) * DATA_STORE_ENTRY_SIZE) / DATA_STORE_FULL_FILE_SIZE;

			// Other than full files, only one smaller file can exist and it must be of this size.
			// When 0, last file is full  so there can be more of these which is checked by above
			int expected_smallest_file_size = ((i+1) * DATA_STORE_ENTRY_SIZE) - expected_full_files * DATA_STORE_FULL_FILE_SIZE;
			debug_print(F("Smallest file must be: "));
			debug_println(expected_smallest_file_size);
			// Iterate all files and check if above above data is true
			File dir = SPIFFS.open(DATA_STORE_PATH);
			if(!dir)
			{
				debug_println(F("Could not open data store dir."));
				return RET_ERROR;
			}

			// Number of full files found in store
			int found_full_files = 0;
			// Number of files with size other than that of a full file
			int found_other_size_files = 0;
			// Size of smallest file found. Must be only 1 and of matching size
			int found_smallest_file_size = 0;
			// Bytes written so far
			int found_bytes_written = 0;

			// Iterate all files written so file and collect info
			File f;
			while(f = dir.openNextFile())
			{
				int cur_size = f.size();
				found_bytes_written += cur_size;

				if(cur_size == DATA_STORE_FULL_FILE_SIZE)
					found_full_files++;
				else
				{
					found_other_size_files++;

					// If more than one found, test fails early, no reason to go further
					if(found_other_size_files > 1)
					{
						debug_println(F("More than 1 non-full files found. Should be only 1. Aborting."));
						return RET_ERROR;
					}

					found_smallest_file_size = f.size();
				}
			}

			// Check results for current run
			if(expected_full_files != found_full_files)
			{
				debug_println(F("Number of files that reached their max size is not the expected one."));
				debug_print(F("Expected: "));
				debug_println(expected_full_files, DEC);
				debug_print(F("Found: "));
				debug_println(found_full_files, DEC);
				debug_println(F("Aborting"));

				Flash::ls();

				return RET_ERROR;
			}
			if(found_smallest_file_size != expected_smallest_file_size)
			{
				debug_println(F("Smallest file (currently being filled) is not of expected size."));
				debug_print(F("Expected: "));
				debug_println(expected_smallest_file_size, DEC);
				debug_print(F("Found: "));
				debug_println(found_smallest_file_size, DEC);
				debug_println(F("Aborting"));

				Flash::ls();

				return RET_ERROR;
			}
			if(expected_bytes_written != found_bytes_written)
			{
				debug_println(F("Number of written bytes different than expected."));
				debug_print(F("Expected: "));
				debug_println(expected_bytes_written, DEC);
				debug_print(F("Found: "));
				debug_println(found_bytes_written, DEC);
				debug_println(F("Aborting"));

				Flash::ls();

				return RET_ERROR;
			}
		}
		debug_print(F("Finished creating dummy data. Created entries: "));
		debug_println(DATA_STORE_ELEMENTS_TO_WRITE, DEC);

		Flash::ls();

		//
		// Read data back with read and cross check with dummy_data_entries. Find 
		//
		Utils::serial_style(STYLE_BLUE);
		debug_println(F("# Reading back"));
		Utils::serial_style(STYLE_RESET);
		DataStoreReader<WaterSensorData::Entry> reader(&store);
		WaterSensorData::Entry *read_back = NULL;

		// Each entry will be searched in dummy_data_entries (by timestamp) and if found it will be marked here
		// At the end this array must be all true
		bool found_dummy_entries[DATA_STORE_ELEMENTS_TO_WRITE] = {false};

		// Entries that failed CRC check
		int failed_crc_entries = 0;

		while(reader.next_file())
		{
			while((read_back = reader.next_entry()))
			{
				// Check crc
				if(!reader.entry_crc_valid())
				{
					debug_println(F("Invalid CRC. Entry: "));
					WaterSensorData::print(read_back);
					failed_crc_entries++;
				}

				// Find entry in dummy_data_entries, check if same and mark as found
				for(int i = 0; i < DATA_STORE_ELEMENTS_TO_WRITE; i++)
				{
					// Find by timestamp (which is actually just an incrementing number)
					if(dummy_data_entries[i].timestamp == read_back->timestamp)
					{				
						// Check if identical (entry by entry in case padding is enabled)
						if(dummy_data_entries[i].conductivity == read_back->conductivity &&
							dummy_data_entries[i].dissolved_oxygen == read_back->dissolved_oxygen &&
							dummy_data_entries[i].ph == read_back->ph &&
							dummy_data_entries[i].temperature == read_back->temperature &&
							dummy_data_entries[i].water_level == read_back->water_level)
						{
							// Mark as found
							found_dummy_entries[i] = true;
							break;
						}
						else
						{
							debug_println(F("Found stored entry, but data different. Found: "));
							WaterSensorData::print(read_back);
							debug_println(F("Expected: "));
							WaterSensorData::print(&dummy_data_entries[i]);

							return RET_ERROR;
						}
					}
				}
			}
		}

		// Any entries failing CRC?
		if(failed_crc_entries > 0)
		{
			Utils::serial_style(STYLE_RED);
			debug_print(F("Entries failed CRC check: "));
			debug_println(failed_crc_entries, DEC);
			Utils::serial_style(STYLE_RESET);
			return RET_ERROR;
		}

		// Finished reading back all files, check all entries found
		int not_found_count = 0;
		for(int i = 0; i < DATA_STORE_ELEMENTS_TO_WRITE; i++)
		{
			if(!found_dummy_entries[i])
			{
				not_found_count++;
			}
		}

		if(not_found_count > 0)
		{
			Utils::serial_style(STYLE_RED);
			debug_print(F("Not all stored entries found when reading back. Not found entries: "));
			debug_println(not_found_count, DEC);
			Utils::serial_style(STYLE_RESET);
			return RET_ERROR;
		}

		//
		// Clean up
		//
		Utils::serial_style(STYLE_BLUE);
		debug_println(F("# Formatting for clean up."));
		Utils::serial_style(STYLE_RESET);
		if(!SPIFFS.format())
		{
			debug_println(F("# Format failed."));
			return RET_ERROR;
		}

		debug_println(F("Done!"));
		Flash::ls();

		return RET_OK;
	}

	/******************************************************************************
	 * Sleep
	 * Use dummy wakeup schedule and check if calc_next_wakeup returns correct vals
	 ******************************************************************************/
	RetResult wakeup_times()
	{
		const SleepScheduler::WakeupScheduleEntry ref_schedule[] = 
		{
			{ SleepScheduler::WakeupReason::REASON_READ_WATER_SENSORS, 2},
			{ SleepScheduler::WakeupReason::REASON_READ_WATER_SENSORS, 5},
			{ SleepScheduler::WakeupReason::REASON_CALL_HOME, 7},
			{ SleepScheduler::WakeupReason::REASON_READ_WATER_SENSORS, 10},
			{ SleepScheduler::WakeupReason::REASON_CALL_HOME, 40},
			{ SleepScheduler::WakeupReason::REASON_READ_WATER_SENSORS, 42},
			{ SleepScheduler::WakeupReason::REASON_CALL_HOME, 84},
		};
		const int ref_schedule_len = sizeof(ref_schedule) / sizeof(ref_schedule[0]);

		for(int i = 0; i < WAKEUP_TIMES_SERIES_LEN; i++)
		{
			// Check which is closer
			for(int j = 0; j < ref_schedule_len; j++)
			{

			}
		}

		// TODO: Not ready


		return RET_OK;
	}

	/******************************************************************************
	* Configuration store
	******************************************************************************/
	RetResult device_config()
	{
		Utils::serial_style(STYLE_RED);
		debug_println(F("This test will delete existing device config."));
		Utils::serial_style(STYLE_RESET);
		//
		// Erase current config entry in NVS
		//
		Utils::serial_style(STYLE_BLUE);
		debug_println(F("# Resetting"));
		Utils::serial_style(STYLE_RESET);
		Preferences prefs;
		if(!prefs.begin(DEVICE_CONFIG_NVS_NAMESPACE_NAME))
		{
			debug_println(F("Could not begin NVS."));
			return RET_ERROR;
		}

		if(!prefs.remove(DEVICE_CONFIG_NVS_NAMESPACE_NAME))
		{
			debug_println(F("Could not remove existing NVS key, probably doesn't exist."));
		}
		else
		{
			debug_println(F("Removed existing key."));
		}
		prefs.end();

		//
		// Init config store
		//
		Utils::serial_style(STYLE_BLUE);
		debug_println(F("# Initializing config store"));
		Utils::serial_style(STYLE_RESET);
		DeviceConfig::init();

		//
		// Write dummy data to store
		//
		Utils::serial_style(STYLE_BLUE);
		debug_println(F("# Writing dummy data to store."));
		Utils::serial_style(STYLE_RESET);

		// Create dummy vals
		DeviceConfig::Data dummy_config;
		dummy_config.clean_reboot = random(INT_MAX) % 2;
		dummy_config.ota_flashed = random(INT_MAX) % 2;


		// Do not include crc32 field in the calculation
		dummy_config.crc32 = 0;
		dummy_config.crc32 = Utils::crc32((uint8_t*)&dummy_config, sizeof(dummy_config));

		// Set
		DeviceConfig::set_clean_reboot(dummy_config.clean_reboot);
		DeviceConfig::set_ota_flashed(dummy_config.ota_flashed);

		// Write
		DeviceConfig::commit();

		DeviceConfig::print(&dummy_config);

		//
		// Read back and verify
		//
		Utils::serial_style(STYLE_BLUE);
		debug_println(F("# Reading back"));
		Utils::serial_style(STYLE_RESET);

		const DeviceConfig::Data *read_back = DeviceConfig::get();

		// Reading or CRC failed
		if(read_back == nullptr)
		{
			debug_println(F("Could not read back config."));
			return RET_ERROR;
		}

		DeviceConfig::print(read_back);

		// Clear to finish
		prefs.remove(DEVICE_CONFIG_NVS_NAMESPACE_NAME);

		debug_println(F("Done!"));

		return RET_OK;
	}

	/******************************************************************************
	 * Run all tests and print report
	******************************************************************************/    
	void run_all()
	{
		int test_count = sizeof(test_funcs) / sizeof(test_funcs[0]);

		// Build array of all tests
		// Test ids are assumed sequential from 0
		TestId tests[test_count];
		for(int i=0; i<test_count; i++)
		{
			tests[i] = (TestId)i;
		}

		run(tests, test_count);
	}

	/******************************************************************************
	* Run tests specified by the test array
	* @param tests Array of tests to run
	* @param count Array size
	******************************************************************************/ 
	void run(TestId tests[], int count)
	{
		if(count < 1)
			return;

		char block_title[] = "Running XXX tests";
		snprintf(block_title, sizeof(block_title), "Running %d tests", count);
		Utils::print_block(F(block_title));
		debug_println();

		const int test_count = sizeof(test_funcs) / sizeof(test_funcs[0]);

		// Test results
		// Make space for all tests, even those that arent going to be ran because
		// array index is test id
		TestResult results[test_count];
		memset(&results, IGNORED, sizeof(results));
		char name[100] = "";
		
		for(int i=0; i<count; i++)
		{
			TestId test_id = tests[i];

			debug_println();
			snprintf(name, sizeof(name), "%s (%d/%d)", test_names[test_id], i+1, count);
			Utils::print_separator(F(name));
			debug_println();

			if(test_funcs[test_id]() == RET_OK)
			{
				results[test_id] = SUCCESS;
			}
			else
			{
				results[test_id] = FAILURE;
			}
		}

		// Print results
		debug_println();
		Utils::print_separator(F("Results"));
		debug_println();

		bool overall_success = true;
		for(int i = 0; i < test_count; i++)
		{
			switch(results[i])
			{
			case SUCCESS:
				Utils::serial_style(STYLE_GREEN);
				debug_print(F("[SUCCESS] "));
				Utils::serial_style(STYLE_RESET);
				break;
			case FAILURE:
				Utils::serial_style(STYLE_RED);
				debug_print(F("[FAILURE] "));
				Utils::serial_style(STYLE_RESET);

				overall_success = false;
				break;
			case IGNORED:
				Utils::serial_style(STYLE_YELLOW);
				debug_print(F("[IGNORED] "));
				Utils::serial_style(STYLE_RESET);
				break;		
			default:
				debug_println(F("Invalid test result value."));
			}

			// Print name
			debug_println(test_names[i]);
		}

		if(overall_success)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("All tests passed!"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			Utils::serial_style(STYLE_RED);
			debug_println(F("Tests failed."));
			Utils::serial_style(STYLE_RESET);
		}
	}
} // Tests
