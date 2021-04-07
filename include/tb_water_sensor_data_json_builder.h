#ifndef TB_WATER_SENSOR_DATA_JSON_BUILDER_H
#define TB_WATER_SENSOR_DATA_JSON_BUILDER_H

#include "struct.h"
#include "const.h"
#include "app_config.h"
#include "water_sensor_data.h"
#include "json_builder_base.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

/******************************************************************************
* Helper class to build Thingsboard telemetry JSON from sensor data structures
******************************************************************************/
class TbWaterSensorDataJsonBuilder : public JsonBuilderBase<WaterSensorData::Entry, WATER_SENSOR_DATA_JSON_DOC_SIZE>
{
public:
	RetResult add(const WaterSensorData::Entry *entry);
};

#endif