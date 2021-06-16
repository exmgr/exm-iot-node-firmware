#include "fo_sniffer.h"
#include <SPI.h>
#include "rtc.h"
#include "common.h"
#include "data_store.h"
#include "device_config.h"
#include <driver/gpio.h>
#include "globals.h"
#include "fo_data.h"
#include "log.h"
#include "fo_buffer.h"

namespace FoSniffer
{
	/**
	 * Private functions
	 */
	RetResult decode_packet(uint8_t *buff, FoDecodedPacket *decoded);
	uint8_t calc_crc(uint8_t const buff[], int len, uint8_t polynomial, uint8_t init);
	uint8_t calc_checksum(uint8_t const buff[]);	
	uint8_t uv_to_index(int uv);

	/** Time of last valid packet */
	uint32_t _last_packet_tstamp = 0;

	/** Last received packet */
	// TODO: Useless along with get_packet??? Never needed externall
	FoDecodedPacket _last_decoded_packet = {0};

	/** Used by the RF module. Custom SPI object must be used to set pins */
	SPIClass _spi;

	/**
	 * RF object used for sniffing
	 * INT1 pin is not used. Possible error when set to a valid pin so a large number is
	 * set instead
	*/
	RFM95 _rf = new Module(PIN_RF_SS, PIN_RF_DI0, 444, _spi);

	/**
	 * Sniffer currently in sync with the tx interval of the weather statopm
	 * Considered in sync when packet received succesfully at expected time (ie. after
	 * waiting known number of seconds from last packet)
	*/
	bool _in_sync = false;

	/** 
	 * Number of successive failures to sniff weather station
	 */ 
	uint8_t _rx_failures = 0;

	/**
	 * Tstamp of last sync attempt. 
	 */
	uint32_t _last_sync_tstamp = 0;

	/** Holds last X decoded packets */
	FoBuffer _packet_buff;

	/******************************************************************************
	 * Init
	 *****************************************************************************/
	RetResult init()
	{
		pinMode(PIN_RF_DI0, INPUT);

		_spi.begin(PIN_RF_SCK, PIN_RF_MISO, PIN_RF_MOSI, PIN_RF_SS);


		int state = _rf.beginFSK();

		if (state != ERR_NONE)
		{
			Serial.print(F("Could not init FSK mode, code: "));
			Serial.println(state);

			return RET_ERROR;
		}

		state = _rf.setFrequency(FO_SNIFFER_FREQ);
		state = _rf.setBitRate(FO_SNIFFER_BIT_RATE);
		state = _rf.setFrequencyDeviation(FO_SNIFFER_FREQ_DEV);
		state = _rf.setRxBandwidth(FO_SNIFFER_RX_BW);
		state = _rf.setEncoding(0);
		// state = _rf.packetMode();

		_rf.setNodeAddress(DeviceConfig::get_fo_sniffer_id());

		state = _rf.setSyncWord((uint8_t*)FO_SNIFFER_SYNC_WORD, 2);

		if (state != ERR_NONE)
		{
			Serial.print(F("Unable to set configuration, code "));
			Serial.println(state);
			return RET_ERROR;
		}

		// if(fsk.disableAddressFiltering() != ERR_NONE)
		// {
		// 	Serial.println(F("Could not disable address filtering"));
		// 	while(1);
		// }

		// if(fsk.setPreambleLength(5) != ERR_NONE)
		// {
		// 	Serial.println(F("Could not disable preamble"));
		// 	while(1);
		// }

		if (_rf.setCRC(false) != ERR_NONE)
		{
			Serial.println(F("Could not disable crc"));
			return RET_ERROR;
		}

		if (_rf.setPreambleLength(5) != ERR_NONE)
		{
			Serial.println(F("Could not set preamble"));
			return RET_ERROR;
		}

		return RET_OK;
	}
	
	/******************************************************************************
	 * Calculate time left to when sniffer needs to start sniffing
	 * Takes into account tolerance (starts a little earlier to account for time drift)
	 * @return Seconds to next sniff event. -1 when time unknown
	 *****************************************************************************/
	int calc_secs_to_next_sniff()
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
		secs_to_next = FO_SNIFFER_PACKET_INTERVAL_SEC - secs_to_next - FO_SNIFFER_WAIT_PACKET_EARLY_WAKEUP_SEC;

		if(secs_to_next <= 0)
		{
			// We're late by X seconds becase early wake up tolerance is larger than time left to next sniff
			// Skip to next sniff wakeup event
			return (secs_to_next * -1) + (FO_SNIFFER_PACKET_INTERVAL_SEC - FO_SNIFFER_WAIT_PACKET_EARLY_WAKEUP_SEC);
		}
		else
		{
			return secs_to_next;
		}
	}

	/******************************************************************************
	 * Handle sniff event
	 *****************************************************************************/
	RetResult handle_sniff_event()
	{
		RetResult ret = RET_ERROR;

		if(_in_sync)
		{
			Serial.println("In sync, waiting for packet");
			ret = sleep_to_packet(FO_SNIFFER_PACKET_WAIT_TIME_MS);

			if(ret == RET_ERROR)
			{
				Log::log(Log::FO_SNIFFER_SNIFF_FAILED);
				_in_sync = false;
			}
			else
			{
				// Success, store packet
				_packet_buff.add_packet(&_last_decoded_packet);
				_in_sync = true;
			}
		}

		if(!_in_sync)
		{
			debug_println(F("Not in sync, trying to sync."));

			Log::log(Log::FO_SNIFFER_NOT_IN_SYNC);

			ret = sleep_to_packet(FO_SNIFFER_SYNC_WAIT_TIME_MS);
			if(ret == RET_ERROR)
			{
				debug_println(F("Could not sync, aborting"));

				Log::log(Log::FO_SNIFFER_SYNC_FAILED);
			}
			else
			{
				_packet_buff.add_packet(&_last_decoded_packet);
				_in_sync = true;
				
				Log::log(Log::FO_SNIFFER_SCAN_RESULT, _last_decoded_packet.node_address);

			}
		}

		if(ret == RET_ERROR)
		{
			_rx_failures++;

			debug_print_e(F("FO rx failed, failures: "));
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
		}
		else
		{
			// Reset failure counter
			_rx_failures = 0;
		}

		return ret;
	}

	/******************************************************************************
	 * Decode a buffer into a FoDecodedPacket struct
	 *****************************************************************************/
	RetResult decode_packet(uint8_t *buff, FoDecodedPacket *decoded)
	{
		decoded->node_address = buff[1];

		// Wind direction
		decoded->wind_dir = ((buff[3] & 0x80) << 1) | buff[2];

		// Temperature
		uint16_t temp_raw = ((buff[3] & 0b111) << 8) | buff[4];

		// 10.5C = 0x1F9, -10.5C = 0x127 (val has an offset of 400 added)
		decoded->temp = (float)(temp_raw - 400) / 10;

		// Humidity
		decoded->hum = buff[5];

		// Wind speed
		// If wsp flag is set
		uint8_t extra_wind_speed_bits = 0;
		if(buff[3] & 0b1000000)
		{
			// Wind speed is 9bit
			Serial.println(F("WSP flag SET!!"));
			extra_wind_speed_bits =  (buff[3] & 0b10000) >> 4;
		}
		else
		{
			// Wind speed is 10bit
			Serial.println(F("WSP flag NOT SET!!"));
			extra_wind_speed_bits =  (buff[3] & 0b110000) >> 4;
		}

		// decoded->wind_speed = buff[6] * FO_WIND_SPEED_COEFF;
		decoded->wind_speed = ((extra_wind_speed_bits << 8) | buff[6]) * 0.0644;

		// Gust speed
		decoded->wind_gust = buff[7] * FO_WIND_GUST_COEFF;

		// Rainfaill
		decoded->rain = ((buff[8] << 8) | buff[9]) * FO_RAIN_MM_PER_CLICK; // Convert rain counter clicks to mm

		// UV
		decoded->uv = (buff[10] << 8) | buff[11];

		// UV Index
		decoded->uv_index = uv_to_index(decoded->uv);

		// Illuminance (lux)
		decoded->light = (buff[12] << 16) | (buff[13] << 8) | buff[10];
		// Convert to lux
		decoded->light /= 10;

		// Solar radiation (W/m^2)
		decoded->solar_radiation = decoded->light * FO_SNIFFER_LUX_TO_SOLAR_RADIATION_COEFF;

		// CRC
		decoded->crc = buff[15];
		decoded->checksum = buff[16];

		uint8_t calced_crc = calc_crc((uint8_t*)decoded, 15, 0x31, 0),
		calced_checksum = calc_checksum((uint8_t*)decoded);

		// Return false if integrity tests failed
		if(decoded->crc != calc_crc(buff, 15, 0x31, 0) && decoded->checksum != calc_checksum(buff))
		{
			Serial.println(F("Packet integrity check failed."));

			Serial.printf("Packet CRC is %02x, should be %02x\n", decoded->crc, calced_crc);
			Serial.printf("Packet checksum is %02x, should be %02x\n", decoded->checksum, calced_checksum);

			return RET_ERROR;
		}
		else
		{
			Serial.println(F("Packet integrity OK"));
			return RET_OK;
		}

		// Success
		return RET_OK;
	}

	/******************************************************************************
	* Sleep until a packet received interrupt is fired or max_sleep_ms passed
	* @param timeout_ms If no INT fires within max_sleep_ms, device will wake up
	* @param 
	******************************************************************************/
	RetResult sleep_to_packet(uint32_t max_sleep_ms)
	{
		// Receive packet
		static uint8_t buff[256] = "";

		int16_t state = 0;

		// Start receive mode
		state = _rf.startReceive(5, SX127X_RX);
		if(state != ERR_NONE)
		{
			Serial.println(F("Could not start RX."));
			return RET_ERROR;
		}

		Serial.println(F("Going to sleep until packet receive INT fires..."));
		Serial.flush();
		
		// Set up external INT on DI0 pin and a timer at the same time
		// If no INT is received within max_sleep_ms, timeout
		Serial.flush();
		esp_sleep_enable_timer_wakeup((uint64_t)max_sleep_ms * 1000);
		esp_sleep_enable_ext0_wakeup(PIN_RF_DI0, 1);
		esp_light_sleep_start();
		
		esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();

		if(wakeup_cause == esp_sleep_wakeup_cause_t::ESP_SLEEP_WAKEUP_TIMER)
		{
			Serial.println(F("Timeout wakeup."));

			return RET_ERROR;
		}
		else if(wakeup_cause == esp_sleep_wakeup_cause_t::ESP_SLEEP_WAKEUP_EXT0)
		{
			Serial.println(F("INT wakeup."));

			_rf.setNodeAddress(DeviceConfig::get_fo_sniffer_id());
			state = _rf.readData((uint8_t*)&buff[2], sizeof(buff)-2);

			buff[0] = FO_SNIFFER_FAMILY_CODE;
			buff[1] = DeviceConfig::get_fo_sniffer_id();

			Serial.printf("\n\n########################## RAW PACKET ###############################\n");
			Utils::print_buff_hex(buff, 30);
			Serial.printf("\n\n#####################################################################\n");
			Serial.println();

			RetResult decode_ret = decode_packet(buff, &_last_decoded_packet);

			if(decode_ret != RET_ERROR)
			{
				FoBuffer::print_packet(&_last_decoded_packet);

				// Update time of last received packet
				_last_packet_tstamp = RTC::get_timestamp();
			}

			return decode_ret;
		}
		else
		{
			Serial.println(F("Invalid wakeup"));
			return RET_ERROR;
		}

		return RET_ERROR;
	}

	/******************************************************************************
	 * Wait until a packet is received or timeout.
	 * @param timeout_ms 		Ms to wait before timing out
	 * @param ignore_address	No address filtering (receive from any device)
	 *****************************************************************************/
	RetResult wait_for_packet(uint32_t timeout_ms, bool ignore_address)
	{
		uint32_t wait_start_millis = millis();

		while(millis() - wait_start_millis < timeout_ms)
		{
			// Receive packet
			// Large enough buffer because _rf.receive seems to ignore len and stack smashing occurrs
			uint8_t buff[256] = "";

			// RF ic strips everything up to the node address
			// Start writing to buffer leaving space to add family code and node address manually
			// to calculate CRC and checksum
			int state = 0;

			if(ignore_address)
			{
				_rf.disableAddressFiltering();

				state = _rf.receive((uint8_t*)&buff[1], sizeof(buff) - 1);
			}
			else
			{
				// Re enable address filtering in case it was disabled, by setting address
				_rf.setNodeAddress(DeviceConfig::get_fo_sniffer_id());

				state = _rf.receive((uint8_t*)&buff[2], sizeof(buff) - 2);

				buff[1] = DeviceConfig::get_fo_sniffer_id();
			}

			buff[0] = FO_SNIFFER_FAMILY_CODE;
			
			if (state == ERR_NONE)
			{
				Serial.println(F("Packet received!"));
				Serial.flush();

				// If valid packet received print and return
				// In case of invalid packets, nothing is done. Waiting to receive more data
				RetResult decode_ret = decode_packet(buff, &_last_decoded_packet);
				
				if(decode_ret != RET_ERROR)
				{
					Serial.println(F("Received OK"));

					debug_print(F("Node: "))
					debug_println_i(_last_decoded_packet.node_address, HEX);
					FoBuffer::print_packet(&_last_decoded_packet);

					// Update time of last received packet
					_last_packet_tstamp = RTC::get_timestamp();
				}

				Serial.printf("\n\n########################## RAW PACKET ###############################\n");
				Utils::print_buff_hex(buff, 30);
				Serial.printf("\n\n#####################################################################\n");
				Serial.println();

				return decode_ret;
			}
			else if (state == ERR_RX_TIMEOUT)
			{
				// Timeout occurred while waiting for a packet
				// Serial.println(F("timeout!"));
			}
			else if (state == ERR_CRC_MISMATCH)
			{
				// Internan CRC check works only in packet mode - not used
				// CRC is instead calculated manually
				Serial.println(F("CRC error!"));

				return RET_ERROR;
			}
			else
			{
				Serial.print(F("Failed, code "));
				Serial.println(state);

				return RET_ERROR;
			}
		}

		return RET_ERROR;
	}

	/******************************************************************************
	* Wait for a valid RF packet from any FO station in range, and (optionally)
	* update config with sniffer id
	* @param timeout_ms Scan time in (ms)
	* @param update_config Update fo node address in device config
	* @return 0 when no FO station found, id otherwise
	******************************************************************************/
	uint8_t scan_fo_id(bool update_config)
	{
		if(FO_SOURCE != FO_SOURCE_SNIFFER)
		{
			debug_println_i(F("Scanning for ID is only for Sniffer mode."));
			return 0;
		}

		debug_println(F("Scanning for new FO id..."));

		Log::log(Log::FO_SNIFFER_SCANNING);

		int ret = 0;

		uint32_t start_ms = millis();

		// Try scanning for devices for FO_SNIFFER_SCAN_TIME_MS
		// If invalid packet is received (wait for packet returns error), calculate
		// time left and start scanning again

		// TODO: When no Lora module is present this spams console with error

		do
		{
			uint32_t timeout = FO_SNIFFER_SCAN_TIME_MS - (millis() - start_ms);

			debug_print(F("Scanning for (mS): "));
			debug_println(timeout, DEC);

			if(wait_for_packet(timeout, true) == RET_OK)
			{
				debug_print_i(F("Found FO station ID: 0x"));
				debug_println_i(_last_decoded_packet.node_address, HEX);

				ret = _last_decoded_packet.node_address;

				Log::log(Log::FO_SNIFFER_SCAN_RESULT, ret);

				if(update_config)
				{
					debug_println(F("Saving new FO address"));
					DeviceConfig::set_fo_sniffer_id(ret);
					DeviceConfig::commit();
				}

				break;
			}
			else
			{
				Serial.println(F("No FO weather station found."));

				ret = 0;
			}
		}while(millis() - start_ms < FO_SNIFFER_SCAN_TIME_MS);

		Log::log(Log::FO_SNIFFER_SCAN_FINISHED);
		
		return ret;
	}

	/******************************************************************************
	 * Get last received packet
	 *****************************************************************************/
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

	/******************************************************************************
	 * Calculate CRC
	 *****************************************************************************/
	uint8_t calc_crc(uint8_t const buff[], int len, uint8_t polynomial, uint8_t init)
	{
		uint8_t remainder = init;

		for (int i = 0; i < len; ++i)
		{
			remainder ^= buff[i];
			for (int j = 0; j < 8; ++j)
			{
				if (remainder & 0x80)
					remainder = (remainder << 1) ^ polynomial;
				else
					remainder = (remainder << 1);
			}
		}
		return remainder;
	}

	/******************************************************************************
	 * Calculate checksum
	 *****************************************************************************/
	uint8_t calc_checksum(uint8_t const buff[])
	{
		uint8_t checksum = 0;
		for (int i = 0; i < 16; i++)
		{
			checksum += buff[i];
		}
		
		return checksum;
	}

	/******************************************************************************
	* Calculate UV index from UV value
	******************************************************************************/
	uint8_t uv_to_index(int uv)
	{
		uint8_t index = 0;

		if(uv >= 99 && uv <= 539)
			index = 1;
		else if(uv >= 540 && uv <= 999)
			index = 2;
		else if(uv >= 1000 && uv <= 1399)
			index = 3;
		else if(uv >= 1400 && uv <= 1842)
			index = 4;
		else if(uv >= 1843 && uv <= 2291)
			index = 5;
		else if(uv >= 2292 && uv <= 2733)
			index = 6;
		else if(uv >= 2734 && uv <= 3137)
			index = 7;
		else if(uv >= 3138 && uv <= 3647)
			index = 8;
		else if(uv >= 3648 && uv <= 4195)
			index = 9;
		else if(uv >= 4196 && uv <= 4706)
			index = 10;
		else if(uv >= 4707 && uv <= 5208)
			index = 11;
		else if(uv >= 5209 && uv <= 5734)
			index = 12;
		else if(uv >= 5735 && uv <= 6275)
			index = 13;
		else if(uv >= 6276 && uv <= 6777)
			index = 14;
		else if(uv >= 6778)
			index = 15;

		return index;
	}

	/******************************************************************************
	 * Print a packet to console
	 *****************************************************************************/
	void print_packet(FoDecodedPacket *packet)
	{
		Serial.printf("\n\n########################## DECODED PACKET ###########################\n");

		FoDecodedPacket decoded = {0};

		Serial.printf("Temperature: %2.1fC\n", packet->temp);
		Serial.printf("Humidity: %d\n%", packet->hum);
		Serial.printf("Rain: %4.2fmm\n", packet->rain);

		Serial.printf("Wind speed: %2.2f\n", packet->wind_speed);
		Serial.printf("Wind Dir: %d\n", packet->wind_dir);
		Serial.printf("Wind Gust: %2.2f\n", packet->wind_gust);

		Serial.printf("UV: %d\n", packet->uv);
		Serial.printf("Light: %d\n", packet->light);

		Serial.printf("CRC: %02x\n", packet->crc);
		Serial.printf("CRC: %02x\n", packet->checksum);

		Serial.printf("\n######################################################################\n");
	}
}