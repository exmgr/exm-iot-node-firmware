#include "water_sensors.h"
#include "aquatroll.h"
#include "water_level.h"
#include "water_presence.h"
#include "rtc.h"
#include "utils.h"
#include "log.h"
#include "common.h"
#include "driver/rtc_io.h"

namespace WaterSensors
{
	/******************************************************************************
	 * Init
	 *****************************************************************************/
	RetResult init()
	{
		// Make sure sensors are off
		off();

		// Setup RTC GPIO
		esp_sleep_pd_config(esp_sleep_pd_domain_t::ESP_PD_DOMAIN_RTC_PERIPH, esp_sleep_pd_option_t::ESP_PD_OPTION_ON);
		rtc_gpio_pulldown_en(PIN_WATER_SENSORS_PWR);
		rtc_gpio_set_direction(PIN_WATER_SENSORS_PWR, rtc_gpio_mode_t::RTC_GPIO_MODE_OUTPUT_ONLY);

		return RET_OK;
	}

	/******************************************************************************
	 * Turn water sensors ON
	 * Both water sensors' power is controlled from the same GPIO
	 *****************************************************************************/
	RetResult on()
	{
		debug_println(F("Water sensors ON."));

		#ifdef TCALL_H
			// Turn IP5306 power boost OFF to reduce idle current
			Utils::ip5306_set_power_boost_state(true);
		#endif

		// TODO: Power on/off mgmt should be moved to Power::switch
		pinMode(PIN_WATER_SENSORS_PWR, OUTPUT);
		digitalWrite(PIN_WATER_SENSORS_PWR, 1);
		delay(WATER_SENSORS_POWER_ON_DELAY_MS);

		delay(200);

        return RET_OK;
	}

	/******************************************************************************
	 * Turn water sensors OFF
	 *****************************************************************************/
	RetResult off()
	{
		debug_println(F("Water sensors OFF."));

		// TODO: Power on/off mgmt should be moved to Power::switch

		pinMode(PIN_WATER_SENSORS_PWR, OUTPUT);
		digitalWrite(PIN_WATER_SENSORS_PWR, 0);

		// Set to input instead of driving low to prevent LED on TSIM from turning ON
		// pinMode(PIN_WATER_SENSORS_PWR, INPUT);

		rtc_gpio_set_level(PIN_WATER_SENSORS_PWR, 0);
		delay(100);

		#ifdef TCALL_H
			// Turn IP5306 power boost OFF to reduce idle current
			Utils::ip5306_set_power_boost_state(false);
		#endif

        return RET_OK;
	}

	/******************************************************************************
	* Read all water sensors and log their data in memory
	* @return RET_ERROR only if failed to read BOTH sensors
	******************************************************************************/
	RetResult log()
	{
		WaterSensorData::Entry data = {0};
		int tries = 3;
		RetResult ret_quality = RET_ERROR, ret_level;

		if(!FLAGS.WATER_QUALITY_SENSOR_ENABLED & !FLAGS.WATER_LEVEL_SENSOR_ENABLED)
		{
			Serial.println(F("No water sensor is enabled."));
			return RET_ERROR;
		}

		WaterSensors::on();

		if(FLAGS.WATER_QUALITY_SENSOR_ENABLED)
		{
			Log::log(Log::WATER_SENSORS_MEASUREMENT_LOG);

			//
			// Read water quality sensor
			// Try reading X times. If fails, cycle power and try once more.
			WaterSensors::on();

			do
			{
				ret_quality = Aquatroll::measure(&data);

				if(ret_quality != RET_OK)
				{
					Utils::serial_style(STYLE_RED);
					debug_print(F("Failed to read water quality sensor."));
					Utils::serial_style(STYLE_RESET);

					if(tries > 1)
					{
						debug_print(F("Retrying..."));
						delay(WATER_QUALITY_RETRY_WAIT_MS);
					}
					debug_println();

					// Next try is last, cycle power
					if(tries == 2)
					{
						debug_println(F("Cycling sensor power."));	
						WaterSensors::off();
						WaterSensors::on();
					}
				}
				else
					break;

			}while(--tries);

			// Log possible error
			if(ret_quality == RET_ERROR)
			{
				Log::log(Log::WATER_QUALITY_MEASUREMENT_FAILED);
			}
		}

		//
		// Read water level
		//
		tries = 3;
		if(FLAGS.WATER_LEVEL_SENSOR_ENABLED)
		{
			do
			{
				ret_level = WaterLevel::measure(&data);

				if(ret_level != RET_OK)
				{
					debug_print_e(F("Failed to read water level sensor."));

					Log::log(Log::WATER_LEVEL_MEASURE_FAILED, WaterLevel::get_last_error());

					if(tries > 1)
					{
						debug_print(F("Retrying..."));
						delay(WATER_LEVEL_RETRY_WAIT_MS);
					}
					debug_println();

					// Next try is last, cycle power
					if(tries == 2)
					{
						debug_println(F("Cycling sensor power."));	
						WaterSensors::off();
						WaterSensors::on();
					}
				}
				else
				{
					break;
				}
			}while(--tries);
		
			// Log possible error
			// if(ret_level != RET_OK)
			// {
			// 	Log::log(Log::WATER_LEVEL_MEASURE_FAILED);
			// }
		}

		//
		// Read water presence sensor
		//
		if(FLAGS.WATER_PRESENCE_SENSOR_ENABLED)
		{
			WaterPresence::measure(&data);
		}

		WaterSensors::off();

		// If both sensors failed no reason to log error, return error
		if(ret_level != RET_OK && ret_quality != RET_OK)
		{
			debug_println(F("Reading both water sensor data failed, data will not be stored."));
			return RET_ERROR;
		}

		//
		// Set timestamp and save
		//
		data.timestamp = RTC::get_timestamp();

		debug_println(F("Water sensor data:"));
		WaterSensorData::print(&data);

		WaterSensorData::add(&data);
		WaterSensorData::get_store()->commit();

		return RET_OK;
	}
}