#include "HeliosInstance.h"
#include "Led.h"
#include "TimeControl.h"

HeliosInstance::HeliosInstance() : pat(), m_localTick(0)
{
}

HeliosInstance::~HeliosInstance()
{
}

bool HeliosInstance::init()
{
  m_localTick = 0;
  pat.init();
  return true;
}

void HeliosInstance::tick()
{
  m_localTick += 1;
  Time::setCurtime(m_localTick);
  pat.play();
  // Pattern updates internal state, color retrieved via getCurColor() using Led
}

RGBColor HeliosInstance::getCurColor()
{
  // Note: Led is static/shared across instances
  // For preview use-case where we read immediately after tick, this works
  return Led::get();
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
}
