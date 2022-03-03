#pragma once
#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define NUMBER_OF_SAMPLES 8

class BoostSupply
{
private:
    esp_adc_cal_characteristics_t adc_chars;
    adc1_channel_t m_feedback_pin;
    int sample_index = 0;
    int samples[NUMBER_OF_SAMPLES] = {};

public:
    BoostSupply(gpio_num_t pwm_pin, adc1_channel_t feedback_pin);
    void service();
};