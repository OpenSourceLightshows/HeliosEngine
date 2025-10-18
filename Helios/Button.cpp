#include "Button.h"
#include "TimeControl.h"
#include "HeliosConfig.h"

#ifdef HELIOS_EMBEDDED
#ifdef HELIOS_8051
#include "ca51f152.h"
// Button connected to P3.3 (INT1)
#define BUTTON_PIN 3
#else
#include <avr/interrupt.h>
#include <avr/io.h>
#ifdef HELIOS_ARDUINO
#include <arduino.h>
#endif
#define BUTTON_PIN 3
#define BUTTON_PORT 2
#endif
#endif

// Forward declaration
#ifdef __cplusplus
extern "C" {
#endif
void helios_wakeup(void);
void helios_terminate(void);
#ifdef __cplusplus
}
#endif

#ifdef HELIOS_CLI
// Forward declarations for CLI functions
static uint8_t button_process_pre_input(void);
static uint8_t button_process_post_input(void);
#endif

#ifdef HELIOS_8051
// 8051 has limited internal RAM, so use external RAM for all static variables
#define STATIC_VAR static __xdata
#else
#define STATIC_VAR static
#endif

// static members of Button
STATIC_VAR uint32_t m_pressTime = 0;
STATIC_VAR uint32_t m_releaseTime = 0;
STATIC_VAR uint32_t m_holdDuration = 0;
STATIC_VAR uint32_t m_releaseDuration = 0;
STATIC_VAR uint8_t m_releaseCount = 0;
STATIC_VAR uint8_t m_buttonState = 0;
STATIC_VAR uint8_t m_newPress = 0;
STATIC_VAR uint8_t m_newRelease = 0;
STATIC_VAR uint8_t m_isPressed = 0;
STATIC_VAR uint8_t m_shortClick = 0;
STATIC_VAR uint8_t m_longClick = 0;
STATIC_VAR uint8_t m_holdClick = 0;

#ifdef HELIOS_CLI
STATIC_VAR uint8_t m_pinState = 0;
STATIC_VAR uint8_t m_enableWake = 0;
// an input queue for the button, each tick one even is processed
// out of this queue and used to produce input
#define INPUT_QUEUE_SIZE 4096
STATIC_VAR char m_inputQueue[INPUT_QUEUE_SIZE];
STATIC_VAR uint32_t m_queueHead = 0;
STATIC_VAR uint32_t m_queueTail = 0;
#endif

// initialize a new button object with a pin number
uint8_t button_init(void)
{
  m_pressTime = 0;
  m_releaseTime = 0;
  m_holdDuration = 0;
  m_releaseDuration = 0;
  m_newPress = 0;
  m_newRelease = 0;
  m_shortClick = 0;
  m_longClick = 0;
  m_holdClick = 0;
  m_buttonState = button_check();
  m_releaseCount = !m_buttonState;
  m_isPressed = m_buttonState;
#ifdef HELIOS_CLI
  m_pinState = 0;
  m_enableWake = 0;
  m_queueHead = 0;
  m_queueTail = 0;
#endif
#ifdef HELIOS_EMBEDDED
#ifdef HELIOS_ARDUINO
  pinMode(3, INPUT);
#elif defined(HELIOS_8051)
  // Configure P3.3 as input (already done in helios_init)
  // Disable external interrupt 1 initially
  EX1 = 0;
#else
  // turn off wake
  PCMSK &= ~(1 << PCINT3);
  GIMSK &= ~(1 << PCIE);
#endif
#endif
  return 1;
}

// enable wake on press
void button_enable_wake(void)
{
#ifdef HELIOS_EMBEDDED
#ifdef HELIOS_8051
  // Configure INT1 to trigger on falling edge (button press)
  IT1 = 1;  // Edge triggered
  EX1 = 1;  // Enable external interrupt 1
  EA = 1;   // Enable global interrupts
#else
  // Configure INT0 to trigger on falling edge
  PCMSK |= (1 << PCINT3);
  GIMSK |= (1 << PCIE);
  sei();
#endif
#else // HELIOS_CLI
  m_enableWake = 1;
#endif
}

#ifdef HELIOS_EMBEDDED
#ifdef HELIOS_8051
// External interrupt 1 ISR for button wake
void int1_isr(void) __interrupt(2) {
  // Disable interrupt
  EX1 = 0;
  helios_wakeup();
}
#else
ISR(PCINT0_vect) {
  PCMSK &= ~(1 << PCINT3);
  GIMSK &= ~(1 << PCIE);
  helios_wakeup();
}
#endif
#endif

// directly poll the pin for whether it's pressed right now
uint8_t button_check(void)
{
#ifdef HELIOS_EMBEDDED
#ifdef HELIOS_ARDUINO
  return digitalRead(3) == HIGH;
#elif defined(HELIOS_8051)
  // Read P3.3 state (active high assumed)
  return (P3 & (1 << BUTTON_PIN)) != 0;
#else
  return (PINB & (1 << 3)) != 0;
#endif
#elif defined(HELIOS_CLI)
  // then just return the pin state as-is, the input event may have
  // adjusted this value
  return m_pinState;
#else
  return 0;
#endif
}

// detect if the button is being held for a long hold (past long click)
uint8_t button_hold_pressing(void)
{
  uint16_t holDur = (uint16_t)(button_hold_duration());
  if (holDur > HOLD_CLICK_START && holDur <= HOLD_CLICK_END && button_is_pressed()) {
    return 1;
  }
  return 0;
}

// poll the button pin and update the state of the button object
void button_update(void)
{
#ifdef HELIOS_CLI
  // process any pre-input events in the queue
  uint8_t processed_pre = button_process_pre_input();
#endif

  uint8_t newButtonState = button_check();
  m_newPress = 0;
  m_newRelease = 0;
  if (newButtonState != m_buttonState) {
    m_buttonState = newButtonState;
    m_isPressed = m_buttonState;
    if (m_isPressed) {
      m_pressTime = time_get_current_time();
      m_newPress = 1;
    } else {
      m_releaseTime = time_get_current_time();
      m_newRelease = 1;
      m_releaseCount++;
    }
  }
  if (m_isPressed) {
    m_holdDuration = (time_get_current_time() >= m_pressTime) ? (uint32_t)(time_get_current_time() - m_pressTime) : 0;
  } else {
    m_releaseDuration = (time_get_current_time() >= m_releaseTime) ? (uint32_t)(time_get_current_time() - m_releaseTime) : 0;
  }
  m_shortClick = (m_newRelease && (m_holdDuration <= SHORT_CLICK_THRESHOLD));
  m_longClick = (m_newRelease && (m_holdDuration > SHORT_CLICK_THRESHOLD) && (m_holdDuration < HOLD_CLICK_START));
  m_holdClick = (m_newRelease && (m_holdDuration >= HOLD_CLICK_START) && (m_holdDuration <= HOLD_CLICK_END));

#ifdef HELIOS_CLI
  // if there was no pre-input event this tick, process a post input event
  // to ensure there is only one event per tick processed
  if (!processed_pre) {
    button_process_post_input();
  }

  if (m_enableWake) {
    if (m_isPressed || m_shortClick || m_longClick) {
      helios_wakeup();
    }
  }
#endif
}

uint8_t button_on_press(void)
{
  return m_newPress;
}

uint8_t button_on_release(void)
{
  return m_newRelease;
}

uint8_t button_is_pressed(void)
{
  return m_isPressed;
}

uint8_t button_on_short_click(void)
{
  return m_shortClick;
}

uint8_t button_on_long_click(void)
{
  return m_longClick;
}

uint8_t button_on_hold_click(void)
{
  return m_holdClick;
}

uint32_t button_press_time(void)
{
  return m_pressTime;
}

uint32_t button_release_time(void)
{
  return m_releaseTime;
}

uint32_t button_hold_duration(void)
{
  return m_holdDuration;
}

uint32_t button_release_duration(void)
{
  return m_releaseDuration;
}

uint8_t button_release_count(void)
{
  return m_releaseCount;
}

#ifdef HELIOS_CLI
static uint8_t button_process_pre_input(void)
{
  if (m_queueHead == m_queueTail) {
    return 0;
  }
  char command = m_inputQueue[m_queueHead];
  switch (command) {
  case 'p': // press
    button_do_press();
    break;
  case 'r': // release
    button_do_release();
    break;
  case 't': // toggle
    button_do_toggle();
    break;
  case 'q': // quit
    helios_terminate();
    break;
  case 'w': // wait
    // wait is pre input I guess
    break;
  default:
    // return here! do not pop the queue
    // do not process post input events
    return 0;
  }
  // now pop whatever pre-input command was processed
  m_queueHead = (m_queueHead + 1) % INPUT_QUEUE_SIZE;
  return 1;
}

static uint8_t button_process_post_input(void)
{
  if (m_queueHead == m_queueTail) {
    // probably processed the pre-input event already
    return 0;
  }
  // process input queue from the command line
  char command = m_inputQueue[m_queueHead];
  switch (command) {
  case 'c': // click button
    button_do_short_click();
    break;
  case 'l': // long click button
    button_do_long_click();
    break;
  default:
    // should never happen
    return 0;
  }
  m_queueHead = (m_queueHead + 1) % INPUT_QUEUE_SIZE;
  return 1;
}

void button_do_short_click(void)
{
  m_newRelease = 1;
  m_shortClick = 1;
  m_pressTime = time_get_current_time();
  m_holdDuration = SHORT_CLICK_THRESHOLD - 1;
  m_releaseCount++;
}

void button_do_long_click(void)
{
  m_newRelease = 1;
  m_longClick = 1;
  m_pressTime = time_get_current_time();
  m_holdDuration = SHORT_CLICK_THRESHOLD + 1;
  m_releaseCount++;
}

void button_do_hold_click(void)
{
  m_newRelease = 1;
  m_holdClick = 1;
  m_pressTime = time_get_current_time();
  m_holdDuration = HOLD_CLICK_START + 1;
  m_releaseCount++;
}

// this will actually press down the button, it's your responsibility to wait
// for the appropriate number of ticks and then release the button
void button_do_press(void)
{
  m_pinState = 1;
}

void button_do_release(void)
{
  m_pinState = 0;
}

void button_do_toggle(void)
{
  m_pinState = !m_pinState;
}

// queue up an input event for the button
void button_queue_input(char input)
{
  uint32_t nextTail = (m_queueTail + 1) % INPUT_QUEUE_SIZE;
  if (nextTail != m_queueHead) {
    m_inputQueue[m_queueTail] = input;
    m_queueTail = nextTail;
  }
}

uint32_t button_input_queue_size(void)
{
  if (m_queueTail >= m_queueHead) {
    return m_queueTail - m_queueHead;
  } else {
    return INPUT_QUEUE_SIZE - m_queueHead + m_queueTail;
  }
}
#endif

