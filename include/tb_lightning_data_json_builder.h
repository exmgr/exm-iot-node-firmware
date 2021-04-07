#ifndef TB_LIGHTNING_DATA_JSON_BUILDER_H
#define TB_LIGHTNING_DATA_JSON_BUILDER_H

#include "struct.h"
#include "const.h"
#include "app_config.h"
#include "lightning_data.h"
#include "json_builder_base.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

/******************************************************************************
* Helper class to build Thingsboard telemetry JSON from data store entries
******************************************************************************/
class TbLightningDataJsonBuilder : public JsonBuilderBase<LightningData::Entry, ATMOS41_DATA_JSON_DOC_SIZE>
{
public:
	RetResult add(const LightningData::Entry *entry);
};

#endif