#ifndef WATER_SENSORS_H
#define WATER_SENSORS_H

#include "struct.h"

namespace WaterSensors
{
    RetResult on();
    RetResult off();

    RetResult log();

    RetResult init();
}

#endif