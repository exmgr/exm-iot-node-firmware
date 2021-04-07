#ifndef TB_ATMOS41_DATA_JSON_BUILDER_H
#define TB_ATMOS41_DATA_JSON_BUILDER_H

#include "struct.h"
#include "const.h"
#include "app_config.h"
#include "atmos41_data.h"
#include "json_builder_base.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

/******************************************************************************
* Helper class to build Thingsboard telemetry JSON from weather data structures
******************************************************************************/
class TbAtmos41DataJsonBuilder : public JsonBuilderBase<Atmos41Data::Entry, ATMOS41_DATA_JSON_DOC_SIZE>
{
public:
	RetResult add(const Atmos41Data::Entry *entry);
};

#endif