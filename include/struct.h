#ifndef STRUCT_H
#define STRUCT_H

#include <inttypes.h>

/**
 * Debug message levels
 */
enum DebugMessageLevel
{
    DEBUG_NORMAL,
    DEBUG_ERROR,
    DEBUG_WARNING,
    DEBUG_INFO
};

/**
 * Device descriptor
 * Used for device-specific configuration
 */
// TODO: Obsolete
// struct DeviceDescriptor
// {
//     // Simple device id
//     uint8_t id;

//     // Mac address
//     char mac[18];
// }__attribute__((packed));

/**
 * Console styles
 */
enum SerialStyle
{
    STYLE_RESET = 0,
    STYLE_BOLD,
    STYLE_FAINT,

    STYLE_BLACK = 30,
    STYLE_RED,
    STYLE_GREEN,
    STYLE_YELLOW,
    STYLE_BLUE,
    STYLE_MAGENTA,
    STYLE_CYAN,
    STYLE_WHITE,

    STYLE_BLACK_BKG = 40,
    STYLE_RED_BKG,
    STYLE_GREEN_BKG,
    STYLE_YELLOW_BKG,
    STYLE_BLUE_BKG,
    STYLE_MAGENTA_BKG,
    STYLE_CYAN_BKG,
    STYLE_WHITE_BKG
};

/**
 * Generic function result
 */
enum RetResult
{
    // Generic OK code
    RET_OK = 0,
    // Generic failure code
    RET_ERROR = 1
};

/**
 * Water Level sensor input channels
 */
enum WaterLevelChannel
{
    WATER_LEVEL_CHANNEL_MAXBOTIX_PWM = 1,
    WATER_LEVEL_CHANNEL_MAXBOTIX_ANALOG,
    WATER_LEVEL_CHANNEL_MAXBOTIX_SERIAL,
    WATER_LEVEL_CHANNEL_DFROBOT_PRESSURE_ANALOG,
    WATER_LEVEL_CHANNEL_DFROBOT_ULTRASONIC_SERIAL
};

/**
 * Aquatroll model
 */
enum AquatrollModel
{
    AQUATROLL_MODEL_400 = 1,
    AQUATROLL_MODEL_500,
    AQUATROLL_MODEL_600
};

/**
 * FineOffset weather station sources
 */
enum FineOffsetSource
{
    FO_SOURCE_SNIFFER = 1,
    FO_SOURCE_UART
};

/**
 * FineOffset UART response field
 */
enum FoUartParam
{
    FO_UART_FIELD_WIND_DIR,
    FO_UART_FIELD_WIND_SPEED,
    FO_UART_FIELD_WIND_GUST,
    FO_UART_FIELD_TEMP,
    FO_UART_FIELD_HUMIDITY,
    FO_UART_FIELD_LIGHT,
    FO_UART_FIELD_UV_INDEX,
    FO_UART_FIELD_RAIN_COUNTER
};

/**
 * A single packet of decoded data from the weather station
 */
struct FoDecodedPacket
{
    // Station reports low battery
    bool low_bat;

    // FO station address
    uint8_t node_address;

    // Temperature, Celsius
	// Range: -40C - 60C
	// 0x7FF invalid
    float temp;   		

    // Relative Humidity, %
	// Range: 1% - 99%
	// 0xFF invalid
    uint8_t hum;   		

    // Cumulative rain in mm
    float rain;

    // Wind direction, deg
	// Range: 0 - 359deg
	// 0x1FF invalid
    uint16_t wind_dir;

    // Wind speed, m/s
	// 0x1FF invalid
    float wind_speed;

    // Wind gust, m/s
	// 0xFF when invalid
    float wind_gust;

    // UV - Manual says its uW/cm^2 but it seems to be a raw value that is 
    // is only used to find the UV index from a UV range table
    // Range: 0 - 20000
    // 0xFFFF invalid
    uint32_t uv;

    // UV Index - Derived from UV
    // Range: 0 - 15
    int8_t uv_index;

    // Illuminance, Lux
    // Range: ?
    // 0xFFFFFF invalid
    uint32_t light; 	// Range: 0 - 3000000

    // Solar radiation - Derived from light, W/M^2
    // Range: ?
    uint32_t solar_radiation;

    uint8_t crc;  		// CRC
    uint8_t checksum; 	// Checksum
} __attribute__((packed));

enum LightningEnvironment
{
	LIGHTNING_ENV_INDOOR = 0x01,
	LIGHTNING_ENV_OUTDOOR = 0x0E
};

enum LightningIntReason
{
    LIGHTNING_INT_REASON_NOISE = 0x01,
    LIGHTNING_INT_REASON_DISTURBER = 0x04,
    LIGHTNING_INT_REASON_LIGHTNING = 0x08
};

/**
 * Stats of a telemetry data submit operation
 */
struct DataStoreSubmitStats
{
    int total_entries;
    int submitted_entries;
    int successful_entries;
    int crc_failed_entries;
    int failed_requests;
    int total_requests;
};

/**
 * Describes configuration flags set in app_config
 */
struct FLAGS_T
{
    bool DEBUG_MODE : 1;
    bool LOG_RAW_SDI12_COMMS : 1;
    bool WIFI_DEBUG_CONSOLE_ENABLED : 1;
    bool WIFI_DATA_SUBMISSION_ENABLED;

    bool BATTERY_GAUGE_ENABLED : 1;
    bool SOLAR_CURRENT_MONITOR_ENABLED : 1;
    
    bool NBIOT_MODE : 1;
    bool SLEEP_MINS_AS_SECS : 1;
    bool BATTERY_FORCE_NORMAL_MODE : 1;

    bool WATER_QUALITY_SENSOR_ENABLED : 1;
    bool WATER_LEVEL_SENSOR_ENABLED : 1;
    bool WATER_PRESENCE_SENSOR_ENABLED: 1;
    bool ATMOS41_ENABLED : 1;
    bool SOIL_MOISTURE_SENSOR_ENABLED : 1;

    bool MEASURE_DUMMY_WATER_QUALITY : 1;
    bool MEASURE_DUMMY_WATER_LEVEL : 1;
    bool MEASURE_DUMMY_WEATHER : 1;

    bool LIGHTNING_SENSOR_ENABLED: 1;

    bool RTC_AUTO_SYNC: 1;

    bool EXTERNAL_RTC_ENABLED : 1;

    bool IPFS: 1;
};

#endif