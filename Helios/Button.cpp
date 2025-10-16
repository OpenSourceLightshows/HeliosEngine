#include "Button.h"
#include "TimeControl.h"

#ifdef HELIOS_EMBEDDED
  #ifdef HELIOS_8051
    #include "CA51Hardware.h"
  #elif defined(HELIOS_AVR)
    #include <avr/interrupt.h>
    #include <avr/io.h>
    #ifdef HELIOS_ARDUINO
      #include <arduino.h>
    #endif
  #endif
#endif

#include "Helios.h"

// static members of Button
uint32_t Button::m_pressTime = 0;
uint32_t Button::m_releaseTime = 0;
uint32_t Button::m_holdDuration = 0;
uint32_t Button::m_releaseDuration = 0;
uint8_t Button::m_releaseCount = 0;
bool Button::m_buttonState = false;
bool Button::m_newPress = false;
bool Button::m_newRelease = false;
bool Button::m_isPressed = false;
bool Button::m_shortClick = false;
bool Button::m_longClick = false;
bool Button::m_holdClick = false;

#ifdef HELIOS_CLI
// an input queue for the button, each tick one even is processed
// out of this queue and used to produce input
std::queue<char> Button::m_inputQueue;
// the virtual pin state
bool Button::m_pinState = false;
// whether the button is waiting to wake the device
bool Button::m_enableWake = false;
#endif

// initialize a new button object with a pin number
bool Button::init()
{
  m_pressTime = 0;
  m_releaseTime = 0;
  m_holdDuration = 0;
  m_releaseDuration = 0;
  m_newPress = false;
  m_newRelease = false;
  m_shortClick = false;
  m_longClick = false;
  m_holdClick = false;
  m_buttonState = check();
  m_releaseCount = !m_buttonState;
  m_isPressed = m_buttonState;
#ifdef HELIOS_CLI
  m_pinState = false;
  m_enableWake = false;
#endif
#ifdef HELIOS_EMBEDDED
  #ifdef HELIOS_8051
    // CA51F152XX: Configure button pin as input
    SET_PIN_INPUT(1, BUTTON_PIN);
    // Disable wake interrupt initially
    IE &= ~EX0;  // Disable external interrupt 0
  #elif defined(HELIOS_ARDUINO)
    pinMode(3, INPUT);
  #elif defined(HELIOS_AVR)
    // turn off wake
    PCMSK &= ~(1 << PCINT3);
    GIMSK &= ~(1 << PCIE);
  #endif
#endif
  return true;
}

// enable wake on press
void Button::enableWake()
{
#ifdef HELIOS_EMBEDDED
  #ifdef HELIOS_8051
    // CA51F152XX: Configure external interrupt 0 for button wake
    // IT0: Set interrupt type (1 = edge triggered, 0 = level triggered)
    TCON |= IT0;  // Edge-triggered on falling edge
    IE0 = 0;      // Clear interrupt flag
    IE |= EX0;    // Enable external interrupt 0
    ENABLE_INTERRUPTS();  // Global interrupt enable
  #elif defined(HELIOS_AVR)
    // Configure INT0 to trigger on falling edge
    PCMSK |= (1 << PCINT3);
    GIMSK |= (1 << PCIE);
    sei();
  #endif
#else // HELIOS_CLI
  m_enableWake = false;
#endif
}

#ifdef HELIOS_EMBEDDED
  #ifdef HELIOS_8051
    // 8051 External Interrupt 0 ISR (button wake)
    void button_wake_isr(void) ISR_ATTR(0) {
      IE &= ~EX0;  // Disable external interrupt 0
      IE0 = 0;     // Clear interrupt flag
      Helios::wakeup();
    }
  #elif defined(HELIOS_AVR)
    ISR(PCINT0_vect) {
      PCMSK &= ~(1 << PCINT3);
      GIMSK &= ~(1 << PCIE);
      Helios::wakeup();
    }
  #endif
#endif

// directly poll the pin for whether it's pressed right now
bool Button::check()
{
#ifdef HELIOS_EMBEDDED
  #ifdef HELIOS_8051
    // CA51F152XX: Read button pin state
    return READ_PIN(1, BUTTON_PIN);
  #elif defined(HELIOS_ARDUINO)
    return digitalRead(3) == HIGH;
  #elif defined(HELIOS_AVR)
    return (PINB & (1 << 3)) != 0;
  #endif
#elif defined(HELIOS_CLI)
  // then just return the pin state as-is, the input event may have
  // adjusted this value
  return m_pinState;
#else
  return false;
#endif
}

// detect if the button is being held for a long hold (past long click)
bool Button::holdPressing()
{
  uint16_t holDur = (uint16_t)(Button::holdDuration());
  if (holDur > HOLD_CLICK_START && holDur <= HOLD_CLICK_END && Button::isPressed()) {
    return true;
  }
  return false;
}

// poll the button pin and update the state of the button object
void Button::update()
{
#ifdef HELIOS_CLI
  // process any pre-input events in the queue
  bool processed_pre = processPreInput();
#endif

  bool newButtonState = check();
  m_newPress = false;
  m_newRelease = false;
  if (newButtonState != m_buttonState) {
    m_buttonState = newButtonState;
    m_isPressed = m_buttonState;
    if (m_isPressed) {
      m_pressTime = Time::getCurtime();
      m_newPress = true;
    } else {
      m_releaseTime = Time::getCurtime();
      m_newRelease = true;
      m_releaseCount++;
    }
  }
  if (m_isPressed) {
    m_holdDuration = (Time::getCurtime() >= m_pressTime) ? (uint32_t)(Time::getCurtime() - m_pressTime) : 0;
  } else {
    m_releaseDuration = (Time::getCurtime() >= m_releaseTime) ? (uint32_t)(Time::getCurtime() - m_releaseTime) : 0;
  }
  m_shortClick = (m_newRelease && (m_holdDuration <= SHORT_CLICK_THRESHOLD));
  m_longClick = (m_newRelease && (m_holdDuration > SHORT_CLICK_THRESHOLD) && (m_holdDuration < HOLD_CLICK_START));
  m_holdClick = (m_newRelease && (m_holdDuration >= HOLD_CLICK_START) && (m_holdDuration <= HOLD_CLICK_END));

#ifdef HELIOS_CLI
  // if there was no pre-input event this tick, process a post input event
  // to ensure there is only one event per tick processed
  if (!processed_pre) {
    processPostInput();
  }

  if (m_enableWake) {
    if (m_isPressed || m_shortClick || m_longClick) {
      Helios::wakeup();
    }
  }
#endif
}

#ifdef HELIOS_CLI
bool Button::processPreInput()
{
  if (!m_inputQueue.size()) {
    return false;
  }
  char command = m_inputQueue.front();
  switch (command) {
  case 'p': // press
    Button::doPress();
    break;
  case 'r': // release
    Button::doRelease();
    break;
  case 't': // toggle
    Button::doToggle();
    break;
  case 'q': // quit
    Helios::terminate();
    break;
  case 'w': // wait
    // wait is pre input I guess
    break;
  default:
    // return here! do not pop the queue
    // do not process post input events
    return false;
  }
  // now pop whatever pre-input command was processed
  m_inputQueue.pop();
  return true;
}

bool Button::processPostInput()
{
  if (!m_inputQueue.size()) {
    // probably processed the pre-input event already
    return false;
  }
  // process input queue from the command line
  char command = m_inputQueue.front();
  switch (command) {
  case 'c': // click button
    Button::doShortClick();
    break;
  case 'l': // long click button
    Button::doLongClick();
    break;
  default:
    // should never happen
    return false;
  }
  m_inputQueue.pop();
  return true;
}

void Button::doShortClick()
{
  m_newRelease = true;
  m_shortClick = true;
  m_pressTime = Time::getCurtime();
  m_holdDuration = SHORT_CLICK_THRESHOLD - 1;
  m_releaseCount++;
}

void Button::doLongClick()
{
  m_newRelease = true;
  m_longClick = true;
  m_pressTime = Time::getCurtime();
  m_holdDuration = SHORT_CLICK_THRESHOLD + 1;
  m_releaseCount++;
}

void Button::doHoldClick()
{
  m_newRelease = true;
  m_holdClick = true;
  m_pressTime = Time::getCurtime();
  m_holdDuration = HOLD_CLICK_START + 1;
  m_releaseCount++;
}

// this will actually press down the button, it's your responsibility to wait
// for the appropriate number of ticks and then release the button
void Button::doPress()
{
  m_pinState = true;
}

void Button::doRelease()
{
  m_pinState = false;
}

void Button::doToggle()
{
  m_pinState = !m_pinState;
}

// queue up an input event for the button
void Button::queueInput(char input)
{
  m_inputQueue.push(input);
}

uint32_t Button::inputQueueSize()
{
  return m_inputQueue.size();
}
#endif

// global button
Button button;
