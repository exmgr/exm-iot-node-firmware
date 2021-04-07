#include <HardwareSerial.h>
#include <esp_sleep.h>
#include "sleep_scheduler.h"
#include "common.h"
#include "const.h"
#include "app_config.h"
#include "struct.h"
#include "log.h"
#include "device_config.h"
#include "battery.h"
#include "rtc.h"
#include "fo_sniffer.h"
#include "fo_uart.h"
#include "fo_data.h"

namespace SleepScheduler
{
	//
	// Private vars
	//

	/** Timestamp of last time device went to sleep */
	uint32_t _t_last_sleep = 0;

	/** Millis of last time device woke up from sleep */
	uint32_t _t_last_wakeup_ms = 0;

	/** Millis of last time events were set to be handled
	 *  Used to calculate missed events.
	 */
	uint32_t _t_last_event_ms = 0;

	/** Reasons of last wake up event */
	int _last_wakeup_reasons = 0;

	//
	// Private functions
	//
	RetResult get_current_schedule(SleepScheduler::WakeupScheduleEntry *schedule_out);
	RetResult decide_schedule(SleepScheduler::WakeupScheduleEntry schedule_out[]);
	int calc_secs_to_event(uint32_t t_now_sec, int event_interval_secs);

	/******************************************************************************
	* Decide minutes to sleep and go to sleep
	******************************************************************************/
	RetResult sleep_to_next()
	{
		// Sleep time will be calculated using this timestamp as a reference
		uint32_t t_now_sec = RTC::get_timestamp();

		// Get current schedule
		WakeupScheduleEntry schedule[WAKEUP_SCHEDULE_LEN];
		decide_schedule(schedule);

		// Print info
		Battery::print_mode();
		print_schedule(schedule);

		//
		// If call home event missed because of another event that took too long to finish,
		// set as event and return to execute immediately
		//
		int awake_ms = _t_last_event_ms == 0 ? 0 : (millis() - _t_last_event_ms);
		int awake_sec = awake_ms / 1000;

		int missed_reasons = check_missed_reasons(schedule, t_now_sec - awake_sec, awake_sec);
		missed_reasons = missed_reasons & ~_last_wakeup_reasons;

		if(missed_reasons != 0)
		{
			debug_println_e("=====================================================");
			debug_print_i(F("Missed events found, handling immediately: "));
			debug_println_i(missed_reasons, DEC);
			debug_println_e("=====================================================");

			_last_wakeup_reasons = missed_reasons;
			_t_last_event_ms = millis();

			Log::log(Log::WAKEUP_EVENTS_MISSED, missed_reasons);

			// Return to handle
			return RET_OK;
		}

		//
		// Calc next wake up for all events except FO sniffer
		//

		// Keep var before its changed, to use it in log later
		int prev_last_wakeup_reasons = _last_wakeup_reasons;

		// Output var
		int next_event_seconds_left = 0;
		RetResult ret = calc_next_wakeup(t_now_sec, schedule, &next_event_seconds_left, &_last_wakeup_reasons);

		if(ret != RET_OK)
		{
			Utils::serial_style(STYLE_RED);
			debug_println(F("Could not calculate wake up time!"));
			Utils::serial_style(STYLE_RESET);
		}	

		RTC::print_time();

		debug_printf("Time awake: %d ms (%d s)\n", awake_ms, awake_sec);

		if(FLAGS.SLEEP_MINS_AS_SECS)
		{
			Utils::serial_style(STYLE_YELLOW);
			debug_println(F("Treating minutes as seconds (flag enabled)"));
			Utils::serial_style(STYLE_RESET);
		}

		debug_print(F("Calculated sleep with t: "));
		debug_println(t_now_sec, DEC);

		debug_print(F("Next wake up reasons: "));
		SleepScheduler::print_wakeup_reasons(_last_wakeup_reasons);

		Utils::serial_style(STYLE_YELLOW);
		debug_printf("Sleeping for %d seconds (%.1f min)\n", next_event_seconds_left, (float)next_event_seconds_left / 60);
		Utils::serial_style(STYLE_RESET);

		// If going to sleep from FO wake up only (no other reasons), do not log
		if(prev_last_wakeup_reasons != SleepScheduler::REASON_FO)
			Log::log(Log::SLEEP, awake_sec, next_event_seconds_left);

		// Force max sleep time (fail safe)
		if(next_event_seconds_left > MAX_SLEEP_TIME_SEC)
		{
			next_event_seconds_left = MAX_SLEEP_TIME_SEC;
		}

		//
		// Sleep
		//
		_t_last_sleep = RTC::get_timestamp();	

		Serial.flush();
		
		esp_sleep_enable_timer_wakeup((uint64_t)next_event_seconds_left * 1000000);
		esp_light_sleep_start();

		//
		// Wake up
		//

		//
		// ESP32 internal clock drifts, calculate how much time left for actual wakeup time and sleep again
		//
		// How much time we slept?
		// How much time were we supposed to sleep?
		// supposed - slept = more sleep time
		// Calculated using timestamp from external RTC
		if(FLAGS.EXTERNAL_RTC_ENABLED)
		{
			int t_wakeup = RTC::get_external_rtc_timestamp();

			if(RTC::tstamp_valid(_t_last_sleep) && RTC::tstamp_valid(t_wakeup))
			{
				int underslept_secs = next_event_seconds_left - (t_wakeup - _t_last_sleep);
				if(underslept_secs > 0 && underslept_secs <= MAX_SLEEP_CORRECTION_SEC)
				{
					debug_print(F("Slept at: "));
					debug_println(_t_last_sleep, DEC);
					debug_print(F("Woke up at: "));
					debug_println(t_wakeup, DEC);

					debug_print(F("Correcting sleep. Sleeping for (sec): "));
					debug_println(underslept_secs, DEC);

					Log::log(Log::WAKEUP_CORRECTION, underslept_secs);

					esp_sleep_enable_timer_wakeup((uint64_t)underslept_secs * 1000000);
					esp_light_sleep_start();
				}
				else
				{
					debug_print(F("Cannot correct sleep, invalid value (sec): "));
					debug_println(underslept_secs, DEC);
				}
			}
			else
			{
				debug_println(F("Cannot correct sleep time, ext RTC time invalid."));
			}
		}

		// Keep track of wake up time
		_t_last_wakeup_ms = millis();

		// Keep track of last time events were calculated and function returned to let them be handled
		_t_last_event_ms = millis();

		// ESP32 RTC drifts, sync internal RTC from external on every wake up
		RTC::sync_time_from_ext_rtc();

		// If woke up for FO only (no other reasons), do not log wake up event, increase wakeup counter instead
		if(_last_wakeup_reasons == SleepScheduler::REASON_FO)
		{
			FoData::inc_wakeup_count();
		}
		else
			Log::log(Log::WAKEUP, _last_wakeup_reasons);

		Utils::serial_style(STYLE_YELLOW);
		Utils::print_block(F("Waking up!"));
		Utils::serial_style(STYLE_RESET);

		return RET_OK;
	}

	/******************************************************************************
	* Calculate minutes left until next wake up event in the schedule, if current
	* time is t. Outputs seconds left to nearest wakeup and events occurring at that
	* time
	* @param t_now_sec  Current time in seconds
	* @schedule         Schedule to use for calculation
	* @schedule_len     Size of wake up schedule array
	* @seconds_left		Seconds to next event (output var)
	* @event_reasons	Reasons of next event (output var)
	******************************************************************************/
	RetResult calc_next_wakeup(uint32_t t_now_sec, const WakeupScheduleEntry schedule[], int *seconds_left, int *event_reasons)
	{
		//
		// Iterate schedule and find the closest reason to current time
		//
		// Seconds to nearest event
		int min_seconds_to_event = 0;
		// Nearest event interval seconds
		int min_interval_secs = 0;

		// All reasons occurring at that time
		int reasons = 0;

		for(int i = 0; i < WAKEUP_SCHEDULE_LEN; i++)
		{
			WakeupScheduleEntry entry = schedule[i];

			int wakeup_int = entry.wakeup_int;

			// Ignore events where interval is 0
			if(wakeup_int == 0)
			{
				//printf("Event %d ignored because frequency is out of range: %d\n", entry.reason, wakeup_int);
				continue;
			}

			// Calculate interval between wakeups for given rate
			int interval_secs = wakeup_int * 60;

			// Treat minutes as seconds when flag is enabled for faster debugging
			if(FLAGS.SLEEP_MINS_AS_SECS)
			{
				interval_secs = wakeup_int;
			}

			// Seconds left to event
			int seconds_to_event = calc_secs_to_event(t_now_sec, interval_secs);

			// Failed, ignore
			if(seconds_to_event < 1)
			{
				continue;
			}

			// New minimum found
			if(seconds_to_event < min_seconds_to_event || min_seconds_to_event == 0)
			{
				min_seconds_to_event = seconds_to_event;
				reasons = entry.reason;
			}
			// Event has the same wakeup time as the one(s) already found, add to reasons
			// (events occurring athe same time)
			else if(seconds_to_event == min_seconds_to_event)
			{
				reasons |= entry.reason;
			}
		}

		// debug_printf("Calculated - Time left: %d - Next events: %d\n", min_seconds_to_event , reasons);

		//
		// Calculate wakeup for next FO sniff and add to reasons
		//
		if(DeviceConfig::get_fo_enabled())
		{
			int secs_to_next_sniff = 0;

			if(FO_SOURCE == FO_SOURCE_SNIFFER)
			{
				secs_to_next_sniff = FoSniffer::calc_secs_to_next_sniff();
			}
			else if(FO_SOURCE == FO_SOURCE_UART)
			{
				secs_to_next_sniff = FoUart::calc_secs_to_next_packet();	
			}

			Serial.print(F("Secs to next sniff: "));
			Serial.println(secs_to_next_sniff, DEC);

			if(secs_to_next_sniff > 0)
			{
				// If sniff event is closer than calculated events, set it as next event instead
				if(secs_to_next_sniff < min_seconds_to_event)
				{
					reasons = REASON_FO;
					min_seconds_to_event = secs_to_next_sniff;
				}
				// If sniff event is scheduled to happen with calculated events, include it with next events
				else if(secs_to_next_sniff == min_seconds_to_event)
				{
					reasons |= REASON_FO;
				}
			}
		}
		///

		// No valid event(s) could be calculated, fail
		if(min_seconds_to_event == 0 || reasons == 0)
			return RET_ERROR;
		else
		{
			// Update output vars
			*seconds_left = min_seconds_to_event;
			*event_reasons = reasons;
		}

		return RET_OK;
	}

	/******************************************************************************
	* Calculate seconds left to event from t_now_sec
	******************************************************************************/
	int calc_secs_to_event(uint32_t t_now_sec, int event_interval_secs)
	{
		const int SECONDS_IN_DAY = 86400;

		if(event_interval_secs < 1)
		{
			return -1;
		}

		// Current second from the start of this day
		int cur_sec_in_day = t_now_sec - ((t_now_sec / SECONDS_IN_DAY) * SECONDS_IN_DAY);

		int seconds_to_event = 0;

		// No more events in this day for this reason
		// Next event at the start of next hour
		if(cur_sec_in_day >= SECONDS_IN_DAY - event_interval_secs)
		{
			seconds_to_event = SECONDS_IN_DAY - cur_sec_in_day;
		}
		else
		{
			// Calculate next wakeup for this reason
			seconds_to_event = ((cur_sec_in_day / event_interval_secs + 1) * event_interval_secs) - cur_sec_in_day;
		}

		return seconds_to_event;
	}

	/******************************************************************************
	 * Checks if any events were missed if device woke up at t_since and stayed
	 * awake for awake_sec.
	 * Checks only for the next wakeup events after t_since, not all subsequent ones
	 * @param schedule		Schedule to use to find out the next events after t_since
	 * @param t_since 		Start time from which to calculate next event
	 * @param awake_sec	Seconds the device was awake starting from t_since
	 *****************************************************************************/
	int check_missed_reasons(const WakeupScheduleEntry schedule[], uint32_t t_since, uint32_t awake_sec)
	{
		int reasons = 0;

		for(int i = 0; i < WAKEUP_SCHEDULE_LEN; i++)
		{
			WakeupScheduleEntry entry = schedule[i];

			int wakeup_int = entry.wakeup_int;

			int secs_to_event = calc_secs_to_event(t_since, wakeup_int * 60);

			if(secs_to_event > 0 && secs_to_event <= awake_sec)
			{
				reasons |= entry.reason;
			}
		}

		return reasons;
	}

	/******************************************************************************
	 * Decide which schedule to use depending on current battery mode
	 * If decided schedule is not valid, default schedule is returned so 
	 * a valid schedule is guaranteed.
	 *****************************************************************************/
	RetResult decide_schedule(SleepScheduler::WakeupScheduleEntry schedule_out[])
	{
		BATTERY_MODE battery_mode = Battery::get_current_mode();

		// For log
		uint8_t schedule_id = 0;

		switch (battery_mode)
		{
			case BATTERY_MODE::BATTERY_MODE_NORMAL:
			{
				const DeviceConfig::Data *device_config = DeviceConfig::get();
				memcpy(schedule_out, device_config->wakeup_schedule, sizeof(WAKEUP_SCHEDULE_DEFAULT));

				schedule_id = 1;
				break;
			}
			case BATTERY_MODE::BATTERY_MODE_LOW:
				memcpy(schedule_out, WAKEUP_SCHEDULE_BATT_LOW, sizeof(WAKEUP_SCHEDULE_BATT_LOW));

				schedule_id = 2;
				break;
			default:
				// TODO: How to handle this?
				Utils::serial_style(STYLE_RED);
				debug_println(F("Unknown battery mode, using battery low schedule."));
				memcpy(schedule_out, WAKEUP_SCHEDULE_BATT_LOW, sizeof(WAKEUP_SCHEDULE_BATT_LOW));
				Utils::serial_style(STYLE_RESET);

				schedule_id = 3;
				break;
		}

		if(!schedule_valid(schedule_out))
		{
			Utils::serial_style(STYLE_RED);
			debug_println(F("Schedule invalid, using default schedule"));
			Utils::serial_style(STYLE_RESET);

			Log::log(Log::SCHEDULE_INVALID_USING_DEFAULT, schedule_id);

			memcpy(schedule_out, WAKEUP_SCHEDULE_DEFAULT, sizeof(WAKEUP_SCHEDULE_DEFAULT));
		}

		return RET_OK;
	}
		
	/******************************************************************************
	* Check if one of the reasons of last wake up is "reason" (multiple reasons
	* cam be)
	******************************************************************************/
	bool wakeup_reason_is(WakeupReason reason)
	{
		return _last_wakeup_reasons & reason ? true : false;
	}

	/******************************************************************************
	 * Check if schedule is valid.
	 * A schedule is valid when:
	 * 1 - All of its events have a valid interval value
	 * 2 - It has at least a Call Home event with a valid interval != 0
	 *****************************************************************************/
	bool schedule_valid(const SleepScheduler::WakeupScheduleEntry schedule[])
	{
		bool call_home_valid = false;

		for(int i = 0; i < WAKEUP_SCHEDULE_LEN; i++)
		{
			int wakeup_int = schedule[i].wakeup_int;
			if(Utils::in_array(wakeup_int, WAKEUP_SCHEDULE_VALID_VALUES, sizeof(WAKEUP_SCHEDULE_VALID_VALUES)) == -1)
			{
				return false;
			}

			if(schedule[i].reason == SleepScheduler::REASON_CALL_HOME && wakeup_int > 0)
			{
				call_home_valid = true;
			}
		}

		return call_home_valid;
	}

	/******************************************************************************
	 * Print wake up schedule to serial output
	 *****************************************************************************/
	void print_schedule(SleepScheduler::WakeupScheduleEntry schedule[])
	{
		Utils::print_separator(F("Sleep schedule"));
		
		for(int i = 0; i < WAKEUP_SCHEDULE_LEN; i++)
		{
			switch(schedule[i].reason)
			{
				case REASON_READ_WATER_SENSORS:
					debug_print(F("Water sensors"));
					break;
				case REASON_READ_WEATHER_STATION:
					debug_print(F("Weather station"));
					break;
				case REASON_READ_SOIL_MOISTURE_SENSOR:
					debug_print(F("Soil moisture sensor"));
					break;
				case REASON_CALL_HOME:
					debug_print(F("Calling home"));
					break;
				default:
					debug_print(F("Unknown reason"));
					debug_print(schedule[i].reason, DEC);
					debug_print(F(": "));
					break;
			}
			debug_print(F(": "));
			debug_print(schedule[i].wakeup_int);
			debug_println(F(" min"));
		}

		Utils::print_separator(NULL);
	}

	/******************************************************************************
	 * Print wake up reasons
	 *****************************************************************************/
	void print_wakeup_reasons(int reasons)
	{
		if(reasons & SleepScheduler::REASON_CALL_HOME)
		{
			debug_print(F("Call home - "));
		}
		if(reasons & SleepScheduler::REASON_READ_WATER_SENSORS)
		{
			debug_print(F("Water sensors - "));
		}
		if(reasons & SleepScheduler::REASON_READ_WEATHER_STATION)
		{
			debug_print(F("Weather station - "));
		}
		if(reasons & SleepScheduler::REASON_FO)
		{
			debug_print(F("FO weather station - "));
		}
		if(reasons & SleepScheduler::REASON_READ_SOIL_MOISTURE_SENSOR)
		{
			debug_print(F("Soil Moisture sensor"));
		}

		debug_println();
	}
}