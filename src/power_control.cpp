#include "Arduino.h"
#include "power_control.h"

/******************************************************************************
* Controls various power sources
******************************************************************************/
namespace PowerControl
{
    RetResult switch_12v(bool on)
    {
        if(on)
        {
            digitalWrite(PIN_WATER_SENSORS_PWR, 0);
        }
        else
        {
            digitalWrite(PIN_WATER_SENSORS_PWR, 1);
        }
    }
}