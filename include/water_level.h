#ifndef WATER_LEVEL_H
#define WATER_LEVEL_H
#include "water_sensor_data.h"

namespace WaterLevel
{
	enum ErrorCode
	{
		ERROR_NONE,
		ERROR_OTHER,
        ERROR_TOO_MANY_INVALID_VALUES,
		ERROR_HIGH_VAL_FLUCTUATION,
		ERROR_TIMEOUT,
		ERROR_CRC_FAIL
	};

	RetResult init();

	RetResult measure(WaterSensorData::Entry *data);

	RetResult measure_dummy(WaterSensorData::Entry *data);

	ErrorCode get_last_error();
}

#endif