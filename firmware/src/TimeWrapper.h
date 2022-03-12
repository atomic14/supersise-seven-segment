#pragma once

#include <stdint.h>

class TimeWrapper
{
private:
    uint8_t hours;
    uint8_t minutes;
    uint8_t month;
    uint8_t day;

public:
    TimeWrapper();
    bool update();
    uint8_t get_hours() { return hours; }
    uint8_t get_minutes() { return minutes; }
    uint8_t get_month() { return month; }
    uint8_t get_day() { return day; }
};
