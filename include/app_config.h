#ifndef APP_CONFIG_H
#define APP_CONFIG_H
#include "struct.h"
#include "Arduino.h"
#include "sleep_scheduler.h"
#include "sdi12_sensor.h"

/** FW Version */
const int FW_VERSION = 159;

/******************************************************************************
 * Credentials
 * Should be defined in an app-specific /include/credentials.h file
 *****************************************************************************/

/** Thingsboard server URL */
extern const char TB_SERVER[];
/** Thingsboard server port */
extern const int TB_PORT;

/******************************************************************************
 * General
 *****************************************************************************/

/**
 * Enable redirecting all debug log messages to HTTP log service with Wifi.
 * Increases binary size significantly and slows down firmware execution. To be
 * used only for debugging. Wifi debug console uses the much larger ESP http
 * library because ArduinoHttpClient doesn't support SSL.
 */
#define WIFI_DEBUG_CONSOLE false

/**
 * Enable data submission through Wifi instead of GSM
 * For debugging purposes
 */
#define WIFI_DATA_SUBMISSION true

// Main switches

volatile const FLAGS_T FLAGS
{
    /** Debug mode enabled - set by build env*/
    #ifdef DEBUG
    DEBUG_MODE: true,
    #else
    DEBUG_MODE: false,
    #endif

    /* Enable logging of raw SDI12 communication. For debugging
    only, not memory efficient. */
    LOG_RAW_SDI12_COMMS: false,

    /** Enable redirecting all debug log messages to HTTP log service with Wifi */
    WIFI_DEBUG_CONSOLE_ENABLED: WIFI_DEBUG_CONSOLE,

    /** Submit data though WiFi instead of GSM */
    WIFI_DATA_SUBMISSION_ENABLED: WIFI_DATA_SUBMISSION,

    /** Enable Battery Gauge. */
    BATTERY_GAUGE_ENABLED: false,

    /** Enable solar panel current monitor */
    SOLAR_CURRENT_MONITOR_ENABLED: false,

    /** Enable NBIoT mode. If false GSM is used */
    NBIOT_MODE: false,
    
    /** When sleeping treat minutes as seconds. Used for debugging */
    SLEEP_MINS_AS_SECS: false,

    /** When true disables entering sleep charge and battery low modes when battery 
     * level under threshold */
    BATTERY_FORCE_NORMAL_MODE: false,

    /** Take measurements from water quality sensor */
    WATER_QUALITY_SENSOR_ENABLED: false,
    
    /** Take measurements from water level sensor */
    WATER_LEVEL_SENSOR_ENABLED: false,

    /** Take measurements from water presence sensor */
    WATER_PRESENCE_SENSOR_ENABLED: false,

    /** Take measurements from Atmos41 weather station */
    ATMOS41_ENABLED: false,

    /** Take measurements from soil moisture sensor */
    SOIL_MOISTURE_SENSOR_ENABLED: false,

    /** Return dummy values when measuring water quality */
    MEASURE_DUMMY_WATER_QUALITY: false,

    /** Return dummy values when measuring water level */
    MEASURE_DUMMY_WATER_LEVEL: false,

    /** Return dummy values when measuring weather data */
    MEASURE_DUMMY_WEATHER: false,

    LIGHTNING_SENSOR_ENABLED: false,

    /** Sync RTC automatically on preset intervals */
    RTC_AUTO_SYNC: true,

    /** Use external RTC when syncing time */
    EXTERNAL_RTC_ENABLED: true,

    /** Submit data to IPFS */
    IPFS: true
}; 

/** Print serial comms between the MCU and the GSM module (used by tinyGSM) */
#define PRINT_GSM_AT_COMMS false

/** Size of DataStore's buffer that holds uncommited data. Once this buffer
 is full, data is commited to flash */
const int DATA_STORE_BUFFER_ELEMENTS = 10;

/******************************************************************************
* RTC/Time
******************************************************************************/
/** NTP server used by GSM module for time sync */
const char NTP_SERVER[] PROGMEM = "pool.ntp.org";

/** Intervals on which RTC will be automatically synced when RTC auto sync is ON.
 * Interval is checked on call home so it is rounded to call home intervals */
const int RTC_AUTOSYNC_INTERVAL_MIN = 240;
/** 
 * Channel to use when capturing data from Water Level sensor 
*/
// const WaterLevelChannel WATER_LEVEL_INPUT_CHANNEL = WATER_LEVEL_CHANNEL_DFROBOT_ULTRASONIC_SERIAL;
const WaterLevelChannel WATER_LEVEL_INPUT_CHANNEL = WATER_LEVEL_CHANNEL_MAXBOTIX_PWM;
// const WaterLevelChannel WATER_LEVEL_INPUT_CHANNEL = WATER_LEVEL_CHANNEL_MAXBOTIX_SERIAL;

const AquatrollModel AQUATROLL_MODEL = AQUATROLL_MODEL_400;
// const AquatrollModel AQUATROLL_MODEL = AQUATROLL_MODEL_500;
// const AquatrollModel AQUATROLL_MODEL = AQUATROLL_MODEL_600;

/**
 * FineOffset weather station source: sniffer/uart
 */
// const FineOffsetSource FO_SOURCE = FO_SOURCE_SNIFFER;
const FineOffsetSource FO_SOURCE = FO_SOURCE_UART;

/**
 * Lightning sensor module to use
 */
#define LIGHTNING_SENSOR_MODULE LIGHTNING_SENSOR_DFROBOT

/** DFRobot */
#ifndef LIGHTNING_SENSOR_MODULE
#error "Lightning sensor module not set"
#endif

#if LIGHTNING_SENSOR_MODULE == LIGHTNING_SENSOR_DFROBOT

const uint8_t LIGHTNING_I2C_ADDR = 0x03;
const uint8_t LIGHTNING_DIV_RATIO = 16;
const uint8_t LIGHTNING_TUNE_CAP = 120;

#elif LIGHTNING_SENSOR_MODULE == LIGHTNING_SENSOR_CJMCU
/** CJMCU */
const uint8_t LIGHTNING_I2C_ADDR = 0x01;
const uint8_t LIGHTNING_DIV_RATIO = 16;
const uint8_t LIGHTNING_TUNE_CAP = 8;
#else
#error Invalid Lightning sensor module specified
#endif

/**
 * Battery gauge
 */
const int BAT_GAUGE_FULL_MAH = 3000;



/******************************************************************************
* Sleep/wake up schedules
******************************************************************************/
/**
 * Valid interval values
 * Values are valid when they occur at the same minute/hour every time
 */
const int WAKEUP_SCHEDULE_VALID_VALUES[] = {0, 1, 2, 3, 4, 5, 10, 15, 20, 30, 60, 120, 240, 360, 480, 720, 1440};

/**
 * Default schedule used when battery levels normal and custom settings have
 * not been set by user yet
 */
const SleepScheduler::WakeupScheduleEntry WAKEUP_SCHEDULE_DEFAULT[] =
{
    {SleepScheduler::WakeupReason::REASON_CALL_HOME, 30},
    {SleepScheduler::WakeupReason::REASON_READ_WATER_SENSORS, 10},
    {SleepScheduler::WakeupReason::REASON_READ_WEATHER_STATION, 10},
    {SleepScheduler::WakeupReason::REASON_READ_SOIL_MOISTURE_SENSOR, 10}
};

/** Length of wake up schedule. Len must be known so it can be written to DeviceConfig */
const int WAKEUP_SCHEDULE_LEN = sizeof(WAKEUP_SCHEDULE_DEFAULT) / sizeof(WAKEUP_SCHEDULE_DEFAULT[0]);

/**
 * Used when battery levels critical (user/default settings ignored).
 * Current set schedule is overriden
 */
const SleepScheduler::WakeupScheduleEntry WAKEUP_SCHEDULE_BATT_LOW[WAKEUP_SCHEDULE_LEN] =
{
    {SleepScheduler::WakeupReason::REASON_CALL_HOME, 120},
    {SleepScheduler::WakeupReason::REASON_READ_WATER_SENSORS, 60},
    {SleepScheduler::WakeupReason::REASON_READ_WEATHER_STATION, 60},
    {SleepScheduler::WakeupReason::REASON_READ_SOIL_MOISTURE_SENSOR, 60}
};

/******************************************************************************
 * FineOffset sniffer\
 *****************************************************************************/
const float FO_SNIFFER_FREQ = 868.35;

/******************************************************************************
 * Lightning sensor
 *****************************************************************************/
const LightningEnvironment LIGHTNING_ENVIRONMENT = LIGHTNING_ENV_INDOOR;

/******************************************************************************
 * Power
 *****************************************************************************/
/**
 * From this level and below battery level is considered low.
 * Battery level affects settings used (eg. measuring intervals)
*/ 
const int BATTERY_LEVEL_LOW = 50;

/**
 * Level at which device will sleep until its recharged.
 * When in this mode, device wakes up every SLEEP_CHARGE_CHECK_INT_MINS to check
 * if battery reached BATTERY_LEVEL_SLEEP_RECHARGED.
*/
const int BATTERY_LEVEL_SLEEP_CHARGE = 15;
const int BATTERY_LEVEL_SLEEP_RECHARGED = 25;

const int SLEEP_CHARGE_CHECK_INT_MINS = 60;

#endif