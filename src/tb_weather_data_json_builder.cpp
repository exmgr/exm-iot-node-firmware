#include "tb_atmos41_data_json_builder.h"
#include "utils.h"
#include "common.h"

/******************************************************************************
 * Add packet to request
 *****************************************************************************/
RetResult TbAtmos41DataJsonBuilder::add(const Atmos41Data::Entry *entry)
{
	JsonObject json_entry = _root_array.createNestedObject();

	json_entry[ATMOS41_DATA_KEY_TIMESTAMP] = (long long)entry->timestamp * 1000;
	JsonObject values = json_entry.createNestedObject("values");

	values[ATMOS41_DATA_KEY_SOLAR] = entry->solar;
	values[ATMOS41_DATA_KEY_PRECIPITATION] = entry->precipitation;
	values[ATMOS41_DATA_KEY_STRIKES] = entry->strikes;
	values[ATMOS41_DATA_KEY_WIND_SPEED] = (entry->wind_speed * 100) / 100;
	values[ATMOS41_DATA_KEY_WIND_DIR] = entry->wind_dir;
	values[ATMOS41_DATA_KEY_WIND_GUST] = (entry->wind_gust_speed * 100) / 100;
	values[ATMOS41_DATA_KEY_AIR_TEMP] = round(entry->air_temp * 10) / 10;
	values[ATMOS41_DATA_KEY_VAPOR_PRESSURE] = entry->vapor_pressure;
	values[ATMOS41_DATA_KEY_ATM_PRESSURE] = round(entry->atm_pressure * 10) / 10;
	values[ATMOS41_DATA_KEY_REL_HUMIDITY] = round(entry->rel_humidity * 10) / 10;
	
	// Check the last one, no need to check all, if last doesnt fit into
	// the json doc, doc is full already
	if((values[ATMOS41_DATA_KEY_DEW_POINT] = round(entry->dew_point * 10) / 10) == false)
	{
		debug_println(F("Could not add weather data to JSON."));
		return RET_ERROR;
	}

	return RET_OK;
} 