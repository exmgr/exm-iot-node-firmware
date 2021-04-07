#include "tb_lightning_data_json_builder.h"
#include "utils.h"
#include "common.h"

/******************************************************************************
 * Add packet to request
 *****************************************************************************/
RetResult TbLightningDataJsonBuilder::add(const LightningData::Entry *entry)
{
	JsonObject json_entry = _root_array.createNestedObject();

	json_entry[LIGHTNING_DATA_KEY_TIMESTAMP] = (long long)entry->timestamp * 1000;
	JsonObject values = json_entry.createNestedObject("values");

	values[LIGHTNING_DATA_KEY_TIMESTAMP] = entry->timestamp;
	values[LIGHTNING_DATA_KEY_DISTANCE] = entry->distance;
	
	// Check the last one, no need to check all, if last doesnt fit into
	// the json doc, doc is full already
	if((values[LIGHTNING_DATA_KEY_ENERGY] = entry->energy) == false)
	{
		debug_println(F("Could not add lightning data to JSON."));
		return RET_ERROR;
	}

	return RET_OK;
} 