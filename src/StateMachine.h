#pragma once

#include "StopWatch.h"
#include "TimeWrapper.h"

enum class State
{
    SHOWING_TIME,
    SHOWING_DATE,
    STOP_WATCH_STOPPED,
    STOP_WATCH_RESET,
    STOP_WATCH_RUNNING,
};

enum class Event
{
    BUTTON1_TOUCH = 0,
    MODE_TOUCH = 0,
    BUTTON2_TOUCH = 1,
    START_STOP_TOUCH = 1,
    BUTTON3_TOUCH = 2,
    RESET_TOUCH = 2,
    BUTTON4_TOUCH = 3
};

class StateMachine
{
private:
    State state;
    TimeWrapper *time_wrapper;
    StopWatch *stop_watch;
    uint64_t last_event_time = 0;

    void switch_to_state(State new_state)
    {
        exit_state(state);
        state = new_state;
        enter_state(state);
    }
    void enter_state(State new_state)
    {
        switch (new_state)
        {
        case State::STOP_WATCH_RUNNING:
            stop_watch->start();
            break;
        case State::STOP_WATCH_STOPPED:
            stop_watch->stop();
            break;
        case State::STOP_WATCH_RESET:
            stop_watch->reset();
            break;
        default:
            break;
        }
    }
    void exit_state(State old_state)
    {
        // nothing to do at the moment
    }

public:
    StateMachine(TimeWrapper *time_wrapper, StopWatch *stop_watch) : time_wrapper(time_wrapper), stop_watch(stop_watch)
    {
        state = State::SHOWING_TIME;
    }

    State get_state() { return state; }

    void proccess_event(Event event)
    {
        // simple debounce
        if (millis() - last_event_time < 500)
            return;
        last_event_time = millis();
        switch (event)
        {
        case Event::MODE_TOUCH:
        {
            switch (state)
            {
            case State::SHOWING_TIME:
                switch_to_state(State::SHOWING_DATE);
                break;
            case State::SHOWING_DATE:
                switch_to_state(State::STOP_WATCH_STOPPED);
                break;
            case State::STOP_WATCH_RESET:
            case State::STOP_WATCH_STOPPED:
                switch_to_state(State::SHOWING_TIME);
                break;
            default:
                break;
            }
            break;
        }
        case Event::START_STOP_TOUCH:
        {
            switch (state)
            {
            case State::STOP_WATCH_RESET:
            case State::STOP_WATCH_STOPPED:
                switch_to_state(State::STOP_WATCH_RUNNING);
                break;
            case State::STOP_WATCH_RUNNING:
                switch_to_state(State::STOP_WATCH_STOPPED);
                break;
            default:
                break;
            }
            break;
        }
        case Event::RESET_TOUCH:
        {
            switch (state)
            {
            case State::STOP_WATCH_STOPPED:
                switch_to_state(State::STOP_WATCH_RESET);
                break;
            default:
                break;
            }
            break;
        }
        case Event::BUTTON4_TOUCH:
        {
            // nothing to do
            break;
        }
        }
    }
    int get_display_digits()
    {
        switch (state)
        {
        case State::STOP_WATCH_RESET:
        case State::STOP_WATCH_RUNNING:
        case State::STOP_WATCH_STOPPED:
            return stop_watch->get_elapsed_time() / 10000;
        case State::SHOWING_TIME:
            return time_wrapper->get_hours() * 100 + time_wrapper->get_minutes();
        case State::SHOWING_DATE:
            return time_wrapper->get_day() * 100 + time_wrapper->get_month();
        }
        return 0;
    }
};