#include "fo_buffer.h"
#include "fo_sniffer.h"
#include "rtc.h"
#include "fo_data.h"

/******************************************************************************
* Add packet to buffer
******************************************************************************/
RetResult FoBuffer::add_packet(FoDecodedPacket *packet)
{
    // Buffer full, ignore
    if(_packet_count >= FO_BUFFER_SIZE)
        return RET_ERROR;

    // If first packet, store current time. Aggregate packet timestamp is tstamp
    // of the first packet
    if(_packet_count == 0)
        _first_packet_tstamp = RTC::get_timestamp();
    _last_packet_tstamp = RTC::get_timestamp();

    memcpy(&_buffer[_packet_count], packet, sizeof(FoDecodedPacket));

    _packet_count++;

    // Commit automatically if X seconds passed from last packet
    int sec_since_last_commit = RTC::get_timestamp() - _first_packet_tstamp;
    if(sec_since_last_commit >= FO_AGGREGATE_INTERVAL_SEC)
    {
        debug_print(sec_since_last_commit, DEC);
        debug_println(" seconds passed since last commit, commiting FoSniffer buffer.");
        commit_buffer();
    }
}

/******************************************************************************
* Clear buffer
******************************************************************************/
void FoBuffer::clear()
{
    _first_packet_tstamp = 0;
    _packet_count = 0;
}

/******************************************************************************
* Save buffer to the file system
* After aggregate calcs whole buffer results in a single entry in the filesystem
******************************************************************************/
RetResult FoBuffer::commit_buffer()
{
    // Nothing to commit
    if(_packet_count < 1)
        return RET_OK;

    //
    // Aggregate data
    //

    // Totals of params to calc averages
    float hum_total = 0, temp_total = 0;
    float wind_speed_total = 0, wind_gust_total = 0;
    float solar_radiation_total = 0;
    uint32_t uv_total = 0, uv_index_total = 0, light_total = 0;

    // Total of sin/cos of wind dir, used to calculate mean angle
    float wind_dir_sin_total = 0, wind_dir_cos_total = 0;

    for(int i = 0; i < _packet_count; i++)
    {
        FoDecodedPacket *cur_packet = &_buffer[i];

        temp_total += cur_packet->temp;
        hum_total += cur_packet->hum;
        wind_speed_total += cur_packet->wind_speed;
        wind_gust_total += cur_packet->wind_gust;
        uv_total += cur_packet->uv;
        uv_index_total += cur_packet->uv_index;
        light_total += cur_packet->light;
        solar_radiation_total += cur_packet->solar_radiation;

        wind_dir_sin_total += sin(Utils::deg_to_rad(cur_packet->wind_dir));
        wind_dir_cos_total += cos(Utils::deg_to_rad(cur_packet->wind_dir));
    }

    // Calc wind dir avg
    float wind_dir_avg = atan2(wind_dir_sin_total / _packet_count, wind_dir_cos_total / _packet_count);

    wind_dir_avg = Utils::rad_to_deg(wind_dir_avg);
    if(wind_dir_avg < 0)
        wind_dir_avg += 360;

    //
    // Calc hourly rate from previous commit
    //

    //
    // Build data store entry
    //
    FoData::StoreEntry entry;
    
    entry.timestamp = _first_packet_tstamp;
    entry.packets = _packet_count;

    entry.temp = (float)temp_total / _packet_count;
    entry.hum = (float)hum_total / _packet_count;
    entry.wind_dir = (uint16_t)wind_dir_avg;
    entry.wind_speed = (float)wind_speed_total / _packet_count;
    entry.wind_gust = (float)wind_gust_total / _packet_count;
    entry.uv = uv_total / _packet_count;
    entry.uv_index = uv_index_total / _packet_count;
    entry.light = light_total / _packet_count;
    entry.solar_radiation = solar_radiation_total / _packet_count;
    entry.rain = _buffer[_packet_count - 1].rain;

    // Calc hourly rate from previous commit
    if(_last_packet_tstamp > _first_packet_tstamp)
    {
        uint32_t time_diff_sec = _last_packet_tstamp - _first_packet_tstamp;
        float rain_diff = entry.rain - _buffer[0].rain;
        float rate_hr = (60 * 60 / time_diff_sec) * rain_diff;
        rate_hr = (int)(rate_hr * 100 + 0.5) / 100.0;

        entry.rain_hourly = rate_hr;
    }

    // Update previous rain count
    _prev_rain = entry.rain;

    debug_println_i(F("Commiting FO Buffer,"));
    debug_print(F("Total time (sec): "));
    debug_println(_last_packet_tstamp - _first_packet_tstamp, DEC);
    debug_print(F("Rain rate (hr): "));
    debug_println(entry.rain_hourly, DEC);

    FoData::add(&entry);

    clear();
    return RET_OK;
}

/******************************************************************************
* Print a single packet
******************************************************************************/
void FoBuffer::print_packet(FoDecodedPacket *packet)
{
    debug_printf("\n\n########################## DECODED PACKET ###########################\n");

    FoDecodedPacket decoded = {0};

    debug_printf("Temperature: %2.1fC\n", packet->temp);
    debug_printf("Humidity: %d\n%", packet->hum);
    debug_printf("Rain: %4.2fmm\n", packet->rain);

    debug_printf("Wind speed: %2.2f\n", packet->wind_speed);
    debug_printf("Wind Dir: %d\n", packet->wind_dir);
    debug_printf("Wind Gust: %2.2f\n", packet->wind_gust);

    debug_printf("UV: %d\n", packet->uv);
    debug_printf("UV Index: %d\n", packet->uv_index);
    debug_printf("Light: %d\n", packet->light);
    debug_printf("Solar Radiation: %d\n", packet->solar_radiation);


    debug_printf("CRC: %02x\n", packet->crc);
    debug_printf("CRC: %02x\n", packet->checksum);

    debug_printf("\n######################################################################\n");
}