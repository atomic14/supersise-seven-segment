#include "BoostSupply.h"
#include <driver/ledc.h>
#include <Arduino.h>

static const int MAX_DUTY = ((1000 * 90 * 1024) / 100);
static const int MIN_DUTY = ((1000 * 5 * 1024) / 100);
static const int MID_DUTY = ((1000 * 50 * 1024) / 100);

BoostSupply::BoostSupply(gpio_num_t pwm_pin, adc1_channel_t feedback_pin)
{
    m_feedback_pin = feedback_pin;
    // configure the ADC
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(feedback_pin, ADC_ATTEN_DB_0);

    // get the characteristics of ADC1
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
        ADC_UNIT_1,
        ADC_ATTEN_DB_11,
        ADC_WIDTH_BIT_12,
        1100,
        &adc_chars);
    Serial.printf("ADC calibration type for unit %d: %d\n", ADC_UNIT_1, val_type);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
    {
        Serial.println("Characterized using Two Point Value");
    }
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
    {
        Serial.println("Characterized using eFuse Vref");
    }
    else
    {
        Serial.println("Characterized using Default Vref");
    }
    // setup the PWM output
    ledc_timer_config_t ledc_timer = {};
    ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_timer.duty_resolution = LEDC_TIMER_10_BIT;
    ledc_timer.timer_num = LEDC_TIMER_0;
    ledc_timer.freq_hz = 40000;

    ledc_timer_config(&ledc_timer);

    // configure the duty cycle and gpio number
    ledc_channel_config_t ledc_channel = {0};
    ledc_channel.channel = LEDC_CHANNEL_0;
    ledc_channel.duty = MIN_DUTY / 1000;
    ledc_channel.gpio_num = pwm_pin;
    ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
    ledc_channel.timer_sel = LEDC_TIMER_0;

    ledc_channel_config(&ledc_channel);

    duty = MIN_DUTY;
}

void BoostSupply::service()
{
    // calculate the median value of the feedback pin over a number of samples
    samples[sample_index] = adc1_get_raw(m_feedback_pin);
    sample_index++;
    if (sample_index >= NUMBER_OF_SAMPLES)
    {
        sample_index = 0;
    }
    uint32_t values[NUMBER_OF_SAMPLES];
    for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
    {
        values[i] = samples[i];
    }
    std::sort(values, values + NUMBER_OF_SAMPLES);
    uint32_t value_sense = (values[NUMBER_OF_SAMPLES / 2] + values[NUMBER_OF_SAMPLES / 2 - 1]) / 2;
    feedback_voltage = esp_adc_cal_raw_to_voltage(value_sense, &adc_chars);

    int voltage_diff = 550 - feedback_voltage;
    duty = duty + 5 * voltage_diff;

    duty = std::max(std::min(duty, MAX_DUTY), MIN_DUTY);

    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty / 1000);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}