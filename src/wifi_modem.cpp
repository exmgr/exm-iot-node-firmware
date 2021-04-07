// Include only when WiFi features enabled or binary will be large
#include "app_config.h"
#include "wifi_modem.h"
#include "const.h"
#include "common.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <wifi_modem.h>

/******************************************************************************
* WiFi Modem
* Named WiFiModem to avoid conflicts and confusion with ESP32 wifi libs
******************************************************************************/
namespace WifiModem
{
	//
	// Private vars
	// 
	/******************************************************************************
	* Init
	******************************************************************************/
	void init()
	{
	}

	/******************************************************************************
	* Connect to network
	******************************************************************************/
	RetResult connect()
	{
		Serial.print(F("Connecting to WiFi network: "));
		Serial.print(WIFI_SSID);

		// Already connected? Ignore
		if(WiFi.isConnected())
		{
			debug_println();
			debug_println_i(F("WiFi already connected."));
			return RET_OK;
		}

		WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
		
		int connect_timeout_sec = WIFI_CONNECT_TIMEOUT_SEC;
		while(WiFi.status() != WL_CONNECTED && connect_timeout_sec--)
		{
			delay(1000);
			Serial.print(".");
		}
		Serial.println();

		if(WiFi.status() != WL_CONNECTED)
		{
			if(WiFi.status() == WL_NO_SSID_AVAIL)
			{
				Serial.println(F("SSID not found."));
			}
			else
			{
				Serial.println(F("Could not connected."));
			}
			return RET_ERROR;
		}

		Serial.println(F("WiFi connected!"));

		return RET_OK;
	}

	/******************************************************************************
	* Disconnect from network
	******************************************************************************/
	RetResult disconnect()
	{
		WiFi.disconnect();

		return RET_OK;
	}

	/******************************************************************************
	* Check if connected to network
	* @return True when connected
	******************************************************************************/
	bool is_connected()
	{
		return WiFi.isConnected();
	}
}

