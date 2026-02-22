#include "HeliosInstance.h"
#include "Patterns.h"
#include "Random.h"

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

int HeliosInstance::randomizeSeeded(uint8_t maxColors)
{
  Random ctx(pat.crc32());
  uint8_t randVal = ctx.next8();

  uint8_t safeMaxColors = maxColors > 0 ? maxColors : 8;
  uint8_t numColors = (uint8_t)((randVal + 1) % safeMaxColors);

  pat.colorset().randomizeColors(ctx, numColors, Colorset::COLOR_MODE_RANDOMLY_PICK);

  int patternIndex = (int)(randVal % PATTERN_COUNT);
  Patterns::make_pattern((PatternID)patternIndex, pat);
  pat.restart();
  return patternIndex;
}

PatternArgs HeliosInstance::getArgs()
{
  return pat.getArgs();
}

int HeliosInstance::getNumColors()
{
  return pat.colorset().numColors();
}

RGBColor HeliosInstance::getColorAt(int index)
{
  if (index < 0) {
    return RGBColor();
  }
  return pat.colorset().get((uint8_t)index);
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
