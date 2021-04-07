#include <Arduino.h>
#include "utils.h"
#include "SPIFFS.h"
#include "app_config.h"
#include "struct.h"
#include "CRC32.h"
#include "Wire.h"
#include "const.h"
#include "device_config.h"
#include "rom/rtc.h"
#include "credentials.h"
#include "common.h"

namespace Utils
{
	/********************************************************************************
	 * Get ESP32 mac address
	 * @param out Output buffer. Must be at least 18 bytes in size
	 * @param size String size
	 ********************************************************************************/
	void get_mac(char *out, uint8_t size)
	{
		uint8_t mac[6];
		
		esp_read_mac(mac, ESP_MAC_WIFI_STA);
		
		snprintf(out, size,  "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], out[5]);
	}

	/******************************************************************************
	 * Log mac address
	 *****************************************************************************/
	void log_mac()
	{
		uint8_t mac[8] = {0};

		esp_read_mac(mac, ESP_MAC_WIFI_STA);

		uint32_t meta1 = (uint32_t)mac;
		uint32_t meta2 = (uint32_t)mac[4];

		Log::log(Log::MAC_ADDRESS, meta1, meta2);
	}

	/********************************************************************************
	 * Print a separator line to the debug output with an optional name/title
	 * Helps with debugging
	 * @param name Text to display inside separator
	 ********************************************************************************/
	void print_separator(const __FlashStringHelper *title)
	{
		const int WIDTH = 55;
		debug_print(F("\x18\x18\x18\x18"));

		int padding = 0;		

		if(title == NULL)
		{
			// Total WIDTH chars will be printed
			padding = WIDTH - 4;
		}
		else
		{
			// 5 chars for #### with space, text length, 1 space
			padding = WIDTH - 5 - strlen((char*)title) - 1;
			if(padding < 0)
			{
				// Add 4 chars in any case
				padding = 4;
			}

			// Space before and after title
			debug_print(F(" "));
			debug_print(F(title));
			debug_print(F(" "));
		}

		// Fill padding with chars
		for(int i = 0; i < padding; i++)
		{
			debug_print('\x18');
		}

		debug_println();
	}

	/********************************************************************************
	 * Print a separator block with a title
	 * Helps with debugging
	 * @param title Text to display inside block
	 ********************************************************************************/
	void print_block(const __FlashStringHelper *title)
	{
		const int WIDTH = 55;
		const __FlashStringHelper *text;

		if(title == NULL)
			text = F("");
		else
			text = title;

		debug_println("=======================================================");
		debug_printf("= %-*s%s \n", WIDTH - 3, text, "=");
		debug_println("=======================================================");
	}

	/******************************************************************************
	 * Print a buffer in hex
	 * @param buff Buffer to print
	 * @param len Buffer length
	 * @param break_pos Add linebreak every X bytes
	 *****************************************************************************/
	void print_buff_hex(uint8_t *buff, int len, int break_pos)
	{
		for(int i = 0; i < len; i++)
		{
			Serial.printf("%02x ", buff[i]);

			if((i + 1) % break_pos == 0)
				Serial.println();
		}
		Serial.println();
	}

	/********************************************************************************
	 * Set console output style
	 * @param style A combination of styles from Style enum
	 *******************************************************************************/
	void serial_style(SerialStyle style)
	{
		debug_printf("\033[%dm", style);
	}

    /********************************************************************************
	 * Calculate CRC32 of a data buffer
	 * @param data Data packet
	 *******************************************************************************/
	uint32_t crc32(uint8_t *buff, uint32_t buff_size)
	{
		return CRC32::calculate((uint8_t*)buff, buff_size);
	}

	/******************************************************************************
	 * Set TCall PMIC config
	 *****************************************************************************/
	RetResult ip5306_set_power_boost_state(bool enable)
	{
		// This function has been repuprosed to enable/disable input power (from 4V4) for the
		// 12v stepup that powers the arduino nano and ultrasonic 3v3 
		// TODO rename? split in to two ?
		// (power boost needs no change before sleep)
        
		debug_println(F("Setting IP5306 power registers"));

		Wire.beginTransmission(IP5306_I2C_ADDR);
		Wire.write(IP5306_REG_SYS_CTL0);
	    Wire.write(IP5306_BOOST_FLAGS); 
   		Wire.endTransmission() ;
		
		if (enable)
		{
			// Enable SY8089 4V4 for SIM800 which is also powering 12V Stepup
			pinMode(PIN_GSM_POWER_ON, OUTPUT);
			digitalWrite(PIN_GSM_POWER_ON, HIGH);
		}
		else 
		{
			pinMode(PIN_GSM_POWER_ON, OUTPUT);
			digitalWrite(PIN_GSM_POWER_ON, LOW);
		}
	
		//TODO check success?
		return RET_OK;


		// if(Wire.requestFrom(IP5306_I2C_ADDR, 1))
		// {
		// 	data = Wire.read();

		// 	Wire.beginTransmission(IP5306_I2C_ADDR);
		// 	Wire.write(IP5306_REG_SYS_CTL0);

		// 	if(enable)
		// 		Wire.write(data |  IP5306_BOOST_OUT_BIT); 
		// 	else
		// 		Wire.write(data & (~IP5306_BOOST_OUT_BIT));  

		// 	Wire.endTransmission();

		// 	debug_println(F("Setting power boost state successful."));
		// 	return RET_OK;
		// }

		// Utils::serial_style(STYLE_RED);
		// debug_println(F("Setting power boost state failed."));
		// Utils::serial_style(STYLE_RESET);

		// return RET_ERROR;;
	}

	/******************************************************************************
	 * Explode URL into host, port and path
	 * Fails if not BOTH host and path parts are found (port is optional)
	 *****************************************************************************/
	RetResult url_explode(char *in, int *port_out, char *host_out, int host_max_size, char *path_out, int path_max_size)
	{
		char *cursor = in;
		
		// Check if there's a protocol and ignore
		char *proto_start = strstr(cursor, "//");
		if(proto_start != nullptr)
		{
			// Ignore protocol part
			cursor = proto_start + 2;
		}
		
		char *host_end = strchr(cursor, ':');
		char *port_start = nullptr;
		
		// No : found, so host will be up to the next /, find it
		if(host_end == nullptr)
		{
			host_end = strchr(cursor, '/');
		}
		else
		{
			// Host end found and there's a port part too
			port_start = host_end;
		}

		// No next / found means host ends at end of string but also means there's no resource path, abort
		if(host_end != nullptr)
		{
			int host_len = host_end - cursor;
			if(host_len > host_max_size)
			{
				debug_println(F("Host output buffer too small."));
				return RET_ERROR;
			}
			
			if(host_out != nullptr)
			{
				strncpy(host_out, cursor, host_len);
			}
			
			// Set cursor
			cursor = host_end;
		}
		
		// Get port if it exists
		if(port_start != nullptr)
		{
			char *port_end = strchr(port_start, '/');
			if(port_end != nullptr)
			{
				char port_str[6] = "";
				
				strncpy(port_str, port_start, 5);

				if(port_out != nullptr)
				{
					sscanf(port_start+1, "%5d", port_out);
				}
				
			}
			else
			{
				// Port end not found
				return RET_ERROR;
			}
			
			cursor = port_end;
		}
		
		// The rest is path
		// If its only a single char (a '/'), there's no path, abort
		int path_len = strlen(cursor);
		if(path_len < 2)
		{
			return RET_ERROR;
		}
		else if(path_len > path_max_size)
		{
			debug_println(F("Path output buffer too small."));
			return RET_ERROR;
		}

		if(path_out != nullptr)
		{
			strncpy(path_out, cursor, path_max_size);
		}
		
		return RET_OK;
	}

	/******************************************************************************
	 * Build TB attributes API URL for this device
	 *****************************************************************************/
	// TODO: Useless?? Can be inline
	RetResult tb_build_attributes_url_path(char *buff, int buff_size)
	{
		snprintf(buff, buff_size, TB_SHARED_ATTRIBUTES_URL_FORMAT, DeviceConfig::get_tb_device_token());

		return RET_OK;
	}

	/******************************************************************************
	 * Build TB telemetry API URL for this device
	 *****************************************************************************/
	RetResult tb_build_telemetry_url_path(char *buff, int buff_size)
	{
		snprintf(buff, buff_size, TB_TELEMETRY_URL_FORMAT, DeviceConfig::get_tb_device_token());

		return RET_OK;
	}
	
	/******************************************************************************
	 * Mark restart as clean and restart
	 *****************************************************************************/
	RetResult restart_device()
	{
		DeviceConfig::set_clean_reboot(true);
		DeviceConfig::commit();

		ESP.restart();
	}

	/******************************************************************************
	* Read ADC and convert to mV
	* @param pin ADC pin
	* @param samples Number of samples to read
	* @param sampling_delay_ms mS to wait between samples
	* @return Value in mV
	******************************************************************************/
	int read_adc_mv(uint8_t pin, int samples, int sampling_delay_ms)
	{
		if(samples < 0)
		{
			return -1;
		}

		if(sampling_delay_ms < 0)
		{
			return -1;
		}

		int level_raw = 0;
		for(int i = 0; i < samples; i++)
		{
			level_raw += analogRead(pin);
			delay(sampling_delay_ms);
		}
		level_raw /= samples;

		// Convert to mV
		int mv = ((float)3600 / 4096) * level_raw;

		return mv;
	}

	/******************************************************************************
	 * Print reset reason
	 *****************************************************************************/
	void print_reset_reason()
	{
		RESET_REASON reason = rtc_get_reset_reason(0);

		switch(reason)
		{
			case RESET_REASON::POWERON_RESET:
				debug_println(F("Power ON")); 
				break;
			case RESET_REASON::SW_RESET:
				debug_println(F("Software reset (digital core)"));
				break;
			case RESET_REASON::OWDT_RESET:
				debug_println(F("Legacy watchdog (digital core)"));
				break;
			case RESET_REASON::DEEPSLEEP_RESET:
				debug_println(F("Deep sleep (digital core)"));
				break;
			case RESET_REASON::SDIO_RESET:
				debug_println(F("SLC module (digital core)"));
				break;
			case RESET_REASON::TG0WDT_SYS_RESET:
				debug_println(F("TG0 Watchdog (digital core)"));
				break;
			case RESET_REASON::TG1WDT_SYS_RESET:
				debug_println(F("TG1 Watchdog (digital core)"));
				break;
			case RESET_REASON::RTCWDT_SYS_RESET:
				debug_println(F("RTC Watchdog (digital core)"));
				break;
			case RESET_REASON::INTRUSION_RESET:
				debug_println(F("Intrusion tested to reset CPU"));
				break;
			case RESET_REASON::TGWDT_CPU_RESET:
				debug_println(F("Time group reset CPU"));
				break;
			case RESET_REASON::SW_CPU_RESET:
				debug_println(F("Software reset (CPU)"));
				break;
			case RESET_REASON::RTCWDT_CPU_RESET:
				debug_println(F("RTC Watchdog (CPU)"));
				break;
			case RESET_REASON::EXT_CPU_RESET:
				debug_println(F("Ext (CPU)"));
				break;
			case RESET_REASON::RTCWDT_BROWN_OUT_RESET:
				debug_println(F("Brown out"));
				break;
			case RESET_REASON::RTCWDT_RTC_RESET:
				debug_println(F("RTC watch dog reset (digital core and rtc module)"));
				break;
		}
	}

	/******************************************************************************
	 * Print falgs and their vals
	 *****************************************************************************/
	void print_flags()
	{
		Utils::print_separator(F("Flags"));

		debug_print(F("- Debug mode: "));
		if(FLAGS.DEBUG_MODE)
		{
			serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- WiFi Debug console: "));
		if(FLAGS.WIFI_DEBUG_CONSOLE_ENABLED)
		{
			serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- NBIoT mode: "));
		if(FLAGS.NBIOT_MODE)
		{
			serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- Sleep minutes treated as seconds: "));
		if(FLAGS.SLEEP_MINS_AS_SECS)
		{
			serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- Water quality sensor: "));
		if(FLAGS.WATER_QUALITY_SENSOR_ENABLED)
		{
			serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- Water level sensor: "));
		if(FLAGS.WATER_LEVEL_SENSOR_ENABLED)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- Atmos41 weather station: "));
		if(FLAGS.ATMOS41_ENABLED)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- Water Quality measurements return dummy values: "));
		if(FLAGS.MEASURE_DUMMY_WATER_QUALITY)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- Water Level measurements return dummy values: "));
		if(FLAGS.MEASURE_DUMMY_WATER_LEVEL)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- Weather measurements return dummy values: "));
		if(FLAGS.MEASURE_DUMMY_WEATHER)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- External RTC: "));
		if(FLAGS.EXTERNAL_RTC_ENABLED)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- Force normal battery mode: "));
		if(FLAGS.BATTERY_FORCE_NORMAL_MODE)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- GSM AT command output to console: "));		
		if(PRINT_GSM_AT_COMMS)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}

		debug_print(F("- Lightning sensor: : "));		
		if(FLAGS.LIGHTNING_SENSOR_ENABLED)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("Enabled"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			debug_println_e(F("Disabled"))
		}


		Utils::print_separator(NULL);
	}

	/******************************************************************************
	* Check if element exists in array
	* @param val Value to check against
	* @param arr Array
	* @param size Size of array
	* @return Position of element in array, -1 if not found
	******************************************************************************/
	template<typename T>
	int in_array(T val, const T arr[], int size)
	{
		if(size < 1) return -1;

		for (int i = 0; i < size; i++)
		{
			if(arr[i] == val)
			{
				return i;
			}
		}

		return -1;
	}
	template int in_array<int>(int val, const int arr[], int size);

	/******************************************************************************
	 * Checks fw parameters that would prevent the device from operating correctly
	 * if not set properly.
	 *****************************************************************************/
	int boot_self_test()
	{
		debug_println(F("Boot self test"));
		RetResult ret = RET_OK;

		//
		// Wake up schedule
		//
		const DeviceConfig::Data *device_config = DeviceConfig::get();

		if(!SleepScheduler::schedule_valid(WAKEUP_SCHEDULE_DEFAULT))
		{
			debug_println(F("Default wakeup schedule invalid."));
			ret = RET_ERROR;
		}

		if(!SleepScheduler::schedule_valid(WAKEUP_SCHEDULE_BATT_LOW))
		{
			debug_println(F("Battery low schedule invalid."));
			ret = RET_ERROR;
		}

		if(ret == RET_OK)
		{
			Utils::serial_style(STYLE_GREEN);
			debug_println(F("Boot self test passed"));
			Utils::serial_style(STYLE_RESET);
		}
		else
		{
			Utils::serial_style(STYLE_RED);
			debug_println(F("Boot self test failed"));
			Utils::serial_style(STYLE_RESET);
		}

		return ret;
	}

	/******************************************************************************
	 * Check credentials in DeviceConfig, if not populated use FallBack
	 ******************************************************************************/
	void check_credentials()
	{
		// Check TB device token
		if(strlen(DeviceConfig::get_tb_device_token()) == 0 || 
		strcmp(DeviceConfig::get_tb_device_token(), FALLBACK_TB_DEVICE_TOKEN) == 0 )
		{
			debug_print(DEBUG_LEVEL_ERROR_STYLE);
			Utils::print_separator(NULL);

			debug_println(F("TB device token not configured, fallback token is used!"));

			if(strlen(DeviceConfig::get_tb_device_token()) == 0)
			{
				DeviceConfig::set_tb_device_token((char*)FALLBACK_TB_DEVICE_TOKEN);
				DeviceConfig::commit();
			}

			Utils::print_separator(NULL);
			Utils::serial_style(STYLE_RESET);
		}

		// Check APN
		if(strlen(DeviceConfig::get_cellular_apn()) == 0 ||
		strcmp(DeviceConfig::get_cellular_apn(), FALLBACK_CELL_APN) == 0 )
		{
			debug_print(DEBUG_LEVEL_ERROR_STYLE);
			Utils::print_separator(NULL);

			debug_println(F("Cellular provider APN not configured, fallback APN is used!"));

			if(strlen(DeviceConfig::get_cellular_apn()) == 0)
			{
				DeviceConfig::set_cellular_apn((char*)FALLBACK_CELL_APN);
				DeviceConfig::commit();				
			}

			Utils::print_separator(NULL);
			Utils::serial_style(STYLE_RESET);
		}
	}

	/******************************************************************************
	 * Convert degrees to radians
	 *****************************************************************************/
	float deg_to_rad(float deg)
	{
		return deg * M_PI / 180;
	}

	/******************************************************************************
	 * Convert radians to degrees
	 *****************************************************************************/
	float rad_to_deg(float rad)
	{
		return rad * (180 / M_PI);
	}


	/******************************************************************************
	 * Build TB telemetry JSON for IPFS entry
	 *****************************************************************************/
	void build_ipfs_file_json(String hash, uint32_t timestamp, char *buff, int buff_size)
	{
		StaticJsonDocument<1024> _json_doc;
		JsonArray _root_array;
		_json_doc.clear();
		_root_array = _json_doc.template to<JsonArray>();
		JsonObject json_entry = _root_array.createNestedObject();

		json_entry[WATER_SENSOR_DATA_KEY_TIMESTAMP] = (long long)timestamp * 1000;
		JsonObject values = json_entry.createNestedObject("values");

		values["ipfs_hash"] = hash;

		serializeJson(_json_doc, buff, buff_size);
	}
}