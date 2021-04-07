#include "data_store.h"
#include "water_sensor_data.h"
#include "soil_moisture_data.h"
#include "lightning_data.h"
#include "atmos41_data.h"
#include "fo_data.h"
#include "sdi12_log.h"
#include "log.h"
#include "utils.h"
#include "flash.h"
#include "common.h"

/******************************************************************************
 * DataStore
 * Represents a store of data structures in SPIFFS.
 * Data can be add()ed which is then stored in a buffer until the buffer is full
 * or commit() is called, in which cases it is appended to a file in SPIFFS.
 * A file has a max size. When max size is reached, a new file is created and 
 * subsequent structures are written there.
 * Data in a DataStore can be traversed with a DataStoreReader class.
 ******************************************************************************/

/******************************************************************************
 * Constructor
 * @param dir Dir in SPIFFS Where data will be stored
 * @param elements_per_file Max entries to store in a file before creating a new one
 ******************************************************************************/
template <class TStruct>
DataStore<TStruct>::DataStore(const char *dir_path, int max_entries_per_file)
{
	_dir_path = dir_path;
	_max_entries_per_file = max_entries_per_file;
}

/******************************************************************************
 * Add data structure to buffer. If buffer is full, data is automatically commited
 * to make space in buffer.
 ******************************************************************************/
template <class TStruct>
RetResult DataStore<TStruct>::add(TStruct *data)
{
    // Buffer full?
    if (_buffer_element_count >= DATA_STORE_BUFFER_ELEMENTS)
    {
        // Commit to flash
        commit();

        debug_println(F("Data store full, commiting and erasing."));
    }

	// Prepare metadata of new entry (crc32)
	Entry new_entry = {0};
	new_entry.crc32 = Utils::crc32((uint8_t*)data, sizeof(TStruct));

	// Copy struct to body
	memcpy(&new_entry.data, data, sizeof(TStruct));

	// Copy new entry to buffer
    memcpy((void *)&_buffer[_buffer_element_count], &new_entry, sizeof(new_entry));

    _buffer_element_count++;	

    return RET_OK;
}

/******************************************************************************
 * Save all data to flash and erase buffer
 * Data is appended to a file until max file size is reached. In that case a new
 * file is created and writing continues to that file. File names are 
 * 
 ******************************************************************************/
template <class TStruct>
RetResult DataStore<TStruct>::commit()
{
	if (Flash::mount() != RET_OK)
		return RET_ERROR;

	// If current data file not set yet, get one
	if(strlen(_current_data_file_path) < 1)
	{
		if(update_current_data_file_path() != RET_OK)
		{
			debug_println(F("Could not get data file to write to."));
			return RET_ERROR;
		}
	}
	
	// debug_print(F("File: "));
	// debug_println(_current_data_file_path);

	File f;

	// Counter of entries left to write to flash
	int entries_left = get_buffer_element_count();

	// Until all entries have been written (starting from end of buffer)
	while(entries_left)
	{
		// Try to open current data file.
		f = SPIFFS.open(_current_data_file_path, "a");

		if(!f)
		{
			debug_println(F("Could not open data file for append."));
			return RET_ERROR;
		}

		// Entries to write is how many space we have left in this file / size of an entry
		int entries_for_current_file = (_max_entries_per_file * sizeof(Entry) - f.size()) / sizeof(Entry);

		if(entries_for_current_file > 0)
		{
			if(entries_left < entries_for_current_file)
				entries_for_current_file = entries_left;

			// If writing fails this will be false
			bool write_success = true;

			// Write entries one by one from end of buffer. If correct number of bytes is written,
			// last buffer element is removed. In the case of full disk, data corruption is minimized
			for(int i = 0; i < entries_for_current_file; i++)
			{
				// Get buffer entry to write
				const Entry *buff_entry = NULL;
				buff_entry = get_buffer_element(entries_left - i - 1);

				// Reading out of bounds check (redundant)
				if(buff_entry == NULL)
				{
					debug_print(F("Element index doesn't exist in buffer: "));
					debug_println(entries_left - i - 1, DEC);

					write_success = false;
					break;
				}

				// Write a single netry
				int written_bytes = f.write((uint8_t*)buff_entry, sizeof(Entry));
				if(written_bytes != sizeof(Entry))
				{
					debug_println(F("Could not write entry."));
					debug_print(F("Entry size: "));
					debug_println(sizeof(Entry), DEC);
					debug_print(F("Written: "));
					debug_println(written_bytes, DEC);

					write_success = false;
					break;
				}

				// Remove last element from buffer
				_buffer_element_count--;

				// TODO: Check for entries left out of loop by subtracting every time the expected to be written number of entries
				entries_left--;
			}

			// Writing failed, abort
			if(!write_success)
			{
				debug_print(F("Writing failed, aborting. Entries left in buffer: "));
				debug_println(entries_left);
				f.close();
				return RET_ERROR;
			}
		}

		// If no more entries fit into this file or end reached and we still have entries to write,
		// it is time to get a new data file
		if(entries_for_current_file < 1 || entries_left > 0)
		{
			// File has reached max size, get new file
			if(update_current_data_file_path() != RET_OK)
			{
				debug_printf("Could not get data file to write to.");
				
				f.close(); 
				return RET_ERROR;
			}
		}
	}

	return RET_OK;
}

/******************************************************************************
 * Clear buffer data
 ******************************************************************************/
template <class TStruct>
RetResult DataStore<TStruct>::clear_buffer()
{
	_buffer_element_count = 0;

	return RET_OK;
}

/******************************************************************************
 * Clear all saved data from flash storage
 ******************************************************************************/
template <class TStruct>
RetResult DataStore<TStruct>::clear_all()
{
	// Clear buffer
	clear_buffer();

	// Clear flash
	// Rmdir doesnt't work since SPIFFS is flat and dirs are only somewhat
	// emulated, so delete one by one
	File dir = SPIFFS.open(get_dir_path());
	File file;

	while(file = dir.openNextFile())
	{
		SPIFFS.remove(file.name());
	}

	return RET_OK;
}

/******************************************************************************
 * Get number of items in buffer
 ******************************************************************************/
template <class TStruct>
unsigned int DataStore<TStruct>::get_buffer_element_count() const
{
	return _buffer_element_count;
}

/******************************************************************************
 * Get pointer to buffer
 * Used by reader.
 ******************************************************************************/
template <class TStruct>
const typename DataStore<TStruct>::Entry* DataStore<TStruct>::get_buffer_element(unsigned int index) const
{
	if(_buffer_element_count == 0 || index > _buffer_element_count - 1)
		return NULL;

	return &(_buffer[index]);
}

/******************************************************************************
 * Get store dir path
 ******************************************************************************/
template <class TStruct>
const char* DataStore<TStruct>::get_dir_path() const
{
	return _dir_path;
}

/******************************************************************************
 * Update path of file that next write will be done to. First try to find a 
 * file that has still space left (didn't reach max element per file limit)
 * If failed, create a new file.
 ******************************************************************************/
template <class TStruct>
RetResult DataStore<TStruct>::update_current_data_file_path()
{
	File dir = SPIFFS.open(_dir_path);
	if(!dir)
	{
		debug_print(F("Could not open store dir: "));
		debug_println(_dir_path);
		return RET_ERROR;
	}

	// Find smallest file
	int smallest_size = -1;
	char smallest_file_path[FILE_PATH_BUFFER_SIZE] = {0};
	File cur_file;

	while(cur_file = dir.openNextFile())
	{
		if(cur_file.size() < smallest_size || smallest_size < 0)
		{
			smallest_size = cur_file.size();
			strncpy(smallest_file_path, cur_file.name(), sizeof(smallest_file_path));
		}
	}
	cur_file.close();
	dir.close();

	// debug_print(F("Smallest size: ")); // del
	// debug_println(smallest_size, DEC);

	// Smallest file found, check if there is space in it for at least one entry
	// else create a new file
	if(smallest_size >= 0 && smallest_size + sizeof(Entry) <= _max_entries_per_file * sizeof(Entry))
	{
		// debug_print(F("Use existing: "));
		// debug_println(smallest_file_path);

		// Update current file path
		strncpy(_current_data_file_path, smallest_file_path, sizeof(_current_data_file_path));
		return RET_OK;
	}
	else
	{
		// debug_println(F("No file found or all files at max limit, creating new."));

		// Smallest file not found (no files exist) or all files full, decide a new filename
		// and create
		// Filename is current epoch time. In the unlikely event that the filename is taken
		// (eg. problems with RTC) append a number and see if it is taken, until an unused
		// name is found. Do this a limited number of times before failing else this could
		// lead into endless loops under certain circumstances.
		char new_file_path[FILE_PATH_BUFFER_SIZE] = {0};
		int tries = 100;
		bool success = false;

		do
		{
			snprintf(new_file_path, sizeof(new_file_path), "%s/%d_%d", _dir_path, (int)time(NULL), FILENAME_POSTFIX_MAX - tries);
			
			if (!SPIFFS.exists(new_file_path))
			{
				success = true;
				break;
			}

		} while (--tries);

		if (!success) 
		{
			debug_println(F("Could not decide new file name."));
			return RET_ERROR;
		}

		// Create file
		debug_print(F("Creating new file: "));
		debug_println(new_file_path);

		File f = SPIFFS.open(new_file_path, FILE_WRITE);
		if(!f)
		{
			debug_print(F("Could not create new data file: "));
			debug_println(new_file_path);
			return RET_ERROR;
		}

		f.close();

		// Update current file path
		strncpy(_current_data_file_path, new_file_path, sizeof(_current_data_file_path));

		return RET_OK;
	}
}

// Forward declarations
template class DataStore<WaterSensorData::Entry>;
template class DataStore<Atmos41Data::Entry>;
template class DataStore<SoilMoistureData::Entry>;
template class DataStore<Log::Entry>;
template class DataStore<SDI12Log::Entry>;
template class DataStore<FoData::StoreEntry>;
template class DataStore<LightningData::Entry>;