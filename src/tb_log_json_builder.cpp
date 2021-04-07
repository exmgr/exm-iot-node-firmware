#include "tb_log_json_builder.h"
#include "common.h"

/******************************************************************************
 * Add packet to request
 *****************************************************************************/
RetResult TbLogJsonBuilder::add(const Log::Entry *entry)
{
	JsonObject json_entry = _root_array.createNestedObject();

	json_entry["ts"] = (long long)entry->timestamp;
	JsonObject values = json_entry.createNestedObject("values");

	char log_entry[48] = "";

	// Build log value as a comma separated string
	snprintf(log_entry, sizeof(log_entry), "%d,%d,%d", entry->code, entry->meta1, entry->meta2);

	// values["log"] = log_entry;

	if(values.getOrAddMember("log").set(log_entry) == false)
	{
        debug_println(F("Could not add log entries to JSON."));
        return RET_ERROR;
	}

	return RET_OK;
}