#pragma once

#include <Arduino.h>

class StopWatch
{
private:
    int64_t elapsed_time = 0;
    int64_t start_time = 0;
    bool is_running = false;

public:
    void start()
    {
        if (is_running)
        {
            return;
        }
        start_time = esp_timer_get_time();
        is_running = true;
    }
    void stop()
    {
        if (!is_running)
        {
            return;
        }
        elapsed_time += esp_timer_get_time() - start_time;
        is_running = false;
    }
    void reset()
    {
        elapsed_time = 0;
        is_running = false;
    }
    int64_t get_elapsed_time()
    {
        if (is_running)
        {
            return elapsed_time + (esp_timer_get_time() - start_time);
        }
        return elapsed_time;
    }
};