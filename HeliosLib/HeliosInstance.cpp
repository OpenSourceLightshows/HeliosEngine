#include "HeliosInstance.h"

HeliosInstance::HeliosInstance() : pat()
{
}

HeliosInstance::~HeliosInstance()
{
}

void HeliosInstance::tick()
{
  pat.tick();
}

RGBColor HeliosInstance::getCurColor()
{
  return pat.getCurColor();
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
  pat.restart();
}
