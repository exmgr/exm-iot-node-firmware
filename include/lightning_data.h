#ifndef LIGHTNING_DATA_H
#define LIGHTNING_DATA_H

#include "app_config.h"
#include "struct.h"
#include "data_store.h"

namespace LightningData
{
    /**
     * Water sensor data packet
     * All ranges/resolutions are for Aquatroll400
     * NOTE: MUST be aligned to 4 byte boundary to avoid padding. If not, CRC32 calculations
     * may fail
     */
    struct Entry
    {
        uint32_t timestamp;

        uint16_t distance;
        uint32_t energy;
    } __attribute__((packed));

    RetResult add(Entry *data);
    DataStore<Entry>* get_store();

    void print(const Entry *data);
} // namespace LightningData

#endif