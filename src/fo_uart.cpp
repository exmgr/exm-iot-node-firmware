#include "fo_uart.h"
#include "common.h"
#include "rtc.h"
#include "fo_data.h"
#include "fo_buffer.h"
#include "device_config.h"

namespace FoUart
{
	/** Time of last valid packet */
	uint32_t _last_packet_tstamp = 0;

    /** Last received packet */
	// TODO: Useless along with get_packet??? Never needed externall
	FoDecodedPacket _last_decoded_packet = {0};

	/** Holds last X parsed packets */
	FoBuffer _packet_buff;

	/** 
	 * Number of successive failures to sniff weather station
	 */ 
	uint8_t _rx_failures = 0;

    /******************************************************************************
	 * Init
	 *****************************************************************************/
    RetResult init()
    {
		pinMode(PIN_FO_UART_REQ_DATA, OUTPUT);
		digitalWrite(PIN_FO_UART_REQ_DATA, 0);
    }

    /******************************************************************************
    * Calculate secs to when the next packet will be available for retrieving
    ******************************************************************************/
    int calc_secs_to_next_packet()
    {
        uint32_t now = RTC::get_timestamp();

		// First run
		if(_last_packet_tstamp == 0)
		{
			// ASAP so we can sync
			return 1;
		}

		uint32_t secs_since_last_packet = now - _last_packet_tstamp;

		Serial.print(F("Seconds since last packet: "));
		Serial.println(secs_since_last_packet, DEC);

		// Divide by packet interval and count from there
		int secs_to_next = secs_since_last_packet - ( (secs_since_last_packet / FO_SNIFFER_PACKET_INTERVAL_SEC) * FO_SNIFFER_PACKET_INTERVAL_SEC);
		secs_to_next = FO_UART_PACKET_INTERVAL_SEC - secs_to_next;

		if(secs_to_next <= 0)
		{
            return 0;
		}
		else
		{
			return secs_to_next;
		}
    }

    /******************************************************************************
    * Request Weather Station to output data via UART by toggling pin
	* @return True when packet fully received, false when packet not received or
	* 		  received partially
    ******************************************************************************/
	RetResult request_packet()
	{
		Serial.println(F("Requesting data from FO weather station (UART)."));

		// Setting request pin to HIGH makes the weather station output data via UART
		// Data can be requested only once every 16sec
		// Must be set back to 0 when done
		pinMode(PIN_FO_UART_REQ_DATA, OUTPUT);
		digitalWrite(PIN_FO_UART_REQ_DATA, 0);
		delay(10);
		digitalWrite(PIN_FO_UART_REQ_DATA, 1);

		HardwareSerial uart(FO_UART_PORT);
		uart.begin(9600, SERIAL_8N1, PIN_FO_UART_RX, 999, false, FO_UART_RX_TIMEOUT_MS);

		// Rx buffer, must fit 1 line of response
		char buff[128] = "";

		// Index of current parsed field in param array
		// int cur_field_index = 0;

		// Number of recognized params found so far
		int found_param_count = 0;

		// Return result
		RetResult ret = RET_ERROR;

		// Break reading loops
		bool break_read = false;

		// Receive data until done or timeout
		uint32_t start_ms = millis();
		while(millis() - start_ms < FO_UART_RX_TIMEOUT_MS)
		{
			if(break_read)
			{
				break;
			}

			if(!uart.available())
				continue;
			int len = 0;
			
			// while(len = uart.readBytes(buff, sizeof(buff)-1))
			uart.setTimeout(FO_UART_RX_TIMEOUT_MS);
			while((len = uart.readBytesUntil('\n', buff, sizeof(buff)-1)))
			{
				buff[len] = '\0';

				// Print received line
				// debug_println(buff);

				// Skip until first field is found
				char param_name[25] = "";
				// const char *param_name = FO_UART_RESPONSE_PARAM_NAMES[cur_field_index];
				float param_val = 0;

				// Build format string eg. "WindDir = %f"
				// snprintf(format, sizeof(format), "%s = %%f", param_name);

				// Parse from current response line
				// At first, skip lines until first field found. After that fields are expected
				// to be returned one by one in the order specified by param array, otherwise fail
				int found = sscanf(buff, "%s = %f", param_name, &param_val);

				if(found == 0)
				{
					if(found_param_count == 0)
					{
						// Still looking for start, keep looking
						memset(buff, 0, sizeof(buff));
						continue;
					}
					else
					{
						// Next value is not what was expected, fail
						ret = RET_ERROR;
						break_read = true;

						debug_println_e(F("No param returned or invalid format."));
						Log::log(Log::FO_ERROR_UNEXPECTED_VALUE, found_param_count);

						debug_println(F("Found params: "));
						debug_println(found_param_count, DEC);

						memset(buff, 0, sizeof(buff));

						break;
					}
				}
				// Parsed both param & value
				else if(found == 2)
				{
					found_param_count++;

					if(strcmp(param_name, FO_UART_RESPONSE_PARAM_NAMES[FO_UART_FIELD_WIND_DIR]) == 0)
					{
						_last_decoded_packet.wind_dir = param_val;
					}
					else if(strcmp(param_name, FO_UART_RESPONSE_PARAM_NAMES[FO_UART_FIELD_WIND_SPEED]) == 0)
					{
						_last_decoded_packet.wind_speed = param_val * FO_WIND_SPEED_COEFF;
					}
					else if(strcmp(param_name, FO_UART_RESPONSE_PARAM_NAMES[FO_UART_FIELD_WIND_GUST]) == 0)
					{
						_last_decoded_packet.wind_gust = param_val * FO_WIND_GUST_COEFF;
					}
					else if(strcmp(param_name, FO_UART_RESPONSE_PARAM_NAMES[FO_UART_FIELD_TEMP]) == 0)
					{
						_last_decoded_packet.temp = (float)param_val/10;
					}
					else if(strcmp(param_name, FO_UART_RESPONSE_PARAM_NAMES[FO_UART_FIELD_HUMIDITY]) == 0)
					{
						_last_decoded_packet.hum = param_val;
					}
					else if(strcmp(param_name, FO_UART_RESPONSE_PARAM_NAMES[FO_UART_FIELD_LIGHT]) == 0)
					{
						_last_decoded_packet.light = param_val * 10;
					}
					else if(strcmp(param_name, FO_UART_RESPONSE_PARAM_NAMES[FO_UART_FIELD_UV_INDEX]) == 0)
					{
						_last_decoded_packet.uv_index = param_val;
					}
					else if(strcmp(param_name, FO_UART_RESPONSE_PARAM_NAMES[FO_UART_FIELD_RAIN_COUNTER]) == 0)
					{
						_last_decoded_packet.rain = param_val * FO_RAIN_MM_PER_CLICK;
					}
					else
					{
						// Unknown param, decrease count
						found_param_count--;
					}

					if(found_param_count == FO_UART_PARAM_COUNT)
					{
						// All params read, break
						Serial.println(F("All FO params read, stopping."));
						break_read = true;
						break;
					}
				}
			}
		}

		// Calculate derived values
		_last_decoded_packet.solar_radiation = _last_decoded_packet.light * FO_SNIFFER_LUX_TO_SOLAR_RADIATION_COEFF;

		// Not all params read
		if(FO_UART_PARAM_COUNT != found_param_count)
		{
			debug_print_e(F("Invalid FO param count: "));
			debug_println_e(found_param_count, DEC);
			Log::log(Log::FO_ERROR_INVALID_PARAM_COUNT, found_param_count);

			ret = RET_ERROR;
		}
		else
		{
			ret = RET_OK;

			// Update time of last received packet
			_last_packet_tstamp = RTC::get_timestamp();
		}

		// Reset request pin back to 0 otherwise weather station will output data immediately when
		// available
		digitalWrite(PIN_FO_UART_REQ_DATA, 0);

		return ret;
	}

	/******************************************************************************
	* Handle scheduled receive event (triggered on an interval)
	* Request data from weather station and add to buffer
	******************************************************************************/
	RetResult handle_scheduled_event()
	{
		if(RET_OK == FoUart::request_packet())
		{
			Serial.println(F("Packet received!"));
			FoBuffer::print_packet(FoUart::get_last_packet());

			_packet_buff.add_packet(&_last_decoded_packet);

			// Reset failure counter
			_rx_failures = 0;

			return RET_OK;
		}
		else
		{
			_rx_failures++;

			debug_print_e(F("RX failed, failures: "));
			debug_println_e(_rx_failures, DEC);

			// Disable FO weather station after X successive failures
			// Admin must reactivate it again via remote control
			if(_rx_failures >= FO_FAILED_RX_THRESHOLD)
			{
				debug_println_e(F("FineOffset RX failures reached threshold, disabling FO weather station."));

				DeviceConfig::set_fo_enabled(false);
				DeviceConfig::commit();

				Log::log(Log::FO_DISABLED_RX_FAILED, _rx_failures);

				_rx_failures = 0;
			}

			return RET_ERROR;
		}
	}

	/******************************************************************************
	* Get last received packet
	******************************************************************************/
	FoDecodedPacket* get_last_packet()
	{
		return &_last_decoded_packet;
	}

	/******************************************************************************
	* Commit buffer
	******************************************************************************/
	RetResult commit_buffer()
	{
		return _packet_buff.commit_buffer();
	}
}