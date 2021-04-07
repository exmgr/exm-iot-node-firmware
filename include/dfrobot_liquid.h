#ifndef DFROBOT_LIQUID_H
#define DFROBOT_LIQUID_H
#include "water_sensor_data.h"

namespace DFRobotLiquid
{
    RetResult init();

	RetResult measure(WaterSensorData::Entry *data);

	RetResult measure_dummy(WaterSensorData::Entry *data);
}

#endif