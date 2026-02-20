#pragma once

#include "Pattern.h"
#include "Colorset.h"

class HeliosInstance
{
public:
  HeliosInstance();
  ~HeliosInstance();

  static HeliosInstance* create() { return new HeliosInstance(); }

  bool init();
  void tick();
  RGBColor getCurColor();
  void setColorset(Colorset &colorset);
  void setArgs(PatternArgs &args);
  void setMode(PatternArgs &args, Colorset &colorset);

private:
  Pattern pat;
  uint32_t m_localTick;
  RGBColor m_lastColor;
};
