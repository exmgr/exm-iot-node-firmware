#include "remote_control.h"
#include "HardwareSerial.h"
#include "water_sensors.h"
#include "device_config.h"
#include "gsm.h"
#include "sleep_scheduler.h"
#include "http_request.h"
#include "globals.h"
#include <TinyGsmClient.h>
#include <Update.h>
#include "utils.h"
#include "ota.h"
#include "call_home.h"
#include "test_utils.h"
#include "fo_sniffer.h"
#include "rtc.h"
#include "common.h"

/******************************************************************************
 * Routines for controlling the device remotely through thingsboard.
 * Responsible for requesting remote control data from TB and then executing
 * the requested configs/actions.
 *****************************************************************************/
namespace RemoteControl
{
	//
	// Private funcs
	//
	RetResult json_to_data_struct(const JsonObject &json, RemoteControl::Data *data);

	RetResult handle_user_config(JsonObject json);
	RetResult handle_reboot(JsonObject json);
	RetResult handle_format_spiffs(JsonObject json);
	RetResult handle_rtc_sync(JsonObject json);
	void set_reboot_pending(bool val);

	void set_last_error(int error);

	/** 
	 * Set when reboot command is received in remote control data so that device can be 
	 * rebooted when calling home handling ends.
	 */
	bool _reboot_pending = false;

	/** Last error code */
	int _last_error = 0;

	/******************************************************************************
	 * Handle remote control
	 * First some of the current settings are published as client attributes to the
	 * device in TB.
	 * Then remote control data is requested (which are shared attributes) and applied
	 *****************************************************************************/
	RetResult start()
	{
		debug_println(F("Remote control handling."));

		set_last_error(ERROR_NONE);

		//
		// Send request
		//
		char url[URL_BUFFER_SIZE_LARGE] = "";

		Utils::tb_build_attributes_url_path(url, sizeof(url));

		HttpRequest http_req(GSM::get_modem(), TB_SERVER);
		http_req.set_port(TB_PORT);
				
		RetResult ret = RET_ERROR;

		debug_println(F("Getting TB shared attributes."));
		ret = http_req.get(url, g_resp_buffer, sizeof(g_resp_buffer));

		if(ret != RET_OK)
		{
			debug_println(F("Could not send request for remote control data."));
			
			Log::log(Log::RC_REQUEST_FAILED, http_req.get_response_code());

			set_last_error(ERROR_REQUEST_FAILED);

			return RET_ERROR;
		}

		debug_println(F("Remote control raw data:"));
		debug_println(F(g_resp_buffer));

		//
		// Deserialize received data
		//
		StaticJsonDocument<REMOTE_CONTROL_JSON_DOC_SIZE> json_remote;
		DeserializationError error = deserializeJson(json_remote, g_resp_buffer, strlen(g_resp_buffer));

		// Could not deserialize
		if(error)
		{
			Utils::serial_style(STYLE_RED);
			debug_println(F("Could not deserialize received JSON, aborting."));
			Utils::serial_style(STYLE_RESET);
			
			Log::log(Log::RC_PARSE_FAILED);

			return RET_ERROR;
		}

		debug_println(F("Remote control JSON: "));
		serializeJsonPretty(json_remote, Serial);
		debug_println();

		// Get shared attributes key
		if(!json_remote.containsKey("shared"))
		{
			debug_println(F("Returned JSON has no remote attributes."));
			Log::log(Log::RC_INVALID_FORMAT);
			
			return RET_ERROR;
		}
		JsonObject json_shared = json_remote["shared"];

		// At least id key must be present
		if(!json_shared.containsKey(RC_TB_KEY_REMOTE_CONTROL_DATA_ID))
		{
			debug_println(F("Returned JSON has no data id."));
			Log::log(Log::RC_INVALID_FORMAT);

			return RET_ERROR;
		}

		//
		// Check if ID is new
		// If remote control ID is old, remote control data is ignored
		// ID is new when it is different than the one stored from the previous remote control
		long long new_data_id = (int)json_shared[RC_TB_KEY_REMOTE_CONTROL_DATA_ID];

		debug_print(F("Remote control data id: Current "));
		debug_print(DeviceConfig::get_last_rc_data_id());
		debug_print(F(" - Received "));
		debug_println((int)new_data_id);

		// Is it new?
		if(new_data_id == DeviceConfig::get_last_rc_data_id())
		{
			Utils::serial_style(STYLE_RED);
			debug_println(F("Received remote control data is old, ignoring."));
			Utils::serial_style(STYLE_RESET);	
			return RET_OK;
		}
		else
		{
			Log::log(Log::RC_APPLYING_NEW_DATA, new_data_id, DeviceConfig::get_last_rc_data_id());

			// Data id is new, update in config to avoid rc data from being applied every time
			DeviceConfig::set_last_rc_data_id(new_data_id);
			DeviceConfig::commit();
		}

		Utils::serial_style(STYLE_BLUE);
		debug_println(F("Received new remote control data. Applying..."));
		Utils::serial_style(STYLE_RESET);

		//
		// Handle/apply
		//

		// Handle user config
		RemoteControl::handle_user_config(json_shared);

		// Handle SPIFFS format
		RemoteControl::handle_format_spiffs(json_shared);

		// Handle RTC sync
		RemoteControl::handle_rtc_sync(json_shared);

		// Handle OTA if OTA requested
		if(json_shared.containsKey(RC_TB_KEY_DO_OTA) && ((bool)json_shared[RC_TB_KEY_DO_OTA]) == true)
		{
			OTA::handle_rc_data(json_shared);

			TestUtils::print_stack_size();
			
			// Send logs to report OTA events
			CallHome::handle_logs();
		}

		// Handle reboot
		RemoteControl::handle_reboot(json_shared);

		return RET_OK;
	}

	/******************************************************************************
	 * User config
	 *****************************************************************************/
	RetResult handle_user_config(JsonObject json)
	{
		debug_println(F("Applying new user config."));

		//
		// Water sensors measure interval
		//
		if(json.containsKey(RC_TB_KEY_MEASURE_WATER_SENSORS_INT))
		{
			int was_int = (int)json[RC_TB_KEY_MEASURE_WATER_SENSORS_INT];

			debug_print(F("Water sensors read interval: "));
			debug_print(F("Current "));
			debug_print(DeviceConfig::get_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_READ_WATER_SENSORS));
			debug_print(F(" - New "));
			debug_print(was_int);
			debug_println(F(". "));

			// Check if value valid. 
			if(Utils::in_array(was_int, WAKEUP_SCHEDULE_VALID_VALUES, sizeof(WAKEUP_SCHEDULE_VALID_VALUES)) == -1)
			{
				debug_println(F("Invalid value, ignoring."));
				Log::log(Log::RC_WATER_SENSORS_READ_INT_SET_FAILED, was_int);
			}
			else
			{
				DeviceConfig::set_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_READ_WATER_SENSORS, was_int);
				debug_println(F("Applied."));

				Log::log(Log::RC_WATER_SENSORS_READ_INT_SET_SUCCESS, was_int);
			}			
		}

		//
		// Weather station measure interval
		//
		if(json.containsKey(RC_TB_KEY_MEASURE_WEATHER_STATION_INT))
		{
			int wes_int = (int)json[RC_TB_KEY_MEASURE_WEATHER_STATION_INT];

			debug_print(F("Weather station read interval: "));
			debug_print(F("Current "));
			debug_print(DeviceConfig::get_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_READ_WEATHER_STATION));
			debug_print(F(" - New "));
			debug_print(wes_int);
			debug_println(F(". "));

			// Check if value valid. 
			if(Utils::in_array(wes_int, WAKEUP_SCHEDULE_VALID_VALUES, sizeof(WAKEUP_SCHEDULE_VALID_VALUES)) == -1)
			{
				debug_println(F("Invalid value, ignoring."));
				Log::log(Log::RC_WEATHER_STATION_READ_INT_SET_FAILED, wes_int);
			}
			else
			{
				DeviceConfig::set_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_READ_WEATHER_STATION, wes_int);
				debug_println(F("Applied."));

				Log::log(Log::RC_WEATHER_STATION_READ_INT_SET_SUCCESS, wes_int);
			}			
		}

		//
		// Soil moisture measure interval
		//
		if(json.containsKey(RC_TB_KEY_MEASURE_SOIL_MOISTURE_SENSORS_INT))
		{
			int sm_int = (int)json[RC_TB_KEY_MEASURE_SOIL_MOISTURE_SENSORS_INT];

			debug_print(F("Soil moisture read interval: "));
			debug_print(F("Current "));
			debug_print(DeviceConfig::get_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_READ_SOIL_MOISTURE_SENSOR));
			debug_print(F(" - New "));
			debug_print(sm_int);
			debug_println(F(". "));

			// Check if value valid. 
			if(Utils::in_array(sm_int, WAKEUP_SCHEDULE_VALID_VALUES, sizeof(WAKEUP_SCHEDULE_VALID_VALUES)) == -1)
			{
				debug_println(F("Invalid value, ignoring."));
				Log::log(Log::RC_SOIL_MOISTURE_READ_INT_SET_FAILED, sm_int);
			}
			else
			{
				DeviceConfig::set_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_READ_SOIL_MOISTURE_SENSOR, sm_int);
				debug_println(F("Applied."));

				Log::log(Log::RC_SOIL_MOISTURE_READ_INT_SET_SUCCESS, sm_int);
			}			
		}

		//
		// Call home interval
		//
		if(json.containsKey(RC_TB_KEY_CALL_HOME_INT))
		{
			int ch_int = (int)json[RC_TB_KEY_CALL_HOME_INT];

			debug_print(F("Call home interval: "));
			debug_print(F("Current "));
			debug_print(DeviceConfig::get_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_CALL_HOME));
			debug_print(F(" - New "));
			debug_println(ch_int);

			// Check if value valid. Call Home int cannot be 0
			if(Utils::in_array(ch_int, WAKEUP_SCHEDULE_VALID_VALUES, sizeof(WAKEUP_SCHEDULE_VALID_VALUES)) == -1 ||
				ch_int == 0) 
			{
				debug_println(F("Invalid value, ignoring."));
				Log::log(Log::RC_CALL_HOME_INT_SET_FAILED, ch_int);
			}
			else
			{
				DeviceConfig::set_wakeup_schedule_reason_int(SleepScheduler::WakeupReason::REASON_CALL_HOME, ch_int);
				debug_println(F("Applied."));

				Log::log(Log::RC_CALL_HOME_INT_SET_SUCCESS, ch_int);
			}			
		}

		// FO scan
		if(json.containsKey(RC_TB_KEY_DO_FO_SCAN) && ((bool)json[RC_TB_KEY_DO_FO_SCAN]) == true)
		{
			debug_println(F("Scanning for FO weather station"));

			FoSniffer::scan_fo_id(true);
		}

		// FO weather station enab;ed
		if(json.containsKey(RC_TB_KEY_FO_ENABLED))
		{
			debug_println(F("FO weather station status"));

			bool prev_val = DeviceConfig::get_fo_enabled();

			DeviceConfig::set_fo_enabled((bool)json[RC_TB_KEY_FO_ENABLED]);
			DeviceConfig::commit();

			Log::log(Log::FO_ENABLED_STATUS, prev_val, DeviceConfig::get_fo_enabled());

			debug_print(F("Previous value: "));
			debug_print(prev_val, DEC);
			debug_print(F(" - New value value: "));
			debug_print(DeviceConfig::get_fo_enabled(), DEC);
			debug_println();
		}

		DeviceConfig::print_current();

		// Commit all changes
		DeviceConfig::commit();

		return RET_OK;
	}

	/******************************************************************************
	 * Mark device pending flag so device reboots when calling home handling ends
	 *****************************************************************************/
	RetResult handle_reboot(JsonObject json)	
	{	
		if(json.containsKey(RC_TB_KEY_DO_REBOOT))
		{
			bool do_reboot = (bool)json[RC_TB_KEY_DO_REBOOT];
			if(do_reboot)
			{
				Utils::serial_style(STYLE_BLUE);
				debug_println(F("Reboot requested. Device will be rebooted when remote control handling ends."));
				Utils::serial_style(STYLE_RESET);

				// Mark as reboot pending and the device will be rebooted after all processes finish
				set_reboot_pending(true);
			}
		}

		return RET_OK;
	}

	/******************************************************************************
	 * Format SPIFFS
	 *****************************************************************************/
	RetResult handle_format_spiffs(JsonObject json)	
	{	
		if(json.containsKey(RC_TB_KEY_DO_FORMAT_SPIFFS) && ((bool)json[RC_TB_KEY_DO_FORMAT_SPIFFS]) == true)
		{
			debug_println(F("Formatting SPIFFS"));

			// TODO: Submit logs before formatting SPIFFS?

			int bytes_before_format = SPIFFS.usedBytes();

			if(SPIFFS.format())
			{
				debug_println(F("Format complete"));

				Log::log(Log::SPIFFS_FORMATTED, bytes_before_format);
			}
			else
			{
				debug_println(F("Format failed!"));

				// Try to log in case format failed but FS still accessible
				Log::log(Log::SPIFFS_FORMAT_FAILED);

				return RET_ERROR;
			}
		}

		return RET_OK;
	}

	/******************************************************************************
	 * Sync RTC
	 *****************************************************************************/
	RetResult handle_rtc_sync(JsonObject json)	
	{	
		if(json.containsKey(RC_TB_KEY_DO_RTC_SYNC) && ((bool)json[RC_TB_KEY_DO_RTC_SYNC]) == true)
		{
			//
			// Sync RTC
			//
			if(RTC::sync() != RET_OK) // RTC turns GSM ON
			{
				Utils::serial_style(STYLE_RED);
				debug_println(F("Failed to sync time, system has no source of time."));
				Utils::serial_style(STYLE_RESET);

				return RET_ERROR;
			}
			else
			{
				debug_println(F("Time sync successful."));
				RTC::print_time();
			}
		}

		return RET_OK;
	}

	/******************************************************************************
	* Get/set reboot pending flag
	* When a reboot is required after calling home handling finishes (eg. to apply
	* some of the required settings), it must set this flag to make CallingHome
	* reboot the device when its done.
	******************************************************************************/
	bool get_reboot_pending()
	{
		return _reboot_pending;
	}
	void set_reboot_pending(bool val)
	{
		_reboot_pending = val;
	}

	/******************************************************************************
	* Print data
	******************************************************************************/
	void print(const Data *data)
	{
		debug_print(F("ID: "));
		debug_println(data->id, DEC);

		debug_print(F("Water sensors measure int (mins): "));
		debug_println(data->water_sensors_measure_int_mins, DEC);

		// debug_print(F("Soil moisture sensor measure int (mins): "));
		// debug_println(data->water_sensors_measure_int_mins, DEC);

		debug_print(F("Call home int(mins): "));
		debug_println(data->call_home_int_mins, DEC);

		debug_print(F("Reboot: "));
		debug_println(data->reboot, DEC);

		debug_print(F("OTA: "));
		debug_println(data->ota, DEC);
	}

	/******************************************************************************
	 * Get last error code
	 *****************************************************************************/
	int get_last_error()
	{
		return _last_error;
	}

	/******************************************************************************
	 * Set last error code
	 *****************************************************************************/
	void set_last_error(int error)
	{
		_last_error = error;
	}
}