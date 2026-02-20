#include "HeliosInstance.h"

HeliosInstance::HeliosInstance() : pat()
{
}

HeliosInstance::~HeliosInstance()
{
}

bool HeliosInstance::init()
{
  pat.init();
  return true;
}

void HeliosInstance::tick()
{
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
