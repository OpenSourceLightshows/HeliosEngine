#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

#ifdef HELIOS_CLI
#include <queue>
#endif

class Helios;

class Button
{
public:
  explicit Button(Helios &helios);
  // initialize a new button object with a pin number
  bool init();
  // directly poll the pin for whether it's pressed right now
  bool check();
  // poll the button pin and update the state of the button object
  void update();

  // whether the button was pressed this tick
  bool onPress() const { return m_newPress; }
  // whether the button was released this tick
  bool onRelease() const { return m_newRelease; }
  // whether the button is currently pressed
  bool isPressed() const { return m_isPressed; }

  // whether the button was shortclicked this tick
  bool onShortClick() const { return m_shortClick; }
  // whether the button was long clicked this tick
  bool onLongClick() const { return m_longClick; }
  // whether the button was hold clicked this tick
  bool onHoldClick() const { return m_holdClick; }

  // detect if the button is being held past long click
  bool holdPressing();

  // when the button was last pressed
  uint32_t pressTime() const { return m_pressTime; }
  // when the button was last released
  uint32_t releaseTime() const { return m_releaseTime; }

  // how long the button is currently or was last held down (in ticks)
  uint32_t holdDuration() const { return m_holdDuration; }
  // how long the button is currently or was last released for (in ticks)
  uint32_t releaseDuration() const { return m_releaseDuration; }

  // the number of releases
  uint8_t releaseCount() const { return m_releaseCount; }

  // enable wake on press
  void enableWake();

#ifdef HELIOS_CLI
  // these will 'inject' a short/long click without actually touching the
  // button state, it's important that code uses 'onShortClick' or
  // 'onLongClick' to capture this injected input event. Code that uses
  // for example: 'button.holdDuration() >= threshold && button.onRelease()'
  // will never trigger because the injected input event doesn't actually
  // press the button or change the button state it just sets the 'shortClick'
  // or 'longClick' values accordingly
  void doShortClick();
  void doLongClick();
  void doHoldClick();

  // this will actually press down the button, it's your responsibility to wait
  // for the appropriate number of ticks and then release the button
  void doPress();
  void doRelease();
  void doToggle();

  // queue up an input event for the button
  void queueInput(char input);
  uint32_t inputQueueSize() const;
#endif

private:
  // ========================================
  // state data that is populated each check

  // the timestamp of when the button was pressed
  uint32_t m_pressTime;
  // the timestamp of when the button was released
  uint32_t m_releaseTime;

  // the last hold duration
  uint32_t m_holdDuration;
  // the last release duration
  uint32_t m_releaseDuration;

  // the number of times released, will overflow at 255
  uint8_t m_releaseCount;

  // the active state of the button
  bool m_buttonState;

  // whether pressed this tick
  bool m_newPress;
  // whether released this tick
  bool m_newRelease;
  // whether currently pressed
  bool m_isPressed;
  // whether a short click occurred
  bool m_shortClick;
  // whether a long click occurred
  bool m_longClick;
  // whether a long hold occurred
  bool m_holdClick;

#ifdef HELIOS_CLI
  // process pre or post input events from the queue
  bool processPreInput();
  bool processPostInput();

  // an input queue for the button, each tick one even is processed
  // out of this queue and used to produce input
  std::queue<char> m_inputQueue;
  // the virtual pin state that is polled instead of a digital pin
  bool m_pinState;
  // whether the button is waiting to wake the device
  bool m_enableWake;
#endif
  Helios &m_helios;
};

#endif // BUTTON_H
