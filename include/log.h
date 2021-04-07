#ifndef LOG_H
#define LOG_H

#include "app_config.h"
#include "struct.h"
#include "data_store.h"
#include "log_codes.h"

namespace Log
{
    /**
     * Event log entry
     * Code: Error code
     * Meta1: Metadata field 1
     * Meta2: Metadata field 2
     */
    struct Entry
    {
        unsigned long long timestamp;
        Code code;
        int meta1;
        int meta2;
    }__attribute__((packed));

    bool log(Log::Code code, uint32_t meta1 = 0, uint32_t meta2 = 0);
    
    RetResult commit();

    void print(const Log::Entry *entry);

    DataStore<Entry>* get_store();

    void set_enabled(bool enabled);
}

#endif