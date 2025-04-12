#include "Pattern.h"

//#include "../Patterns/PatternBuilder.h"
#include "TimeControl.h"
#include "Colorset.h"

#include "HeliosConfig.h"
#include "Led.h"

#include <string.h> // for memcpy

// Pattern Notation
//
// In order to describe how the inner workings of a pattern operate, a formal
// 'Pattern Notation' can be conceptualized. This notation is based off the
// well known ABNF notation used for RFCs to breakdown complex structures, but
// incorporates some extra aspects to convey the necessary details.
//
// To start, the ABNF notation for a Pattern would go something like this:
//
// A 'pattern' is a repetition of 'groups' possibly with 'separators' inbetween
// The pattern will start with the separator if one is present
//    
//    pattern         = [separator] group * ( [separator] group )
//
// A 'group' is N 'blinks' where N is the 'group size parameter' to the pattern
// If 'group size' is zero then the size of the colorset is used as 'groupsize'
//    
//    group           = groupsize * blink
//
// A 'blink' is simply an 'on' followed by an 'off'
//    
//    blink           = [on] [off]
//
// A 'separator' is like a 'blink', but with three operations: gap, dash, gap
//    
//    separator       = [gap] [dash] [gap]
//
// And finally the various basic words map to the parameters of the pattern
//    
//    on              = LED blinks ON for on_dur
//    off             = LED turns OFF for off_dur
//    dash            = LED turns on for dash_dur
//    gap             = LED turns off for gap_dur
//    groupsize       = number of 'blinks' per cycle
//
// ----------------------------------------------------------------------------
//
// In order to establish a 'Pattern Notation' from this base ABNF notation the
// colorset must be taken into account.
//
// The trick with the colorset is each 'on' or 'dash' will advance the colorset
// to the next color, wrapping around to the first color as it reaches the end.
//
// Consider the 'dash' to be the same as the 'on' and 'gap' is just 'off'.
//
// Then add the colorset, for the sake of pattern notation there is only eight
// possible colors that can be used (first letter is unique):
//
//    R - RED
//    G - GREEN
//    B - BLUE
//    O - ORANGE
//    Y - YELLOW
//    P - PURPLE
//    C - CYAN
//    W - WHITE
//
// Legend for 'Pattern Notation':
//
//   [C] = ON where C is the color, number of C's is the duration
//
//      ex: [RRRR] = RED for four ticks
//          [GG] = GREEN for two ticks
//
//   [ ] = OFF where number of spaces is the duration
//
//      ex: [ ] = OFF for one tick
//          [   ] = OFF for three ticks
//
// Examples using Pattern Notation:
//
// ----------------------------------------------------------------------------
// The group size in this example defaults to the number of colors (3),
//
//  Parameters: (ON:1 OFF:2 GAP:0 DASH:0 GROUP:0)
//  Colorset: (RED, GREEN, BLUE)
//
//   ┌────── Group 1 ──────┐┌────── Group 2 ──────┐┌────── Group 3 ──────┐...
//    [R][  ][G][  ][B][  ]  [R][  ][G][  ][B][  ]  [R][  ][G][  ][B][  ]
//
// ----------------------------------------------------------------------------
// This would appear as a solid ribbon or tracer with no gaps whatsoever.
//
//  Parameters: (ON:3 OFF:0 GAP:0 DASH:0 GROUP:0)
//  Colorset: (RED, GREEN, BLUE)
//
//   ┌─── Group 1 ───┐┌─── Group 2 ───┐┌─── Group 3 ───┐┌─── Group 4 ───┐...
//    [RRR][GGG][BBB]  [RRR][GGG][BBB]  [RRR][GGG][BBB]  [RRR][GGG][BBB] 
//
// ----------------------------------------------------------------------------
// This would be like a 'crush' pattern where all the blinks are tightly packed
// together with large gaps between each group.
//
//  Parameters: (ON:1 OFF:1 GAP:11 DASH:0 GROUP:0)
//  Colorset: (RED, GREEN, BLUE)
//
//   ┌──── Group 1 ─────┐┌─ Separator ─┐┌──── Group 1 ─────┐┌─ Separator ─┐...
//    [R][ ][G][ ][B][ ]  [           ]  [R][ ][G][ ][B][ ]  [           ]
//  
// ----------------------------------------------------------------------------
// If a DASH is present, then the pattern starts on the dash in the separator.
// Since the group size is 0 it will default to the size of the colorset,
// however since there is a DASH present the group size is decremented.
//
// By decrementing the group size when a dash is present it ensures the dash
// always lines up with the first color in the colorset.
//
//  Parameters: (ON:4 OFF:2 GAP:3 DASH:6 GROUP:0)
//  Colorset: (RED, GREEN, BLUE)
//
//   ┌─ Separator ─┐┌────── Group 1 ───────┐┌──── Separator ───┐┌────── Group 2 ───────┐...
//    [RRRRRR][   ]  [GGGG][   ][BBBB][   ]  [   ][RRRRRR][   ]  [GGGG][   ][BBBB][   ]
//
// ----------------------------------------------------------------------------
// Up till now all of the examples have been very linear, however if the group
// size doesn't match the number of colors things start to misalign
//
// Almost each group in this example blinks a different pairing of colors
// because the explicit group size of 2 is at odds with the number of colors.
//
//  Parameters: (ON:3 OFF:2 GAP:5 DASH:0 GROUP:2)
//  Colorset: (RED, GREEN, BLUE)
//
//   ┌─ Group 1 ──┐┌─ Sep ─┐┌─ Group 2 ──┐┌─ Sep ─┐┌─ Group 3 ──┐┌─ Sep ─┐┌─ Group 4 ──┐...
//    [R][ ][G][ ]  [     ]  [B][ ][R][ ]  [     ]  [G][ ][B][ ]  [     ]  [R][ ][G][ ] 
//
// ----------------------------------------------------------------------------
// When the dash is thrown into the mix the group size will decrement to
// acocunt for it. If a group size doesn't match the default group size then
// colors do not align perfectly again
//
//  Parameters: (ON:4 OFF:2 GAP:3 DASH:6 GROUP:3)
//  Colorset: (RED, GREEN, BLUE)
//
//   ┌─ Separator ─┐┌──────────── Group 1 ────────────┐┌──── Separator ───┐┌──────────── Group 2 ────────────┐...
//    [RRRRRR][   ]  [GGGG][   ][BBBB][   ][RRRR][   ]  [   ][GGGGGG][   ]  [BBBB][   ][RRRR][   ][GGGG][   ]
//

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
  m_lastFadeTick(0)
{
}

Pattern::Pattern(const PatternArgs &args) :
  Pattern(args.on_dur, args.off_dur, args.gap_dur,
      args.dash_dur, args.group_size, args.blend_speed, args.fade_range)
{
}

Pattern::~Pattern()
{
}

void Pattern::init()
{
  m_colorset.resetIndex();

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
  } else if (m_args.fade_range) {
    // if there is a fade dur and no blend need to iterate colorset
    m_colorset.getNext();
  }

  // Initialize the fluctuating fade value
  m_fadeValue = 1;
  m_lastFadeTick = 0;
  m_curStep = 0;
}

#include <stdio.h>

void Pattern::tickFade()
{
  // TODO: adjust this to make fade_range multiplied by some constant?
  //uint32_t duration = m_args.fade_range;

  // count the number of steps (times this logic has run)
  // NOTE: This function shouldn't run if fade_range is 0
  m_curStep++;

  uint32_t half_range = (m_args.fade_range) / 2;
  uint32_t mod = m_curStep % m_args.fade_range;

  // Triangle wave: up from 0 to range, then down to 0
  m_fadeValue = (mod < half_range) ? mod : ((uint32_t)m_args.fade_range - mod);

  //printf("m_fadeValue: %d\n", m_fadeValue);

  // iterate color when at lowest point
  if (m_fadeValue == 0) {
    m_colorset.getNext();
    m_fadeValue++;
  }
}

void Pattern::play()
{
  // Sometimes the pattern needs to cycle multiple states in a single frame so
  // instead of using a loop or recursion I have just used a simple goto
replay:

  // this switch will run the 'entry' handlers for each state if it is valid to
  // do so, otherwise, for example if on_dur is zero then it would naturally
  // fall through to the 'off' state and perform the logic for that
  switch (m_state) {
  case STATE_DISABLED:
    // only happens if the on and dash are zero, or there's no colors
    // just do nothing and return
    return;
  case STATE_BLINK_ON:
    // the first possible state of the pattern is the 'on' blink, this only
    // happens if there is a non-zero on_dur in the arguments
    if (m_args.on_dur > 0) {
      // if there is an 'on' duration then run the onBlinkOn callback
      onBlinkOn();
      // decrement the blink/group counter
      --m_groupCounter;
      // then iterate the state forward to STATE_ON for on_dur ticks
      nextState(isFade() ? (m_fadeValue) : m_args.on_dur);
      // done with this tick
      return;
    }
    // otherwise no on_dur so switch state to off
    m_state = STATE_BLINK_OFF;
    // fallthrough
  case STATE_BLINK_OFF:
    // first condition is if the pattern is not at the end of the 'group' yet
    // If this is the end of the grouping then we should fallthrough and
    // begin the gap. If there is still blinks left in the group, or there is
    // no gap and dash then it's possible the 'blink off' state should run
    if (m_groupCounter > 0 || (!m_args.gap_dur && !m_args.dash_dur)) {
      // the primary check to determine if there should be an 'off' state is
      // the off_dur in the arguments, zero means no 'off' state
      if (m_args.off_dur > 0) {
        // otherwise if there is an off_dur then call the onBlinkOff callback
        onBlinkOff();
        // then iterate the state into STATE_OFF for the next off_dur ticks
        nextState(isFade() ? (m_args.fade_range - m_fadeValue) : m_args.off_dur);
        // done with this tick
        return;
      }
      // if we reached here then that means there was 0 off duration but there is
      // either still some blinks left in the group, or no gap and dash. Either of
      // those cases means iterating the state backwards to STATE_BLINK_ON
      if (m_groupCounter > 0 && m_args.on_dur > 0) {
        m_state = STATE_BLINK_ON;
        // fake-fallthrough via goto
        goto replay;
      }
    }
    // switch state to gap
    m_state = STATE_BEGIN_GAP;
    // fallthrough
  case STATE_BEGIN_GAP:
    // When the first 'gap' begins it means the 'group' has ended, therefore
    //  first gap is to reset the group counter so
    // that the next time it reaches the 'on' state the group counter will be
    // reset
    m_groupCounter = m_args.group_size ? m_args.group_size : (m_colorset.numColors() - (m_args.dash_dur != 0));
    if (m_args.gap_dur > 0) {
      beginGap();
      nextState(m_args.gap_dur);
      return;
    }
    // switch state to dash
    m_state = STATE_BEGIN_DASH;
    // fallthrough
  case STATE_BEGIN_DASH:
    if (m_args.dash_dur > 0) {
      beginDash();
      nextState(m_args.dash_dur);
      return;
    }
    // switch state back to gap
    m_state = STATE_BEGIN_GAP2;
    // fallthrough
  case STATE_BEGIN_GAP2:
    if (m_args.dash_dur > 0 && m_args.gap_dur > 0) {
      beginGap();
      nextState(m_args.gap_dur);
      return;
    }
    // switch state to on
    m_state = STATE_BLINK_ON;
    // fake-fallthrough via goto
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
    //tickFade();
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
