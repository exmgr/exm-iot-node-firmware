#ifndef RTC_H
#define RTC_H

#include "struct.h"

namespace RTC
{
    RetResult init();

    RetResult sync();

    uint32_t get_timestamp();

    float get_external_rtc_temp();

    uint32_t get_last_sync_tick();
    void reset_last_sync_tick();

    void print_time();
    void print_temp();

    bool tstamp_valid(uint32_t tstamp);

    // TODO: Temp public
    RetResult sync_gsm_rtc_from_ntp();
    RetResult sync_time_from_gsm_rtc();
    RetResult sync_time_from_http();
    RetResult sync_time_from_ext_rtc();
    RetResult init_external_rtc();
    RetResult set_external_rtc_time(uint32_t timestamp);
    RetResult set_system_time(uint32_t timestamp);

    uint32_t get_external_rtc_timestamp();
    // TODO: up to here
}

#endif