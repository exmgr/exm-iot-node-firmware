#ifndef LOG_CODES_H
#define LOG_CODES_H

namespace Log
{
    enum Code
    {
        //
        // General codes
        // All 0XX codes
        
        // Device boot
        BOOT = 1,

        // Intentional restart
        RESTART = 2,

        // Calling home handling
        CALLING_HOME = 3,

        // Syncing time
        TIME_SYNC = 4,

        // Sensor data submitted
        // Meta1: Total entries submitted
        // Meta2: Number of CRC failures
        SENSOR_DATA_SUBMITTED = 5,

        // Sensor data submission errors
        // Meta 1: Total HTTP requests
        // Meta 2: Failed HTTP requests
        SENSOR_DATA_SUBMISSION_ERRORS = 6,

        // Log data submitted
        // Meta1: Total entries submitted
        // Meta2: Entries that failed CRC check
        LOG_SUBMITTED = 7,

        // CRC error detected on loaded config data
        DEVICE_CONFIG_DATA_CRC_ERRORS = 11,

        // Device going to sleep
        // Logged only for the main sleep event
        // Meta1: Time awake (s)
        // Meta2: Time to sleep (s)
        SLEEP = 12,

        // Device waking up from sleep
        // Logged only for the main sleep event
        // Meta1: Reason
        WAKEUP = 13,

        // Syncing time with NTP failed
        NTP_TIME_SYNC_FAILED = 14,

        // Battery measurements from fuel gauge
        // Meta1: Voltage
        // Meta2: Percentage
        BATTERY = 15,

        // Internal environmental sensor measurements (1)
        // Meta1: Temperature (float)
        // Meta2: Relative humidity (float)
        INT_ENV_SENSOR1 = 16,

        // Internal environmental sensor measurements (2)
        // Meta1: Pressure (hPa - int)
        // Meta2: Alt (meters - int)
        INT_ENV_SENSOR2 = 17,        

        // Remote control: Invalid response received (could not deserialize)
        RC_PARSE_FAILED = 19,

        // Remote control: JSON has invalid format
        RC_INVALID_FORMAT = 20,

        // Temperature measurement from RTC
        // Meta1: Temperature
        RTC_TEMPERATURE = 21,

        // Remote control: HTTP request for data failed
        // Meta1: HTTP response code
        RC_REQUEST_FAILED = 22,

        // New remote control data id received, data will be applied
        // Meta1: Data id
        RC_APPLYING_NEW_DATA = 23,

        // Remote control: new water sensors read interval set
        // Meta1: New value
        RC_WATER_SENSORS_READ_INT_SET_SUCCESS = 24,

        // Remote control: Could not set new water sensors read interval value
        // Meta1: Provided value
        RC_WATER_SENSORS_READ_INT_SET_FAILED = 25,

        // Remote control: new calling home interval set
        // Meta1: New value
        RC_CALL_HOME_INT_SET_SUCCESS = 26,

        // Remote control: Could not set new calling home interval value
        // Meta1: Provided value
        RC_CALL_HOME_INT_SET_FAILED = 27,

        // OTA: Requested by remote control data
        OTA_REQUESTED = 28,

        // OTA: Request doesnt contain fw version
        // Meta1: Current fw version
        OTA_NO_FW_VERSION_SPECIFIED = 29,

        // OTA: Provided FW is the same version as the one already running
        // Meta1: Current fw version
        OTA_FW_SAME_VERSION = 30,

        // OTA: FW URL not provided or empty
        OTA_FW_URL_NOT_SET = 31,

        // OTA: MD5 of provided firmware is not specified
        OTA_MD5_NOT_SET = 32,

        // OTA: Url is invalid
        OTA_URL_INVALID = 33,

        // OTA: File get request failed
        // Meta1: HTTP response code
        OTA_FILE_GET_REQ_FAILED = 34,

        // OTA: File get req did not return OK (200)
        // Meta1: HTTP response code
        OTA_FILE_GET_REQ_BAD_RESPONSE = 35,

        // OTA: File get req response empty
        // Meta1: HTTP response code
        OTA_FILE_GET_REQ_RESP_EMPTY = 36,

        // OTA: Update.begin() failed
        // Meta1: Error code returned by getError()
        OTA_UPDATE_BEGIN_FAILED = 37,

        // OTA: Downloading FW file and writing to partition
        // Meta1: File size as specified in Content-length
        OTA_DOWNLOADING_AND_WRITING_FW = 38,

        // OTA: FW writing complete
        // Meta1: Total bytes
        // Meta2: Bytes flashed
        OTA_WRITING_FW_COMPLETE = 39,

        // OTA: Update not finished
        // Meta1: Error code returned by getError()
        OTA_UPDATE_NOT_FINISHED = 40,

        // OTA: Could not validate and finalize update
        // Meta1: Error code returned by getError()
        OTA_COULD_NOT_FINALIZE_UPDATE = 41,

        // OTA: Downloaded and written to partition succesfully.
        OTA_FINISHED = 42,

        // OTA: Self test passed
        OTA_SELF_TEST_PASSED = 43,

        // OTA: self test failed
        OTA_SELF_TEST_FAILED = 44,

        // OTA: FW is rolled back to old version and device rebooted
        OTA_ROLLING_BACK = 45,

        // OTA: Rolling back to previous FW not possible
        OTA_ROLLBACK_NOT_POSSIBLE = 46,

        // File system free space
        // Meta1: Used bytes
        // Meta2: Free bytes
        FS_SPACE = 47,

        // RTC sync took place
        // Time after sync is log time
        // Meta1: System time at the start of sync
        RTC_SYNC = 48,

        //
        // Could not publish tb client attributes
        // Meta1: HTTP response code
        TB_CLIENT_ATTR_PUBLISH_FAILED = 49,

        //
        // Entered sleep charge
        SLEEP_CHARGE = 50,

        //
        // Sleep charge finished
        // Meta1: Times woke up to check if battery back to acceptable levels
        SLEEP_CHARGE_FINISHED = 51,

        //
        // Could not read water quality sensor
        //
        WATER_QUALITY_MEASUREMENT_FAILED = 52,

        //
        // Could not read water level sensor
        //
        WATER_LEVEL_MEASURE_FAILED = 53,

        //
        // SPIFFS format complete
        // Meta1: Bytes used before format
        SPIFFS_FORMATTED = 54,

        //
        // SPIFFS format failed
        //
        SPIFFS_FORMAT_FAILED = 55,

        //
        // Water quality sensor invalid response
        //
        WATER_QUALITY_INVALID_RESPONSE = 56,

        //
        // Could not get water quality sensor measurements
        // Meta1: Batch number (x in SDI12 command aDx)
        WATER_QUALITY_MEASUREMENT_DATA_REQ_FAILED = 57,

        //
        // Water quality sensor returned all zero vals
        //
        WATER_QUALITY_ZERO_VALS = 58,

        //
        // Water quality sensor data CRC check failed
        //
        WATER_QUALITY_CRC_FAILED = 59,

        //
        // Weather station invalid response
        //
        WEATHER_STATION_INVALID_RESPONSE = 61,

        //
        // Could not get measurement data after measurement
        // Meta1: Batch number (x in SDI12 Command aDx)
        WEATHER_STATION_MEASUREMENT_DATA_REQ_FAILED = 62,

        // Remote control: new weather station read interval set
        // Meta1: New value
        RC_WEATHER_STATION_READ_INT_SET_SUCCESS = 63,

        //
        // Remote control: Could not set new weather station read interval value
        // Meta1: Provided value
        RC_WEATHER_STATION_READ_INT_SET_FAILED = 64,

        //
        // Wake up self test failed
        //
        WAKEUP_SELF_TEST_FAILED = 65,

        //
        // Weather station logging measurement 
        //
        WEATHER_STATION_MEASUREMENT_LOG = 66,

        //
        // Water sensors logging measurement
        //
        WATER_SENSORS_MEASUREMENT_LOG = 67,

        //
        // Provided schedule is invalid, default will be used
        // Meta1: Schedule id (1: Normal, 2: Low, 3:unknown)
        SCHEDULE_INVALID_USING_DEFAULT = 68,

        //
        // Could not determine battery mode
        //
        BATTERY_MODE_UNKNOWN = 69,

        //
        // Weather station meaasurement failed 
        //
        WEATHER_STATION_MEASUREMENT_FAILED = 70,

        //
        // Calling home finished
        //
        CALLING_HOME_END = 71,

        //
        // Device's mac address
        // Meta1: First 4 bytes
        // Meta2: Last 4 bytes
        MAC_ADDRESS = 72,

        //
        // Soil moisture sensors logging measurement
        //
        SOIL_MOISTURE_SENSOR_MEASUREMENT_LOG = 73,

        //
        // Could not read soil moisture sensor
        //
        SOIL_MOISTURE_MEASUREMENT_FAILED = 74,

        //
        // Solar panel voltage
        // Meta1: Voltage in mV
        SOLAR_PANEL_VOLTAGE = 75,

        //
        // Default device config is used
        //
        USING_DEFAULT_DEVICE_CONFIG = 76,

        //
        // Device woke up from sleep charge to check battery status
        //
        SLEEP_CHARGE_CHECK = 77,

        //
        // Soil moisture sensor invalid response
        //
        SOIL_MOISTURE_INVALID_RESPONSE = 78,

        //
        // Could not get soil moisture sensor measurements
        // Meta1: Batch number (x in SDI12 command aDx)
        SOIL_MOISTURE_MEASUREMENT_DATA_REQ_FAILED = 79,

        //
        // Soil moisture sensor returned all zero vals
        //
        SOIL_MOISTURE_ZERO_VALS = 80,

        // Remote control: new soil moisture sensors read interval set
        // Meta1: New value
        RC_SOIL_MOISTURE_READ_INT_SET_SUCCESS = 81,

        // Remote control: Could not set new soil moisture read interval value
        // Meta1: Provided value
        RC_SOIL_MOISTURE_READ_INT_SET_FAILED = 82,

        // FO Scan for new device started
        FO_SNIFFER_SCANNING = 83,

        // FO Scan finished
        // Meta1: Found FO node id
        FO_SNIFFER_SCAN_RESULT = 84,

        // FO Scan finished, no 
        FO_SNIFFER_SCAN_FINISHED = 85,

        // FO enabled status
        // Meta1: Prev status
        // Meta2: New status
        FO_ENABLED_STATUS = 86,

        // FO disabled because RX failures reached threshold
        // Meta1: Successive tries after which it was disabled
        FO_DISABLED_RX_FAILED = 87,

        // FO waited to sniff packet but packet not received
        FO_SNIFFER_SNIFF_FAILED = 88,

        // FO not in sync
        FO_SNIFFER_NOT_IN_SYNC = 89,

        // FO could not sync
        FO_SNIFFER_SYNC_FAILED = 90,

        // Time elapsed during data submission
        // Meta1: Telemetry
        // Meta2: Logs
        DATA_SUBMISSION_ELAPSED = 91,

        // Lightning noise events
        // Meta1: Noise events
        // Meta2: Disturber event
        LIGHTNING_IRQ_REPORT = 92,

        // Submission during call home was aborted
        CALL_HOME_SUBMISSION_ABORTED = 93,

        //
        // Battery gauge data
        // Meta1: mAh battery
        // Meta2: mAh expended
        BAT_GAUGE_DATA = 94,

        //
        // Solar monitor data
        // Meta1: voltage
        // Meta2: Current
        SOLAR_MONITOR_DATA = 95,

        //
        // GSM errors
        // All 1XX codes

        //
        // Couldn not connect GPRS
        //
        GSM_NETWORK_DISCOVERY_FAILED = 100,

        //
        // GSM RSSI
        // Meta1: RSSI
        GSM_RSSI = 101,

        //
        // Sim card not detected
        //
        GSM_NO_SIM_CARD = 102,

        //
        // Could not connect to GSM network
        //
        
        GSM_GPRS_CONNECTION_FAILED = 103,
        
        //
        // GSM init failed
        //
        GSM_INIT_FAILED = 104,

        //
        // GSM Module power turned ON
        //
        GSM_ON = 105,

        //
        // GSM Module power turned OFF
        //
        GSM_OFF = 106,

        //
        // Temporary codes only for debugging
        // All 2XX codes
        CALL_HOME_WAKEUP_MISSED = 200,

        //
        // Wakeup correction took place
        // Meta1: Seconds corrected
        WAKEUP_CORRECTION = 201,

        //
        // Wake up events missed
        // Meta1: Reasons (WakeupReasons bitfield)
        //
        WAKEUP_EVENTS_MISSED = 202,

        //
        // Raw value from water level sensor
        // Meta1: Value
        WATER_LEVEL_RAW_READING = 203,

        //
        // Current schedule log
        //
        SCHEDULE_CALL_HOME_INT = 204,
        SCHEDULE_WATER_SENSORS_INT = 205,
        SCHEDULE_WEATHER_STATION_INT = 206,
        SCHEDULE_FO_INT = 207,
        SCHEDULE_SOIL_MOISTURE_INT = 208,

        /*
        * Meta1: Index of unexpected value
        */
        FO_ERROR_UNEXPECTED_VALUE = 209,

        FO_ERROR_INVALID_PARAM_COUNT = 210,

        /**
         * Wake up count since last aggregated packet 
         * Meta1: Wake up count
         */
        FO_WAKEUPS = 211
    };
}

#endif