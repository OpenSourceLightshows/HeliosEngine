#pragma once

#include "Helios.h"
#include "HeliosCallbacks.h"
#include "HeliosPatternInstance.h"

#ifdef WASM
#include <emscripten/val.h>
class HeliosLibJsCallbacks;
#endif

class HeliosLib
{
public:
  HeliosLib();
  ~HeliosLib();

  bool init();
  void cleanup();
  void tick();
  RGBColor getCurColor();
  void setColorset(Colorset &colorset);
  void setArgs(PatternArgs &args);
  void setMode(PatternArgs &args, Colorset &colorset);
  int randomizeSeeded(uint8_t maxColors);
  PatternArgs getArgs();
  int getNumColors();
  RGBColor getColorAt(int index);

  Helios &helios() { return m_helios; }
  const Helios &helios() const { return m_helios; }
  void setCallbacks(HeliosCallbacks *callbacks);
  HeliosCallbacks *callbacks() const { return m_callbacks; }

#ifdef WASM
  void setCheckPinHook(emscripten::val callback);
  void setLedsInitHook(emscripten::val callback);
  void setLedsShowHook(emscripten::val callback);
  void setLedsBrightnessHook(emscripten::val callback);
#endif

private:
  Helios m_helios;
  HeliosPatternInstance m_preview;
  HeliosCallbacks *m_callbacks;
#ifdef WASM
  HeliosLibJsCallbacks *m_jsCallbacks;
  emscripten::val m_ledsInitHook;
  emscripten::val m_ledsShowHook;
  emscripten::val m_ledsBrightnessHook;
#endif
};
