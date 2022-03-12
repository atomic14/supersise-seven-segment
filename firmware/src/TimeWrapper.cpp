#include <time.h>
#include <Arduino.h>
#include "TimeWrapper.h"

const char *NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET = 0;
const int DAYLIGHT_SAVING_OFFSET = 3600;

TimeWrapper::TimeWrapper()
{
    configTime(GMT_OFFSET, DAYLIGHT_SAVING_OFFSET, NTP_SERVER);
}

bool TimeWrapper::update()
{
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
        this->hours = timeinfo.tm_hour;
        this->minutes = timeinfo.tm_min;
        this->month = timeinfo.tm_mon + 1;
        this->day = timeinfo.tm_mday;
        return true;
    }
    return false;
}
