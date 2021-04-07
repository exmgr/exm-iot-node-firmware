#ifndef TB_LOG_JSON_BUILDER_H
#define TB_LOG_JSON_BUILDER_H

#include "log.h"
#include "const.h"
#include "json_builder_base.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

class TbLogJsonBuilder : public JsonBuilderBase<Log::Entry, LOG_JSON_DOC_SIZE>
{
public:
	RetResult add(const Log::Entry *entry);
};

#endif