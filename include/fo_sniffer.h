#ifndef FO_SNIFFER_H
#define FO_SNIFFER_H

#include <inttypes.h>
#include "LoRaLib.h"
#include "app_config.h"
#include "utils.h"
#include "data_store.h"

namespace FoSniffer
{
	RetResult init();	

	RetResult wait_for_packet(uint32_t timeout_ms, bool ignore_address = false);
	RetResult sleep_to_packet(uint32_t max_sleep_ms);
	
	uint8_t scan_fo_id(bool update_config = false);

	int calc_secs_to_next_sniff();
	RetResult handle_sniff_event();
	RetResult commit_buffer();
	void print_packet(FoDecodedPacket *packet);
	FoDecodedPacket* get_last_packet();
}

#endif