#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>

class Timer
{

  Timer();
  ~Timer();

  // init a timer with a number of alarms and optionally start it
  void init(uint8_t alarm);

  // start the timer but don't change current alarm, this shifts
  // the timer startTime but does not reset it's alarm state
  void start(uint32_t offset = 0);
  // delete all alarms from the timer and reset
  void reset();
  // Will return the true if the timer hit
  bool alarm();


  // the alarm
  uint32_t m_alarm;
  // start time in microseconds
  uint32_t m_startTime;
};

#endif
