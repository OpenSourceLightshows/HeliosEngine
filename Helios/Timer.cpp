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
  // The alarm fires on exact multiples of m_alarm (e.g. ticks 5, 10, 15 for m_alarm=5).
  // After firing, m_startTime resets to `now`, but timeDiff can land anywhere in the
  // next interval — the modulo ensures we only fire on a clean boundary, not just
  // whenever timeDiff >= m_alarm. This prevents the alarm from firing early when
  // alarm() is called mid-interval.
  if (m_alarm && (timeDiff % m_alarm) != 0) {
    return false;
  }
  // update the start time of the timer
  m_startTime = now;
  return true;
}
