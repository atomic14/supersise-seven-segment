#include <Arduino.h>
#include "BoostSupply.h"
#include "Digits.h"
#include <driver/timer.h>
#include <driver/gpio.h>

// GPIO_33
#define BOOST_FEEDBACK_PIN ADC1_CHANNEL_5
#define BOOST_PWM_PIN GPIO_PIN_NUM_25

BoostSupply *boost_supply;
Digits *digits;
hw_timer_t *timer = NULL;

void timer_callback()
{
  // monitor and apply any adjustments to the boost supply
  boost_supply->service();
  // clock out all the segments to the shift registers
  for (int digit = 0; digit < 4; digit++)
  {
    for (int segment = 0; segment < 7; segment++)
    {
      // set the value
      digitalWrite(SERIAL_OUT_PIN, digits->get_digit_segment(digit, segment));
      // pulse the clock pin
      digitalWrite(SERIAL_CLOCK_PIN, HIGH);
      digitalWrite(SERIAL_CLOCK_PIN, LOW);
    }
  }
  // push the segment values to the output registers
  digitalWrite(SERIAL_REG_PIN, HIGH);
  digitalWrite(SERIAL_REG_PIN, LOW);
}

void setup()
{
  Serial.begin(115200);
  // start up the boost power supply
  boost_supply = new BoostSupply(BOOST_PWM_PIN, BOOST_FEEDBACK_PIN);
  // create our 4 seven segment digits
  digits = new Digits(4);
  // start up a timer to service the boost supply and write out the segments
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &timer_callback, true);
  timerAlarmWrite(timer, 100, true);
  timerAlarmEnable(timer);
  // setup the shift registeros
  pinMode(SERIAL_OUT_PIN, OUTPUT);
  pinMode(SERIAL_CLOCK_PIN, OUTPUT);
  pinMode(SERIAL_REG_PIN, OUTPUT);
  pinMode(SERIAL_CLR, OUTPUT);
  pinMode(SERIAL_ENABLE, OUTPUT);
  // clear the shift register
  digitalWrite(SERIAL_CLR, HIGH);
  digitalWrite(SERIAL_CLR, LOW);
  // copy to the output registers
  digitalWrite(SERIAL_REG_PIN, HIGH);
  digitalWrite(SERIAL_REG_PIN, LOW);
  // and enable the output
  digitalWrite(SERIAL_ENABLE, HIGH);
}

void loop()
{
  // interesting stats
  Serial.printf("duty:%f,v:%f\n", boost_supply->get_duty(), boost_supply->get_feedback_voltage());
  delay(100)
}