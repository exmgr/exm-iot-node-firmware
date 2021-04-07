#ifndef WATER_QUALITY_H
#define WATER_QUALITY_H

#include "water_sensor_data.h"

namespace Aquatroll
{
    RetResult init();

    RetResult measure(WaterSensorData::Entry *data);

    RetResult measure_aquatroll400(WaterSensorData::Entry *data);
    RetResult measure_aquatroll500(WaterSensorData::Entry *data);
    RetResult measure_aquatroll600(WaterSensorData::Entry *data);

    RetResult measure_dummy(WaterSensorData::Entry *data);
}

#endif