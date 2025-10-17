#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>

typedef struct timer_t timer_t;

struct timer_t
{
  /* the alarm */
  uint32_t m_alarm;
  /* start time in microseconds */
  uint32_t m_startTime;
};

/* Initialize a timer struct to default values */
void timer_init_default(timer_t *timer);

/* Init a timer with a number of alarms and optionally start it */
void timer_init(timer_t *timer, uint8_t alarm);

/* Start the timer but don't change current alarm, this shifts
 * the timer startTime but does not reset it's alarm state */
void timer_start(timer_t *timer, uint32_t offset);

/* Delete all alarms from the timer and reset */
void timer_reset(timer_t *timer);

/* Will return true if the timer hit */
uint8_t timer_alarm(timer_t *timer);

#endif
