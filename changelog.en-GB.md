[1.50]
* Added IPFS support. Weather station measurements are submitted to IPFS during data submission.

[1.49]
* RTC auto-sync feature, where RTC is automatically synced on preset intervals.

[1.48]
* Added NTP sync support to TSIM7000.

[1.45]
* Lightning sensor tuned for improved precision. 

[1.44]
* Device can now keep working if external RTC fails by relying solely on the internal RTC.

[1.43]
* More robust handling on errors during data submission.
* Improved wind speed calculations for FineOffset weather stations.
* Added lightning sensor mode selection switch
* Added support for LTC2941 coulomb counter.
* Added INA218 current monitor support for monitoring solar power.

[1.42]
* Added Lightning sensor on/off flag.
* Added RTC sync remote control command.

[1.41]
* Added Aquatroll600 support.

[1.36]
* Added support for AS3936 lightning sensor.

[1.35]
* Added support DFRobot A02YYUW ultrasonic sensor. 

[1.33]
* FineOffset RF sniffing is automatically disabled to conserve energy after a number of unsuccessful attempts to receive a packet from a weather station.

[1.32]
* Added FineOffset sniffer auto-scan feature where the device listens for packets from a weather station and pairs itself to that specific weather station.
* Device now sleeps when waiting for the RF module to return a packet and is awaken on interrupt when a packet is received, to conserve energy.
* Submit more detailed configuration as device attributes.

[1.24]
* Improved ultrasonic sensor data filtering.

[1.23]
* Soil moisture measurement intervals can now be changed via remote control.

[1.19]
* Device can now be booted into "config mode" by holding user button for 3 seconds during boot. In config mode, device-specific configuration can be set via a serial console.

[1.18]
* Added support for RF sniffing telemetry data from FineOffset weather stations and logging them as telemetry.

[1.17]
* Implemented custom SDI12 driver.
* Added support for DFRobot throw-in liquid pressure sensor.
* Added support for Teros12 soil moisture sensor.
* Added support for DFRobot ultrasonic level sensor.
* Added support for serial channel on Maxbotix ultrasonic sensors.
* Added support for Aquatroll500 water quality sensor.

[1.16]
* Added TTGo TSIM support

[1.14]
* Adafruit FONA support discontinued
* SDI12 communications can be logged and submitted to TB for debugging.

[1.13]
* Added support for redirecting serial debug output via WiFi to timber.io for debugging.

[1.12]
* Added debug message level macros
* Added Thingsboard fallback device for when user config is corrupted.

[1.11]
* Sleep/sensor intervals are now measured as a reference of current time instead of the device boot time for more predictable operation.
* Added dew point to weather data

[1.8]
* Use external RTC to compensate drifting of external RTC for more precise wakeup intervals.

[1.7]
* Added support for Atmos41 weather station.
* SPIFFS file system can be formmated via remote-control.
* Added enable/disable flags for water level/quality sensors.
* Application-config flags are submitted as Thingsboard device attributes.

[1.3]
* Added support for PWM channel on MaxBotix ultrasonic sensors.
* Current device configuration is submitted as Thingsboard device attributes.

[1.2]
* Added low battery mode in which user-set sleep intervals are ignored and preset low-rate intervals are used instead to save battery.
* Added sleep-charge mode which puts the device in sleep when battery level is critically low and all functionality is disabled. Device functionality is restored only when battery is charged above a preset level.

[1.1]
* More detailed logging.
* Self-test after OTA. If fails, firmware is rolled back to previous version.

[1.0]
* Added support for OTA.
* GSM backend completely refactored to use TinyGSM.
* Added per-board config file.
* Added Support for WiPy.
* Added support for Adafruit Feather.

[0.3]
* Read battery voltage/pct from GSM module and add as diagnostics.
* Sync time from HTTP when everything else fails.
* Added BME280 for monitoring internal environment conditions.

[0.2]
* Send diagnostics as telemetry to Thingsboard.
* Added NBIoT support when using SIM7000.
* Added NodeMCU support.

[0.1]
* Added TTGO TFox compatibility.
* Added remote config.