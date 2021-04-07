#include "tb_sdi12_log_json_builder.h"
#include "utils.h"
#include "common.h"

/******************************************************************************
 * Add packet to request
 *****************************************************************************/
RetResult TbSDI12LogJsonBuilder::add(const SDI12Log::Entry *entry)
{
	JsonObject json_entry = _root_array.createNestedObject();

	json_entry[SDI12_LOG_KEY_TIMESTAMP] = (long long)entry->timestamp;
	JsonObject values = json_entry.createNestedObject("values");

	// Check the last one, no need to check all, if last doesnt fit into
	// the json doc, doc is full already
	String str(entry->response);
	if((values[SDI12_LOG_KEY_RAW_DATA] = str) == false)
	{
		debug_println(F("Could not add SDI12 debug data to JSON."));
		return RET_ERROR;
	}

	return RET_OK;
} 