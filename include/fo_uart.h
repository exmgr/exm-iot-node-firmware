#ifndef FO_UART_H
#define FO_UART_H

#include "struct.h"
#include <inttypes.h>

namespace FoUart
{
    /**
	 * A single packet of data parsed from the UART response
	 */
    // struct ParsedPacket
	// {
	// 	float temp;   		// Temperature 10.5C = 0x1F9, -10.5C = 0x127, added 400offset (Range -40 to -60). 0x7ff invalid
	// 	uint8_t hum;   		// Range: 1-99, 0xff=invalid
	// 	float rain; 		// Rain counter

	// 	uint16_t wind_dir;	// 0 - 359, invalid = 0x1ff
	// 	float wind_speed; 	// 2 highest bits of wind speed
	// 	float wind_gust;  	// 0xff=invalid

	// 	uint32_t uv;    	// Range: 0 to 20000. 0xffff = invalid
	// 	uint32_t light; 	// Range: 0 - 300000
	// } __attribute__((packed));

    RetResult init();

    int calc_secs_to_next_packet();
    RetResult request_packet();
	RetResult handle_scheduled_event();
	FoDecodedPacket *get_last_packet();
	RetResult commit_buffer();
}

#endif