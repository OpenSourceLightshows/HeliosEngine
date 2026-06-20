#include <stdlib.h>

#include "Timer.h"

#include "TimeControl.h"

Timer::Timer() :
  m_alarm(0),
  m_startTime(0)
{
}

Timer::~Timer()
{
}

void Timer::init(uint8_t alarm)
{
  reset();
  m_alarm = alarm;
  start();
}

void Timer::start(uint32_t offset)
{
  // reset the start time
  m_startTime = Time::getCurtime() + offset;
}

void Timer::reset()
{
  m_alarm = 0;
  m_startTime = 0;
}

bool Timer::alarm()
{
  if (!m_alarm) {
    return false;
  }
  uint32_t now = Time::getCurtime();
  // time since start (forward or backwards)
  int32_t timeDiff = (int32_t)(int64_t)(now - m_startTime);
  if (timeDiff < 0) {
    return false;
  }
  // if no time passed it's first alarm that is starting
  if (timeDiff == 0) {
    return true;
  }
  // At 1MHz a 32-bit software modulo costs ~240 AVR cycles — called every tick,
  // that's a significant fraction of budget. The original (timeDiff % m_alarm == 0)
  // was checking if the alarm interval divided evenly, but because start() always
  // resets m_startTime the moment the alarm fires, timeDiff simply counts up from
  // 0 to m_alarm and then resets. So "has the alarm fired?" is just (timeDiff >= m_alarm),
  // no division needed.
  if (timeDiff < (int32_t)m_alarm) {
    return false;
  }
  // update the start time of the timer
  m_startTime = now;
  return true;
}
