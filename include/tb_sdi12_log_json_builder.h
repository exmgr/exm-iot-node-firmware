#ifndef tb_sdi12_log_json_builder_H
#define tb_sdi12_log_json_builder_H

#include "struct.h"
#include "const.h"
#include "app_config.h"
#include "sdi12_log.h"
#include "json_builder_base.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

/******************************************************************************
* Helper class to build Thingsboard telemetry JSON from weather data structures
******************************************************************************/
class TbSDI12LogJsonBuilder : public JsonBuilderBase<SDI12Log::Entry, SDI12_LOG_JSON_DOC_SIZE>
{
public:
	RetResult add(const SDI12Log::Entry *entry);
};

#endif