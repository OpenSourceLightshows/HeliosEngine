#include "HeliosInstance.h"
#include "Led.h"
#include "TimeControl.h"

HeliosInstance::HeliosInstance() : pat(), m_localTick(0), m_lastColor(RGB_OFF)
{
}

HeliosInstance::~HeliosInstance()
{
}

bool HeliosInstance::init()
{
  m_localTick = 0;
  m_lastColor = RGB_OFF;
  pat.init();
  return true;
}

void HeliosInstance::tick()
{
  m_localTick += 1;
  Time::setCurtime(m_localTick);
  // Restore this instance's last color before play(). Pattern states like OFF/ON
  // may not write Led every tick, so this prevents cross-instance color bleed.
  Led::set(m_lastColor);
  pat.play();
  m_lastColor = Led::get();
  // Pattern updates internal state, color retrieved via getCurColor() using Led
}

RGBColor HeliosInstance::getCurColor()
{
  return m_lastColor;
}

void HeliosInstance::setColorset(Colorset &colorset)
{
  pat.setColorset(colorset);
}

void HeliosInstance::setArgs(PatternArgs &args)
{
  pat.setArgs(args);
}

void HeliosInstance::setMode(PatternArgs &args, Colorset &colorset)
{
  pat.setArgs(args);
  pat.setColorset(colorset);
  pat.init();
  m_lastColor = RGB_OFF;
}
