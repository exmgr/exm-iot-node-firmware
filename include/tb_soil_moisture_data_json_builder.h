#ifndef TB_SOIL_MOISTURE_DATA_JSON_BUILDER_H
#define TB_SOIL_MOISTURE_DATA_JSON_BUILDER_H

#include "struct.h"
#include "const.h"
#include "app_config.h"
#include "soil_moisture_data.h"
#include "json_builder_base.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

/******************************************************************************
* Helper class to build Thingsboard telemetry JSON from sensor data structures
******************************************************************************/
class TbSoilMoistureDataJsonBuilder : public JsonBuilderBase<SoilMoistureData::Entry, SOIL_MOISTURE_DATA_JSON_DOC_SIZE>
{
public:
	RetResult add(const SoilMoistureData::Entry *entry);
};

#endif