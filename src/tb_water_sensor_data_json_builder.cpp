#include "tb_water_sensor_data_json_builder.h"
#include "utils.h"
#include "common.h"

/******************************************************************************
 * Add packet to request
 *****************************************************************************/
RetResult TbWaterSensorDataJsonBuilder::add(const WaterSensorData::Entry *entry)
{
	JsonObject json_entry = _root_array.createNestedObject();

	json_entry[WATER_SENSOR_DATA_KEY_TIMESTAMP] = (long long)entry->timestamp * 1000;
	JsonObject values = json_entry.createNestedObject("values");

	// Do not include water quality fields if they are all 0
	// The same does not apply for the water level sensor
	// TODO: Should this function fail if no keys are added at all (both wl and quality)
	// Because calling functions keeps track of how many entries there are in the packet
	// Another possible solution is to add an int entry_count member and a getter which will
	// be used by calling function
	if(entry->dissolved_oxygen != 0 || entry->temperature != 0 ||
		entry->ph != 0 || entry->conductivity != 0 || entry->ph != 0 ||
		entry->orp != 0 || entry->pressure != 0 || entry->depth_cm != 0 ||
		entry->depth_ft != 0 || entry->tss != 0)
	{
		values[WATER_SENSOR_DATA_KEY_DISSOLVED_OXYGEN] = entry->dissolved_oxygen;
		values[WATER_SENSOR_DATA_KEY_TEMPERATURE] = entry->temperature;
		values[WATER_SENSOR_DATA_KEY_CONDUCTIVITY] = entry->conductivity;
		values[WATER_SENSOR_DATA_KEY_PH] = entry->ph;
		values[WATER_SENSOR_DATA_KEY_ORP] = entry->orp;
		values[WATER_SENSOR_DATA_KEY_PRESSURE] = entry->pressure;
		values[WATER_SENSOR_DATA_KEY_DEPTH_CM] = entry->depth_cm;
		values[WATER_SENSOR_DATA_KEY_DEPTH_FT] = entry->depth_ft;
		values[WATER_SENSOR_DATA_KEY_TSS] = entry->tss;
	}
	
	// Check the last one, no need to check all, if last doesnt fit into
	// the json doc, doc is full already
	if((values[WATER_SENSOR_DATA_KEY_WATER_LEVEL] = entry->water_level) == false)
	{
		debug_println(F("Could not add sensor data to JSON."));
		return RET_ERROR;
	}

	return RET_OK;
} 