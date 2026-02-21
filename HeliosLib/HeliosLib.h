#pragma once

#include "Helios.h"
#include "HeliosCallbacks.h"

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
  HeliosCallbacks *m_callbacks;
#ifdef WASM
  HeliosLibJsCallbacks *m_jsCallbacks;
#endif
};
