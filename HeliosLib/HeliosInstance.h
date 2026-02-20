#pragma once

#include "Helios.h"
#include "Pattern.h"
#include "Colorset.h"

#ifdef WASM
#include <emscripten/bind.h>
using namespace emscripten;
#endif

/**
 * HeliosInstance - An independent instance of the Helios engine.
 * 
 * Unlike the static Helios class which has global state, HeliosInstance
 * allows multiple independent engines to coexist. Each instance has its
 * own pattern, colorset, and state.
 * 
 * This enables multiple mode previews on the same page (like a grid of
 * mode cards) without interference.
 */
class HeliosInstance
{
public:
  HeliosInstance();
  ~HeliosInstance();

  // Initialize this instance
  bool init();

  // Run one tick of the engine
  void tick();

  // Get the current LED color after tick
  RGBColor getCurColor() const;

  // Access the current pattern for this instance
  Pattern &cur_pattern() { return pat; }

  // Set the colorset on the current pattern
  void setColorset(Colorset &colorset);

  // Set pattern args on the current pattern
  void setArgs(PatternArgs &args);

  // Fully configure and reinitialize the current pattern
  void setMode(PatternArgs &args, Colorset &colorset);

private:
  Pattern pat;
  // Add other instance-specific state here as needed
};

#ifdef WASM
// EMSCRIPTEN_BINDINGS for HeliosInstance
inline void bindHeliosInstance()
{
  class_<HeliosInstance>("HeliosInstance")
    .constructor<>()
    .function("init", &HeliosInstance::init)
    .function("tick", &HeliosInstance::tick)
    .function("getCurColor", &HeliosInstance::getCurColor)
    .function("cur_pattern", &HeliosInstance::cur_pattern, return_value_policy::reference())
    .function("setColorset", &HeliosInstance::setColorset)
    .function("setArgs", &HeliosInstance::setArgs)
    .function("setMode", &HeliosInstance::setMode);
}
#endif
