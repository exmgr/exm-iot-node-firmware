#include "data_store_reader.h"
#include "water_sensor_data.h"
#include "soil_moisture_data.h"
#include "fo_data.h"
#include "sdi12_log.h"
#include "atmos41_data.h"
#include "log.h"
#include "utils.h"
#include "water_sensor_data.h"
#include "flash.h"
#include "common.h"
#include "lightning_data.h"

/******************************************************************************
* Constructor
* @param store Store object to read from
******************************************************************************/
template <class TStruct>
DataStoreReader<TStruct>::DataStoreReader(const DataStore<TStruct> *store)
{
	_store = store;
}

/******************************************************************************
* Default constructor (private)
******************************************************************************/
template <class TStruct>
DataStoreReader<TStruct>::DataStoreReader()
{}

/******************************************************************************
* Destructor
* Close file and cleanup
******************************************************************************/
template <class TStruct>
DataStoreReader<TStruct>::~DataStoreReader()
{
	_dir.close();
	_cur_file.close();
}

/******************************************************************************
* Create handles. Must be called before calling anything else.
******************************************************************************/
template <class TStruct>
RetResult DataStoreReader<TStruct>::begin()
{
	// todo: what happens if begin is not called? next should return false
	if(Flash::mount() != RET_OK)
	{
		debug_println(F("Could not start SPIFFS."));
		return RET_ERROR;
	}

	reset();

	return RET_OK;
}

/******************************************************************************
* Get next file in store
* @return True while there are still files in store
******************************************************************************/
template <class TStruct>
bool DataStoreReader<TStruct>::next_file()
{
	bool success = false;

	//
	// Open first file and switch to reading
	//
	if(_state_files == STATE_PREPARE)
	{
		_dir = SPIFFS.open(_store->get_dir_path());

		// Can't open dir means there are no files (dirs in SPIFFS are virtual)
		if(!_dir)
		{
			_state_files = STATE_READING_FINISHED;
		}
		else
		{
			_state_files = STATE_READING;
		}
	}

	//
	// Get next file
	//
	if(_state_files == STATE_READING)
	{
		_cur_file = _dir.openNextFile();

		// No more files, finish
		if(!_cur_file)
		{
			debug_println(F("No more files to open. Finish."));
			_state_files = STATE_READING_FINISHED;
		}
		else
		{
			success = true;

			// New file to read, let entry reader know
			reset_data_state();
		}
	}

	//
	// Done
	//
	if(_state_files == STATE_READING_FINISHED)
	{
		success = false;
	}

	return success;
}

/******************************************************************************
* Get next item in current file
* @return True while there are still entries in current file
******************************************************************************/
template <class TStruct>
TStruct* DataStoreReader<TStruct>::next_entry()
{
	bool success = false;


	if(_state_data == STATE_PREPARE)
	{
		// If reading of files in progress, only then proceed to data reading
		if(_state_files == STATE_READING)
			_state_data = STATE_READING;
		else
			success = false;
	}

	if(_state_data == STATE_READING)
	{
		int bytes_read = _cur_file.readBytes((char*)&_cur_entry, sizeof(_cur_entry));

		// No more data, reading of data finished
		if(bytes_read != sizeof(_cur_entry))
		{
			_state_data = STATE_READING_FINISHED;
		}
		else
		{
			// Successfully read
			success = true;
		}
	}

	if(_state_data == STATE_READING_FINISHED)
	{
		success = false;
	}

	if(!success)
		return NULL;
	else
	{
		return &_cur_entry.data;
	}
}

/******************************************************************************
 * Check if current entry's CRC is valid
 ******************************************************************************/
template <class TStruct>
bool DataStoreReader<TStruct>::entry_crc_valid()
{
	if(_state_data != STATE_READING)
		return false;

	return Utils::crc32( (uint8_t*)&_cur_entry.data, sizeof(_cur_entry.data) ) == _cur_entry.crc32;
}

/******************************************************************************
 * Delete current file
 ******************************************************************************/
template <class TStruct>
RetResult DataStoreReader<TStruct>::delete_file()
{
	if(!_cur_file)
		return RET_ERROR;

	// Keep name before closing file so we can delete it
	char path[FILE_PATH_BUFFER_SIZE] = {0};
	strncpy(path, _cur_file.name(), FILE_PATH_BUFFER_SIZE);

	_cur_file.close();

	if(SPIFFS.remove(path))
	{
		reset_data_state();

		return RET_OK;
	}
	else
	{
		return RET_ERROR;
	}
}

/******************************************************************************
 * Reset reader to enable re-iteration
 ******************************************************************************/
template <class TStruct>
void DataStoreReader<TStruct>::reset()
{
	// Close open files if any
	_cur_file.close();

	// Close dir handle
	_dir.close();
}

/******************************************************************************
 * Reset state of data iterator back to default state
 * Must be done every time a new file is loaded
 ******************************************************************************/
template <class TStruct>
RetResult DataStoreReader<TStruct>::reset_data_state()
{
	_state_data = STATE_PREPARE;

	return RET_OK;
}
		

// Define uses
template class DataStoreReader<WaterSensorData::Entry>;
template class DataStoreReader<Atmos41Data::Entry>;
template class DataStoreReader<SoilMoistureData::Entry>;
template class DataStoreReader<Log::Entry>;
template class DataStoreReader<FoData::StoreEntry>;
template class DataStoreReader<LightningData::Entry>;
template class DataStoreReader<SDI12Log::Entry>;