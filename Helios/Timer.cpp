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
  // if no time passed it is the first alarm that is starting
  if (timeDiff == 0) {
    return true;
  }
  // simple >= check, no modulo/division: fire as soon as the period has
  // elapsed and re-anchor to now, so a slipped tick is absorbed smoothly
  // and beats stay evenly spaced (correct on 1MHz hardware). No realign
  // branch -- realigning to an exact grid multiple drops beats on fast
  // patterns where jitter exceeds one period.
  if (timeDiff < (int32_t)m_alarm) {
    return false;
  }
  // update the start time of the timer
  m_startTime = now;
  return true;
}
