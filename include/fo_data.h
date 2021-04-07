#ifndef FO_DATA_H
#define FO_DATA_H

#include <inttypes.h>
#include "struct.h"
#include "data_store.h"

namespace FoData
{
    /**
	 * Data store entry
	 */
	struct StoreEntry
	{
		// Timestamp of first packet
		uint32_t timestamp;

		// Number of packets aggregated in this single entry
		uint16_t packets;

		// Number of wakeups to request/listen for data in this packet
		// Used for debugging, may be removed later
		uint32_t wakeups;

		// Temperature, Celsius
		// Range: -40C - 60C
		// 0x7FF invalid
		float temp;    

		// Relative Humidity, %
		// Range: 1% - 99%
		// 0xFF invalid
		uint8_t hum;   

		// Cumulative rain counter, mm
		float rain; 

		// Hourly rate (mm/h)
		float rain_hourly;

		// Wind direction, deg
		// Range: 0 - 359deg
		// 0x1FF invalid
		uint16_t wind_dir;

		// Wind speed, m/s
		// 0x1FF invalid
		float wind_speed;

		// Wind gust, m/s
		// 0xFF when invalid
		float wind_gust; 

		// UV - Manual says its uW/cm^2 but it seems to be a raw value that is 
		// is only used to find the UV index from a UV range table
		// Range: 0 - 20000
		// 0xFFFF invalid
		uint32_t uv;

		// UV Index - Derived from UV
		// Range: 0 - 15
		uint32_t uv_index;

		// Illuminance, Lux
		// Range: ?
		// 0xFFFFFF invalid
		uint32_t light;

		// Solar radiation - Derived from light, W/M^2
		// Range: ?
		uint32_t solar_radiation;
	}__attribute__((packed));

    RetResult init();
    RetResult add(StoreEntry *data);

    RetResult commit_buffer();
    DataStore<StoreEntry>* get_store();

	void inc_wakeup_count();
    void print(FoData::StoreEntry *packet);
};

#endif