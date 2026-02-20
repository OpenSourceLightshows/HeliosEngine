#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>

class Timer
{
public:
  Timer();
  ~Timer();

  // init a timer with a number of alarms and optionally start it
  void init(uint8_t alarm);
  // init timer using explicit "now" tick source
  void initAt(uint8_t alarm, uint32_t now);

  // start the timer but don't change current alarm, this shifts
  // the timer startTime but does not reset it's alarm state
  void start(uint32_t offset = 0);
  // start timer using explicit "now" tick source
  void startAt(uint32_t now, uint32_t offset = 0);
  // delete all alarms from the timer and reset
  void reset();
  // Will return the true if the timer hit
  bool alarm();
  // same as alarm() but with explicit "now" tick source
  bool alarmAt(uint32_t now);

private:
  // the alarm
  uint32_t m_alarm;
  // start time in microseconds
  uint32_t m_startTime;
};

#endif
