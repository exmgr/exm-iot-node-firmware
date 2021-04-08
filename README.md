
## Initialization
The preparation of the EXM IoT sensor node for deployment consists of the following steps:
1. Configure and flash firmware
    * Set up credentials
    * Set up app config flags
    * Flash
2. Create Thingsboard device
3. Boot physical device in config mode to set Thingsboard device access token and cellular network APN

### Setting up credentials
Copy ```includes/credentials.h.template``` to ```includes/credentials.h```.
Open the project in [PlatformIO](https://platformio.org), navigate to **includes/credentials.h** and update the following:

* **TB_SERVER/TB_PORT** Thingsboard server URL/port
* **FALLBACK_TB_DEVICE_TOKEN** Token of the Thingsboard device to fall back to when communication fails with the set token or configuration memory is corrupted.
Note that these variables are defined separately for **debug** and **release** builds.

### Setting up app config
Various application specific flags can be set by configuring ```FLAGS``` in ```/include/app_config.h```:
* **LOG_RAW_SDI12_COMMS** Log raw communications with SDI12 sensors (for debugging)
* **WIFI_DEBUG_CONSOLE_ENABLED** Redirect console output to wifi via Timber.io (set up credentials in credentials.h)
* **WIFI_DATA_SUBMISSION_ENABLED** Use WiFi for communications instead of GSM
* **BATTERY_GAUGE_ENABLED** Log battery gauge data
* **SOLAR_CURRENT_MONITOR_ENABLED** Log solar charge current
* **NBIOT_MODE** Enable NBIoT mode
* **SLEEP_MINS_AS_SECS** Treat minutes as seconds when calculating sleep. Useful for debugging.
* **BATTERY_FORCE_NORMAL_MODE** Ignore low/critical battery modes when battery below threshold.
* **WATER_QUALITY_SENSOR_ENABLED** Enable water quality sensor logging.
* **WATER_LEVEL_SENSOR_ENABLED** Enable water level sensor logging.
* **ATMOS41_ENABLED** Enable Atmos41 weather station logging.
* **MEASURE_DUMMY_WATER_QUALITY** Log dummy data for water quality sensor (for debugging).
* **MEASURE_DUMMY_WATER_LEVEL** Log dummy data for water level sensor (for debugging).
* **MEASURE_DUMMY_WEATHER** Log dummy data for weather station (for debugging).
* **LIGHTNING_SENSOR_ENABLED** Enable lightning sensor.
* **RTC_AUTO_SYNC** Automatically sync RTC on intervals.
* **EXTERNAL_RTC_ENABLED** Use external RTC for time keeping instead of ESP32's internal.
* **IPFS** Submit weather data to IPFS (set up credentials in credentials.h)

When done build and flash.

### Setting up device config
To configure device specific settings, the weather station must be booted in config mode while connected via USB to a serial monitor like [PuTTY](https://www.putty.org) or [Minicom](https://www.putty.org). To enter config mode hold the the config mode button on the top right of the PCB and power cycle the device. The on board LED will flash three times to denote that device has booted in config mode.
Enter the following commands (values are entered without brackets):

    TB_DEVICE_TOKEN=[your tb device token]

    APN=[cellular network APN]

To confirm that variable values have been set correctly, they can be read back by appending a questino mark at the end:

    TB_DEVICE_TOKEN=?

    APN=?
	
	
