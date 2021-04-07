#ifndef FO_BUFFER_H
#define FO_BUFFER_H

#include <inttypes.h>
#include "app_config.h"
#include "fo_buffer.h"

/******************************************************************************
* FO Decoded Packet buffer
* Stores decoded packets which can then be aggregated and commited into a 
* single FoData store entry
******************************************************************************/
class FoBuffer
{
public:
    RetResult commit_buffer();
    RetResult add_packet(FoDecodedPacket *packet);
    void clear();

    static void print_packet(FoDecodedPacket *packet);
private:
    /** Buffer */
    FoDecodedPacket _buffer[FO_BUFFER_SIZE];

    /** Count of packets in buffer */
    int _packet_count = 0;

    /** Timestamp of first added packet */
    uint32_t _first_packet_tstamp = 0;

    /** Timestamp of last added packet */
    uint32_t _last_packet_tstamp = 0;

    /** Rain count from previously commited packet. Used to calculate hr rate */
    float _prev_rain = -1;
};

#endif