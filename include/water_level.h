#ifndef WATER_LEVEL_H
#define WATER_LEVEL_H
#include "water_sensor_data.h"

namespace WaterLevel
{
	RetResult init();

	RetResult measure(WaterSensorData::Entry *data);

	RetResult measure_dummy(WaterSensorData::Entry *data);
}

#endif