#include "Pattern.h"

//#include "../Patterns/PatternBuilder.h"
#include "TimeControl.h"
#include "Colorset.h"

#include "HeliosConfig.h"
#include "Led.h"

#include <string.h> // for memcpy

Pattern::Pattern(uint8_t onDur, uint8_t offDur, uint8_t gap,
          uint8_t dash, uint8_t group, uint8_t blend, uint8_t fade) :
  m_args(onDur, offDur, gap, dash, group, blend, fade),
  m_patternFlags(0),
  m_colorset(),
  m_groupCounter(0),
  m_state(STATE_BLINK_ON),
  m_blinkTimer(),
  m_cur(),
  m_next(),
  m_fadeValue(0),
  m_fadeStartTime(0)
{
}

Pattern::Pattern(const PatternArgs &args) :
  Pattern(args.on_dur, args.off_dur, args.gap_dur,
      args.dash_dur, args.group_size, args.blend_speed, args.fade_dur)
{
}

Pattern::~Pattern()
{
}

void Pattern::init()
{
  m_colorset.resetIndex();

  // Reset the fade start time to the current time
  m_fadeStartTime = Time::getCurtime();

  // the default state to begin with
  m_state = STATE_BLINK_ON;
  // if a dash is present then always start with the dash because
  // it consumes the first color in the colorset
  if (m_args.dash_dur > 0) {
    m_state = STATE_BEGIN_DASH;
  }
  // if there's no on duration or dash duration the led is just disabled
  if ((!m_args.on_dur && !m_args.dash_dur) || !m_colorset.numColors()) {
    m_state = STATE_DISABLED;
  }
  m_groupCounter = m_args.group_size ? m_args.group_size : (m_colorset.numColors() - (m_args.dash_dur != 0));

  if (m_args.blend_speed > 0) {
    // convert current/next colors to HSV but only if we are doing a blend
    m_cur = m_colorset.getNext();
    m_next = m_colorset.getNext();
  } else if (m_args.fade_dur) {
    // if there is a fade dur and no blend need to iterate colorset
    m_colorset.getNext();
  }

  // Initialize the fluctuating fade value
  m_fadeValue = 0;
}

void Pattern::tickFade()
{
  uint32_t now = Time::getCurtime();
  // Calculate relative time since pattern was initialized
  uint32_t relativeTime = now - m_fadeStartTime;
  uint32_t duration = m_args.fade_dur * 10;

  // only tick forward every fade_dur ticks
  if (!relativeTime || (relativeTime % duration) != 0) {
    return;
  }

  // count the number of steps based on relative time
  uint32_t steps = relativeTime / duration;
  uint32_t range = m_args.off_dur;

  // make sure the range is non-zero
  if (range == 0) {
    m_fadeValue = 0;
    return;
  }

  uint32_t double_range = range * 2;
  uint32_t mod = steps % double_range;

  // Triangle wave: up from 0 to range, then down to 0
  m_fadeValue = (mod < range) ? mod : (double_range - mod - 1);

  // iterate color when at lowest point
  if (mod == 0) {
    m_colorset.getNext();
  }
}

void Pattern::play()
{
  // tick forward the fade logic each tick
  if (isFade()) {
    tickFade();
  }

  // Sometimes the pattern needs to cycle multiple states in a single frame so
  // instead of using a loop or recursion I have just used a simple goto
replay:

  // its kinda evolving as i go
  switch (m_state) {
  case STATE_DISABLED:
    return;
  case STATE_BLINK_ON:
    if (m_args.on_dur > 0) {
      onBlinkOn();
      --m_groupCounter;
      // When in ON state, use current fading on-time
      nextState(m_args.on_dur + m_fadeValue);
      return;
    }
    m_state = STATE_BLINK_OFF;
  case STATE_BLINK_OFF:
    // the whole 'should blink off' situation is tricky because we might need
    // to go back to blinking on if our colorset isn't at the end yet
    if (m_groupCounter > 0 || (!m_args.gap_dur && !m_args.dash_dur)) {
      if (m_args.off_dur > 0) {
        onBlinkOff();
        nextState(m_args.off_dur - m_fadeValue);
        return;
      }
      if (m_groupCounter > 0 && m_args.on_dur > 0) {
        m_state = STATE_BLINK_ON;
        goto replay;
      }
    }
    m_state = STATE_BEGIN_GAP;
  case STATE_BEGIN_GAP:
    m_groupCounter = m_args.group_size ? m_args.group_size : (m_colorset.numColors() - (m_args.dash_dur != 0));
    if (m_args.gap_dur > 0) {
      beginGap();
      nextState(m_args.gap_dur);
      return;
    }
    m_state = STATE_BEGIN_DASH;
  case STATE_BEGIN_DASH:
    if (m_args.dash_dur > 0) {
      beginDash();
      nextState(m_args.dash_dur);
      return;
    }
    m_state = STATE_BEGIN_GAP2;
  case STATE_BEGIN_GAP2:
    if (m_args.dash_dur > 0 && m_args.gap_dur > 0) {
      beginGap();
      nextState(m_args.gap_dur);
      return;
    }
    m_state = STATE_BLINK_ON;
    goto replay;
  default:
    break;
  }

  if (!m_blinkTimer.alarm()) {
    // no alarm triggered just stay in current state, return and don't transition states
    PRINT_STATE(m_state);
    return;
  }

  // this just transitions the state into the next state, with some edge conditions for
  // transitioning to different states under certain circumstances. Honestly this is
  // a nightmare to read now and idk how to fix it
  if (m_state == STATE_IN_GAP2 || (m_state == STATE_OFF && m_groupCounter > 0)) {
    // this is an edge condition for when in the second gap or off in the non-last off blink
    // then the state actually needs to jump backwards rather than iterate
    m_state = m_args.on_dur ? STATE_BLINK_ON : (m_args.dash_dur ? STATE_BEGIN_DASH : STATE_BEGIN_GAP);
  } else if (m_state == STATE_OFF && (!m_groupCounter || m_colorset.numColors() == 1)) {
    // this is an edge condition when the state is off but this is the last off blink in the
    // group or there's literally only one color in the group then if there is more blinks
    // left in the group we need to cycle back to blink on instead of to the next state
    m_state = (m_groupCounter > 0) ? STATE_BLINK_ON : STATE_BEGIN_GAP;
  } else {
    // this is the standard case, iterate to the next state
    m_state = (PatternState)(m_state + 1);
  }
  // poor-mans recurse with the new state change (this transitions to a new state within the same tick)
  goto replay;
}

// set args
void Pattern::setArgs(const PatternArgs &args)
{
  memcpy(&m_args, &args, sizeof(PatternArgs));
}

void Pattern::onBlinkOn()
{
  PRINT_STATE(STATE_ON);
  if (isBlend()) {
    blendBlinkOn();
    return;
  }

  // Check if this is a fading duration pattern
  if (isFade()) {
    // Just use the current color without advancing to the next one yet
    Led::set(m_colorset.cur());
    return;
  }

  Led::set(m_colorset.getNext());
}

void Pattern::onBlinkOff()
{
  PRINT_STATE(STATE_OFF);
  Led::clear();
}

void Pattern::beginGap()
{
  PRINT_STATE(STATE_IN_GAP);
  Led::clear();
}

void Pattern::beginDash()
{
  PRINT_STATE(STATE_IN_DASH);
  Led::set(m_colorset.getNext());
}

void Pattern::nextState(uint8_t timing)
{
  m_blinkTimer.init(timing);
  m_state = (PatternState)(m_state + 1);
}

// change the colorset
void Pattern::setColorset(const Colorset &set)
{
  m_colorset = set;
}

void Pattern::clearColorset()
{
  m_colorset.clear();
}

bool Pattern::equals(const Pattern *other)
{
  if (!other) {
    return false;
  }
  // compare the colorset
  if (!m_colorset.equals(&other->m_colorset)) {
    return false;
  }
  // compare the args of each pattern for equality
  if (memcmp(&m_args, &other->m_args, sizeof(PatternArgs)) == 0) {
    return false;
  }
  // if those match then it's effectively the same
  // pattern even if anything else is different
  return true;
}

void Pattern::updateColor(uint8_t index, const RGBColor &col)
{
  m_colorset.set(index, col);
  init();
}

uint32_t Pattern::crc32() const
{
  uint32_t hash = 5381;
  for (uint8_t i = 0; i < PATTERN_SIZE; ++i) {
    hash = ((hash << 5) + hash) + ((uint8_t *)this)[i];
  }
  return hash;
}

void Pattern::blendBlinkOn()
{
  // if we reached the next color, then cycle the colorset
  // like normal and begin playing the next color
  if (m_cur == m_next) {
    m_next = m_colorset.getNext();
  }
  // interpolate to the next color
  interpolate(m_cur.red, m_next.red);
  interpolate(m_cur.green, m_next.green);
  interpolate(m_cur.blue, m_next.blue);
  // set the color
  Led::set(m_cur);
}

void Pattern::interpolate(uint8_t &current, const uint8_t next)
{
  if (current < next) {
    uint8_t step = (next - current) > m_args.blend_speed ? m_args.blend_speed : (next - current);
    current += step;
  } else if (current > next) {
    uint8_t step = (current - next) > m_args.blend_speed ? m_args.blend_speed : (current - next);
    current -= step;
  }
}

// ==================================
//  Debug Code
#if DEBUG_BASIC_PATTERN == 1
#include <stdio.h>
void Pattern::printState(PatternState state)
{
  static uint32_t lastPrint = UINT32_MAX;
  if (lastPrint == Time::getCurtime()) {
    return;
  }
  switch (m_state) {
    case STATE_DISABLED:   printf("DIS "); break;
    case STATE_BLINK_ON:   printf("ON  "); break;
    case STATE_ON:         printf("on  "); break;
    case STATE_BLINK_OFF:  printf("OFF "); break;
    case STATE_OFF:        printf("off "); break;
    case STATE_BEGIN_GAP:  printf("GAP1"); break;
    case STATE_IN_GAP:     printf("gap1"); break;
    case STATE_BEGIN_DASH: printf("DASH"); break;
    case STATE_IN_DASH:    printf("dash"); break;
    case STATE_BEGIN_GAP2: printf("GAP2"); break;
    case STATE_IN_GAP2:    printf("gap2"); break;
    default:               printf("(%02u)", m_state); break;
  }
  lastPrint = Time::getCurtime();
}
#endif
