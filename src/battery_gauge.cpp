#include "battery_gauge.h"

namespace BatteryGauge
{
    /******************************************************************************
	 * Init
	 *****************************************************************************/
    RetResult init()
    {
        if(!FLAGS.BATTERY_GAUGE_ENABLED)
            return RET_ERROR;
        debug_println_i(F("Initializing battery monitor."));
        ltc2941.initialize();
        
        ltc2941.setBatteryFullMAh(BAT_GAUGE_FULL_MAH);

        debug_print(F("Setting battery gauge capacity (mAh): "));
        debug_println(BAT_GAUGE_FULL_MAH, DEC);
    }

    /******************************************************************************
	 * Log battery gauge
	 *****************************************************************************/
    RetResult log()
    {
        if(!FLAGS.BATTERY_GAUGE_ENABLED)
            return RET_ERROR;

        debug_println(F("Logging battery gauge"));
        Log::log(Log::BAT_GAUGE_DATA, ltc2941.getmAh(), ltc2941.getmAhExpend());

        return RET_OK;
    }

    /******************************************************************************
	 * Log battery gauge
	 *****************************************************************************/
    RetResult print()
    {
        if(!FLAGS.BATTERY_GAUGE_ENABLED)
            return RET_ERROR;

        debug_println_i(F("Battery gauge data:"))

        debug_print(F("Battery status: "));
        Serial.print(ltc2941.getmAh());
        Serial.print("mAh");

        debug_print(F(" - "));
        Serial.print(ltc2941.getPercent());
        Serial.println("%");

        debug_print(F("Expended: "));
        Serial.print(ltc2941.getmAhExpend());
        Serial.println("mAh");

        debug_println();
    }
}