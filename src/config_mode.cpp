#include <Arduino.h>
#include <HardwareSerial.h>
#include "app_config.h"
#include "config_mode.h"
#include "app_config.h"
#include "device_config.h"
#include <SPIFFS.h>

namespace ConfigMode
{
//
// Private functions
//
bool check_button();
RetResult parse_response(char *resp);
void print_error(const __FlashStringHelper *error);
void print_read_value(const char *cmd, const char *val);
void print_read_value(const char *cmd, int val);

RetResult cmd_tb_device_token(char *val, bool read);
RetResult cmd_apn(char *val, bool read);
RetResult cmd_fo_sniffer_id(char *val, bool read);
RetResult cmd_fo_enabled(char *val, bool read);
RetResult cmd_test(char *val, bool read);
RetResult cmd_spiffs_format(char *val, bool read);

//
// Available commands
//

const char *CMD_TB_DEVICE_TOKEN PROGMEM = "TB_DEVICE_TOKEN";
const char *CMD_APN PROGMEM = "APN";
const char *CMD_FO_SNIFFER_ID PROGMEM = "FO_SNIFFER_ID";
const char *CMD_FO_ENABLED PROGMEM = "FO_ENABLED";
const char *CMD_TEST PROGMEM = "TEST";
const char *CMD_SPIFFS_FORMAT PROGMEM = "SPIFFS_FORMAT";

/******************************************************************************
* Check if device needs to enter config mode. This is decided upon a user 
******************************************************************************/
void handle()
{
	// Check if needs to enter config mode
	// Comment whole if/else to force
	if(check_button())
	{
	    debug_println_i(F("Entering config mode"));
	}
	else
	{
	    return;
	}

	debug_println(F("Waiting command input"));

	char buff[256] = "";
	while (1)
	{
		int read = Serial.readBytesUntil('\n', buff, sizeof(buff) - 1);
		if (read > 0)
		{
			buff[read] = '\0';
			parse_response(buff);
		}
	}
}

/******************************************************************************
* Parse response and call corresponding function to handle it
* TODO: Must parse 3 types:
* CMD=x -> Set to X
* CMD=? -> Get
* And pass this parameter to the handling function 
******************************************************************************/
RetResult parse_response(char *resp)
{
	char *cmd = strtok(resp, "=");
	char *val = strtok(NULL, "=");

	RetResult ret = RET_ERROR;

	// Command type determined by syntax
	bool read = false;

	if(val != NULL && strlen(val) == 1 && val[0] == '?')
		read = true;
	

	if (strcmp(cmd, CMD_TB_DEVICE_TOKEN) == 0)
	{
		ret = cmd_tb_device_token(val, read);
	}
	else if (strcmp(cmd, CMD_APN) == 0)
	{
		ret = cmd_apn(val, read);
	}
	else if (strcmp(cmd, CMD_FO_SNIFFER_ID) == 0)
	{
		ret = cmd_fo_sniffer_id(val, read);
	}
	else if (strcmp(cmd, CMD_FO_ENABLED) == 0)
	{
		ret = cmd_fo_enabled(val, read);
	}
	else if (strcmp(cmd, CMD_TEST) == 0)
	{
		ret = cmd_test(val, read);
	}
	else if (strcmp(cmd, CMD_SPIFFS_FORMAT) == 0)
	{
		ret = cmd_spiffs_format(val, read);
	}
	else
	{
		print_error(F("Unkown command"));
	}

	return ret;
}

/******************************************************************************
* Print error message in a format the receiver can parse
******************************************************************************/
void print_error(const __FlashStringHelper *error)
{
	Serial.print(F("Error: "));
	Serial.println(error);
}

/******************************************************************************
* Print OK message to denote successful execution of a command
******************************************************************************/
void print_ok()
{
	Serial.println(F("OK"));
}

/******************************************************************************
* Print a value that has been read with a read command
* Format: []: [value]
******************************************************************************/
void print_read_value(const char *cmd, const char *val)
{
	Serial.print(cmd);
	Serial.print(": ");
	Serial.println(val);
}
void print_read_value(const char *cmd, int val)
{
	Serial.print(cmd);
	Serial.print(": ");
	Serial.println(val, DEC);
}

/******************************************************************************
* Check whether device should enter config mode
* Config mode is enabled when user holds external button for X msec
******************************************************************************/
bool check_button()
{
	pinMode(PIN_CONFIG_MODE_BTN, INPUT_PULLUP);
	pinMode(PIN_LED, OUTPUT);

	bool enter_config_mode = false;

	// Button start hold time
	uint32_t start_hold_ms = 0;
	while (1)
	{
		if (digitalRead(PIN_CONFIG_MODE_BTN) == 0)
		{
			if (start_hold_ms == 0)
			{
				// Keep led on while holding button
				digitalWrite(PIN_LED, 0);
				start_hold_ms = millis();
			}

			if (millis() - start_hold_ms >= CONFIG_MODE_BTN_HOLD_TIME_MS)
			{
				enter_config_mode = true;
				break;
			}
		}
		else
		{
			break;
		}
	}

	// Restore pin to default state to be used by other code
	pinMode(PIN_CONFIG_MODE_BTN, OUTPUT);

	// Turn LED off
	digitalWrite(PIN_LED, 0);

	// Enter config mode
	if (enter_config_mode)
	{
		// Blink fast to indicate mode entry
		for (uint8_t i = 0; i < 3; i++)
		{
			digitalWrite(PIN_LED, 0);
			delay(200);
			digitalWrite(PIN_LED, 1);
			delay(200);
		}

		return true;
	}

	return false;
}

/******************************************************************************
* Handle command: Set TB device token
******************************************************************************/
RetResult cmd_tb_device_token(char *val, bool read)
{
	if(read)
	{
		DeviceConfig::init();
		const char *token = DeviceConfig::get_tb_device_token();

		print_read_value(CMD_TB_DEVICE_TOKEN, token);

		return RET_OK;
	}
	else
	{
		if (val == NULL)
		{
			print_error(F("Value is required"));
			return RET_ERROR;
		}

		// If token empty or exceeds max length
		if (strlen(val) >= sizeof(DeviceConfig::Data::tb_device_token) || strlen(val) == 0)
		{
			print_error(F("Invalid device token length."));
			return RET_ERROR;
		}

		DeviceConfig::init();
		DeviceConfig::set_tb_device_token(val);
		DeviceConfig::commit();

		Serial.println("OK");
	}
}

/******************************************************************************
* Handle command: Set APN
******************************************************************************/
RetResult cmd_apn(char *val, bool read)
{
	if(read)
	{
		DeviceConfig::init();
		const char *token = DeviceConfig::get_cellular_apn();

		print_read_value(CMD_APN, token);

		return RET_OK;
	}
	else
	{
		if (val == NULL)
		{
			print_error(F("Value is required"));
			return RET_ERROR;
		}

		// If token empty or exceeds max length
		if (strlen(val) >= sizeof(DeviceConfig::Data::cellular_apn))
		{
			print_error(F("APN too long"));
			return RET_ERROR;
		}

		DeviceConfig::init();
		DeviceConfig::set_cellular_apn(val);
		DeviceConfig::commit();

		Serial.println("OK");
	}
}

/******************************************************************************
* Handle command: Set FO sniffer weather station ID
******************************************************************************/
RetResult cmd_fo_sniffer_id(char *val, bool read)
{
	if(read)
	{
		DeviceConfig::init();
		char id_hex[6] = "";

		snprintf(id_hex, sizeof(id_hex), "0x%02x", DeviceConfig::get_fo_sniffer_id());

		print_read_value(CMD_FO_SNIFFER_ID, id_hex);

		return RET_OK;
	}
	else
	{
		if (val == NULL)
		{
			print_error(F("Value is required"));
			return RET_ERROR;
		}

		if (strlen(val) != 2)
		{
			print_error(F("Invalid ID. Must be 2-char hex\n"));
			return RET_ERROR;
		}

		for (uint8_t i = 0; i < 2; i++)
		{
			if (!isxdigit(val[i]))
			{
				print_error(F("Invalid ID. Must be 2-char hex\n"));
				return RET_ERROR;
			}
		}

		int id = 0;
		sscanf(val, "%x", &id);

		DeviceConfig::init();
		DeviceConfig::set_fo_sniffer_id(id);
		DeviceConfig::commit();

		Serial.println("OK");
	}
}

/******************************************************************************
* Handle command: Set FO enabled
******************************************************************************/
RetResult cmd_fo_enabled(char *val, bool read)
{
	if(read)
	{
		DeviceConfig::init();

		print_read_value(CMD_FO_ENABLED, DeviceConfig::get_fo_enabled());

		return RET_OK;
	}
	else
	{
		if (val == NULL)
		{
			print_error(F("Value is required"));
			return RET_ERROR;
		}

		int enabled = -1;
		if(sscanf(val, "%d", &enabled) != 1)
		{
			print_error(F("Invalid value provided, must be 0 or 1."));
			return RET_ERROR;
		}

		if(enabled != 0 && enabled != 1)
		{
			print_error(F("Invalid value provided, must be 0 or 1."));
			return RET_ERROR;
		}

		DeviceConfig::init();
		DeviceConfig::set_fo_enabled(enabled);
		DeviceConfig::commit();

		Serial.println("OK");
	}
}

/******************************************************************************
* Handle command: Format SPIFFS partition
******************************************************************************/
RetResult cmd_spiffs_format(char *val, bool read)
{
	Serial.println(F("Formatting..."));
	
	if(SPIFFS.format())
	{
		print_ok();
	}
	else
	{
		print_error(F("Could not format SPIFFS."));
	}
}

/******************************************************************************
* Handle command: Check if device is connected and listening to serial comms
******************************************************************************/
RetResult cmd_test(char *val, bool read)
{
	print_ok();
}

} // namespace ConfigMode