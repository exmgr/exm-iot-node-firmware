#ifndef ATMOS41_DATA_H
#define ATMOS41_DATA_H

#include "app_config.h"
#include "struct.h"
#include "data_store.h"

namespace Atmos41Data
{
    /**
     * Weather station data packet
     * All ranges/resolutions are for ATMOS41
     * NOTE: MUST be aligned to 4 byte boundary to avoid padding. If not, CRC32 calculations
     * may fail
     */
    struct Entry
    {
        uint32_t timestamp;

        // Solar radiation (W/m^2)
        // Range: 0 - 1750 
        // Resolution: 1
        int16_t solar;

        // Rainfall since last measurement (mm)
        // Range: 0 - 400
        // Resolution: 0.017
        float precipitation;

		// Number of strikes since last measurement
        // Range: 0 - 65535
        // Resolution: 1
		int16_t strikes;

		// Combined wind speed magnitude (N and E) (m/S)
        // Range: 0 - 30
        // Resolution: 0.01 m/S
		float wind_speed;

		// Wind dir from north reference (deg)
        // Range: 0 - 359
        // Resolution: 1
		int16_t wind_dir;

		// Max wind speed since last measurement (m/S)
        // Range: 0 - 30
        // Resolution: 0.01m/s
		float wind_gust_speed;
    
		// Air temperature (C)
        // Range: -50 - 60
        // Resolution: 0.1
		float air_temp;

		// Vapor pressure (kPa)
        // Range: 0 - 47
        // Resolution: 0.01
		float vapor_pressure;

		// Atmospheric pressure (kPa)
        // Note: Range/res not mentioned in integratino manual, assuming it is same
        // as vapor pressure
		float atm_pressure;

        // Relative humidity (calculated from vapor pressure and air temp) (%)
        // Range: 0 - 100
        // Resolution: 0.1
        float rel_humidity;

        // Dew point (calculated from other params)
        //
        //
        float dew_point;

    }__attribute__((packed));

    RetResult add(Entry *data);

    DataStore<Entry>* get_store();

    void print(const Entry *data);
}

#endif