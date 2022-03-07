#pragma once
#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define NUMBER_OF_SAMPLES 16

class BoostSupply
{
private:
    esp_adc_cal_characteristics_t adc_chars;
    adc1_channel_t m_feedback_pin;
    int sample_index = 0;
    int samples[NUMBER_OF_SAMPLES] = {};
    uint32_t feedback_voltage = 0;
    int duty = 0;

public:
    BoostSupply(gpio_num_t pwm_pin, adc1_channel_t feedback_pin);
    void service();
    int get_duty() { return duty; }
    uint32_t get_feedback_voltage() { return feedback_voltage; }
};