#include <ctime>
#include <string>
#include "app_config.h"
#include "const.h"
#include <TinyGsmClient.h> // Must be after const.h where tiny gsm constants are set
#include "gsm.h"
#include "rtc.h"
#include "struct.h"
#include "utils.h"
#include "log.h"
#include <Wire.h>
#include "common.h"
#include "wifi_modem.h"
#include "device_config.h"

#define LOGGING 1
#include <ArduinoHttpClient.h>

#define _gsm_serial Serial1

namespace GSM
{
//
// Private vars
//
/** Hardware serial port to be used by the GSM lib */
// HardwareSerial _gsm_serial(GSM_SERIAL_PORT);

/** Tick of power off */
uint32_t _power_toggle_ms = 0;

/** TinyGSM instance */
#if PRINT_GSM_AT_COMMS
#include <StreamDebugger.h>
/** StreamDebugger object to ouput communication between GSM module and MCU to serial console */
StreamDebugger debugger(_gsm_serial, Serial);
TinyGsm _modem(debugger);
#else
TinyGsm _modem(_gsm_serial);
#endif

//
// Private functions
//
void power_cycle();
void pwr_key_toggle();
void pwr_reset();
bool is_fona_serial_open();
int pwr_toggle_in_progress();
void init_uart();

/******************************************************************************
 * Init GSM functions 
 ******************************************************************************/
void init()
{
	#if WIFI_DATA_SUBMISSION
		WifiModem::init();
		return;
	#endif

	pinMode(PIN_GSM_PWR_KEY, OUTPUT);
	pinMode(PIN_GSM_RESET, OUTPUT);
	// pinMode(PIN_GSM_POWER_ON, OUTPUT);

	// If GSM is ON on init, turn it off
	if(is_on(500))
	{
		debug_println_w(F("GSM is ON on init, turning OFF."));
		off();
	}
}

/*****************************************************************************
* Power ON and connect
*****************************************************************************/
RetResult on()
{
	#if WIFI_DATA_SUBMISSION
		// No on in WiFiMode, always succeed
		return RET_OK;
	#endif

	debug_println(F("GSM ON"));

	Log::log(Log::GSM_ON);

	// If power OFF in progress, wait until enough time passed before proceeding
	uint32_t pwr_toggle_left_ms = pwr_toggle_in_progress();
	if(pwr_toggle_left_ms > 0)
	{
		debug_println_i(F("Power toggle in progress from previous request, waiting until it's finished."));
		delay(pwr_toggle_left_ms);
	}

	if(is_on(500))
	{
		debug_println_i(F("GSM already on"));
		return RET_OK;
	}

	// Reset before toggling power pin. In case it was already ON (which means power ON was not detected properly),
	// this will prevent module from powering OFF.
	pwr_reset();
	pwr_key_toggle();
	debug_println(F("Turning ON"));
	
	delay(GSM_WAIT_AFTER_PWR_ON_MS);

	#ifdef TCALL_H
		// Turn IP5306 power boost OFF to reduce idle current
		Utils::ip5306_set_power_boost_state(true);
	#endif

	// If end not called before calling begin again, it results in a guru meditation error sometimes
	// (needs confirmation)
	init_uart();
	delay(8000);

	//_modem.restart();
	debug_println("Modem init...");

	if (!_modem.init())
	{
		Log::log(Log::GSM_INIT_FAILED);
		debug_println(F("Could not init."));
		return RET_ERROR;
	}

	String modem_info = _modem.getModemInfo();
	debug_print(F("GSM module: "));
	debug_println(modem_info.c_str());

	return RET_OK;
}

/*****************************************************************************
* Power OFF
*****************************************************************************/
RetResult off()
{
	#if WIFI_DATA_SUBMISSION
		WifiModem::disconnect();
		return RET_OK;
	#endif

	debug_println(F("GSM OFF"));

	Log::log(Log::GSM_OFF);
	
	// If power toggle in progress, wait until enough time passed before re-toggling
	uint32_t pwr_toggle_left_ms = pwr_toggle_in_progress();
	if(pwr_toggle_left_ms > 0)
	{
		debug_println_i(F("Power toggle in progress from previous request, waiting until it's finished."));
		delay(pwr_toggle_left_ms);
	}

	if(!is_on())
	{
		debug_println_i(F("GSM Already OFF."));
		return RET_OK;
	}

	Serial.println(F("Turning OFF"));
	pwr_key_toggle();

	_power_toggle_ms = millis();

	#ifdef TCALL_H
		// Turn IP5306 power boost OFF to reduce idle current
		Utils::ip5306_set_power_boost_state(false);
	#endif

	return RET_OK;
}


/******************************************************************************
 * Toggle power key to turn ON/OFF
 *****************************************************************************/
void pwr_key_toggle()
{
	_power_toggle_ms = millis();

	// TSIM
	digitalWrite(PIN_GSM_PWR_KEY, 0);
	delay(100);
	digitalWrite(PIN_GSM_PWR_KEY, 1);
	delay(1500);
	digitalWrite(PIN_GSM_PWR_KEY, 0);
}

/******************************************************************************
 * Connect to the network and enable GPRS
 *****************************************************************************/
RetResult connect()
{
	// Override GSM
	#if WIFI_DATA_SUBMISSION
		debug_println(F("Connecting WiFi"));
		return WifiModem::connect();
	#endif

	debug_println(F("Initializing GSM"));

	// Configure NBIOT
	#ifdef TINY_GSM_MODEM_SIM7000
		if(FLAGS.NBIOT_MODE)
		{
			debug_println(F("NBIoT mode"));
			_modem.setPreferredMode(38);
			_modem.setPreferredLTEMode(2);
			_modem.setOperatingBand(20); 		
		}
		else
		{
			debug_println(F("GSM mode"));
			_modem.setPreferredMode(13); //2 Auto // 13 GSM only // 38 LTE only

		}
	#endif

	debug_println(F("Waiting for network connection..."));
	if (!_modem.waitForNetwork(GSM_DISCOVERY_TIMEOUT_MS))
	{
		debug_println_e(F("Network discovery failed."));
		Log::log(Log::GSM_NETWORK_DISCOVERY_FAILED);
		return RET_ERROR;
	}

	debug_println_i(F("GSM connected"));

	int8_t rssi = get_rssi();
	Utils::serial_style(STYLE_BLUE);
	debug_print(F("RSSI: "));
	debug_println(rssi, DEC);

	debug_print(F("Operator: "));
	debug_println(_modem.getOperator());
	Utils::serial_style(STYLE_RESET);

	if (enable_gprs(true) != RET_OK)
	{
		Log::log(Log::GSM_GPRS_CONNECTION_FAILED);
		return RET_ERROR;
	}

	GSM::print_system_info();

	return RET_OK;
}

/******************************************************************************
 * Calls connect() X times until it succeeds and power cycles GSM module
 * inbetween failures
 ******************************************************************************/
RetResult connect_persist()
{
	int tries = GSM_TRIES;
	int success = false;

	// Try to connect X times before aborting, power cycling in between
	while (tries--)
	{
		if (connect() == RET_ERROR)
		{
			debug_print(F("Connection failed. "));

			if (tries)
			{
				debug_println(F("Cycling power and retrying."));
			}

			// Before last try, reset module to defaults because
			// sometimes if the device started in NBIoT mode, when it is reverted back to GSM mode it has
			// trouble connecting to network. So far the only thing that remedies this is an ATZ command
			// (reset to defaults)
			if(tries == 1)
			{
				GSM::factory_reset();
			}

			power_cycle();
		}
		else
		{
			success = true;
			break;
		}
	}

	if (!success)
	{
		debug_println(F("Could not connect. Aborting."));

		return RET_ERROR;
	}
	else
		return RET_OK;
}

/*****************************************************************************
* Power cycle
*****************************************************************************/
void power_cycle()
{
	// Override GSM
	#if WIFI_DATA_SUBMISSION
		return;
	#endif

	off();
	delay(500);
	on();
}

/*****************************************************************************
* Update SIM module's time from NTP and return it
*****************************************************************************/
RetResult update_ntp_time()
{	
	#if WIFI_DATA_SUBMISSION
		configTime(0, 0, NTP_SERVER);

		// If time is now valid, success
		if(RTC::tstamp_valid(RTC::get_timestamp()))
		{
			return RET_OK;
		}
		else
		{
			return RET_ERROR;
		}
	#elif defined(TINY_GSM_MODEM_SIM7000)
		debug_print(F("NTP Server: "));
		debug_println(NTP_SERVER);

		_modem.sendAT(GF("+CNTPCID=1"));
		if (_modem.waitResponse(10000L)!= 1)
			return RET_ERROR;

		// _modem.sendAT(GF("+CNTP=pool.ntp.org,0,1,2"));

		_modem.sendAT(GF("+CNTP="), NTP_SERVER, ',', 0, ',', 1, ',', 2);
		if (_modem.waitResponse(10000L)  != 1)
			return RET_ERROR;

		_modem.sendAT(GF("+CNTP"));
		if(_modem.waitResponse(15000L, GF(GSM_NL "+CNTP:")))
		{
	        String code = _modem.stream.readStringUntil(',');
			debug_println();
			if(code.toInt() != 1)
			{
				debug_print_e(F("NTP Error: "));
				debug_println(code.toInt(), DEC);
				return RET_ERROR;
			}

			_modem.streamSkipUntil('"');
			String timestring = _modem.stream.readStringUntil('"');

			debug_print(F("NTP Returned: "));
			debug_println(timestring);

			tm ntp_time = {0};
			if(parse_time(timestring.c_str(), &ntp_time, false) == RET_OK)
			{
				return RTC::tstamp_valid(mktime(&ntp_time)) ? RET_OK : RET_ERROR;
			}
			else
				return RET_ERROR;
		}
		else
			return RET_ERROR;

		return RET_OK;
	#else
		debug_print(F("Updating time from: "));
		debug_println(NTP_SERVER);

		if (_modem.NTPServerSync(F(NTP_SERVER), 0) == -1)
		{
			return RET_ERROR;
		}
	#endif

	return RET_OK;
}

/*****************************************************************************
* Get time from module
*****************************************************************************/
RetResult get_time(tm *out)
{
	#if WIFI_DATA_SUBMISSION
		debug_println(F("GSM override is enabled, get_time is disabled."));
		return RET_ERROR;
	#endif
	
	char buff[26] = "";

	// Returned format: "30/08/87,05:49:54+00"
	String date_time = _modem.getGSMDateTime(DATE_FULL);

	debug_print(F("Got time: "));
	debug_println(date_time.c_str());

	if (date_time.length() < 1)
	{
		debug_println(F("Could not get time from GSM."));
		return RET_ERROR;
	}

	return parse_time(date_time.c_str(), out, true);
}

/******************************************************************************
* Parse time string returned by SIM time functions
******************************************************************************/
RetResult parse_time(const char *timestring, tm *out, bool gsm_time)
{
	debug_print(F("Parsing time string: "));
	debug_println(timestring);

	// NTP and GSM time functions return differently formmated year in the timestring
	char format_ntp[] = "%04d/%02d/%02d,%02d:%02d:%02d";
	char format_gsm[] = "%02d/%02d/%02d,%02d:%02d:%02d";
	char *format = gsm_time ? format_gsm : format_ntp;

	if (6 != sscanf(timestring, format,
				&(out->tm_year), &(out->tm_mon), &(out->tm_mday),
				&(out->tm_hour), &(out->tm_min), &(out->tm_sec)))
	{
		debug_println_e(F("Could not parse time string."));

		// 0 vars parsed
		return RET_ERROR;
	}

	// Year in GSM time is YY so counts from 2000
	// Year in NTP time is YYYY
	// Must count from 1900 for tm
	if(gsm_time)
		out->tm_year += 100;
	else
		out->tm_year -= 1900;

	// Month counts from 01, must count from 00
	out->tm_mon--;

	// char buff[100] = "";
	// strftime(buff, 26, "%Y-%m-%d %H:%M:%S", out);
	// debug_print(F("Parsed time struct: "));
	// debug_println(buff);

	time_t timestamp = mktime(out);
	debug_print(F("Parsed time: "));
	debug_println(ctime(&timestamp));

	return RET_OK;
}

/******************************************************************************
 * Enable/disable GPRS
 *****************************************************************************/
RetResult enable_gprs(bool enable)
{
	#if WIFI_DATA_SUBMISSION
		// On wifi mode, always succeeds
		return RET_OK;
	#endif

	if (enable)
	{
		const char *apn = DeviceConfig::get_cellular_apn();
		// Get APN from device descriptor
		debug_print(F("Connecting to APN: "));
		debug_println(apn);

		if (!_modem.gprsConnect(apn, "", ""))
		{
			debug_println(F("Could not connect data."));
			return RET_ERROR;
		}

		debug_println(F("Data connected."));
	}
	else
	{
		if (!_modem.gprsDisconnect())
		{
			debug_println(F("Could not disable data connection."));
			return RET_ERROR;
		}

		debug_println(F("Data disconnected."));
	}

	return RET_OK;
}

/******************************************************************************
* Convert value returned by SIM into dBm and return
* @return RSSI in dBm
******************************************************************************/
int get_rssi()
{
	#if WIFI_DATA_SUBMISSION
		debug_println(F("GSM override is enabled, RSSI will be 0."));
		return 0;
	#endif

	uint16_t rssi_sim = _modem.getSignalQuality();
	int16_t rssi = 0;

	if (rssi_sim == 0)
		rssi = -115;
	if (rssi_sim == 1)
		rssi = -111;
	if (rssi_sim == 31)
		rssi = -52;
	if ((rssi_sim >= 2) && (rssi_sim <= 30))
		rssi = map(rssi_sim, 2, 30, -110, -54);

	return rssi;
}

/******************************************************************************
* Get network system mode
******************************************************************************/
RetResult print_system_info()
{
	_modem.sendAT(F("+CPSI?"));

	if (_modem.waitResponse("+CPSI:") != 1)
	{
		return RET_ERROR;
    }

	String res = _modem.stream.readStringUntil('\n');

	debug_println_i(F("UE system information"));
	debug_println(res);

	return RET_OK;
}

/******************************************************************************
 * Get battery info from module
 * @param 
 *****************************************************************************/
RetResult get_battery_info(uint16_t *voltage, uint16_t *pct)
{
	#if WIFI_DATA_SUBMISSION
		debug_println(F("GSM override is enabled, getting battery info from GSM is disabled."));
		return RET_ERROR;
	#endif

	if (!(*voltage = _modem.getBattVoltage()))
	{
		*voltage = 0;
		debug_println(F("Could not read voltage from GSM module"));
		return RET_ERROR;
	}
	

	if (!(*pct = _modem.getBattPercent()))
	{
		*pct = 0;
		debug_println(F("Could not read battery pct from GSM module."));
		return RET_ERROR;
	}
	
	return RET_OK;
}

/******************************************************************************
* Factory reset modem
******************************************************************************/
RetResult factory_reset()
{
	debug_println_i(F("Restoring defaults on GSM modem."));

	_modem.streamWrite("ATZ", "\r\n");
    _modem.stream.flush();

    if(_modem.waitResponse() != 1)
	{
		return RET_ERROR;
    }
	else
		return RET_OK;
}

/******************************************************************************
 * Check if SIM card present by checking if its ready
 *****************************************************************************/
bool is_sim_card_present()
{
	#if WIFI_DATA_SUBMISSION
		// No SIM card in GSM mode, always succeed
		return RET_OK;
	#endif

	return _modem.getSimStatus() == SIM_READY;
}

/******************************************************************************
* Test AT interface to checkheck if GSM module power is on
* Will return false if device is still booting
* @return True when power is ON
******************************************************************************/
bool is_on(uint32_t timeout)
{
	#if WIFI_DATA_SUBMISSION
		// Always on in Wifi mode
		return RET_OK;
	#endif

	init_uart();

	// modem.init() must have been already run for this to work??
	return _modem.testAT(timeout);
}

/******************************************************************************
* Check if GPRS connection is on
* @return True when GPRS is connected
******************************************************************************/
bool is_gprs_connected()
{
	#if WIFI_DATA_SUBMISSION
		return WifiModem::is_connected();
	#endif

	return _modem.isGprsConnected();
}

/******************************************************************************
* Check whether a power ON/OFF is in progress (ie. power ON/OFF was requested
* and the predefined time has not passed yet).
* If interval between power ON/OFF is too small, toggling power might not have
* any effect (ie. requesting power OFF while the device is still booting after
* a power ON)
* @return mS left until PWR toggle is finished
******************************************************************************/
int pwr_toggle_in_progress()
{
	uint32_t time_since_pwr_toggle = millis() - _power_toggle_ms;
	if(_power_toggle_ms > 0 && time_since_pwr_toggle <= GSM_WAIT_AFTER_PWR_TOGGLE_MS)
	{
		return GSM_WAIT_AFTER_PWR_TOGGLE_MS - time_since_pwr_toggle;
	}
	else
		return 0;
}

/******************************************************************************
 * Toggle reset pin
 *****************************************************************************/
void pwr_reset()
{

	debug_println_i(F("Resetting"));
	digitalWrite(PIN_GSM_RESET, 0);
	delay(500);
	digitalWrite(PIN_GSM_RESET, 1);
}

/******************************************************************************
 * Init UART port for GSM module
 *****************************************************************************/
void init_uart()
{
	// Without end() = guru meditation error. (only when .begin already ran?)
	// Without long enough delay = same error
	_gsm_serial.end(); 
	delay(200);

	_gsm_serial.begin(GSM_SERIAL_BAUD, SERIAL_8N1, PIN_GSM_RX, PIN_GSM_TX);
	delay(50);
}

/******************************************************************************
 * Get TinyGsm object
 *****************************************************************************/
TinyGsm *get_modem()
{
	return &_modem;
}

} // namespace GSM