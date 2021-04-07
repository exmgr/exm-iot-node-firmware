#include "battery.h"
#include "const.h"
#include "log.h"
#include "utils.h"
#include "gsm.h"
#include "app_config.h"
#include "common.h"
#include "lightning.h"

namespace Battery
{
    //
    // Private members
    //

    //
    // Private functions
    //
    uint8_t mv_to_pct(uint16_t mv);

    /******************************************************************************
     * Init fuel
     ******************************************************************************/
    RetResult init()
    {
        // Configure ADC
        analogSetCycles(ADC_CYCLES);

        pinMode(PIN_ADC_BAT, ANALOG);
        pinMode(PIN_ADC_SOLAR, ANALOG);

        return RET_OK;
    }

    /******************************************************************************
     * Read battery mV with ADC (TCALL)
     * @param
     *****************************************************************************/
    RetResult read_adc(uint16_t *voltage, uint16_t *pct)
    {
       	uint32_t in = 0;
        for (int i = 0; i < ADC_BATTERY_LEVEL_SAMPLES; i++)
        {
            in += (uint32_t)analogRead(PIN_ADC_BAT);
        }
        in = (int)in / ADC_BATTERY_LEVEL_SAMPLES;

        uint16_t bat_mv = ((float)in / 4096) * 3600 * 2;

        *voltage = bat_mv;
        *pct = mv_to_pct(bat_mv);
               
        return RET_OK;
    }

    /******************************************************************************
    * Measure battery voltage from ADC and log
    ******************************************************************************/
    RetResult log_adc()
    {
        uint16_t voltage = 0;
        uint16_t pct = 0;

        if(read_adc(&voltage, &pct) == RET_ERROR)
        {
            return RET_ERROR;
        }

        Log::log(Log::BATTERY, voltage, pct);

        // No percent value, use -1
        Utils::serial_style(STYLE_BLUE);
        debug_printf("ADC Battery: %dmV | %d%% |\n", voltage, pct);
        Utils::serial_style(STYLE_RESET);

        return RET_OK;
    }

    /******************************************************************************
    * Measure solar voltage and log
    ******************************************************************************/
    RetResult log_solar_adc()
    {
        uint16_t voltage = 0;

        
        if(read_solar_mv(&voltage) == RET_ERROR)
        {
            return RET_ERROR;
        }

        Log::log(Log::SOLAR_PANEL_VOLTAGE, voltage);

        // No percent value, use -1
        Utils::serial_style(STYLE_BLUE);
        debug_printf("Solar panel: %dmV\n", voltage);
        Utils::serial_style(STYLE_RESET);

        return RET_OK;
    }

    /******************************************************************************
     * Convert battery voltage to pct with the help of a look up table
     *****************************************************************************/
    uint8_t mv_to_pct(uint16_t mv)
    {
        const uint8_t array_size = sizeof(BATTERY_PCT_LUT) / sizeof(BATTERY_PCT_LUT[0]);
        uint8_t pct_out = 0;

        // mV larger than the maximum = 100%
        if(mv >= BATTERY_PCT_LUT[array_size - 1].mv)
        {
            pct_out = 100;
        }
        else
        {
            const BATTERY_PCT_LUT_ENTRY *cur_entry = nullptr;
            const BATTERY_PCT_LUT_ENTRY *next_entry = nullptr;

            for(int i = 0; i < array_size - 1; i++)
            {
                cur_entry = &(BATTERY_PCT_LUT[i]);
                next_entry = &(BATTERY_PCT_LUT[i+1]);
                
                if(mv > cur_entry->mv && mv < next_entry->mv)
                {
                    if(mv - cur_entry->mv < next_entry->mv - mv)
                    {
                        pct_out = cur_entry->pct;
                    }
                    else
                    {
                        pct_out = next_entry->pct;
                    }
                    break;
                }
                else if(mv == cur_entry->mv)
                {
                    pct_out = cur_entry->pct;
                    break;
                }
            }
        }

        return pct_out;
    }

    /******************************************************************************
     * Get battery mode depending on level
     *****************************************************************************/
    BATTERY_MODE get_current_mode()
    {
        uint16_t mv = 0, pct = 0;

        if(FLAGS.BATTERY_FORCE_NORMAL_MODE)
        {
            return BATTERY_MODE::BATTERY_MODE_NORMAL;
        }
        
        read_adc(&mv, &pct);

        debug_print(F("Battery: "));
        debug_print(mv, DEC);
        debug_print("mV - ");
        debug_print(pct);
        debug_println("%");

        if(pct > BATTERY_LEVEL_LOW)
        {
            return BATTERY_MODE::BATTERY_MODE_NORMAL;
        }
        else if(pct > BATTERY_LEVEL_SLEEP_CHARGE)
        {
            return BATTERY_MODE::BATTERY_MODE_LOW;
        }
        else if(pct == 0 && mv == 0)
        {
            Utils::serial_style(STYLE_RED);
            debug_println(F("Battery voltage is 0, state unknown, assuming normal battery mode."));
            Utils::serial_style(STYLE_RESET);

            return BATTERY_MODE::BATTERY_MODE_NORMAL;
        }
        else
        {
            return BATTERY_MODE::BATTERY_MODE_SLEEP_CHARGE;
        }
    }

    /******************************************************************************
     * Battery is low, device goes into sleep charge mode where it sleeps and all
     * functions are disabled until battery is over the charged threshold.
     * Device wakes up from sleep every X mins to check.
     *****************************************************************************/
    void sleep_charge()
    {
        if(Battery::get_current_mode() != BATTERY_MODE::BATTERY_MODE_SLEEP_CHARGE)
            return;

        debug_println(F("Battery critical, going into sleep charge mode."));
        Log::log(Log::SLEEP_CHARGE);

        //
        // Prepare
        //
        // Turn lightning sensor OFF to prevent INTs waking up device
        Lightning::off();
    
        Battery::log_adc();
        Battery::log_solar_adc();

        // Set sleep time and go to sleep
        uint64_t time_to_sleep_ms = SLEEP_CHARGE_CHECK_INT_MINS * 60000;

        if(FLAGS.SLEEP_MINS_AS_SECS)
            time_to_sleep_ms /= 60;

        esp_sleep_enable_timer_wakeup((uint64_t)time_to_sleep_ms * 1000);

        int wakeup_count = 0;

        while(true)
        {
            debug_printf("Sleeping for (sec): %llu \n", time_to_sleep_ms / 1000);
            Serial.flush();
            esp_light_sleep_start();
            debug_println(F("Wake up"));

            wakeup_count++;

            uint16_t mv = 0, pct = 0;
            Battery::read_adc(&mv, &pct);		

            if(pct < BATTERY_LEVEL_SLEEP_RECHARGED)
            {
                debug_println(F("Battery level not quite there yet... Going back to sleep."));
                Log::log(Log::SLEEP_CHARGE_CHECK, wakeup_count);
            }
            else
            {
                debug_println(F("Battery charged up to threshold. Exiting sleep charge mode."));

                Log::log(Log::SLEEP_CHARGE_FINISHED, wakeup_count);
                
                Battery::log_adc();
                
                break;
            }
        }

        // Turn lightning back ON
        Lightning::on();
    }

    /******************************************************************************
	* Read solar ADC pin and convert to mv
    * @param mV output var
	* @return Read value in 
	******************************************************************************/
	RetResult read_solar_mv(uint16_t *voltage)
	{
        int mv = Utils::read_adc_mv(PIN_ADC_SOLAR, 10, 10);
        
        if(mv < 0)
            return RET_ERROR;

        // Compensate for1/2 divider
        *voltage = mv * 2;

        return RET_OK;
	}

    /******************************************************************************
     * Print current battery mode to serial output
     *****************************************************************************/
    void print_mode()
    {
        uint16_t mv = 0, pct = 0;
        read_adc(&mv, &pct);
        debug_printf("Getting battery mode. %dmV | %d%% \n", mv, pct);
        debug_print(F("Battery mode: "));

        if(FLAGS.BATTERY_FORCE_NORMAL_MODE)
        {
            Utils::serial_style(STYLE_RED);
            debug_println(F("FORCED NORMAL"));
            Utils::serial_style(STYLE_RESET);
            return;
        }

        switch (get_current_mode())
        {
        case BATTERY_MODE::BATTERY_MODE_LOW:
            debug_println(F("Low"));
            break;
        case BATTERY_MODE::BATTERY_MODE_NORMAL:
            debug_println(F("Normal"));
            break;
        case BATTERY_MODE::BATTERY_MODE_SLEEP_CHARGE:
            debug_println(F("Sleep charge"));
            break;
        default:
            Utils::serial_style(STYLE_RED);
            debug_println(F("Unknown"));
            Utils::serial_style(STYLE_RESET);
            break;
        }
    }
}