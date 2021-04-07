#include "const.h"
#include "app_config.h"
#include "water_level.h"
#include "water_sensor_data.h"
#include "utils.h"
#include "common.h"
#include "log.h"
#include "log_codes.h"

namespace WaterLevel
{
	//
	// Private functions
	//
	RetResult measure_maxbotix_pwm(WaterSensorData::Entry *data);
	RetResult measure_maxbotix_analog(WaterSensorData::Entry *data);
	RetResult measure_maxbotix_serial(WaterSensorData::Entry *data);
	RetResult measure_dfrobot_pressure_analog(WaterSensorData::Entry *data);
	RetResult measure_dfrobot_ultrasonic_serial(WaterSensorData::Entry *data);

	/******************************************************************************
	 * Init
	 *****************************************************************************/	
	RetResult init()
	{
		switch(WATER_LEVEL_INPUT_CHANNEL)
		{
		case WATER_LEVEL_CHANNEL_MAXBOTIX_ANALOG:
		case WATER_LEVEL_CHANNEL_DFROBOT_PRESSURE_ANALOG:
			pinMode(PIN_WATER_LEVEL_ANALOG, ANALOG);
			break;
		case WATER_LEVEL_CHANNEL_MAXBOTIX_SERIAL:
		case WATER_LEVEL_CHANNEL_DFROBOT_ULTRASONIC_SERIAL:
			pinMode(PIN_WATER_LEVEL_SERIAL_RX, INPUT);
			break;
		case WATER_LEVEL_CHANNEL_MAXBOTIX_PWM:
			pinMode(PIN_WATER_LEVEL_PWM, INPUT);
			break;
		}		

		analogReadResolution(12);
	}

	/******************************************************************************
	 * Read water level sensor and populate SensorData entry structure
	 * Use appropriate function depending on the WATER_LEVEL_INPUT_CHANNEL switch
	 *****************************************************************************/
	RetResult measure(WaterSensorData::Entry *data)
	{
		switch(WATER_LEVEL_INPUT_CHANNEL)
		{
		case WATER_LEVEL_CHANNEL_MAXBOTIX_PWM:
			return measure_maxbotix_pwm(data);
			break;
		case WATER_LEVEL_CHANNEL_MAXBOTIX_ANALOG:
			return measure_maxbotix_analog(data);
			break;
		case WATER_LEVEL_CHANNEL_MAXBOTIX_SERIAL:
			return measure_maxbotix_serial(data);
			break;
		case WATER_LEVEL_CHANNEL_DFROBOT_PRESSURE_ANALOG:
			return measure_dfrobot_pressure_analog(data);
			break;
		case WATER_LEVEL_CHANNEL_DFROBOT_ULTRASONIC_SERIAL:
			return measure_dfrobot_ultrasonic_serial(data);
			break;
		default:
			Utils::serial_style(STYLE_RED);
			debug_println(F("Invalid water level sensor channel"));
			Utils::serial_style(STYLE_RESET);

			return RET_ERROR;
		}
	}

	/******************************************************************************
	 * Read water level sensor and populate SensorData entry structure
	 * Use sensors PWM channel
	 *****************************************************************************/
	RetResult measure_maxbotix_pwm(WaterSensorData::Entry *data)
	{
		debug_println(F("Measuring water level (PWM)"));

        // Return dummy values switch
        if(FLAGS.MEASURE_DUMMY_WATER_LEVEL)
        {
            return measure_dummy(data);
        }


		int meas_left = WATER_LEVEL_MEASUREMENTS_COUNT;
		int failures = 0;
		int cur_el = 0;

		int samples[WATER_LEVEL_MEASUREMENTS_COUNT] = {0};
		float avg = 0;
		do
		{
			int level = pulseIn(PIN_WATER_LEVEL_PWM, 1, WATER_LEVEL_PWM_TIMEOUT_MS * 1000);
			Serial.print(F("Level: "));
			Serial.println(level, DEC);

			// Ignore invalid values
			// 0 returned when no pulse before timeout
			// Sensor returns max range when no target detected within rage
			// Since PWM has an offset take into account a small tolerance
			if(level == 0 || level >= (WATER_LEVEL_MAX_RANGE_MM - WATER_LEVEL_PWM_MAX_VAL_TOL))
			{
				if(++failures >= WATER_LEVEL_PWM_FAILED_MEAS_LIMIT)
				{
					return RET_ERROR;
				}
			}

			avg += level;
			samples[cur_el++] = level;

			delay(WATER_LEVEL_DELAY_BETWEEN_MEAS_MS);
		}while(--meas_left > 0);

		// Print samples
		for (size_t i = 0; i < WATER_LEVEL_MEASUREMENTS_COUNT; i++)
		{
			// TODO: Temp
			// Log::log(Log::WATER_LEVEL_RAW_READING, samples[i]);

			debug_print(samples[i]);
			debug_print(", ");
		}
		debug_println();

		// Pre-filtered values avg
		avg /= WATER_LEVEL_MEASUREMENTS_COUNT;

		// Calc standard deviation
		float std_dev = 0;
		for(int i=0; i<WATER_LEVEL_MEASUREMENTS_COUNT; i++)
		{
			std_dev += pow((float)samples[i] - avg, 2);
		}
		std_dev /= WATER_LEVEL_MEASUREMENTS_COUNT;
		std_dev = sqrt(std_dev);

		// Mark outliers by setting them to -1
		debug_print(F("Std dev: "));
		debug_println(std_dev, DEC);

		debug_print(F("Filtering outliers: "));
		for(int i=0; i<WATER_LEVEL_MEASUREMENTS_COUNT; i++)
		{
			if(samples[i] < (avg - std_dev) || samples[i] > (avg + std_dev))
			{
				debug_print(samples[i], DEC);
				debug_print(F(" "));

				// Filter outlier
				samples[i] = -1;
			}
		}
		debug_println();

		// Calculate avg for valid values
		float level = 0;
		int valid_val_count = 0;
		for(int i=0; i<WATER_LEVEL_MEASUREMENTS_COUNT; i++)
		{
			if(samples[i] != -1)
			{
				valid_val_count++;
				level += samples[i];
			}
		}

		if(valid_val_count == 0)
		{
			return RET_ERROR;
		}

		// Calc average
		level /= valid_val_count;

		// Convert mm to cm
		data->water_level = (float)level / 10;

		return RET_OK;
	}

	/******************************************************************************
	 * Read water level sensor and populate SensorData entry structure
	 * Use sensors ANALOG channel
	 *****************************************************************************/
	RetResult measure_maxbotix_analog(WaterSensorData::Entry *data)
    {
		debug_println(F("Measuring water level (analog)"));

        // Return dummy values switch
        if(FLAGS.MEASURE_DUMMY_WATER_LEVEL)
        {
            return measure_dummy(data);
        }

		// TODO: Refactor to use Utils::read_adc_mv

		// X measurements, calc average
		int level_raw = 0;
		for(int i = 0; i < WATER_LEVEL_MEASUREMENTS_COUNT; i++)
		{
			level_raw += analogRead(PIN_WATER_LEVEL_ANALOG);
			delay(WATER_LEVEL_DELAY_BETWEEN_MEAS_MS);
		}
		level_raw /= WATER_LEVEL_MEASUREMENTS_COUNT;

		// Convert to mV
		int mv = ((float)3600 / 4096) * level_raw;

		// Convert to CM
		int cm = (mv / WATER_LEVEL_MV_PER_MM) * 10;

		data->water_level = cm;

		return RET_OK;
	}

	/******************************************************************************
	 * Read DFrobot pressure water level sensor and populate SensorData entry structure
	 *****************************************************************************/
	RetResult measure_dfrobot_pressure_analog(WaterSensorData::Entry *data)
	{
        debug_println(F("Measuring water level (DFRobot pressure analog)"));

        // Return dummy values switch
        if(FLAGS.MEASURE_DUMMY_WATER_LEVEL)
        {
            return measure_dummy(data);
        }

		// X measurements, calc average
		int level_raw = 0;
		for(int i = 0; i < WATER_LEVEL_MEASUREMENTS_COUNT; i++)
		{
			level_raw += analogRead(PIN_WATER_LEVEL_ANALOG);
			delay(WATER_LEVEL_DELAY_BETWEEN_MEAS_MS);
		}
		level_raw /= WATER_LEVEL_MEASUREMENTS_COUNT;

		// Convert to mV
		int mv = ((float)3600 / 4096) * level_raw;

		data->water_level = mv;

		debug_print(F("Measured: "));
		debug_println(data->water_level, DEC);

		return RET_OK;	
	}

	/******************************************************************************
	 * Read DFrobot ultrasonic water level sensor and populate SensorData entry structure
	 *****************************************************************************/
	RetResult measure_dfrobot_ultrasonic_serial(WaterSensorData::Entry *data)
	{
        debug_println(F("Measuring water level (DFRobot ultrasonic serial)"));

        // Return dummy values switch
        if(FLAGS.MEASURE_DUMMY_WATER_LEVEL)
        {
            return measure_dummy(data);
        }

		int level_avg = 0;

		HardwareSerial us_serial(2);
		us_serial.begin(9600, SERIAL_8N1, PIN_WATER_LEVEL_SERIAL_RX, 0);

		for(int i=0; i < 10; i++)
		{
			char buff[5] = "";
			us_serial.readBytes(buff, 4);
	
			if(buff[0] != 0xFF)
			{
				Serial.println(F("Level sensor returned invalid data."));
				continue;
			}

			uint16_t level = (buff[1] << 8) | buff[2];
			level_avg += level;
		}

		level_avg /= 10;
		
		Serial.print(F("Measured level: "));
		Serial.println(level_avg, DEC);

		// Convert to cm
		data->water_level = level_avg / 10;

		return RET_OK;	
	}

	/******************************************************************************
	 * Read MaxBotix ultrasonic water level sensor and populate SensorData entry structure
	 *****************************************************************************/
	RetResult measure_maxbotix_serial(WaterSensorData::Entry *data)
	{
        debug_println(F("Measuring water level (MaxBotix ultrasonic serial)"));

		const int MEASUREMENTS = 10;

        // Return dummy values switch
        if(FLAGS.MEASURE_DUMMY_WATER_LEVEL)
        {
            return measure_dummy(data);
        }

		int level_avg = 0;

		HardwareSerial us_serial(1);
		us_serial.begin(9600, SERIAL_8N1, PIN_WATER_LEVEL_SERIAL_RX, 0, true);

		for (int i = 0; i < MEASUREMENTS; i++)
		{
			char buff[10] = "";
			us_serial.readBytes(buff, 5);
			int level = 0;

			int read = sscanf(buff, "R%d\r", &level);	

			if (read != 1)
			{
				Serial.println(F("Level sensor returned invalid data."));
				continue;
			}

			level_avg += level;
		}

		level_avg /= MEASUREMENTS;

		Serial.print(F("LEVEL: "));
		Serial.println(level_avg, DEC);

		data->water_level = level_avg;

		return RET_OK;
	}

	/******************************************************************************
     * Fill water level struct with dummy data
     * Used for debugging only
     * @param data Output structure
     *****************************************************************************/
    RetResult measure_dummy(WaterSensorData::Entry *data)
    {
        debug_println(F("Returning dummy values"));

		data->water_level = 64;
        data->water_level += (float)random(-400, 400) / 100;

		// Emulate waiting time
        delay(1000);

        return RET_OK;
    }
}