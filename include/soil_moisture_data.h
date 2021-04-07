#ifndef SOIL_MOISTURE_DATA_H
#define SOIL_MOISTURE_DATA_H

#include "app_config.h"
#include "struct.h"
#include "data_store.h"

namespace SoilMoistureData
{
    /**
     * Data packet
     */
    struct Entry
    {
        uint32_t timestamp;

        // Volumetric Water Content
        // Res: 0.001m^3/m^3
        float vwc;

        // Range: -40 - +60C / Res: 0.1C
        float temperature;

        // Range: 0 - 20000uS/cm / Res: 1uS/cm
        float conductivity;
    }__attribute__((packed));

    RetResult add(Entry *data);
    DataStore<Entry>* get_store();

    void print(const Entry *data);
}

#endif