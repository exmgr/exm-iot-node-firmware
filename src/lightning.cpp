#include "const.h"
#include "app_config.h"
#include "common.h"
#include "lightning.h"
#include "lightning_data.h"
#include "SparkFun_AS3935.h"
#include "log.h"
#include "rtc.h"

namespace Lightning
{
    void print_current_settings();

    /** Lightning sensor instance */
    SparkFun_AS3935 _sensor(LIGHTNING_I2C_ADDR);

    /** Last IRQ time. Used to debounce wakeups. If too soon, wakeups will be ignored. */
    uint32_t _last_irq_millis = 0;

    /** Tick of last time noise/distruber events were logged. Noise/disturber events are logged as sums in a 
     * timewindow to preserve memory (ie. event count over an hour) */
    uint32_t _last_irq_report_millis = 0;

    /** Count of noise events since last time the value was saved into a log */
    uint32_t _noise_events = 0;

    /** Count of disturber events since last log (same as noise) */
    uint32_t _distruber_events = 0;

    /******************************************************************************
	 * Initialize lightning sensor and set up interrupts
	 *****************************************************************************/	
	RetResult on()
	{
        Utils::print_separator(F("Lightning Sensor"));

        debug_print_i(F("Lightning module: "));
        if(LIGHTNING_SENSOR_MODULE == LIGHTNING_SENSOR_CJMCU)
        {
            Serial.println(F("CJMCU"));
        }
        else if(LIGHTNING_SENSOR_MODULE == LIGHTNING_SENSOR_DFROBOT)
        {
            Serial.println(F("DFRobot"));
        }

        debug_print(F("I2C Addr: "));
        debug_println(LIGHTNING_I2C_ADDR, DEC);
        debug_print(F("Module: "));
        debug_println(LIGHTNING_SENSOR_MODULE, DEC);

        if(!_sensor.begin())
        {
            debug_println_e(F("Lightning sensor can't start."));
            return RET_ERROR;
        }
        else
        {
            debug_println(F("Lightning sensor started."));   
        }

        //
        // Print default values
        //
        _sensor.resetSettings();
        Utils::print_separator(NULL);
        debug_println_i(F("Printing default values"));
        print_current_settings();
        Utils::print_separator(NULL);
        
        // Serial.println(F("Done"));
        // while(1)
        // {
            
        // }

        //
        // Tuning
        //
        
        // Div ratio
        _sensor.changeDivRatio(LIGHTNING_DIV_RATIO);

        // Tune cap
        _sensor.tuneCap(LIGHTNING_TUNE_CAP);

        // Calibrate
        if(_sensor.calibrateOsc())
        {
            debug_println(F("Lightning sensor calibrated."));
        }
        else
        {
            debug_println_e(F("Could not calibrate lightning sensor"));
        }

        // Disable disturber report to avoid unnecessary wakeup
        _sensor.maskDisturber(LIGHTNING_MASK_DISTURBERS);

        // Indoor/outdoor
        _sensor.setIndoorOutdoor(LightningEnvironment::LIGHTNING_ENV_OUTDOOR);

        // Noise floor
        _sensor.setNoiseLevel(LIGHTNING_NOISE_FLOOR);

        // Watchdog
        _sensor.watchdogThreshold(LIGHTNING_WATCHDOG_THRES);

        // Spike rejection
        _sensor.spikeRejection(LIGHTNING_SPIKE_REJECTION);

        // Lightning threshold
        _sensor.lightningThreshold(LIGHTNING_THRES);

        Utils::print_separator(NULL);
        debug_println_i(F("Current lightning sensor config"));
        print_current_settings();
        Utils::print_separator(NULL);

        // Resetting settings
        // Serial.println(F("Resetting settings"));
        // _sensor.resetSettings();

        // _sensor.displayOscillator(true, 3);
        // while(1);

        pinMode(PIN_LIGHTNING_IRQ, INPUT_PULLDOWN);
        esp_sleep_pd_config(esp_sleep_pd_domain_t::ESP_PD_DOMAIN_RTC_PERIPH, esp_sleep_pd_option_t::ESP_PD_OPTION_ON);
        esp_sleep_enable_ext0_wakeup(PIN_LIGHTNING_IRQ, 1);

        debug_println_i(F("Lightning sensor init OK."));

        return RET_OK;
    }

    /******************************************************************************
    * Power down sensor
    ******************************************************************************/
    void off()
    {
        // Start i2c in case not already started
        _sensor.begin();

        _sensor.readInterruptReg();

        esp_sleep_pd_config(esp_sleep_pd_domain_t::ESP_PD_DOMAIN_RTC_PERIPH, esp_sleep_pd_option_t::ESP_PD_OPTION_OFF);

        _sensor.powerDown();
    }

    /******************************************************************************
    * Handle IRQ from lightning sensor
    ******************************************************************************/
    RetResult handle_irq()
    {
        debug_println_i(F("Lightning IRQ"));
        RetResult ret = RET_OK;

        if(_last_irq_millis != 0 && millis() - _last_irq_millis <= LIGHTNING_IRQ_DEBOUNCE_MS)
        {
            debug_println_e(F("IRQ re-fired too soon, ignoring..."));
            return RET_ERROR;
        }

        LightningIntReason int_reason = (LightningIntReason)_sensor.readInterruptReg();

        if(int_reason == LightningIntReason::LIGHTNING_INT_REASON_NOISE)
        {
            debug_println_w(F("Lightning sensor: Noise!"));
            _noise_events++;
        }
        else if(int_reason == LightningIntReason::LIGHTNING_INT_REASON_DISTURBER)
        {
            debug_println_w(F("Lightning sensor: Disturber!"));
            _distruber_events++;
        }
        else if(int_reason == LightningIntReason::LIGHTNING_INT_REASON_LIGHTNING)
        {
            LightningData::Entry entry = {0};

            entry.distance = _sensor.distanceToStorm();
            entry.energy = _sensor.lightningEnergy();
            entry.timestamp = RTC::get_timestamp();

            LightningData::add(&entry);

            debug_println_i(F("Lightning detected!"));
            LightningData::print(&entry);
        }
        else
        {
            debug_println_e(F("Unknown IRQ reason!"));
            ret = RET_ERROR;
        }

        // Time to log noise/disturber counters and reset?
        if(int_reason == LightningIntReason::LIGHTNING_INT_REASON_NOISE || int_reason == LightningIntReason::LIGHTNING_INT_REASON_DISTURBER)
        {
            if(millis() - _last_irq_report_millis >= LIGHTNING_REPORT_LOG_INTERVAL_SEC * 1000 && _last_irq_report_millis != 0)
            {
                Log::log(Log::LIGHTNING_IRQ_REPORT, _noise_events, _distruber_events);

                _noise_events = 0;
                _distruber_events = 0;

                _last_irq_report_millis = millis();
            }
        }

        _last_irq_millis = millis();

        return ret;
    }


    /******************************************************************************
    * Read settings registers and print
    ******************************************************************************/
    void print_current_settings()
    {
        debug_print(F("DIV ratio: "));
        debug_println(_sensor.readDivRatio(), DEC);

        debug_print(F("Indoor/Outdoor: "));
        debug_print(_sensor.readIndoorOutdoor());
        switch(_sensor.readIndoorOutdoor())
        {
        case INDOOR:
            debug_println(F(" - Indoor"));
            break; 
        case OUTDOOR:
            debug_println(F(" - Outdoor"));
            break; 
        default:
            debug_println_e(F("Unknown environment value"));
        }

        debug_print(F("Lightning threshold: "));
        debug_println(_sensor.readLightningThreshold(), DEC);

        debug_print(F("Mask disturbers: "));
        debug_println(_sensor.readMaskDisturber(), DEC);

        debug_print(F("Noise level: "));
        debug_println(_sensor.readNoiseLevel(), DEC);

        debug_print(F("Watchdog threshold: "));
        debug_println(_sensor.readWatchdogThreshold(), DEC);

        debug_print(F("Spike rejection: "));
        debug_println(_sensor.readSpikeRejection(), DEC);

        debug_print(F("Tune cap: "));
        debug_println(_sensor.readTuneCap(), DEC);
    }
} // namespace Lightning
