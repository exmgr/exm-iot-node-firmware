#ifndef BATTERY_GAUAGE_H
#define BATTERY_GAUAGE_H
#include "log.h"
#include "struct.h"
#include "common.h"
#include "app_config.h"
#include "LTC2941.h"

namespace BatteryGauge
{
    RetResult init();
    RetResult log();
    RetResult print();
}

#endif