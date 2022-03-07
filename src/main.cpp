#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <driver/ledc.h>
#include <FreeRTOS.h>
#include <freertos/task.h>
#include "driver/timer.h"

// GPIO_33
#define SENSE_CHANNEL ADC1_CHANNEL_5

#define MAX_DUTY ((1000 * 90 * 1024) / 100)
#define MIN_DUTY ((1000 * 5 * 1024) / 100)
#define MID_DUTY ((1000 * 50 * 1024) / 100)

// samples to average over
#define NUMBER_OF_SAMPLES 16

static esp_adc_cal_characteristics_t adc1_chars;
volatile int duty = MID_DUTY;
volatile int sense_v = 0;
int x = 0;
int count = 0;
int bitIndex = 0;
int digit = 0;
volatile int samples[NUMBER_OF_SAMPLES] = {0};
volatile int sample_index = 0;
volatile int raw_value = 0;

uint8_t digitMapping[] = {
    0b01110111, // 0
    0b00010100, // 1
    0b01101101, // 2
    0b01011101, // 3
    0b00011110, // 4
    0b01011011, // 5
    0b01111011, // 6
    0b00010101, // 7
    0b01111111, // 8
    0b01011111, // 9
};

void power_task()
{
  raw_value = adc1_get_raw(SENSE_CHANNEL);
  samples[sample_index] = raw_value;
  sample_index++;
  if (sample_index >= NUMBER_OF_SAMPLES)
  {
    sample_index = 0;
  }
  int32_t values[NUMBER_OF_SAMPLES];
  for (int i = 0; i < NUMBER_OF_SAMPLES; i++)
  {
    values[i] = samples[i];
  }
  std::sort(values, values + NUMBER_OF_SAMPLES);
  int32_t value_sense = (values[NUMBER_OF_SAMPLES / 2] + values[NUMBER_OF_SAMPLES / 2 - 1]) / 2;
  sense_v = esp_adc_cal_raw_to_voltage(value_sense, &adc1_chars);

  int voltage_diff = 550 - sense_v;

  duty = duty + 5 * voltage_diff;

  if (duty > MAX_DUTY)
  {
    duty = MAX_DUTY;
  }
  if (duty < MIN_DUTY)
  {
    duty = MIN_DUTY;
  }

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty / 1000);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);

  // for (int i = 0; i < 28; i++)
  // {
  //   // output the digit value
  //   digitalWrite(GPIO_NUM_14, 0);
  //   // clock the digit in
  //   digitalWrite(GPIO_NUM_25, HIGH);
  //   digitalWrite(GPIO_NUM_25, LOW);
  // }
  // // clock the new values to the output
  // digitalWrite(GPIO_NUM_27, HIGH);
  // digitalWrite(GPIO_NUM_27, LOW);
  // x++;
  // if (x == 4000)
  // {
  //   x = 0;
  //   count++;
  //   if (count == 10)
  //   {
  //     count = 0;
  //   }
  // }
  // int on = digitMapping[count] & (1 << bitIndex);
  // // enable the correct digit
  // digitalWrite(GPIO_NUM_14, digit == 0);
  // // turn off the segments
  // digitalWrite(GPIO_NUM_25, LOW);
  // // set the correct segment
  // digitalWrite(GPIO_NUM_15, bitIndex & 1);
  // digitalWrite(GPIO_NUM_27, bitIndex & 2);
  // digitalWrite(GPIO_NUM_26, bitIndex & 4);
  // // enable output to the segment
  // digitalWrite(GPIO_NUM_25, on);

  // bitIndex++;
  // if (bitIndex == 7)
  // {
  //   bitIndex = 0;
  //   digit++;
  //   if (digit == 4)
  //   {
  //     digit = 0;
  //   }
  // }
}

void get_adc_calibration(adc_unit_t adc_unit, esp_adc_cal_characteristics_t *adc_chars)
{
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      adc_unit,
      ADC_ATTEN_DB_0,
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
  adc1_config_channel_atten(SENSE_CHANNEL, ADC_ATTEN_DB_0);

  // get the characteristics of ADC1
  get_adc_calibration(ADC_UNIT_1, &adc1_chars);

  // setup the timer
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
  ledc_channel.gpio_num = GPIO_NUM_25;
  ledc_channel.speed_mode = LEDC_HIGH_SPEED_MODE;
  ledc_channel.timer_sel = LEDC_TIMER_0;

  ledc_channel_config(&ledc_channel);

  // xTaskCreatePinnedToCore(power_task, "power_task", 2048, NULL, 5, NULL, 1);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &power_task, true);
  timerAlarmWrite(timer, 100, true);
  timerAlarmEnable(timer);

  pinMode(GPIO_NUM_25, OUTPUT);
  pinMode(GPIO_NUM_26, OUTPUT);
  pinMode(GPIO_NUM_27, OUTPUT);
  pinMode(GPIO_NUM_15, OUTPUT);
  pinMode(GPIO_NUM_14, OUTPUT);
}

void loop()
{
  Serial.printf("duty:%f,v:%f\n", duty / (1024.0f * 10.0f), sense_v / 1000.0f);
  // int real_value = esp_adc_cal_raw_to_voltage(raw_value, &adc1_chars);
  // Serial.printf("Raw:%d,Filtered:%d\n", real_value + 50, sense_v);

  // Serial.printf("%f %d %d\n", t, real_value, sense_v);
  // delay(100);
}