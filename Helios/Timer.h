#ifndef TIMER_H
#define TIMER_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct helios_timer_t helios_timer_t;

struct helios_timer_t
{
  // the alarm
  uint32_t m_alarm;
  // start time in microseconds
  uint32_t m_startTime;
};

// Initialize a timer struct to default values
void timer_init_default(helios_timer_t *timer);

// Init a timer with a number of alarms and optionally start it
void timer_init(helios_timer_t *timer, uint8_t alarm);

// 
void timer_start(helios_timer_t *timer, uint32_t offset);

// Delete all alarms from the timer and reset
void timer_reset(helios_timer_t *timer);

// Will return true if the timer hit
uint8_t timer_alarm(helios_timer_t *timer);

#ifdef __cplusplus
}
#endif

#endif
