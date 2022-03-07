#pragma once

#include <Arduino.h>

class StopWatch
{
private:
    uint64_t elapsed_time = 0;
    uint64_t start_time = 0;
    bool is_running = false;

public:
    void start()
    {
        if (is_running)
        {
            return;
        }
        start_time = millis();
    }
    void stop()
    {
        if (!is_running)
        {
            return;
        }
        elapsed_time += millis() - start_time;
        is_running = false;
    }
    void reset()
    {
        elapsed_time = 0;
        is_running = false;
    }
    uint64_t get_elapsed_time()
    {
        if (is_running)
        {
            return elapsed_time + (millis() - start_time);
        }
        return elapsed_time;
    }
};