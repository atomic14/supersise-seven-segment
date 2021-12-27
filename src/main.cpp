#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <driver/ledc.h>
#include <FreeRTOS.h>
#include <freertos/task.h>
#include "driver/timer.h"

// GPIO_33
#define SENSE_CHANNEL ADC1_CHANNEL_5

#define MAX_DUTY ((97 * 8192 / 100) * 256)
#define MIN_DUTY ((5 * 8192 / 100) * 256)

// samples to average over
#define NUMBER_OF_SAMPLES 8

static esp_adc_cal_characteristics_t adc1_chars;
static int duty = 0;
uint sense_v = 0;
int integral = 0;
int count = 0;

void power_task()
{
  count++;
  uint32_t value_sense = 0;
  for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
  {
    value_sense += adc1_get_raw(SENSE_CHANNEL);
  }
  value_sense /= NUMBER_OF_SAMPLES;
  sense_v = esp_adc_cal_raw_to_voltage(value_sense, &adc1_chars);

  int voltage_diff = 1500 - sense_v;
  integral += voltage_diff;

  duty = 1200 * voltage_diff + 100 * integral;

  duty = std::max(std::min(duty, MAX_DUTY), MIN_DUTY);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty >> 11);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
}

void get_adc_calibration(adc_unit_t adc_unit, esp_adc_cal_characteristics_t *adc_chars)
{
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      adc_unit,
      ADC_ATTEN_DB_11,
      ADC_WIDTH_BIT_12,
      1100,
      adc_chars);
  Serial.printf("ADC calibration type for unit %d: %d\n", adc_unit, val_type);
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
}

hw_timer_t *timer = NULL;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(SENSE_CHANNEL, ADC_ATTEN_DB_11);

  // get the characteristics of ADC1
  get_adc_calibration(ADC_UNIT_1, &adc1_chars);

  // setup the timer
  ledc_timer_config_t ledc_timer = {};

  ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_timer.duty_resolution = LEDC_TIMER_10_BIT;
  ledc_timer.timer_num = LEDC_TIMER_0;
  ledc_timer.freq_hz = 20000;

  ledc_timer_config(&ledc_timer);

  // configure the duty cycle and gpio number
  ledc_channel_config_t ledc_channel = {0};
  ledc_channel.channel = LEDC_CHANNEL_0;
  ledc_channel.duty = (MIN_DUTY * 1024) / 100;
  ledc_channel.gpio_num = GPIO_NUM_21;
  ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_channel.timer_sel = LEDC_TIMER_0;

  ledc_channel_config(&ledc_channel);

  // xTaskCreatePinnedToCore(power_task, "power_task", 2048, NULL, 5, NULL, 1);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &power_task, true);
  timerAlarmWrite(timer, 500, true);
  timerAlarmEnable(timer);

  // Configure the alarm value and the interrupt on alarm.
  // timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 0x00000001ULL);
  // timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  // timer_isr_register(TIMER_GROUP_0, TIMER_0, power_task,
  //                    NULL, ESP_INTR_FLAG_IRAM, NULL);

  // timer_start(TIMER_GROUP_0, TIMER_0);
}

void loop()
{
  Serial.printf("%d,%f,%f,%d\n", integral, ((100 * duty) >> 11) / 1024.0f, sense_v / 1000.0f, count);
  count = 0;
  delay(1);
}