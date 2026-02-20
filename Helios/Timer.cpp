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
  initAt(alarm, Time::getCurtime());
}

void Timer::initAt(uint8_t alarm, uint32_t now)
{
  reset();
  m_alarm = alarm;
  startAt(now);
}

void Timer::start(uint32_t offset)
{
  startAt(Time::getCurtime(), offset);
}

void Timer::startAt(uint32_t now, uint32_t offset)
{
  // reset the start time
  m_startTime = now + offset;
}

void Timer::reset()
{
  m_alarm = 0;
  m_startTime = 0;
}

bool Timer::alarm()
{
  return alarmAt(Time::getCurtime());
}

bool Timer::alarmAt(uint32_t now)
{
  if (!m_alarm) {
    return false;
  }
  // time since start (forward or backwards)
  int32_t timeDiff = (int32_t)(int64_t)(now - m_startTime);
  if (timeDiff < 0) {
    return false;
  }
  // if no time passed it's first alarm that is starting
  if (timeDiff == 0) {
    return true;
  }
  // if the current alarm duration is not a multiple of the current tick
  if (m_alarm && (timeDiff % m_alarm) != 0) {
    // then the alarm was not hit
    return false;
  }
  // update the start time of the timer
  m_startTime = now;
  return true;
}
