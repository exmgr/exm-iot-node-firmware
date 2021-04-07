/******************************************************************************
 * DataSToreReader template
 * Used to iterate through data stored in the buffer or flash memory by a
 * DataStore. The process is transparent, the class returns elements
 * from the buffer one by one and when the end is reached, it switches to the
 * flash memory until all data is iterated.
 ******************************************************************************/

#ifndef DATA_STORE_READER
#define DATA_STORE_READER

#include "data_store.h"

template <class TStruct>
class DataStoreReader
{
public:
    ~DataStoreReader();
	DataStoreReader(const DataStore<TStruct> *store);

    bool next_file();
    TStruct* next_entry();

    RetResult begin();
    void reset();

    bool entry_crc_valid();
    RetResult delete_file();

private:
	// Default constructor private
    DataStoreReader();

    RetResult reset_data_state();

    /** Data store to traverse */
    const DataStore<TStruct> *_store = NULL;

    /** Handle to store dir */
    File _dir;

    /** Current file (when iterating) */
    File _cur_file;

    /** Buffer to which entries are read and their data field  returned */
    typename DataStore<TStruct>::Entry _cur_entry = {0};

    /** Current state of file reader */
    uint8_t _state_files = STATE_PREPARE;

    /** Current state of data reader */
    uint8_t _state_data = STATE_PREPARE;

	/** Available reader states */
    enum STATE
    {
        STATE_PREPARE = 1,
        STATE_READING,
        STATE_READING_FINISHED
    };
};

#endif