#include "HeliosLib.h"
#include "Patterns.h"
#include "Random.h"

#ifdef WASM
#include <emscripten/bind.h>
#include <string>

using namespace emscripten;

static bool isFunction(const val &callback)
{
  if (callback.isNull() || callback.isUndefined()) {
    return false;
  }
  return callback.typeOf().as<std::string>() == "function";
}

class HeliosLibJsCallbacks : public HeliosCallbacks
{
public:
  HeliosLibJsCallbacks() :
    m_checkPinHook(val::undefined()),
    m_ledsInitHook(val::undefined()),
    m_ledsShowHook(val::undefined()),
    m_ledsBrightnessHook(val::undefined())
  {
  }

  void setCheckPinHook(val callback) { m_checkPinHook = callback; }
  void setLedsInitHook(val callback) { m_ledsInitHook = callback; }
  void setLedsShowHook(val callback) { m_ledsShowHook = callback; }
  void setLedsBrightnessHook(val callback) { m_ledsBrightnessHook = callback; }

  bool checkPinHook(bool defaultState) override
  {
    if (!isFunction(m_checkPinHook)) {
      return defaultState;
    }
    return m_checkPinHook(defaultState).as<bool>();
  }

  void ledsInit(const RGBColor &initialColor, int count) override
  {
    if (!isFunction(m_ledsInitHook)) {
      return;
    }
    m_ledsInitHook(initialColor.red, initialColor.green, initialColor.blue, count);
  }

  void ledsShow(const RGBColor &color, uint8_t brightness) override
  {
    if (!isFunction(m_ledsShowHook)) {
      return;
    }
    m_ledsShowHook(color.red, color.green, color.blue, brightness);
  }

  void ledsBrightness(uint8_t brightness) override
  {
    if (!isFunction(m_ledsBrightnessHook)) {
      return;
    }
    m_ledsBrightnessHook(brightness);
  }

private:
  val m_checkPinHook;
  val m_ledsInitHook;
  val m_ledsShowHook;
  val m_ledsBrightnessHook;
};

// js is dumb and has issues doing this cast I guess
PatternID intToPatternID(int val)
{
  return (PatternID)val;
}

EMSCRIPTEN_BINDINGS(Vortex) {
  // Bind the HSVColor class
  class_<HSVColor>("HSVColor")
    .constructor<>()
    .constructor<uint8_t, uint8_t, uint8_t>()
    .constructor<uint32_t>()
    .function("empty", &HSVColor::empty)
    .function("clear", &HSVColor::clear)
    .function("raw", &HSVColor::raw)
    .property("hue", &HSVColor::hue)
    .property("sat", &HSVColor::sat)
    .property("val", &HSVColor::val);

  // Bind the RGBColor class
  class_<RGBColor>("RGBColor")
    .constructor<>()
    .constructor<uint8_t, uint8_t, uint8_t>()
    .constructor<uint32_t>()
    .function("empty", &RGBColor::empty)
    .function("clear", &RGBColor::clear)
    .function("adjustBrightness", &RGBColor::adjustBrightness)
    .function("raw", &RGBColor::raw)
    .property("red", &RGBColor::red)
    .property("green", &RGBColor::green)
    .property("blue", &RGBColor::blue);

  // pattern id constants
  enum_<PatternID>("PatternID")
    .value("PATTERN_NONE", PatternID::PATTERN_NONE)
    // Strobe
    .value("PATTERN_RIBBON", PatternID::PATTERN_RIBBON)
    .value("PATTERN_ULTRA_DOPS", PatternID::PATTERN_ULTRA_DOPS)
    .value("PATTERN_DOPS", PatternID::PATTERN_DOPS)
    .value("PATTERN_STROBE", PatternID::PATTERN_STROBE)
    .value("PATTERN_HYPNOSTROBE", PatternID::PATTERN_HYPNOSTROBE)
    .value("PATTERN_STROBIE", PatternID::PATTERN_STROBIE)
    .value("PATTERN_RAZOR", PatternID::PATTERN_RAZOR)
    .value("PATTERN_FLARE", PatternID::PATTERN_FLARE)
    .value("PATTERN_BURST", PatternID::PATTERN_BURST)
    .value("PATTERN_GLOW", PatternID::PATTERN_GLOW)
    .value("PATTERN_FLICKER", PatternID::PATTERN_FLICKER)
    .value("PATTERN_FLASH", PatternID::PATTERN_FLASH)
    // Morph
    .value("PATTERN_MORPH", PatternID::PATTERN_MORPH)
    .value("PATTERN_MORPH_STROBE", PatternID::PATTERN_MORPH_STROBE)
    .value("PATTERN_MORPH_STROBIE", PatternID::PATTERN_MORPH_STROBIE)
    .value("PATTERN_MORPH_GLOW", PatternID::PATTERN_MORPH_GLOW)
    // Dash
    .value("PATTERN_DASH_DOPS", PatternID::PATTERN_DASH_DOPS)
    .value("PATTERN_DASH_DOT", PatternID::PATTERN_DASH_DOT)
    .value("PATTERN_WAVE_PARTICLE", PatternID::PATTERN_WAVE_PARTICLE)
    .value("PATTERN_LIGHTSPEED", PatternID::PATTERN_LIGHTSPEED)
    // Fade
    .value("PATTERN_FADE", PatternID::PATTERN_FADE)
    .value("PATTERN_MORPH_FADE", PatternID::PATTERN_MORPH_FADE)
    .value("PATTERN_GLITCH_FADE", PatternID::PATTERN_GLITCH_FADE);

  // colorset class
  class_<Colorset>("Colorset")
    .constructor<>()
    .constructor<RGBColor, RGBColor, RGBColor, RGBColor, RGBColor, RGBColor, RGBColor, RGBColor>()
    .constructor<uint8_t, const uint32_t *>()
    .function("init", &Colorset::init)
    .function("clear", &Colorset::clear)
    .function("equals", select_overload<bool(const Colorset & set) const>(&Colorset::equals))
    .function("get", &Colorset::get)
    .function("set", &Colorset::set)
    .function("skip", &Colorset::skip)
    .function("cur", &Colorset::cur)
    .function("setCurIndex", &Colorset::setCurIndex)
    .function("resetIndex", &Colorset::resetIndex)
    .function("curIndex", &Colorset::curIndex)
    .function("getPrev", &Colorset::getPrev)
    .function("getNext", &Colorset::getNext)
    .function("peek", &Colorset::peek)
    .function("peekNext", &Colorset::peekNext)
    .function("numColors", &Colorset::numColors)
    .function("onStart", &Colorset::onStart)
    .function("onEnd", &Colorset::onEnd)
    .function("addColor", select_overload<bool(RGBColor)>(&Colorset::addColor))
    .function("addColorHSV", &Colorset::addColorHSV)
    .function("adjustBrightness", &Colorset::adjustBrightness)
    .function("removeColor", &Colorset::removeColor)
    .function("operator[]", &Colorset::operator[]);

  // pattern args class
  class_<PatternArgs>("PatternArgs")
    .constructor<>()
    .constructor<uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t>()
    .property("on_dur", &PatternArgs::on_dur)
    .property("off_dur", &PatternArgs::off_dur)
    .property("gap_dur", &PatternArgs::gap_dur)
    .property("dash_dur", &PatternArgs::dash_dur)
    .property("group_size", &PatternArgs::group_size)
    .property("blend_speed", &PatternArgs::blend_speed)
    .property("fade_dur", &PatternArgs::fade_dur);

  class_<HeliosLib>("HeliosLib")
    .constructor<>()
    .function("init", &HeliosLib::init)
    .function("tick", &HeliosLib::tick)
    .function("cleanup", &HeliosLib::cleanup)
    .function("getCurColor", &HeliosLib::getCurColor)
    .function("setColorset", &HeliosLib::setColorset)
    .function("setArgs", &HeliosLib::setArgs)
    .function("setMode", &HeliosLib::setMode)
    .function("randomizeSeeded", &HeliosLib::randomizeSeeded)
    .function("getArgs", &HeliosLib::getArgs)
    .function("getNumColors", &HeliosLib::getNumColors)
    .function("getColorAt", &HeliosLib::getColorAt)
    .function("setCheckPinHook", &HeliosLib::setCheckPinHook)
    .function("setLedsInitHook", &HeliosLib::setLedsInitHook)
    .function("setLedsShowHook", &HeliosLib::setLedsShowHook)
    .function("setLedsBrightnessHook", &HeliosLib::setLedsBrightnessHook);

  // bind others as necessary
}
#endif

// Helios Lib code

HeliosLib::HeliosLib() :
  m_helios(),
  m_preview(nullptr),
  m_callbacks(nullptr)
#ifdef WASM
  , m_jsCallbacks(new HeliosLibJsCallbacks()),
    m_ledsInitHook(val::undefined()),
    m_ledsShowHook(val::undefined()),
    m_ledsBrightnessHook(val::undefined())
#endif
{
  m_preview = new Pattern(m_helios);
#ifdef WASM
  setCallbacks(m_jsCallbacks);
#endif
}

HeliosLib::~HeliosLib()
{
  delete m_preview;
  m_preview = nullptr;
#ifdef WASM
  delete m_jsCallbacks;
  m_jsCallbacks = nullptr;
#endif
}

bool HeliosLib::init()
{
  // Disable real-time timestep for WASM: JS requestAnimationFrame controls timing,
  // so the busy-wait loop in tickClock() would block the browser thread.
  m_helios.time().enableTimestep(false);
  m_preview->init();
  return true;
}

void HeliosLib::cleanup()
{
}

void HeliosLib::tick()
{
  m_helios.time().tickClock();
  m_preview->play();
#ifdef WASM
  if (isFunction(m_ledsShowHook)) {
    RGBColor col = m_helios.led().get();
    m_ledsShowHook(col.red, col.green, col.blue, 255);
  }
#endif
}

RGBColor HeliosLib::getCurColor()
{
  return m_helios.led().get();
}

void HeliosLib::setColorset(Colorset &colorset)
{
  m_preview->setColorset(colorset);
}

void HeliosLib::setArgs(PatternArgs &args)
{
  m_preview->setArgs(args);
  m_preview->init();
}

void HeliosLib::setMode(PatternArgs &args, Colorset &colorset)
{
  m_preview->setArgs(args);
  m_preview->setColorset(colorset);
  m_preview->init();
}

int HeliosLib::randomizeSeeded(uint8_t maxColors)
{
  Random ctx(m_preview->crc32());
  uint8_t randVal = ctx.next8();

  uint8_t requestedMaxColors = maxColors > 0 ? maxColors : 1;
  if (requestedMaxColors > NUM_COLOR_SLOTS) {
    requestedMaxColors = NUM_COLOR_SLOTS;
  }
  uint8_t requestedColors = (uint8_t)((randVal % requestedMaxColors) + 1);

  m_preview->colorset().randomizeColors(ctx, requestedColors, Colorset::COLOR_MODE_RANDOMLY_PICK);
  while (m_preview->colorset().numColors() > requestedColors) {
    m_preview->colorset().removeColor((uint8_t)(m_preview->colorset().numColors() - 1));
  }

  int patternIndex = (int)(randVal % PATTERN_COUNT);
  Patterns::make_pattern((PatternID)patternIndex, *m_preview);
  m_preview->init();
  return patternIndex;
}

PatternArgs HeliosLib::getArgs()
{
  return m_preview->getArgs();
}

int HeliosLib::getNumColors()
{
  return m_preview->colorset().numColors();
}

RGBColor HeliosLib::getColorAt(int index)
{
  if (index < 0) {
    return RGBColor();
  }
  return m_preview->colorset().get((uint8_t)index);
}

void HeliosLib::setCallbacks(HeliosCallbacks *callbacks)
{
  m_callbacks = callbacks;
  m_helios.setCallbacks(callbacks);
}

#ifdef WASM
void HeliosLib::setCheckPinHook(emscripten::val callback)
{
  if (!m_jsCallbacks) {
    return;
  }
  m_jsCallbacks->setCheckPinHook(callback);
  setCallbacks(m_jsCallbacks);
}

void HeliosLib::setLedsInitHook(emscripten::val callback)
{
  m_ledsInitHook = callback;
  if (isFunction(m_ledsInitHook)) {
    RGBColor col = m_helios.led().get();
    m_ledsInitHook(col.red, col.green, col.blue, 1);
  }
  if (!m_jsCallbacks) {
    return;
  }
  m_jsCallbacks->setLedsInitHook(callback);
  setCallbacks(m_jsCallbacks);
}

void HeliosLib::setLedsShowHook(emscripten::val callback)
{
  m_ledsShowHook = callback;
  if (!m_jsCallbacks) {
    return;
  }
  m_jsCallbacks->setLedsShowHook(callback);
  setCallbacks(m_jsCallbacks);
}

void HeliosLib::setLedsBrightnessHook(emscripten::val callback)
{
  m_ledsBrightnessHook = callback;
  if (isFunction(m_ledsBrightnessHook)) {
    m_ledsBrightnessHook(255);
  }
  if (!m_jsCallbacks) {
    return;
  }
  m_jsCallbacks->setLedsBrightnessHook(callback);
  setCallbacks(m_jsCallbacks);
}
#endif
