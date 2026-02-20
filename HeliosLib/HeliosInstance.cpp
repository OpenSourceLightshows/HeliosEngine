#include "HeliosInstance.h"
#include "Led.h"

HeliosInstance::HeliosInstance() : pat()
{
}

HeliosInstance::~HeliosInstance()
{
}

bool HeliosInstance::init()
{
  // Initialize the pattern to a default state
  pat.init();
  return true;
}

void HeliosInstance::tick()
{
  // Play the pattern - this updates internal state
  pat.play();

  // Get the current color from the pattern and set the LED
  RGBColor col = pat.cur_color();
  Led::set(col);
}

RGBColor HeliosInstance::getCurColor() const
{
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
