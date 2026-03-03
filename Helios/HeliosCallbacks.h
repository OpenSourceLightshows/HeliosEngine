#pragma once

#include <inttypes.h>

#include "Colortypes.h"

class HeliosCallbacks
{
public:
  virtual ~HeliosCallbacks() {}

  // Input hooks
  virtual bool checkPinHook(bool defaultState) { return defaultState; }

  // Output hooks
  virtual void ledsInit(const RGBColor &initialColor, int count) { (void)initialColor; (void)count; }
  virtual void ledsShow(const RGBColor &color, uint8_t brightness) { (void)color; (void)brightness; }
  virtual void ledsBrightness(uint8_t brightness) { (void)brightness; }

  // Optional storage hooks, return true if handled by callback
  virtual bool storageRead(uint8_t address, uint8_t &outValue)
  {
    (void)address;
    (void)outValue;
    return false;
  }

  virtual bool storageWrite(uint8_t address, uint8_t value)
  {
    (void)address;
    (void)value;
    return false;
  }
};
