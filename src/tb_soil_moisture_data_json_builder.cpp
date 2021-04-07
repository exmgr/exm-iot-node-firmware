#include "tb_soil_moisture_data_json_builder.h"
#include "utils.h"
#include "common.h"

/******************************************************************************
 * Add packet to request
 *****************************************************************************/
RetResult TbSoilMoistureDataJsonBuilder::add(const SoilMoistureData::Entry *entry)
{
	JsonObject json_entry = _root_array.createNestedObject();

	json_entry[SOIL_MOISTURE_DATA_KEY_TIMESTAMP] = (long long)entry->timestamp * 1000;
	JsonObject values = json_entry.createNestedObject("values");

	values[SOIL_MOISTURE_DATA_KEY_VWC] = entry->vwc;
	values[SOIL_MOISTURE_DATA_KEY_TEMPERATURE] = entry->temperature;
	
	// Check the last one, no need to check all, if last doesnt fit into
	// the json doc, doc is full already
	if((values[SOIL_MOISTURE_DATA_KEY_CONDUCTIVITY] = entry->conductivity) == false)
	{
		debug_println(F("Could not add soil moisture sensor data to JSON."));
		return RET_ERROR;
	}

	return RET_OK;
} 