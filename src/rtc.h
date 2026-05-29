#pragma once
#include <stdint.h>

struct time_t {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

void rtc_init();
time_t rtc_get_time();