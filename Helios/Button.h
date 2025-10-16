#include <stdint.h>

#ifdef HELIOS_CLI
#include <queue>
#endif

class Button
{

  // initialize a new button object with a pin number
  static bool init();
  // directly poll the pin for whether it's pressed right now
  static bool check();
  // poll the button pin and update the state of the button object
  static void update();

  // whether the button was pressed this tick
  static bool onPress() { return m_newPress; }
  // whether the button was released this tick
  static bool onRelease() { return m_newRelease; }
  // whether the button is currently pressed
  static bool isPressed() { return m_isPressed; }

  // whether the button was shortclicked this tick
  static bool onShortClick() { return m_shortClick; }
  // whether the button was long clicked this tick
  static bool onLongClick() { return m_longClick; }
  // whether the button was hold clicked this tick
  static bool onHoldClick() { return m_holdClick; }

  // detect if the button is being held past long click
  static bool holdPressing();

  // when the button was last pressed
  static uint32_t pressTime() { return m_pressTime; }
  // when the button was last released
  static uint32_t releaseTime() { return m_releaseTime; }

  // how long the button is currently or was last held down (in ticks)
  static uint32_t holdDuration() { return m_holdDuration; }
  // how long the button is currently or was last released for (in ticks)
  static uint32_t releaseDuration() { return m_releaseDuration; }

  // the number of releases
  static uint8_t releaseCount() { return m_releaseCount; }

  // enable wake on press
  static void enableWake();

#ifdef HELIOS_CLI
  // these will 'inject' a short/long click without actually touching the
  // button state, it's important that code uses 'onShortClick' or
  // 'onLongClick' to capture this injected input event. Code that uses
  // for example: 'button.holdDuration() >= threshold && button.onRelease()'
  // will never trigger because the injected input event doesn't actually
  // press the button or change the button state it just sets the 'shortClick'
  // or 'longClick' values accordingly
  static void doShortClick();
  static void doLongClick();
  static void doHoldClick();

  // this will actually press down the button, it's your responsibility to wait
  // for the appropriate number of ticks and then release the button
  static void doPress();
  static void doRelease();
  static void doToggle();

  // queue up an input event for the button
  static void queueInput(char input);
  static uint32_t inputQueueSize();
#endif


  // ========================================
  // state data that is populated each check

  // the timestamp of when the button was pressed
  static uint32_t m_pressTime;
  // the timestamp of when the button was released
  static uint32_t m_releaseTime;

  // the last hold duration
  static uint32_t m_holdDuration;
  // the last release duration
  static uint32_t m_releaseDuration;

  // the number of times released, will overflow at 255
  static uint8_t m_releaseCount;

  // the active state of the button
  static bool m_buttonState;

  // whether pressed this tick
  static bool m_newPress;
  // whether released this tick
  static bool m_newRelease;
  // whether currently pressed
  static bool m_isPressed;
  // whether a short click occurred
  static bool m_shortClick;
  // whether a long click occurred
  static bool m_longClick;
  // whether a long hold occurred
  static bool m_holdClick;

#ifdef HELIOS_CLI
  // process pre or post input events from the queue
  static bool processPreInput();
  static bool processPostInput();

  // an input queue for the button, each tick one even is processed
  // out of this queue and used to produce input
  static std::queue<char> m_inputQueue;
  // the virtual pin state that is polled instead of a digital pin
  static bool m_pinState;
  // whether the button is waiting to wake the device
  static bool m_enableWake;
#endif
};
