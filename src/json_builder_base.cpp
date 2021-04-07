#include "json_builder_base.h"
#include "log.h"
#include "water_sensor_data.h"
#include "soil_moisture_data.h"
#include "sdi12_log.h"
#include "atmos41_data.h"
#include "lightning_data.h"
#include "fo_data.h"
#include "common.h"
#include "common.h"

/******************************************************************************
 * Default constructor
 * Initialize ArduinoJSON document on construct
 *****************************************************************************/
template <typename TStruct, int TDocSize>
JsonBuilderBase<TStruct, TDocSize>::JsonBuilderBase()
{
    reset();
}

/******************************************************************************
 * Build and write output json to buffer
 *****************************************************************************/
template <typename TStruct, int TDocSize>
RetResult JsonBuilderBase<TStruct, TDocSize>::build(char *buff_out, int buff_size, bool beautify)
{ 	
	if(beautify)
	{
		serializeJsonPretty(_json_doc, buff_out, buff_size);
	}
	else
	{
		serializeJson(_json_doc, buff_out, buff_size);
	}

	return RET_OK;
}

/******************************************************************************
 * Reset object for reuse
 *****************************************************************************/
template <typename TStruct, int TDocSize>
RetResult JsonBuilderBase<TStruct, TDocSize>::reset()
{
	_json_doc.clear();
	_root_array = _json_doc.template to<JsonArray>();

	return RET_OK;
}

/******************************************************************************
 * Serialize beautified and print
 * Used for debugging
 *****************************************************************************/
template <typename TStruct, int TDocSize>
void JsonBuilderBase<TStruct, TDocSize>::print()
{
    // Buff size should cover all cases
	// TODO: Decide buff size
    char buff[2048] = {0};

    build(buff, sizeof(buff), true);

    debug_println(buff);
    debug_print(F("Length: "));
    debug_println(strlen(buff), DEC);
}

template <typename TStruct, int TDocSize>
bool JsonBuilderBase<TStruct, TDocSize>::is_empty()
{
	return _json_doc.size() < 1;
}

/******************************************************************************
 * JsonDoc accessor
 *****************************************************************************/
template <typename TStruct, int TDocSize>
StaticJsonDocument<TDocSize>* JsonBuilderBase<TStruct, TDocSize>::get_json_doc()
{
	return &_json_doc;
}


// Forward declarations
template class JsonBuilderBase<Log::Entry, LOG_JSON_DOC_SIZE>;
template class JsonBuilderBase<WaterSensorData::Entry, WATER_SENSOR_DATA_JSON_DOC_SIZE>;
template class JsonBuilderBase<Atmos41Data::Entry, ATMOS41_DATA_JSON_DOC_SIZE>;
template class JsonBuilderBase<SoilMoistureData::Entry, ATMOS41_DATA_JSON_DOC_SIZE>;
template class JsonBuilderBase<FoData::StoreEntry, FO_DATA_JSON_DOC_SIZE>;
template class JsonBuilderBase<SDI12Log::Entry, ATMOS41_DATA_JSON_DOC_SIZE>;
template class JsonBuilderBase<LightningData::Entry, LIGHTNING_DATA_JSON_DOC_SIZE>;