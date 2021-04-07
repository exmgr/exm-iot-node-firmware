#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <inttypes.h>
#include "struct.h"
#include "ipfs_client.h"

namespace Utils
{
    void get_mac(char *out, uint8_t size);
    void log_mac();

    // TODO: Obsolete
    // const DeviceDescriptor* get_device_descriptor();

    void print_separator(const __FlashStringHelper *name);

    void print_block(const __FlashStringHelper *title);

    void print_buff_hex(uint8_t *buff, int len, int break_pos = 8);

    void serial_style(SerialStyle style);

    uint32_t crc32(uint8_t *buff, uint32_t buff_size);

    RetResult ip5306_set_power_boost_state(bool enable);

    RetResult url_explode(char *in, int *port_out, char *host_out, int host_max_size, char *path_out, int path_max_size);

    RetResult tb_build_attributes_url_path(char *buff, int buff_size);

    RetResult tb_build_telemetry_url_path(char *buff, int buff_size);

    RetResult restart_device();

    void check_credentials();

    int read_adc_mv(uint8_t pin, int samples, int sampling_delay_ms);

    void print_flags();

    void print_reset_reason();

    template<typename T>
	int in_array(T val, const T arr[], int size);

    int boot_self_test();

    float deg_to_rad(float deg);
	float rad_to_deg(float rad);

    void build_ipfs_file_json(String hash, uint32_t timestamp, char *buff, int buff_size);
}

#endif