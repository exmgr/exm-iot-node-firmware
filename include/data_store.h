#ifndef DATA_STORE_H
#define DATA_STORE_H

#include <inttypes.h>
#include "SPIFFS.h"
#include "app_config.h"
#include "struct.h"
#include "const.h"

template <typename TStruct>
class DataStore
{
public:
    //
    // Structs
    //

    /** A single entry in the data store */
    struct Entry
    {
        /** CRC32 of entry content */
        uint32_t crc32;

        /** The data struct itself */
        TStruct data;
    }__attribute__((packed));

    DataStore(const char *dir_path, int max_entries_per_file);

    RetResult add(TStruct *data);

    RetResult commit();
    
    RetResult clear_buffer();

    RetResult clear_all();

    unsigned int get_buffer_element_count() const;

    const Entry* get_buffer_element(unsigned int index) const;

	const char* get_dir_path() const;
protected:
	// Default constructor private
	DataStore();

private:

    //
    // Methods
    //

    RetResult update_current_data_file_path();
    //
    // Vars
    //

    /** Holds path of file where last data was written, to avoid finding it every time */
    char _current_data_file_path[FILE_PATH_BUFFER_SIZE] = {0};

    /** Data buffer. Data is stored temporarily here until buffer is full or when
     * commit() is called in which case it is saved into flash memory and emptied */
    Entry _buffer[DATA_STORE_BUFFER_ELEMENTS];

    /** Count of elements in buffer */
    uint32_t _buffer_element_count = 0;

	/** Dir in SPIFFS where data will be stored */
	const char* _dir_path = NULL;

    /** When writing data to flash, break it into x elements per file.
  	 *	A file is removed only when all of its data is marked as deleted. */
    int _max_entries_per_file = 0;
};

#endif