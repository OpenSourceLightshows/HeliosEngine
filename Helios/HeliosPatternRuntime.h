#pragma once

#include "Pattern.h"

class HeliosPatternRuntime : public Pattern
{
public:
  HeliosPatternRuntime() : Pattern(), m_localTick(0), m_curColor() { restart(); }

  void advanceTick() { m_localTick += 1; }

  void tick()
  {
    advanceTick();
    Pattern::play();
  }

  RGBColor getCurColor() const { return m_curColor; }

  void restart()
  {
    m_localTick = 0;
    m_curColor.clear();
    Pattern::init();
  }

protected:
  uint32_t now() const override { return m_localTick; }
  void outputSet(const RGBColor &col) override { m_curColor = col; }
  void outputClear() override { m_curColor.clear(); }

private:
  uint32_t m_localTick;
  RGBColor m_curColor;
};
