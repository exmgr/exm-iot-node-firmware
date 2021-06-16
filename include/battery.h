#ifndef BATTERY_H
#define BATTERY_H
#include "struct.h"
#include "const.h"

namespace Battery
{
    RetResult init();
    RetResult read_adc(uint16_t *voltage, uint16_t *pct);
    RetResult log_adc();
    RetResult log_solar_adc();
    BATTERY_MODE get_current_mode();
    BATTERY_MODE get_last_mode();
    void sleep_charge();
    void print_mode();
    RetResult read_solar_mv(uint16_t *voltage);
}

#endif