#ifndef SOLAR_MONITOR_H
#define SOLAR_MONITOR_H
#include "log.h"
#include "struct.h"
#include "common.h"
#include "app_config.h"
#include <Adafruit_INA219.h>

namespace SolarMonitor
{
    RetResult init();
    RetResult log();
    RetResult print();
}

#endif