#ifndef OTA_H
#define OTA_H

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"
#include "const.h"
#include "struct.h"

namespace OTA
{
	RetResult handle_rc_data(JsonObject rc_json);
	RetResult handle_first_boot();
}

#endif