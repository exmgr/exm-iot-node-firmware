#include "flash.h"
#include "SPIFFS.h"
#include "utils.h"
#include "const.h"
#include "struct.h"
#include "log.h"
#include "common.h"

namespace Flash
{
	/********************************************************************************
	* Mount SPIFFS partition
	*******************************************************************************/
	RetResult mount()
	{
		int tries = 2;
		bool success = false;

		while(tries--)
		{
			if(SPIFFS.begin(false, "/spiffs", 25))
			{
				success = true;
				break;
			}
			else
			{
				debug_println(F("Could not mount SPIFFS."));
				if(tries > 1)
					debug_println(F("Retrying..."));					
			}
		}

		// If mounting failed, format partition then try mounting again
		if(!success)
		{
			debug_println(F("Could not mount SPIFFS, formatting partition..."));
			SPIFFS.format();

			if(SPIFFS.begin(false, "/spiffs", 50))
			{
				debug_println(F("Partition mount successful."));
				return RET_OK;
			}
			else
			{
				Utils::serial_style(STYLE_RED);
				debug_println(F("Mounting failed."));
				Utils::serial_style(STYLE_RESET);	
				return RET_ERROR;
			}
		}

		return RET_OK;
	}

	/********************************************************************************
	* Mount partition and read a file into a buffer
	*******************************************************************************/
	RetResult read_file(const char *path, uint8_t *dest, int bytes)
	{
		if(Flash::mount() != RET_OK)
		{
			debug_print(F("Could not mount flash."));
			return RET_ERROR;
		}

		File f = SPIFFS.open(path, FILE_READ);

		// File doesn't exist
		if(!f)
		{
			debug_println(F("Could not load file."));
			return RET_ERROR;
		}

		// Try to read file
		if(bytes != f.read(dest, bytes))
		{
			debug_println(F("Could not read file."));
			return RET_ERROR;
		}

		return RET_OK;
	}

	/********************************************************************************
	* Print all files and dirs in flash
	*******************************************************************************/
	void ls()
	{
		Utils::print_separator(F("Flash memory contents"));

		if(Flash::mount() != RET_OK)
		{
		    return;
		}

		File root = SPIFFS.open("/");
		if(!root)
		{
			debug_println(F("Could not open root."));
			return;
		}

		File cur_file;
		
		while(cur_file = root.openNextFile())
		{
			debug_print(cur_file.name());

			// Print size
			debug_print(F(" ["));
			debug_print(cur_file.size());
			debug_print(F("]"));
			debug_println();
		}

		Utils::print_separator(F("End flash memory contents"));
	}

	/******************************************************************************
	 * Format SPIFFS and log
	 *****************************************************************************/
	RetResult format()
	{
		if(SPIFFS.format())
		{
			Log::log(Log::SPIFFS_FORMATTED);
			return RET_OK;
		}
		else
		{
			Log::log(Log::SPIFFS_FORMAT_FAILED);
			return RET_ERROR;
		}
	}
}