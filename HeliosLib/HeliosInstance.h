#pragma once

#include "Colorset.h"
#include "HeliosPatternInstance.h"

class HeliosInstance
{
public:
  HeliosInstance();
  ~HeliosInstance();
  void tick();
  RGBColor getCurColor();
  void setColorset(Colorset &colorset);
  void setArgs(PatternArgs &args);
  void setMode(PatternArgs &args, Colorset &colorset);

private:
  HeliosPatternInstance pat;
};
