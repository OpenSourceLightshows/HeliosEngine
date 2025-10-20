#ifndef STM8_INIT_H
#define STM8_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

// STM8 hardware initialization functions
void stm8_init_clock(void);
void stm8_init_gpio(void);
void stm8_init_timers(void);
void stm8_init_interrupts(void);

#ifdef __cplusplus
}
#endif

#endif // STM8_INIT_H
