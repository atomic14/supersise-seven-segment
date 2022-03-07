#include <Arduino.h>
#include "BoostSupply.h"
#include "Digits.h"
#include "TimeWrapper.h"
#include "StopWatch.h"
#include "StateMachine.h"
#include <driver/timer.h>
#include <driver/gpio.h>

const adc1_channel_t BOOST_FEEDBACK_PIN = ADC1_CHANNEL_5;
const gpio_num_t BOOST_PWM_PIN = GPIO_NUM_25;
const gpio_num_t SERIAL_OUT_PIN = GPIO_NUM_25;
const gpio_num_t SERIAL_CLOCK_PIN = GPIO_NUM_25;
const gpio_num_t SERIAL_LATCH_PIN = GPIO_NUM_25;
const gpio_num_t SERIAL_CLR_PIN = GPIO_NUM_25;
const gpio_num_t SERIAL_ENABLE_PIN = GPIO_NUM_25;

BoostSupply *boost_supply;
Digits *digits;
TimeWrapper *time_wrapper;
StopWatch *stop_watch;
StateMachine *state_machine;
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
  digitalWrite(SERIAL_LATCH_PIN, HIGH);
  digitalWrite(SERIAL_LATCH_PIN, LOW);
}

void touch_button1_isr()
{
  state_machine->proccess_event(Event::BUTTON1_TOUCH);
}

void touch_button2_isr()
{
  state_machine->proccess_event(Event::BUTTON2_TOUCH);
}

void touch_button3_isr()
{
  state_machine->proccess_event(Event::BUTTON3_TOUCH);
}

void touch_button4_isr()
{
  state_machine->proccess_event(Event::BUTTON4_TOUCH);
}

void setup()
{
  Serial.begin(115200);
  // start up the boost power supply
  boost_supply = new BoostSupply(BOOST_PWM_PIN, BOOST_FEEDBACK_PIN);
  // create our 4 seven segment digits
  digits = new Digits(4);
  // create the stopwatch
  stop_watch = new StopWatch();
  // kick off time
  time_wrapper = new TimeWrapper();
  // this is the state machine that controls everything
  state_machine = new StateMachine(time_wrapper, stop_watch);
  // start up a timer to service the boost supply and write out the segments
  timer = timerBegin(0, 80, true); // this will run a timer with a 1MHz clock (80MHz/80 = 1MHz)
  timerAttachInterrupt(timer, &timer_callback, true);
  timerAlarmWrite(timer, 50, true); // wake up everytime we hit 50 ticks (20KHz)
  timerAlarmEnable(timer);
  // setup the shift registeros
  pinMode(SERIAL_OUT_PIN, OUTPUT);
  pinMode(SERIAL_CLOCK_PIN, OUTPUT);
  pinMode(SERIAL_LATCH_PIN, OUTPUT);
  pinMode(SERIAL_CLR_PIN, OUTPUT);
  pinMode(SERIAL_ENABLE_PIN, OUTPUT);
  // clear the shift register
  digitalWrite(SERIAL_CLR_PIN, HIGH);
  digitalWrite(SERIAL_CLR_PIN, LOW);
  // copy to the output registers
  digitalWrite(SERIAL_LATCH_PIN, HIGH);
  digitalWrite(SERIAL_LATCH_PIN, LOW);
  // and enable the output
  digitalWrite(SERIAL_ENABLE_PIN, HIGH);

  // set up the touch buttons
  touchAttachInterrupt(T9, touch_button1_isr, 50);
  touchAttachInterrupt(T9, touch_button2_isr, 50);
  touchAttachInterrupt(T9, touch_button3_isr, 50);
  touchAttachInterrupt(T9, touch_button4_isr, 50);
}

void loop()
{
  // interesting stats
  Serial.printf("duty:%f,v:%f\n", boost_supply->get_duty() / 1024000.0f, boost_supply->get_feedback_voltage() / 1000.0);
  // update the current time
  time_wrapper->update();
  // update the digits depending on our current state
  int get_dsplay_digits = state_machine->get_display_digits();
  for (int d = 0; d < 4; d++)
  {
    digits->set_digit(d, get_dsplay_digits % 10);
    get_dsplay_digits /= 10;
  }
  delay(100);
}