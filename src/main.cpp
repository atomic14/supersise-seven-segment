#include <Arduino.h>
#include <WiFi.h>
#include "BoostSupply.h"
#include "Digits.h"
#include "TimeWrapper.h"
#include "StopWatch.h"
#include "StateMachine.h"
#include <driver/timer.h>
#include <driver/gpio.h>

const adc1_channel_t BOOST_FEEDBACK_PIN = ADC1_CHANNEL_5;
const gpio_num_t BOOST_PWM_PIN = GPIO_NUM_25;
const gpio_num_t SERIAL_OUT_PIN = GPIO_NUM_21;
const gpio_num_t SERIAL_CLOCK_PIN = GPIO_NUM_18;
const gpio_num_t SERIAL_LATCH_PIN = GPIO_NUM_23;
const gpio_num_t SERIAL_CLR_PIN = GPIO_NUM_19;
const gpio_num_t SERIAL_ENABLE_PIN = GPIO_NUM_22;

BoostSupply *boost_supply;
Digits *digits;
TimeWrapper *time_wrapper;
StopWatch *stop_watch;
StateMachine *state_machine;
hw_timer_t *timer = NULL;

int timer_count = 0;
void timer_callback()
{
  // monitor and apply any adjustments to the boost supply
  boost_supply->service();
  timer_count++;
}

void touch_button1_isr()
{
  state_machine->proccess_event(Event::BUTTON1_TOUCH);
}

void touch_button2_isr()
{
  state_machine->proccess_event(Event::START_STOP_TOUCH);
}

void touch_button3_isr()
{
  state_machine->proccess_event(Event::RESET_TOUCH);
}

void touch_button4_isr()
{
  state_machine->proccess_event(Event::BUTTON4_TOUCH);
}

void setup()
{
  Serial.begin(115200);
  // connect to wifi
  Serial.println("Connecting to wifi...");
  WiFi.begin("CMGResearch_old", "02087552867");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
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
  timerAlarmWrite(timer, 500, true); // wake up everytime we hit 100 ticks (10KHz)
  timerAlarmEnable(timer);
  // setup the shift registeros
  pinMode(SERIAL_OUT_PIN, OUTPUT);
  pinMode(SERIAL_CLOCK_PIN, OUTPUT);
  pinMode(SERIAL_LATCH_PIN, OUTPUT);
  pinMode(SERIAL_CLR_PIN, OUTPUT);
  pinMode(SERIAL_ENABLE_PIN, OUTPUT);
  // clear the shift register
  digitalWrite(SERIAL_CLR_PIN, LOW);
  digitalWrite(SERIAL_CLR_PIN, HIGH);
  // copy to the output registers
  digitalWrite(SERIAL_LATCH_PIN, HIGH);
  digitalWrite(SERIAL_LATCH_PIN, LOW);
  // and enable the output
  digitalWrite(SERIAL_ENABLE_PIN, LOW);
  // digitalWrite(SERIAL_ENABLE_PIN, HIGH);

  // set up the touch buttons
  touchAttachInterrupt(T9, touch_button1_isr, 20);
  touchAttachInterrupt(T7, touch_button2_isr, 40);
  touchAttachInterrupt(T6, touch_button3_isr, 40);
  touchAttachInterrupt(T0, touch_button4_isr, 20);
}

int count = 0;
int wait = 0;
void loop()
{
  // interesting stats
  // Serial.printf("duty:%f,v:%f,t:55\n", 100.0f * boost_supply->get_duty() / 1024000.0f, boost_supply->get_feedback_voltage() / 10.0);
  // update the current time
  if (!time_wrapper->update())
  {
    Serial.printf("time update failed\n");
  }
  // update the digits depending on our current state
  int get_display_digits = state_machine->get_display_digits();
  // int get_display_digits = count;
  Serial.printf("digits:%d\n", get_display_digits);
  for (int d = 3; d >= 0; d--)
  {
    digits->set_digit(d, get_display_digits % 10);
    get_display_digits /= 10;
  }
  // clock out all the segments to the shift registers
  for (int digit = 0; digit < 4; digit++)
  {
    for (int segment = 0; segment < 7; segment++)
    {
      // digitalWrite(SERIAL_OUT_PIN, (segment + digit * 7) < count);
      // digitalWrite(SERIAL_OUT_PIN, HIGH);
      // // set the value
      digitalWrite(SERIAL_OUT_PIN, digits->get_digit_segment(digit, segment));
      // // pulse the clock pin
      digitalWrite(SERIAL_CLOCK_PIN, HIGH);
      digitalWrite(SERIAL_CLOCK_PIN, LOW);
    }
  }
  // wait++;
  // if (wait == 300)
  // {
  //   wait = 0;
  //   count++;
  //   if (count > 4 * 7)
  //   {
  //     count = 0;
  //   }
  // }
  // // push the segment values to the output registers
  digitalWrite(SERIAL_LATCH_PIN, HIGH);
  digitalWrite(SERIAL_LATCH_PIN, LOW);
  // Serial.printf("%d\n", timer_count);
  delay(100);
  // Serial.printf("State:%d\n", state_machine->get_state());

  // Serial.printf("T9:%d,T7:%d,T6:%d,T0:%d\n", touchRead(T9), touchRead(T7), touchRead(T6), touchRead(T0));
}