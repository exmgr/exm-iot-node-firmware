#ifndef WATER_PRESENCE_H
#define WATER_PRESENCE_H
#include "water_sensor_data.h"

namespace WaterPresence
{
	RetResult init();

	RetResult measure(WaterSensorData::Entry *data);
}

#endif