#ifndef INT_ENV_SENSOR_H
#define INT_ENV_SENSOR_H

#include "struct.h"

namespace IntEnvSensor
{
    RetResult init();
    RetResult read(float *temp, float *hum, int *press, int *alt);
    RetResult log();
}

#endif