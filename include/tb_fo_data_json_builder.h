#ifndef TB_FO_DATA_JSON_BUILDER_H
#define TB_FO_DATA_JSON_BUILDER_H

#include "struct.h"
#include "const.h"
#include "app_config.h"
#include "fo_data.h"
#include "json_builder_base.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

/******************************************************************************
* Helper class to build Thingsboard telemetry JSON from FO data structures
******************************************************************************/
class TbFoDataJsonBuilder : public JsonBuilderBase<FoData::StoreEntry, FO_DATA_JSON_DOC_SIZE>
{
public:
	RetResult add(const FoData::StoreEntry *entry);
};

#endif