#include "tb_fo_data_json_builder.h"
#include "utils.h"
#include "common.h"
#include "fo_data.h"

/******************************************************************************
 * Add packet to request
 *****************************************************************************/
RetResult TbFoDataJsonBuilder::add(const FoData::StoreEntry *entry)
{
	JsonObject json_entry = _root_array.createNestedObject();

	json_entry[FO_DATA_KEY_TIMESTAMP] = (long long)entry->timestamp * 1000;
	JsonObject values = json_entry.createNestedObject("values");

	values[FO_DATA_KEY_PACKETS] = entry->packets;
	values[FO_DATA_KEY_TEMP] = entry->temp;
	values[FO_DATA_KEY_HUMIDITY] = entry->hum;	
	values[FO_DATA_KEY_RAIN] = entry->rain;
	values[FO_DATA_KEY_RAIN_RATE_HR] = entry->rain_hourly;
	values[FO_DATA_KEY_WIND_DIR] = entry->wind_dir;
	values[FO_DATA_KEY_WIND_SPEED] = entry->wind_speed;
	values[FO_DATA_KEY_WIND_GUST] = entry->wind_gust;
	values[FO_DATA_KEY_UV] = entry->uv;
	values[FO_DATA_KEY_UV_INDEX] = entry->uv_index;
	values[FO_DATA_KEY_SOLAR_RADIATION] = entry->solar_radiation;

	// Check the last one, no need to check all, if last doesnt fit into
	// the json doc, doc is full already
	if(((values[FO_DATA_KEY_LIGHT] = entry->light)) == false)
	{
		debug_println(F("Could not add FO data to JSON."));
		return RET_ERROR;
	}

	return RET_OK;
} 