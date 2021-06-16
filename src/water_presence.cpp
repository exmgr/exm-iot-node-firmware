 #include "const.h"
#include "app_config.h"
#include "water_presence.h"
#include "water_sensor_data.h"
#include "utils.h"
#include "common.h"
#include "log.h"
#include "log_codes.h"

namespace WaterPresence
{
	//
	// Private functions
	//

	/******************************************************************************
	 * Init
	 *****************************************************************************/	
	RetResult init()
	{
        pinMode(PIN_WATER_PRESENCE, INPUT);
	}

	/******************************************************************************
	 * Read water level sensor and populate SensorData entry structure
	 * Use appropriate function depending on the WATER_LEVEL_INPUT_CHANNEL switch
	 *****************************************************************************/
	RetResult measure(WaterSensorData::Entry *data)
	{
        data->presence = digitalRead(PIN_WATER_PRESENCE);

        return RET_OK;
	}
}