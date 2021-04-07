#include "solar_monitor.h"

namespace SolarMonitor
{
    Adafruit_INA219 _ina219;

    /******************************************************************************
	 * Init
	 *****************************************************************************/
    RetResult init()
    {
        if (!FLAGS.SOLAR_CURRENT_MONITOR_ENABLED)
            return RET_ERROR;

        debug_println_i(F("Initializing solar monitor."));

        if(!_ina219.begin())
        {
            debug_println_e(F("Failed to start solar monitor."));
            return RET_ERROR;
        }

        return RET_OK;
    }

    /******************************************************************************
	 * Log solar monitor info
	 *****************************************************************************/
    RetResult log()
    {
        if (!FLAGS.SOLAR_CURRENT_MONITOR_ENABLED)
            return RET_ERROR;

        debug_println(F("Logging solar monitor."));
        Log::log(Log::SOLAR_MONITOR_DATA, (int)(_ina219.getBusVoltage_V() * 1000), _ina219.getCurrent_mA());

        return RET_OK;
    }

    /******************************************************************************
	 * Print solar monitor
	 *****************************************************************************/
    RetResult print()
    {
        if(!FLAGS.SOLAR_CURRENT_MONITOR_ENABLED)
            return RET_ERROR;

        debug_println_i(F("Solar monitor data:"))

        debug_print(F("Bus voltage: "));
        Serial.print(_ina219.getBusVoltage_V());
        Serial.println("V");

        debug_print(F("Current: "));
        Serial.print(_ina219.getCurrent_mA());
        Serial.println("mA");

        debug_println();
    }

} // namespace SolarMonitor