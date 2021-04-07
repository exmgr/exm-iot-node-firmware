#include "device_config.h"
#include "common.h"
#include "app_config.h"

/******************************************************************************
* Config uses the "Preferences" API (which has own partition in flash)
* to store all values that need to persist between reboots. SPIFFS is not used
* because config must not be lost in case SPIFFS gets corrupted.
*
* All configurable device data is stored in Config.
* Default data is used when no config set yet (first run) and when loading 
* data from Preferences api fails.
******************************************************************************/
namespace DeviceConfig
{
	//
	// Private vars
	//
	/** Default config used when no config set (first boot) or config is corrupted */
	const DeviceConfig::Data DEVICE_CONFIG_DEFAULT = 
	{
		crc32: 0,
		clean_reboot: false,
		wakeup_schedule: {},

		ota_flashed: false,

		last_rc_data_id: 0,

		tb_device_token: {'\0'},

		cellular_apn: {'\0'},

		fo_sniffer_id: 0,

		fo_enabled: false,

		/** Last received/
		last_remote_control_data: RemoteControl::Data(0, 0, 0, 0) */
	};
	
	/** Currently loaded config. Struct is kept up to date every time new config is set */
	Data _current_config = DEVICE_CONFIG_DEFAULT;

	/** Preferences api store */
	Preferences _prefs;

	//
	// Private functions
	//
	RetResult begin();
	RetResult end();
	RetResult load();
	RetResult write_defaults();

	/******************************************************************************
	* Initialization
	* Must be run once on boot to 
	******************************************************************************/
	RetResult init()
	{
		// Try to load config. If failed, we can only assume that key has not been created
		// yet (first run on new device) since reads fail the same way whether the key
		// doesn't exist or for other reason (no way to tell apart)
		if(load() != RET_OK)
		{
			debug_println(F("Config store key not set "));

			write_defaults();

			// Does load still fail? If yes, the key not existing wasn't the problem.
			if(load() != RET_OK)
			{
				// Load failed but the defaults have already been written to the
				// current config struct and will be used.
				debug_println_e(F("Could not load config. Init failed. Defaults will be used."));
				Log::log(Log::USING_DEFAULT_DEVICE_CONFIG);
				return RET_ERROR;
			}
		}

		return RET_OK;
	}

	/******************************************************************************
	* Begin NVS. Must be called before every read/write procedure
	******************************************************************************/
	RetResult begin()
	{
		// Close any pending handles before beginning new
		end();
		
		if(!_prefs.begin(DEVICE_CONFIG_NVS_NAMESPACE_NAME))
		{
			end();
			debug_println(F("Could not begin config NVS store."));
			return RET_ERROR;
		}
		
		return RET_OK;
	}

	/******************************************************************************
	* End NVS. Must be called after every read/write procedure.
	******************************************************************************/
	RetResult end()
	{
		_prefs.end();
	}

	/******************************************************************************
	* Get currently loaded config.
	******************************************************************************/
	const Data* get()
	{
		return &_current_config;
	}

	/******************************************************************************
	* Write config to memory. Must be run every time config is changed to make it
	* persistent.
	******************************************************************************/
	RetResult commit()
	{
		RetResult ret = RET_ERROR;

		if(begin() == RET_ERROR)
		{
			return RET_ERROR;
		}

		// Update CRC32 before writing
		// Calc CRC32 without the crc32 field
		_current_config.crc32 = 0;
		_current_config.crc32 = Utils::crc32((uint8_t*)&_current_config, sizeof(_current_config));

		// Write
		int bytes_written = _prefs.putBytes(DEVICE_CONFIG_NVS_NAMESPACE_NAME, &_current_config, sizeof(_current_config));
		if(bytes_written != sizeof(_current_config))
		{
			debug_print(F("Could not write config to NVS. Bytes written: "));
			debug_println(bytes_written, DEC);
			debug_print(F("Expected: "));
			debug_println(sizeof(_current_config), DEC);

			ret = RET_ERROR;
		}
		else
		{
			ret = RET_OK;
		}

		end();

		return ret;
	}

	/******************************************************************************
	* Load from flash
	******************************************************************************/
	RetResult load()
	{
		if(begin() != RET_OK)
			return RET_ERROR;

		// Load to temp struct first in case it fails
		Data loaded_config;
		RetResult ret = RET_ERROR;

		int read_bytes = _prefs.getBytes(DEVICE_CONFIG_NVS_NAMESPACE_NAME, &loaded_config, sizeof(loaded_config));

		// Read successful
		if(read_bytes == sizeof(loaded_config))
		{
			// Check crc. CRC is checked with the CRC field = 0, so set to 0 but keep backup first
			uint32_t crc32_bkp = loaded_config.crc32;
			loaded_config.crc32 = 0;

			// CRC check failed. Log event and abort.
			if(crc32_bkp != Utils::crc32((uint8_t*)&loaded_config, sizeof(loaded_config)))
			{
				debug_println(F("Config failed CRC32 check. Loading aborted."));

				Log::log(Log::DEVICE_CONFIG_DATA_CRC_ERRORS);

				ret = RET_ERROR;
			}
			// All OK. Copy loaded config to current config
			else
			{
				// Restore crc32
				loaded_config.crc32 = crc32_bkp;

				memcpy(&_current_config, &loaded_config, sizeof(_current_config));

				ret = RET_OK;
			}
		}
		// Bytes read, but invalid count.
		else
		{
			debug_println(F("Could not load Config from flash. Invalid byte count."));
			debug_print(F("Bytes read: "));
			debug_println(read_bytes, DEC);
			debug_print(F("Expected: "));
			debug_println(sizeof(loaded_config), DEC);
			ret = RET_ERROR;
		}

		end();

		return ret;
	}

	/******************************************************************************
	* Write default config structure to NVS. Runs when NVS cannot be started 
	* because key doesn't exist yet (first boot of device)
	******************************************************************************/
	RetResult write_defaults()
	{
		debug_println(F("Writing default config..."));

		// Copy defaults struct to current
		memcpy(&_current_config, &DEVICE_CONFIG_DEFAULT, sizeof(DEVICE_CONFIG_DEFAULT));
		// Copy default wake up schedule to current
		set_wakeup_schedule((SleepScheduler::WakeupScheduleEntry*) &WAKEUP_SCHEDULE_DEFAULT);

		commit();

		return RET_OK;
	}

	/******************************************************************************
	* Print struct to serial output
	******************************************************************************/
	void print(const Data *data)
	{
		Utils::print_separator(F("DEVICE CONFIG"));
	
		debug_print(F("CRC32: "));
		debug_println(data->crc32, DEC);

		debug_print(F("Clean reboot: "));
		debug_println(data->clean_reboot, DEC);

		debug_print(F("OTA flashed: "));
		debug_println(data->ota_flashed, DEC);

		debug_print(F("Last remote config data id: "));
		debug_println(data->last_rc_data_id);

		debug_print(F("Water sensors measure interval (mins): "));
		debug_println(get_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_READ_WATER_SENSORS), DEC);

		debug_print(F("Weather station measure interval (mins): "));
		debug_println(get_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_READ_WEATHER_STATION), DEC);

		debug_print(F("Calling home interval (mins): "));
		debug_println(get_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_CALL_HOME), DEC);

		debug_print(F("TB device token: "));
		debug_println(data->tb_device_token);

		debug_print(F("FO weather station: "));
		if(data->fo_enabled)
		{
			debug_println("Enabled");
		}
		else
		{
			debug_println("Disabled");
		}
			
		debug_print(F("FO Sniffer ID: "));
		debug_println(data->fo_sniffer_id, HEX);


		Utils::print_separator(NULL);
	}

	/******************************************************************************
	 * Print current config struct
	 *****************************************************************************/
	void print_current()
	{
		print(&_current_config);
	}

	/******************************************************************************
	* Basic accessors
	******************************************************************************/
	RetResult set_clean_reboot(bool val)
	{
		_current_config.clean_reboot = val;

		return RET_OK;
	}
	bool get_clean_reboot()
	{
		return _current_config.clean_reboot;
	}

	RetResult set_ota_flashed(bool val)
	{
		_current_config.ota_flashed = val;
		
		return RET_OK;
	}
	bool get_ota_flashed()
	{
		return _current_config.ota_flashed;
	}

	RetResult set_wakeup_schedule(const SleepScheduler::WakeupScheduleEntry *schedule)
	{
		memcpy(_current_config.wakeup_schedule, schedule, sizeof(_current_config.wakeup_schedule));
	}

	/******************************************************************************
	* Find a reason in the sleep schedule and update its interval
	******************************************************************************/
	RetResult set_wakeup_schedule_reason_int(SleepScheduler::WakeupReason reason, int wakeup_int)
	{
		// TODO: Check for min/max values should be done here instead only when received by remote config?
		for(int i = 0; i < WAKEUP_SCHEDULE_LEN; i++)
		{
			if(_current_config.wakeup_schedule[i].reason == reason)
			{
				_current_config.wakeup_schedule[i].wakeup_int = wakeup_int;
				return RET_OK;
			}
		}

		// Not found

		return RET_ERROR;
	}

	/******************************************************************************
	* Find a reason in the sleep schedule return its interval
	******************************************************************************/
	int get_wakeup_schedule_reason_int(SleepScheduler::WakeupReason reason)
	{
		for(int i = 0; i < WAKEUP_SCHEDULE_LEN; i++)
		{
			if(_current_config.wakeup_schedule[i].reason == reason)
			{
				return _current_config.wakeup_schedule[i].wakeup_int;
			}
		}

		// Not found
		return -1;
	}

	/******************************************************************************
	 * Get wakeup schedule
	 *****************************************************************************/
	RetResult get_wakeup_schedule(SleepScheduler::WakeupScheduleEntry *schedule)
	{
		memcpy(schedule, &_current_config, sizeof(WAKEUP_SCHEDULE_DEFAULT));
	}

	/******************************************************************************
	 * Last remote config data id
	 *****************************************************************************/
	int get_last_rc_data_id()
	{
		return _current_config.last_rc_data_id;
	}

	RetResult set_last_rc_data_id(int id)
	{
		_current_config.last_rc_data_id = id;

		return RET_OK;
	}

	/******************************************************************************
	* Get TB device token
	******************************************************************************/
	const char* get_tb_device_token()
	{
		return _current_config.tb_device_token;
	}

	/******************************************************************************
	* Set TB device token
	******************************************************************************/
	RetResult set_tb_device_token(char *token)
	{
		strncpy(_current_config.tb_device_token, token, sizeof(_current_config.tb_device_token));
	}

	/******************************************************************************
	* Set cellular APN
	******************************************************************************/
	const char* get_cellular_apn()
	{
		return _current_config.cellular_apn;
	}

	/******************************************************************************
	* Get cellular APN
	******************************************************************************/
	RetResult set_cellular_apn(char *apn)
	{
		strncpy(_current_config.cellular_apn, apn, sizeof(_current_config.cellular_apn));
	}

	/******************************************************************************
	* Set FO sniffer weather station ID
	******************************************************************************/
	const uint8_t get_fo_sniffer_id()
	{
		return _current_config.fo_sniffer_id;
	}

	/******************************************************************************
	* Get FO sniffer weather station ID
	******************************************************************************/
	RetResult set_fo_sniffer_id(uint8_t id)
	{
		_current_config.fo_sniffer_id = id;
	}

	/******************************************************************************
	* Get FO weather station enabled status
	******************************************************************************/
	const bool get_fo_enabled()
	{
		return _current_config.fo_enabled;
	}

	/******************************************************************************
	* Get FO sniffer weather station ID
	******************************************************************************/
	RetResult set_fo_enabled(bool enabled)
	{
		_current_config.fo_enabled = enabled;
	}
}