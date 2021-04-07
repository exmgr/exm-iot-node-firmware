#ifndef WATER_SENSOR_DATA_H
#define WATER_SENSOR_DATA_H

#include "app_config.h"
#include "struct.h"
#include "data_store.h"

namespace WaterSensorData
{
    /**
     * Water sensor data packet
     * All ranges/resolutions are for Aquatroll400,500,600
     * NOTE: MUST be aligned to 4 byte boundary to avoid padding. If not, CRC32 calculations
     * may fail
     */
    struct Entry
    {
        uint32_t timestamp;

        // Range: -5 - 50C 0.01C
        float temperature;

        // Range: 0-60mg/L / Res: 0.01mg/L
        float dissolved_oxygen;

        // Range: 0-100000uS/cm - Res: 0.1uS/cm
        float conductivity;

        // Range: 0-14 - Res 0.01
        float ph;

        // Range: -+1400mV - Res. 0.1
        float orp;

        // Units: psi - Res: 0.01
        float pressure;

        // Units: cm 
        float depth_cm;

        // Units: ft
        float depth_ft;

        // Total suspended solids
        float tss;

        // Range: 50 - 999cm
        float water_level;
    }__attribute__((packed));

    RetResult add(Entry *data);

    DataStore<Entry>* get_store();

    void print(const Entry *data);
}

#endif
