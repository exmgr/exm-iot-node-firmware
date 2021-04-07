#ifndef SLEEP_H
#define SLEEP_H

#include "struct.h"

namespace SleepScheduler
{
    // Reason that triggered wake up
    enum WakeupReason
    {
        REASON_NONE = 0,
        REASON_CALL_HOME = 1,
        REASON_READ_WATER_SENSORS = 1 << 1,
        REASON_READ_WEATHER_STATION = 1 << 2,
        REASON_FO = 1 << 3,
        REASON_READ_SOIL_MOISTURE_SENSOR = 1 << 4
    };

    // Describes an entry in the wake up schedule
    // "Wakeup" and not "Sleep" because it represents wake up intervals and
    // not sleep intervals (ie. how often it wakes up, not how often it sleeps)
    struct WakeupScheduleEntry
    {
        WakeupScheduleEntry() {}
        WakeupScheduleEntry(WakeupReason _reason, int _wakeup_int)
                : reason(_reason), wakeup_int(_wakeup_int) {}
        // Reason that triggered wake up
        WakeupReason reason;
        // Wake up interval in minutes
        int wakeup_int;
    }__attribute__((packed));

    RetResult sleep_to_next();

    RetResult calc_next_wakeup(uint32_t t_now, const WakeupScheduleEntry schedule[], int *seconds_left, int *event_reasons);

    bool wakeup_reason_is(WakeupReason reason);
    bool schedule_valid(const SleepScheduler::WakeupScheduleEntry schedule[]);
    void print_schedule(SleepScheduler::WakeupScheduleEntry schedule[]);
    void print_wakeup_reasons(int reasons);
    int check_missed_reasons(const WakeupScheduleEntry schedule[], uint32_t t_since, uint32_t awake_sec);
}

#endif