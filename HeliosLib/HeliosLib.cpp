#include "HeliosLib.h"
#include "HeliosInstance.h"

#ifdef WASM
#include <emscripten/bind.h>
#include <string>

using namespace emscripten;

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

  bool checkPinHook(uint32_t pin, bool defaultState) override
  {
    if (!isFunction(m_checkPinHook)) {
      return defaultState;
    }
    return m_checkPinHook(pin, defaultState).as<bool>();
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
  static bool isFunction(const val &callback)
  {
    if (callback.isNull() || callback.isUndefined()) {
      return false;
    }
    return callback.typeOf().as<std::string>() == "function";
  }

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
    .value("PATTERN_LIGHTSPEED", PatternID::PATTERN_LIGHTSPEED);

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
    .constructor<uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t>()
    .property("on_dur", &PatternArgs::on_dur)
    .property("off_dur", &PatternArgs::off_dur)
    .property("gap_dur", &PatternArgs::gap_dur)
    .property("dash_dur", &PatternArgs::dash_dur)
    .property("group_size", &PatternArgs::group_size)
    .property("blend_speed", &PatternArgs::blend_speed);

  // HeliosInstance class for independent engine instances
  // This allows multiple mode previews on the same page
  class_<HeliosInstance>("HeliosInstance")
    .constructor<>()
    .function("tick", &HeliosInstance::tick)
    .function("getCurColor", &HeliosInstance::getCurColor)
    .function("setColorset", &HeliosInstance::setColorset)
    .function("setArgs", &HeliosInstance::setArgs)
    .function("setMode", &HeliosInstance::setMode)
    .function("randomizeSeeded", &HeliosInstance::randomizeSeeded)
    .function("getArgs", &HeliosInstance::getArgs)
    .function("getNumColors", &HeliosInstance::getNumColors)
    .function("getColorAt", &HeliosInstance::getColorAt)
    .function("setLedsInitHook", &HeliosInstance::setLedsInitHook)
    .function("setLedsShowHook", &HeliosInstance::setLedsShowHook)
    .function("setLedsBrightnessHook", &HeliosInstance::setLedsBrightnessHook);

  class_<HeliosLib>("HeliosLib")
    .constructor<>()
    .function("init", &HeliosLib::init)
    .function("tick", &HeliosLib::tick)
    .function("cleanup", &HeliosLib::cleanup)
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
  m_callbacks(nullptr)
#ifdef WASM
  , m_jsCallbacks(new HeliosLibJsCallbacks())
#endif
{
#ifdef WASM
  setCallbacks(m_jsCallbacks);
#endif
}

HeliosLib::~HeliosLib()
{
#ifdef WASM
  delete m_jsCallbacks;
  m_jsCallbacks = nullptr;
#endif
}

bool HeliosLib::init()
{
  return m_helios.init();
}

void HeliosLib::cleanup()
{
}

void HeliosLib::tick()
{
  m_helios.tick();
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
  if (!m_jsCallbacks) {
    return;
  }
  m_jsCallbacks->setLedsInitHook(callback);
  setCallbacks(m_jsCallbacks);
}

void HeliosLib::setLedsShowHook(emscripten::val callback)
{
  if (!m_jsCallbacks) {
    return;
  }
  m_jsCallbacks->setLedsShowHook(callback);
  setCallbacks(m_jsCallbacks);
}

void HeliosLib::setLedsBrightnessHook(emscripten::val callback)
{
  if (!m_jsCallbacks) {
    return;
  }
  m_jsCallbacks->setLedsBrightnessHook(callback);
  setCallbacks(m_jsCallbacks);
}
#endif
