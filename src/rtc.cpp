#include <ctime>
#include <Wire.h>
#include <RtcDS3231.h>
#include "Arduino.h"
#include "rtc.h"
#include "gsm.h"
#include "const.h"
#include "sys/time.h"
#include "log.h"
#include "utils.h"
#include "http_request.h"
#include "common.h"

namespace RTC
{
    //
    // Private vars
    //
    RtcDS3231<TwoWire> _ext_rtc(Wire);

    /** Tick of last RTC sync */
    uint32_t _last_sync_tick = 0;

    //
    // Private functions
    //

    /******************************************************************************
     * Initialization
     *****************************************************************************/
    RetResult init()
    {
        RetResult ret = RET_OK;
        if(FLAGS.EXTERNAL_RTC_ENABLED)
        {
            ret = init_external_rtc();
        }

        return ret;
    }

    /******************************************************************************
     * Update ESP32 internal RTC (which is the main source of time keeping for the
     * whole application).
     * Flow:
     * 1. Sync GSM module RTC from NTP, then update system time from GSM module
     * 2. Update system time from plain HTTP
     * 3. Update system time from external RTC (if enabled and returned value valid)
     * 4. Update system time from GSM module RTC which has GSM time (because NTP failed)
     * 
     * Note: Requires GSM to be ON
     * 
     * @return RET_ERROR if all of the above methods failed, and there is no valid
     *                   system time
     *****************************************************************************/
    RetResult sync()
    {
        debug_println(F("Syncing RTC"));

        // For log
        uint32_t tstamp_before_sync = get_timestamp();

        RetResult ret = RET_ERROR;
        
        // No need to update external RTC because this is where we got the time from
        bool update_ext_rtc = true;

        // Try to get time from GSM/NTP
        if(GSM::is_gprs_connected() || GSM::connect_persist() == RET_OK)
        {
            if(GSM::is_gprs_connected())
            {
                debug_println_i(F("Already connected"));
            }
            
            // Try to sync GSM module's RTC from NTP
            if(sync_gsm_rtc_from_ntp() == RET_OK && sync_time_from_gsm_rtc() == RET_OK)
            {
                Utils::serial_style(STYLE_BLUE);
                debug_println(F("System time synced with NTP."));
                Utils::serial_style(STYLE_RESET);
                ret = RET_OK;
            }
            else
            {
				Utils::serial_style(STYLE_RED);
				debug_println(F("Sync time time from NTP failed."));
				Utils::serial_style(STYLE_RESET);

                // Get time from plain HTTP
                if(sync_time_from_http() == RET_OK)
                {
                    Utils::serial_style(STYLE_BLUE);
                    debug_println(F("System time synced with HTTP."));
                    Utils::serial_style(STYLE_RESET);
                    ret = RET_OK;
                }
                else
                {
                    debug_println(F("Synctime time from HTTP failed."));

                    // Get time from external RTC (if enabled)
                    if(FLAGS.EXTERNAL_RTC_ENABLED && sync_time_from_ext_rtc() == RET_OK)
                    {
                        Utils::serial_style(STYLE_BLUE);
                        debug_println(F("System time synced with external RTC."));
                        Utils::serial_style(STYLE_RESET);

                        update_ext_rtc = false;
                        ret = RET_OK;
                    }
                    else
                    {
                        debug_println(F("Sync time from external RTC failed."));

                        // At this point, syncing GSM RTC from NTP has failed, so time returned from
                        // the gsm module is GSM time, (as long as there is network connectivity)
                        if(sync_time_from_gsm_rtc() == RET_OK)
                        {
                            Utils::serial_style(STYLE_BLUE);
                            debug_println(F("System time synced with GSM time."));
                            Utils::serial_style(STYLE_RESET);
                            ret = RET_OK;
                        }
                        else
                        {
                            debug_println(F("Synctime time GSM time failed."));
                        }
                    }
                }
            }
        }
        else
        {
            // Get time from external RTC (if enabled)
            if(FLAGS.EXTERNAL_RTC_ENABLED && sync_time_from_ext_rtc() == RET_OK)
            {
                update_ext_rtc = false;
                ret = RET_OK;
            }
            else
            {
                debug_println(F("Synctime time from external RTC failed."));
            }
        }

        // If time didn't come from external RTC, then update RTC from system time
        // (only if getting time succeeded)
        if(FLAGS.EXTERNAL_RTC_ENABLED && update_ext_rtc && ret == RET_OK)
        {
            set_external_rtc_time(time(NULL));
        }

        // Log after finishing so the log entry has the correct timestamp
        Log::log(Log::RTC_SYNC, tstamp_before_sync);

        // Keep track of last time sync, failed or not
        _last_sync_tick = millis();
        

        return ret;
    }

    /******************************************************************************
    * Update GSM module's RTC from NTP
    ******************************************************************************/
    RetResult sync_gsm_rtc_from_ntp()
    {
        RetResult ret = RET_ERROR;
        int tries = GSM_TRIES;
        debug_println_i(F("Syncing GSM module time with NTP."));

        //
        // Try to update GSM module time from NTP
        // Note: If GPRS fails to connect, module returns GSM time which may have 
        // wrong timezone
        while(tries--)
        {
            ret = GSM::update_ntp_time();
            if(ret != RET_OK && tries - 1)
            {
                debug_println(F("Retrying..."));

                debug_print(F("Tries: "));
                debug_println(tries);
                delay(GSM_RETRY_DELAY_MS);
            }
            else
            {
                break;
            }
        }

        return ret;
    }

    /******************************************************************************
     * Get time from GSM module and update ESP32 internal RTC
     * Time in GSM could be synced from NTP (if NTP sync was run and succeeded) or
     * could be GSM time.
     *****************************************************************************/
    RetResult sync_time_from_gsm_rtc()
    {
        int tries = GSM_TRIES;

        //
        // Get time from GSM and set system time
        //
        tm tm_now = {0};
        tries = GSM_TRIES;

        debug_println_i(F("Getting time from GSM module."));

        RetResult ret = RET_ERROR;
        while(tries--)
        {
            ret = GSM::get_time(&tm_now);
            if(ret != RET_OK && tries - 1)
            {
                debug_println(F("Retrying..."));
                delay(GSM_RETRY_DELAY_MS);
            }
            else
            {
                break;
            }
        }
        if(ret != RET_OK)
            return ret;

		uint32_t timestamp = mktime(&tm_now);
		
		if(!tstamp_valid(timestamp))
		{
			debug_println(F("GSM time invalid."));
			return RET_ERROR;
		}

        if(set_system_time(mktime(&tm_now)) == RET_ERROR)
            return RET_ERROR;
      
        return RET_OK;
    }

    /******************************************************************************
    * Get time from backend server with a single GET request that returns current
    * timestamp in its body. Keep track of time passed since the req to compensate.
    * Body must contain only the timestamp.
    * Keeps track of time it took for the req to execute, to offset final timestamp
    * and obtain semi-accurate time
    ******************************************************************************/
   	RetResult sync_time_from_http()
    {
        debug_println(F("Syncing time from HTTP"));

		// Fir cakcykatubg iffset
        uint32_t start_time_ms = millis(), end_time_ms = 0;

		// Response info
        char resp[20] = "";
        uint16_t status_code = 0;

		// Break URL into parts
		int port = 0;
		char url_host[URL_HOST_BUFFER_SIZE] = "";
		char url_path[URL_BUFFER_SIZE] = "";
		if(Utils::url_explode((char*)HTTP_TIME_SYNC_URL, &port, url_host, sizeof(url_host), url_path, sizeof(url_path)) == RET_ERROR)
		{
			debug_println(F("Invalid HTTP time sync url."));
			return RET_ERROR;
		}

		// Request
        HttpRequest http_req(GSM::get_modem(), url_host);

        if(port > 0)
            http_req.set_port(port);
        if(http_req.get(url_path, resp, sizeof(resp)) == RET_ERROR)
        {
            debug_println(F("HTTP request failed"));
            return RET_ERROR;
        }
        end_time_ms = millis();

        resp[sizeof(resp) - 1] = '\0';

        debug_print(F("Received: "));
        debug_println(resp);

        // If not 10 chars, not a timestamp
        if(strlen(resp) != 10)
        {
            debug_print(F("Data received is not timestamp: "));
            debug_println(resp);
            return RET_ERROR;
        }

        unsigned long timestamp = 0;
        sscanf(resp, "%u", &timestamp);

		// Calculate offset
        int offset_sec = round((float)(end_time_ms - start_time_ms) / 1000);
        debug_print(F("Offsetting timestamp to compensate for req time (s): "));
        debug_println(offset_sec, DEC);

        timestamp -= offset_sec;

        if(!tstamp_valid(timestamp))
        {
            debug_print(F("Invalid timestamp received or resp. not a timestamp: "));
            debug_println(timestamp, DEC);
            return RET_ERROR;
        }

        // Update system time
        if(set_system_time(timestamp) == RET_ERROR)
        {
            debug_print(F("Could not update system time with timestamp: "));
            debug_println(timestamp, DEC);
            return RET_ERROR;
        }

        return RET_OK;
    }

    /******************************************************************************
    * Update system time from external RTC
    ******************************************************************************/
    RetResult sync_time_from_ext_rtc()
    {
        if(!FLAGS.EXTERNAL_RTC_ENABLED)
        {
            return RET_ERROR;
        }

        uint32_t ext_rtc_tstamp = _ext_rtc.GetDateTime().Epoch32Time();

        if(!tstamp_valid(ext_rtc_tstamp))
        {
            debug_print(F("Got invalid timestamp from ext rtc: "));
            debug_println(ext_rtc_tstamp, DEC);
            return RET_ERROR;
        }

        debug_print(F("Setting system time from ext RTC: "));
        debug_println(ext_rtc_tstamp);

        timeval tv_now = {0};
        tv_now.tv_sec = ext_rtc_tstamp;

        // Set system time
        if(settimeofday(&tv_now, NULL) != 0)
        {
            debug_println(F("Could not set system time."));
            return RET_ERROR;
        }

        print_time();

        return RET_OK;
    }

    /******************************************************************************
    * Init external RTC
    ******************************************************************************/
    RetResult init_external_rtc()
    {
        // Retry several times before failing
        // int tries = 3;
        // if(Wire.begin(PIN_RTC_SDA, PIN_RTC_SCL) == false)
        // {
        //     debug_println(F("Could not begin I2C comms with ext RTC."));
        //     if(tries--)
        //     {
        //         debug_println(F("Retrying..."));
        //     }
        //     else
        //     {
        //         debug_println(F("Failed. Aborting."));
        //         return RET_ERROR;
        //     }
        // }

        if(!_ext_rtc.IsDateTimeValid())
        {
            Utils::serial_style(STYLE_RED);
            debug_println(F("RTC osc stop detected."));
            Utils::serial_style(STYLE_RESET);

            if (_ext_rtc.LastError() != 0)
            {
                debug_printf("Could not communicate with ext RTC.");
                return RET_ERROR;
            }
            else
            {
                debug_println(F("RTC invalid time but no error code."));
                return RET_ERROR;
            }
        }

        if (!_ext_rtc.GetIsRunning())
        {
            debug_println("Ext RTC was not running, starting.");
            _ext_rtc.SetIsRunning(true);
        }

        _ext_rtc.Enable32kHzPin(false);
        _ext_rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 

        debug_print(F("External RTC init complete. Time: "));
        debug_println(_ext_rtc.GetDateTime().Epoch32Time());
 
        return RET_OK;
    }

    /******************************************************************************
    * Set external RTC time
    ******************************************************************************/
    RetResult set_external_rtc_time(uint32_t timestamp)
    {
        // External RTC counts time in secs from 2000, subtract offset
        uint32_t secs_since_2000 = timestamp - SECONDS_IN_2000;
        
        _ext_rtc.SetDateTime(secs_since_2000);
        if(_ext_rtc.LastError() != 0)
        {
            debug_print(F("Could not set ext RTC time to: "));
            debug_println(timestamp);
            debug_print("(");
            debug_print(secs_since_2000, DEC);
            debug_println(")");

            return RET_ERROR;
        }
        else
        {
            debug_print(F("External RTC time set: "));
            debug_println(timestamp, DEC);

            return RET_OK;
        }
        
    }

    /******************************************************************************
    * Set system time
    ******************************************************************************/
    RetResult set_system_time(uint32_t timestamp)
    {
        timeval tval_now = {0};

        // Check if time valid
        tval_now.tv_sec = timestamp;
        if(tval_now.tv_sec == -1)
        {
            debug_println(F("Cannot set time to invalid timestamp."));
            return RET_ERROR;
        }

        // Set system time
        if(settimeofday(&tval_now, NULL) != 0)
        {
            debug_println(F("Could not set system time"));
            return RET_ERROR;
        }

        return RET_OK;
    }

    /******************************************************************************
     * Get time from external RTC
     *****************************************************************************/
    uint32_t get_external_rtc_timestamp()
    {
        return _ext_rtc.GetDateTime().Epoch32Time();
    }

    /******************************************************************************
    * Get current time stamp
    ******************************************************************************/
    uint32_t get_timestamp()
    {
        return time(NULL);
    }

    /******************************************************************************
     * Get external2 RTC temperature
     *****************************************************************************/
    float get_external_rtc_temp()
    {
        // Force compensation update to force sensor to update temp
        _ext_rtc.ForceTemperatureCompensationUpdate(false);

        // Wait for measurement to update because calling func with blocking mode has no
        // timeout and can freeze the system (A+ quality arduino libraries)
        delay(200);

        return _ext_rtc.GetTemperature().AsFloatDegC();
    }

     /******************************************************************************
     * Get last sync tick
     *****************************************************************************/
    uint32_t get_last_sync_tick()
    {
        return _last_sync_tick;
    }

     /******************************************************************************
     * Reset last sync tick
     *****************************************************************************/
    void reset_last_sync_tick()
    {
        _last_sync_tick = 0;
    }    

    /******************************************************************************
    * Check timestamp for validity by comparing to a recent tstamp
    ******************************************************************************/
    bool tstamp_valid(uint32_t tstamp)
    {
        return (tstamp > FAIL_CHECK_TIMESTAMP_START && tstamp < FAIL_CHECK_TIMESTAMP_END);
    }

    /******************************************************************************
     * Print current time to serial output
     *****************************************************************************/
    void print_time()
    {
        debug_print("Time: ");
        time_t cur_tstamp = time(NULL);

        Serial.print(F("External RTC timestamp: "));
        Serial.println(RTC::get_external_rtc_timestamp(), DEC);
        Serial.print(F("System RTC timestamp: "));
        Serial.println(RTC::get_timestamp(), DEC);
        
        debug_print(ctime(&cur_tstamp));
        debug_print("(");
        debug_print(cur_tstamp, DEC);
        debug_println(")");
	}

    /******************************************************************************
    * Print temperature
    ******************************************************************************/
    void print_temp()
    {
        debug_print("RTC Temperature: ");
        debug_print((int) _ext_rtc.GetTemperature().AsFloatDegC());
        debug_println("C");	
    }
}