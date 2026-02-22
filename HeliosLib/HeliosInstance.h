#pragma once

#include "Colorset.h"
#include "HeliosPatternInstance.h"

#ifdef WASM
#include <emscripten/val.h>
#endif

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
  int randomizeSeeded(uint8_t maxColors);
  PatternArgs getArgs();
  int getNumColors();
  RGBColor getColorAt(int index);
#ifdef WASM
  void setLedsInitHook(emscripten::val callback);
  void setLedsShowHook(emscripten::val callback);
  void setLedsBrightnessHook(emscripten::val callback);
#endif

private:
  HeliosPatternInstance pat;
#ifdef WASM
  emscripten::val m_ledsInitHook;
  emscripten::val m_ledsShowHook;
  emscripten::val m_ledsBrightnessHook;
#endif
};
