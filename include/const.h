#ifndef CONST_H
#define CONST_H
#include <inttypes.h>
#include "sleep_scheduler.h"
#include "credentials.h"


/******************************************************************************
 * Misc
 *****************************************************************************/
/** Serial output baud */
const int SERIAL_MONITOR_BAUD = 115200;

/**
 * Seconds from unix epoch to 01/01/2000 00:00:00. Used to convert time from
 * RTC to UNIX timestamp used by ESP32s internal RTC
*/
const int SECONDS_IN_2000 = 946684800;

/** Buffer size for URLs */
const int URL_BUFFER_SIZE = 128;

/** Buffer size for URL's host part (used when exploding URLs) */
const int URL_HOST_BUFFER_SIZE = 60;

/** Larger buffer size for storing larger URL with many params (eg. thingsboard shared attrs) */
const int URL_BUFFER_SIZE_LARGE = 256;

/** Max sleep time for any event (main sleep). Used as a fail safe in case of wrong calculations */
const int MAX_SLEEP_TIME_SEC = 60 * 60 * 24; // One day

const int MAX_SLEEP_CORRECTION_SEC = 60 * 5; // 5 mins

/** Min/max allowed values for calling home interval (mins) */
const int CALL_HOME_INT_MINS_MIN = 1;           // 1 min
const int CALL_HOME_INT_MINS_MAX = 24 * 60 * 2; // 2 days

/** Min/max allowed values for water sensors measuring interval (mins) */
const int MEASURE_SENSORS_INT_MINS_MIN = 1;           		// 1 min
const int MEASURE_WATER_SENSORS_INT_MINS_MAX = 24 * 60 * 1; // 1 day

/** A timestamp must be in this range to be considered valid. y2k bug all over again.
 * Temp solution
 * value */
const unsigned long FAIL_CHECK_TIMESTAMP_START = 1567157191;
const unsigned long FAIL_CHECK_TIMESTAMP_END = 2072091600;

/** Samples configuration for analog */
const int ADC_CYCLES = 128;

/** Samples to average when reading battery voltage with internal ADC */
const int ADC_BATTERY_LEVEL_SAMPLES = 50;

/** Time user has to hold button to enter config mode */
const int CONFIG_MODE_BTN_HOLD_TIME_MS = 2000;

/******************************************************************************
* Debug
******************************************************************************/
const char DEBUG_LEVEL_ERROR_STYLE[] = "\033[31m"; // Red
const char DEBUG_LEVEL_WARNING_STYLE[] = "\033[33m"; // Yellow
const char DEBUG_LEVEL_INFO_STYLE[] = "\033[34m"; // Blue

const char TIMBER_API_URL[] = "https://logs.timber.io/sources/" TIMBER_SOURCE_ID "/frames";
const char TIMBER_AUTH_HEADER[] = "Bearer " TIMBER_API_KEY;

/** Length of Wifi Serial buffer for storing messages temporarily */
const int WIFI_SERIAL_BUFFER_SIZE = 512;

/******************************************************************************
* WiFi
******************************************************************************/
/** Timeout when trying to connect to network */
const int WIFI_CONNECT_TIMEOUT_SEC = 10;

/******************************************************************************
 * Data stores
 *****************************************************************************/
/** Maximum length for file path buffers */
const int FILE_PATH_BUFFER_SIZE = 25;

/** Max tries when looking for an unused filename before failing */
const int FILENAME_POSTFIX_MAX = 100;

/******************************************************************************
 * Telemetry data
 *****************************************************************************/
const int TELEMETRY_DATA_JSON_OUTPUT_BUFF_SIZE = 2048;

/******************************************************************************
 * Water Sensor data
 *****************************************************************************/
/** Path in data store where sensor data is stored */
const char* const WATER_SENSOR_DATA_PATH = "/was";

/** Arduino JSON doc size */
const int WATER_SENSOR_DATA_JSON_DOC_SIZE = 1024;
/** Sensor data entries to group into a single json packet for submission */
const int WATER_SENSOR_DATA_ENTRIES_PER_SUBMIT_REQ = 8;

// Telemetry key names
const char WATER_SENSOR_DATA_KEY_TIMESTAMP[] = "ts";
const char WATER_SENSOR_DATA_KEY_DISSOLVED_OXYGEN[] = "s_do";
const char WATER_SENSOR_DATA_KEY_TEMPERATURE[] = "s_temp";
const char WATER_SENSOR_DATA_KEY_PH[] = "s_ph";
const char WATER_SENSOR_DATA_KEY_CONDUCTIVITY[] = "s_cond";
const char WATER_SENSOR_DATA_KEY_ORP[] = "s_orp";
const char WATER_SENSOR_DATA_KEY_PRESSURE[] = "s_press";
const char WATER_SENSOR_DATA_KEY_DEPTH_CM[] = "s_depth_cm";
const char WATER_SENSOR_DATA_KEY_DEPTH_FT[] = "s_depth_ft";
const char WATER_SENSOR_DATA_KEY_TSS[] = "s_tss";
const char WATER_SENSOR_DATA_KEY_WATER_LEVEL[] = "s_wl";

/******************************************************************************
 * Soil moisture data
 *****************************************************************************/
/** Path in data store where sensor data is stored */
const char* const SOIL_MOISTURE_DATA_PATH = "/sm";

/** Arduino JSON doc size */
const int SOIL_MOISTURE_DATA_JSON_DOC_SIZE = 1024;
/** Sensor data entries to group into a single json packet for submission */
const int SOIL_MOISTURE_DATA_ENTRIES_PER_SUBMIT_REQ = 8;

// Telemetry key names
const char SOIL_MOISTURE_DATA_KEY_TIMESTAMP[] = "ts";
const char SOIL_MOISTURE_DATA_KEY_VWC[] = "sm_vwc";
const char SOIL_MOISTURE_DATA_KEY_TEMPERATURE[] = "sm_temp";
const char SOIL_MOISTURE_DATA_KEY_CONDUCTIVITY[] = "sm_cond";

/******************************************************************************
 * Atmos41 data
 *****************************************************************************/
/** Path in data store where sensor data is stored */
const char* const ATMOS41_DATA_PATH = "/wes";

/** Arduino JSON doc size */
const int ATMOS41_DATA_JSON_DOC_SIZE = 1024;
/** Sensor data entries to group into a single json packet for submission */
const int ATMOS41_DATA_ENTRIES_PER_SUBMIT_REQ = 4;

// Telemetry key names
const char ATMOS41_DATA_KEY_TIMESTAMP[] = "ts";
const char ATMOS41_DATA_KEY_SOLAR[] = "ws_solar";
const char ATMOS41_DATA_KEY_PRECIPITATION[] = "ws_prec";
const char ATMOS41_DATA_KEY_STRIKES[] = "ws_strikes";
const char ATMOS41_DATA_KEY_WIND_SPEED[] = "ws_w_speed";
const char ATMOS41_DATA_KEY_WIND_DIR[] = "ws_w_dir";
const char ATMOS41_DATA_KEY_WIND_GUST[] = "ws_w_gust";
const char ATMOS41_DATA_KEY_AIR_TEMP[] = "ws_air_temp";
const char ATMOS41_DATA_KEY_VAPOR_PRESSURE[] = "ws_vapor_press";
const char ATMOS41_DATA_KEY_ATM_PRESSURE[] = "ws_atm_press";
const char ATMOS41_DATA_KEY_REL_HUMIDITY[] = "ws_rel_hum";
const char ATMOS41_DATA_KEY_DEW_POINT[] = "ws_dew_pt";

/******************************************************************************
 * Lightning sensor
 *****************************************************************************/
/** Path in data store where sensor data is stored */
const char* const LIGHTNING_DATA_PATH = "/ls";

/** Arduino JSON doc size */
const int LIGHTNING_DATA_JSON_DOC_SIZE = 1024;
/** Sensor data entries to group into a single json packet for submission */
const int LIGHTNING_DATA_ENTRIES_PER_SUBMIT_REQ = 8;

// Telemetry key names
const char LIGHTNING_DATA_KEY_TIMESTAMP[] = "ts";
const char LIGHTNING_DATA_KEY_DISTANCE[] = "li_dist";
const char LIGHTNING_DATA_KEY_ENERGY[] = "li_energy";

/** Ignore disturber events */
const bool LIGHTNING_MASK_DISTURBERS = true;

/** Noise floor level 1 - 7 */
const uint8_t LIGHTNING_NOISE_FLOOR = 5;

/** AFE watchdog threshold */
const uint8_t LIGHTNING_WATCHDOG_THRES = 2;

/** Spike rejection 1 - 11 */
const uint8_t LIGHTNING_SPIKE_REJECTION = 6;

/** Number of lightning strikes before IRQ fires
 * Values [1, 5, 9, 16] */
const uint8_t LIGHTNING_THRES = 1;

/** Time after an IRQ that another IRQ is allowed to fire. IRQs sooner than this
 * will be ignored */
const uint32_t LIGHTNING_IRQ_DEBOUNCE_MS = 1000;

/** Time after which noise/disturber counter will be logged */
const uint32_t LIGHTNING_REPORT_LOG_INTERVAL_SEC = 60 * 60;

// Module specific constants

/*
 * Lightning sensor module types
 */
#define LIGHTNING_SENSOR_CJMCU 1
#define LIGHTNING_SENSOR_DFROBOT 2

/******************************************************************************
 * Log
 *****************************************************************************/
/** Path in data store where log data is stored */
const char* const LOG_DATA_PATH = "/log";
/** Log data entries to group into a single data packet for submission */
const int LOG_ENTRIES_PER_SUBMIT_REQ = 8;
/** Arduino JSON doc size */
const int LOG_JSON_DOC_SIZE = 1024;
/** JSON output buffer size */
const int LOG_JSON_OUTPUT_BUFF_SIZE = 1024;

/******************************************************************************
* SDI12 debug log
******************************************************************************/
const int SDI12_LOG_JSON_DOC_SIZE = 1024;

// Telemetry key names
const char SDI12_LOG_KEY_TIMESTAMP[]  = "ts";
const char SDI12_LOG_KEY_RAW_DATA[]  = "sdi12";

/******************************************************************************
 * Remote Control
 *****************************************************************************/
/** JSON doc size for received remote config data */
const int REMOTE_CONTROL_JSON_DOC_SIZE = 1024;

// Sent/received JSON parameter names
const char RC_TB_KEY_REMOTE_CONTROL_DATA_ID[] = "data_id";
const char RC_TB_KEY_MEASURE_WATER_SENSORS_INT[] = "was_int";
const char RC_TB_KEY_MEASURE_WEATHER_STATION_INT[] = "wes_int";
const char RC_TB_KEY_MEASURE_SOIL_MOISTURE_SENSORS_INT[] = "sm_int";
const char RC_TB_KEY_CALL_HOME_INT[] = "ch_int";
const char RC_TB_KEY_DO_REBOOT[] = "do_reboot";
const char RC_TB_KEY_DO_OTA[] = "do_ota";
const char RC_TB_KEY_FO_ENABLED[] = "fo_en";
const char RC_TB_KEY_DO_FO_SCAN[] = "do_fo_scan";
const char RC_TB_KEY_DO_FORMAT_SPIFFS[] = "do_format";
const char RC_TB_KEY_DO_RTC_SYNC[] = "do_rtc";
const char RC_TB_KEY_FW_URL[] = "fw_url";
const char RC_TB_KEY_FW_VERSION[] = "fw_v";
const char RC_TB_KEY_FW_MD5[] = "fw_md5";

/******************************************************************************
 * Client attributes
 *****************************************************************************/
/** JSON doc size for client attributes request body */
const int CLIENT_ATTRIBUTES_JSON_DOC_SIZE = 1024;

// Client attribute names
const char TB_ATTR_CUR_FW_V[] = "cur_fw_v";
const char TB_ATTR_CUR_WAS_INT[] = "cur_was_int";
const char TB_ATTR_CUR_WES_INT[] = "cur_wes_int";
const char TB_ATTR_CUR_SM_INT[] = "cur_sm_int";
const char TB_ATTR_CUR_CH_INT[] = "cur_ch_int";
const char TB_ATTR_CUR_FO_ID[] = "cur_fo_id";
const char TB_ATTR_CUR_FO_EN[] = "cur_fo_en";
const char TB_ATTR_CUR_SYSTEM_TIME[] = "cur_time";
const char TB_ATTR_UPTIME[] = "uptime";
const char TB_ATTR_FLAGS[] = "flags";
const char TB_ATTR_AQUATROLL_MODEL[] = "troll_model";

/******************************************************************************
 * Calling home
 *****************************************************************************/
/**
 * TB telemetry API URL
 * Params: tb url, device access token
 */
const char TB_TELEMETRY_URL_FORMAT[] = "/api/v1/%s/telemetry";

/**
 * TB API URL for publishing client attributes
 * Params: device access token
 */
const char TB_CLIENT_ATTRIBUTES_URL_FORMAT[] = "/api/v1/%s/attributes?clientKeys=cur_fw_v,cur_was_int,cur_wes_int,cur_sm_int,cur_ch_int,cur_fo_id";

/**
 * TB API URL for getting shared attributes for remote control
 * Params: device access token
*/
const char TB_SHARED_ATTRIBUTES_URL_FORMAT[] = "/api/v1/%s/attributes?sharedKeys=data_id,ch_int,fw_v,fw_url,fw_md5,was_int,wes_int,sm_int,ch_int,do_ota,do_reboot,do_format,do_rtc,do_fo_scan,fo_en";

/** Max failed requests before aborting telemetry submission */

const int FAILED_TELEMETRY_REQ_THRESHOLD = 3;
/******************************************************************************
* DeviceConfig store
******************************************************************************/
/** Preferences api namespace name for DeviceConfig. Also used as the single key
 * where config struct is stored */
const char DEVICE_CONFIG_NVS_NAMESPACE_NAME[] = "DevConf";

/******************************************************************************
 * GSM
 *****************************************************************************/
/** HardwareSerial port to use for communication with the GSM module */
const uint8_t GSM_SERIAL_PORT = 1;

const int GSM_SERIAL_BAUD = 115200;

/** Time to wait for GSM network connection before timeout */
const int GSM_DISCOVERY_TIMEOUT_MS = 30000;

/** Times to try to execute a command on the GSM module before failing */
const int GSM_TRIES = 3;
/** Time to delay between tries */
const int GSM_RETRY_DELAY_MS = 100;

/** Time to wait after turning on GSM module until device ready */
const int GSM_WAIT_AFTER_PWR_ON_MS = 6000;

/** Time to wait after GSM power ON/OFF requested, before device power ready to be toggled again */
const int GSM_WAIT_AFTER_PWR_TOGGLE_MS = 6000;

/** Timeout for AT test command */
const int GSM_TEST_AT_TIMEOUT = 3000;

/** TinyGSM buffer size - Used by tinygsm*/
#define TINY_GSM_RX_BUFFER	1024

/******************************************************************************
 * HTTP
 *****************************************************************************/
/** Generic global HTTP response buffer size. Buffer is large and thus stored in global scope 
 * to avoid stack overflow.
*/
const int GLOBAL_HTTP_RESPONSE_BUFFER_LEN = 4096;

/** Timeout when reading http client stream */
const int HTTP_CLIENT_STREAM_TIMEOUT = 3000;

/** HTTP response timeout */
const int HTTL_CLIENT_REPONSE_TIMEOUT = 15000;

/******************************************************************************
 * SDI12 Sensors
 *****************************************************************************/
/** UART to use for SDI12 comms */
const int SDI12_UART_NUM = 2;

/** Time to wait after sending a command and before reading back the response */
const int SDI12_COMMAND_WAIT_TIME_MS = 50;

/** Receive buffer size. Must be large enough to fit a single response */ 
const int SDI12_RECV_BUFF_SIZE = 48;

/** Number of extra seconds to wait for measurement on top of what the sensors says */
const int SDI12_MEASURE_EXTRA_WAIT_SECS = 1;

/******************************************************************************
 * Weather Station
 *****************************************************************************/
/** Max time to wait for sensor to prepare measurements after a 
 * measure command. Used in case sensor returns garbage values, to prevent
 * waiting for long amounts of time. Value must be adapted to water quality
 * sensor configuration (number of measurements). Allow a few seconds of error */
const int WEATHER_STATION_MEASURE_WAIT_SEC_MAX = 10;

/** Number of measurement values expected from the sensor */
const int WEATHER_STATION_NUMBER_OF_MEASUREMENTS = 9;

/******************************************************************************
 * Aquatroll
 *****************************************************************************/
/** Time to wait for water sensors to boot after powering them up */
const int WATER_SENSORS_POWER_ON_DELAY_MS = 2000;

/** Max time to wait for sensor to prepare measurements after a 
 * measure command. Used in case sensor returns garbage values, to prevent
 * waiting for long amounts of time. Value must be adapted to water quality
 * sensor configuration (number of measurements). Allow a few seconds of error */
const int AQUATROLL_MEASURE_WAIT_SEC_MAX = 20;

/** Number of measurement values expected for Aquatroll 400 */
const int AQUATROLL400_NUMBER_OF_MEASUREMENTS = 8;

/** Number of measurement values expected for Aquatroll 500 */
const int AQUATROLL500_NUMBER_OF_MEASUREMENTS = 8;

/** Number of measurement values expected for Aquatroll 600 */
const int AQUATROLL600_NUMBER_OF_MEASUREMENTS = 7;

/** Number of mS to wait before retrying after an error */
const int WATER_QUALITY_RETRY_WAIT_MS = 1000;

/******************************************************************************
 * Water Level sensor
 *****************************************************************************/

// General
/** Times to measure (and calculate avg) */
const int WATER_LEVEL_MEASUREMENTS_COUNT = 10;
/** Delay in mS between measurements */
const int WATER_LEVEL_DELAY_BETWEEN_MEAS_MS = 5;
/** Max range in centimeters */
const int WATER_LEVEL_MAX_RANGE_MM = 9999;
/** Number of seconds to wait before retrying after an error */
const int WATER_LEVEL_RETRY_WAIT_MS = 1000;

// PWM channel only
/** Timeout when waiting for PWM pulse */
const int WATER_LEVEL_PWM_TIMEOUT_MS = 500;

/** Sensor returns max range when no target detected within range
Since PWM has an offset a tolerance is taken into account when ignoring invalid values */
const int WATER_LEVEL_PWM_MAX_VAL_TOL = 30;

/** 
 * Maximum failures to acquire valid values when reading sensor before aborting
 * measurement completely
 **/
const int WATER_LEVEL_PWM_FAILED_MEAS_LIMIT = 15;

// Analog channel only
/** Millivolts per cm */
const float WATER_LEVEL_MV_PER_MM = (float)3300 / WATER_LEVEL_MAX_RANGE_MM;

/******************************************************************************
 * Teros12 sensor
 *****************************************************************************/

/** Max time to wait for sensor to prepare measurements after a 
 * measure command. Used in case sensor returns garbage values, to prevent
 * waiting for long amounts of time. Value must be adapted to water quality
 * sensor configuration (number of measurements). Allow a few seconds of error */
const int TEROS12_MEASURE_WAIT_SEC_MAX = 10;

/** Number of measurement values expected from the sensor */
const int TEROS12_NUMBER_OF_MEASUREMENTS = 3;

/******************************************************************************
 * FineOffset weather station data
 *****************************************************************************/
/** Data store path */
const char* const FO_DATA_STORE_PATH = "/fo";

/** FO entries to group into a single json packet for submission */
const int FO_DATA_STORE_ENTRIES_PER_SUBMIT_REQ = 5;

/** Arduino JSON doc size */
const int FO_DATA_JSON_DOC_SIZE = 1024;

// Telemetry key names
const char FO_DATA_KEY_TIMESTAMP[] = "ts";
const char FO_DATA_KEY_PACKETS[] = "fo_packets";
const char FO_DATA_KEY_TEMP[] = "fo_temp";
const char FO_DATA_KEY_HUMIDITY[] = "fo_hum";
const char FO_DATA_KEY_RAIN[] = "fo_rain";
const char FO_DATA_KEY_RAIN_RATE_HR[] = "fo_rain_hr";
const char FO_DATA_KEY_WIND_DIR[] = "fo_w_dir";
const char FO_DATA_KEY_WIND_SPEED[] = "fo_w_speed";
const char FO_DATA_KEY_WIND_GUST[] = "fo_w_gust";
const char FO_DATA_KEY_UV[] = "fo_uv";
const char FO_DATA_KEY_LIGHT[] = "fo_light";
const char FO_DATA_KEY_SOLAR_RADIATION[] = "fo_sol_rad";

/** Wind speed coefficient in sniffed packet */
const float FO_WIND_SPEED_COEFF = 0.0644;

/** Wind gust coefficient in sniffed packet */
const float FO_WIND_GUST_COEFF = 0.51;

/** mm of rain for every click of the rain counter */
const float FO_RAIN_MM_PER_CLICK = 0.254;

/** Used to convert lux to W/M^2 */
const float FO_SNIFFER_LUX_TO_SOLAR_RADIATION_COEFF = 0.0079;

/** Aggregate time window. Packets received from the weather station will be
 * aggregated into a single packet and commited to flash every this amount of
 * seconds (approx.) */
const int FO_AGGREGATE_INTERVAL_SEC = 600;

/** FO is disabled after X successive failed RX (both sniff and uart) to prevent battery drainage*/
const uint8_t FO_FAILED_RX_THRESHOLD = 20;

/******************************************************************************
 * FineOffset Buffer
 *****************************************************************************/
const int FO_BUFFER_SIZE = 200;

/******************************************************************************
 * FineOffset weather station sniffer
 *****************************************************************************/
// Rf config
const float FO_SNIFFER_BIT_RATE = 17.2;
const float FO_SNIFFER_FREQ_DEV = 0;
const float FO_SNIFFER_RX_BW = 125;
const uint8_t FO_SNIFFER_SYNC_WORD[] = {0x2D, 0xD4};

/** FineOffset weather station family code */
const uint8_t FO_SNIFFER_FAMILY_CODE = 0x24;

/** Interval at which the weather station sends packets */
const int FO_SNIFFER_PACKET_INTERVAL_SEC = 16;

/**
 * Time to wait for a packet when syncing. Must be weather station tx interval
 * time + tolerance 
 * */
const int FO_SNIFFER_SYNC_WAIT_TIME_MS = 20000;

/** Ms to wake up before the expected time of packet arrival
 * (threshold to make sure we catch the packet) */
const int FO_SNIFFER_WAIT_PACKET_EARLY_WAKEUP_SEC = 3;

/**
 * Time to wait for a packet to arrive during sniffing.
 */
const int FO_SNIFFER_PACKET_WAIT_TIME_MS = 4000;

/* Time to scan for weather stations when scanning for new id */
const int FO_SNIFFER_SCAN_TIME_MS = 25000;

/******************************************************************************
 * FineOffset weather station UART
 *****************************************************************************/
/** UART port to use for communication */

const int FO_UART_PORT = 2;
/** Seconds between packets */
const uint8_t FO_UART_PACKET_INTERVAL_SEC = 16;

/** Seconds to wait before requesting packet after interval has passed (tolerance) */
const uint8_t FO_UART_WAIT_BEFORE_REQ_SEC = 1;

/* Timeout when waitng for weather station to send data after requesting a packet */
const uint16_t FO_UART_RX_TIMEOUT_MS = 3000;

/** Aggregate time window. Packets received from the weather station will be
 * aggregated into a single packet and commited to flash every this amount of
 * seconds (approx.) */
const int FO_UART_AGGREGATE_INT_SEC = 600;

/**
 * UART response fields, in correct order as returned by weather station
 */
const FoUartParam FO_UART_PARAMS[] = {
    FO_UART_FIELD_WIND_DIR, FO_UART_FIELD_WIND_SPEED, FO_UART_FIELD_WIND_GUST, FO_UART_FIELD_TEMP,
    FO_UART_FIELD_HUMIDITY, FO_UART_FIELD_LIGHT, FO_UART_FIELD_UV_INDEX, FO_UART_FIELD_RAIN_COUNTER};

/**
 * Names of fields in FineOffset's UART response. Index must correspond to indexes in FO_UART_PARAMS
 */
const char FO_UART_RESPONSE_PARAM_NAMES[][15] = 
{
    "WindDir",
    "WindSpeed",
    "WindGust",
    "Temp",
    "Humi",
    "Light",
    "UV_Index",
    "RainCnt"
};

/** UART response param count */
const uint16_t FO_UART_PARAM_COUNT = sizeof(FO_UART_RESPONSE_PARAM_NAMES) / sizeof(FO_UART_RESPONSE_PARAM_NAMES[0]);

/******************************************************************************
 * BME280 (internal environment sensor 1)
 *****************************************************************************/
const uint8_t BME280_I2C_ADDR1 = 0x76; 
const uint8_t BME280_I2C_ADDR2 = 0x77; 

/******************************************************************************
 * IP5306 (TCall PMU ic)
 *****************************************************************************/
const uint8_t IP5306_I2C_ADDR = 0x75;
const uint8_t IP5306_REG_SYS_CTL0 = 0x00;
const uint8_t IP5306_BOOST_FLAGS = 0x37;  // 0x37 = 0b110111 TCALL example
      
  /*
 [1]      Boost EN (default 1)            [EXM note: if 0 ESP32 will not boot from battery]
 [1]      Charger EN (1)                  [EXM note: did  not observe difference]
 [1]      Reserved (1)                    [EXM note: did  not observe difference]
 [1]      Insert load auto power EN (1)   [EXM note: did  not observe difference]
 [1]      BOOST output normally open ( 1) [EXM note: if 0 will shutdown on ESP32 sleep after 32s]
 [1]      Key off EN: (0)                 [EXM note: could not detect difference]
  */

/******************************************************************************
 * Things board timeseries/attribute keys
 *****************************************************************************/
//
// Shared attributes
//
// TODO: Bring shared attributes here



/******************************************************************************
 * Battery
 *****************************************************************************/
// TODO: Move to struct.h pr battery.h
enum BATTERY_MODE
{
	BATTERY_MODE_NORMAL = 1,
	BATTERY_MODE_LOW,
	BATTERY_MODE_SLEEP_CHARGE
};

struct BATTERY_PCT_LUT_ENTRY

{
	uint8_t pct;
	uint16_t mv;
};

/**
 * Look up table for lithium battery SoC - voltage
 * Used when measuring battery state with ADC
 */
const BATTERY_PCT_LUT_ENTRY BATTERY_PCT_LUT[] = {
	{0, 2750}, {1, 3050}, {2, 3230}, {3, 3340}, {4, 3430}, {5, 3490}, {6, 3530}, {7, 3550}, {8, 3560}, {9, 3570}, 
	{10, 3580}, {13, 3590}, {14, 3600}, {16, 3610}, {18, 3630}, {23, 3640}, {25, 3650}, {27, 3660}, {31, 3670}, {36, 3680}, 
	{41, 3690}, {43, 3700}, {47, 3710}, {51, 3720}, {53, 3730}, {55, 3740}, {59, 3750}, {62, 3760}, {63, 3770}, {65, 3780}, 
	{66, 3790}, {67, 3800}, {68, 3810}, {70, 3820}, {73, 3830}, {74, 3840}, {76, 3850}, {78, 3860}, {79, 3870}, {81, 3880}, 
	{84, 3890}, {85, 3900}, {86, 3920}, {88, 3930}, {90, 3940}, {91, 3950}, {92, 3960}, {94, 3970}, {95, 3980}, {96, 3990}, 
	{98, 4000}, {99, 4010}, {100, 4030}
};

#endif