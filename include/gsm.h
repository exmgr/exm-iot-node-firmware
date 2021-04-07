#ifndef _GSM_H
#define _GSM_H

#include "struct.h"
#include "app_config.h"
#include "const.h"
#include <TinyGsmClient.h>

namespace GSM
{
    void init();

    RetResult connect();
    RetResult connect_persist();

    RetResult enable_gprs(bool enable);

    RetResult update_ntp_time();
    RetResult get_time(tm *out);
    RetResult parse_time(const char *timestring, tm *out, bool gsm_time);

    RetResult on();
    RetResult off();

    RetResult get_battery_info(uint16_t *voltage, uint16_t *pct);

    TinyGsm* get_modem();

    int get_rssi();
    bool is_sim_card_present();
    bool is_on(uint32_t timeout = GSM_TEST_AT_TIMEOUT);
    bool is_gprs_connected();
    RetResult print_system_info();
    RetResult factory_reset();
}

#endif