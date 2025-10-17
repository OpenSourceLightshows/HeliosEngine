#include <stdlib.h>

#include "Timer.h"

#include "TimeControl.h"

void timer_init_default(helios_timer_t *timer)
{
  timer->m_alarm = 0;
  timer->m_startTime = 0;
}

void timer_init(helios_timer_t *timer, uint8_t alarm)
{
  timer_reset(timer);
  timer->m_alarm = alarm;
  timer_start(timer, 0);
}

void timer_start(helios_timer_t *timer, uint32_t offset)
{
  /* reset the start time */
  timer->m_startTime = time_get_current_time() + offset;
}

void timer_reset(helios_timer_t *timer)
{
  timer->m_alarm = 0;
  timer->m_startTime = 0;
}

uint8_t timer_alarm(helios_timer_t *timer)
{
  if (!timer->m_alarm) {
    return 0;
  }
  uint32_t now = time_get_current_time();
  /* time since start (forward or backwards) */
  int32_t timeDiff = (int32_t)(int64_t)(now - timer->m_startTime);
  if (timeDiff < 0) {
    return 0;
  }
  /* if no time passed it's first alarm that is starting */
  if (timeDiff == 0) {
    return 1;
  }
  /* if the current alarm duration is not a multiple of the current tick */
  if (timer->m_alarm && (timeDiff % timer->m_alarm) != 0) {
    /* then the alarm was not hit */
    return 0;
  }
  /* update the start time of the timer */
  timer->m_startTime = now;
  return 1;
}

