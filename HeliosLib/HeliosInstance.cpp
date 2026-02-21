#include "HeliosInstance.h"

#ifdef WASM
#include <string>

static bool isFunction(const emscripten::val &cb)
{
  if (cb.isNull() || cb.isUndefined()) {
    return false;
  }
  return cb.typeOf().as<std::string>() == "function";
}
#endif

HeliosInstance::HeliosInstance() : pat()
#ifdef WASM
  , m_ledsInitHook(emscripten::val::undefined())
  , m_ledsShowHook(emscripten::val::undefined())
  , m_ledsBrightnessHook(emscripten::val::undefined())
#endif
{
}

HeliosInstance::~HeliosInstance()
{
}

void HeliosInstance::tick()
{
  pat.tick();
#ifdef WASM
  if (isFunction(m_ledsShowHook)) {
    RGBColor col = pat.getCurColor();
    m_ledsShowHook(col.red, col.green, col.blue, 255);
  }
#endif
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

#ifdef WASM
void HeliosInstance::setLedsInitHook(emscripten::val callback)
{
  m_ledsInitHook = callback;
  if (isFunction(m_ledsInitHook)) {
    RGBColor col = pat.getCurColor();
    m_ledsInitHook(col.red, col.green, col.blue, 1);
  }
}

void HeliosInstance::setLedsShowHook(emscripten::val callback)
{
  m_ledsShowHook = callback;
}

void HeliosInstance::setLedsBrightnessHook(emscripten::val callback)
{
  m_ledsBrightnessHook = callback;
  if (isFunction(m_ledsBrightnessHook)) {
    m_ledsBrightnessHook(255);
  }
}
#endif
