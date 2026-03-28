#include <stdint.h>

#include "Helios.h"

#include "ColorConstants.h"
#include "TimeControl.h"
#include "Storage.h"
#include "Pattern.h"
#include "Random.h"
#include "Button.h"
#include "Led.h"

#ifdef HELIOS_EMBEDDED
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#endif

#ifdef HELIOS_CLI
#include <stdio.h>
#endif

#include <stdlib.h>

// some internal macros that shouldn't change
// The number of menus in hue/sat/val selection
#define NUM_MENUS_HUE_SAT_VAL 4
// the number of menus in quadrant selection
#define NUM_MENUS_QUADRANT 7

volatile char helios_version[] = HELIOS_VERSION_STR;

#ifdef HELIOS_EMBEDDED
// global instance for embedded
Helios helios;
#endif

Helios::Helios() :
  cur_state(STATE_MODES),
  global_flags(FLAG_NONE),
  menu_selection(0),
  cur_mode(0),
  selected_slot(0),
  selected_base_quad(0),
  selected_hue(0),
  selected_sat(0),
  selected_val(0),
  default_args(),
  default_colorsets(),
  pat(*this),
  m_storage(*this),
  m_led(*this),
  m_time(*this),
  m_button(*this),
  keepgoing(true)
#ifdef HELIOS_CLI
  , sleeping(false),
  m_defaultCallbacks(),
  m_callbacks(&m_defaultCallbacks)
#endif
{
}

bool Helios::init()
{
  // first initialize all the components of helios
  if (!init_components()) {
    return false;
  }
  // then initialize the hardware for embedded helios
#ifdef HELIOS_EMBEDDED
  // Set PB0, PB1, PB4 as output
  DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB4);
  // Timer0 Configuration for PWM
  TCCR0A = (1 << WGM01) | (1 << WGM00) | (1 << COM0A1) | (1 << COM0B1);
  // No prescaler
  TCCR0B = (1 << CS00);
  // Timer1 for PWM on PB4, Fast PWM, Non-inverting, No prescaler
  TCCR1 = (1 << PWM1A) | (1 << COM1A1) | (1 << CS10);
  // Enable PWM on OC1B
  GTCCR = (1 << PWM1B) | (1 << COM1B1);
  // Enable Timer0 overflow interrupt
  TIMSK |= (1 << TOIE0);
  // Enable interrupts
  sei();
#endif
  return true;
}

bool Helios::init_components()
{
  // initialize various components of Helios
  if (!m_time.init()) {
    return false;
  }
  if (!m_led.init()) {
    return false;
  }
  if (!m_storage.init()) {
    return false;
  }
  if (!m_button.init()) {
    return false;
  }
  // initialize global variables
  cur_state = STATE_MODES;
  menu_selection = 0;
  cur_mode = 0;
  selected_slot = 0;
  selected_base_quad = 0;
  keepgoing = true;
#ifdef HELIOS_CLI
  sleeping = false;
#endif
  // load global flags, and brightness from storage, this
  // includes for example conjure mode and the mode index
  // of the conjure mode if it is enabled
  load_global_flags();
  // finally load whatever current mode index is selected
  // this might be mode 0, or for example a separate index
  // if conjure mode is enabled
  load_cur_mode();
  return true;
}

void Helios::tick()
{
  // sample the button and re-calculate all button globals
  // the button globals should not change anywhere else
  m_button.update();

  // handle the current state of the system, ie whatever state
  // we're in we check for the appropriate input events for that
  // state by checking button globals, then run the appropriate logic
  handle_state();

  // Update the Leds once per frame
  m_led.update();

  // finally tick the clock forward and then sleep till the entire
  // tick duration has been consumed
  m_time.tickClock();
}

void Helios::enter_sleep()
{
#ifdef HELIOS_EMBEDDED
  // clear the led colors
  m_led.clear();
  // Set all pins to input
  DDRB = 0x00;
  // Disable pull-ups on all pins
  PORTB = 0x00;
  // Enable wake on interrupt for the button
  m_button.enableWake();
  // Set sleep mode to POWER DOWN mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  // enter sleep
  sleep_mode();
  // ... interrupt will make us wake here

  // Set PB0, PB1, PB4 as output
  DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB4);
  // wakeup here, re-init
  init_components();
#else
  cur_state = STATE_SLEEP;
  // enable the sleep bool
  sleeping = true;
#endif
}

void Helios::wakeup()
{
#ifdef HELIOS_EMBEDDED
  // nothing needed here, this interrupt firing will make the mainthread resume
#else
  // if the button was held down then they are entering off-menus
  // but if we re-initialize the button it will clear this state
  bool pressed = m_button.isPressed();
  // re-initialize some stuff
  m_time.init();
  m_button.init();
  // so just re-press it
  if (pressed) {
    m_button.doPress();
  }
  cur_state = STATE_MODES;
  // turn off the sleeping flag that only CLI has
  sleeping = false;
#endif
}

void Helios::load_next_mode()
{
  // increment current mode and wrap around
  cur_mode = (uint8_t)(cur_mode + 1) % NUM_MODE_SLOTS;
  // now load current mode again
  load_cur_mode();
}

void Helios::load_cur_mode()
{
  // read pattern from storage at cur mode index
  if (!m_storage.read_pattern(cur_mode, pat)) {
    // and just initialize default if it cannot be read
    Patterns::make_default(cur_mode, pat);
    // try to write it out because storage was corrupt
    m_storage.write_pattern(cur_mode, pat);
  }
  // then re-initialize the pattern runtime
  pat.init();
}

void Helios::save_cur_mode()
{
  m_storage.write_pattern(cur_mode, pat);
}

void Helios::load_global_flags()
{
  // read the global flags from index 0 config
  global_flags = (Flags)m_storage.read_global_flags();
  if (has_flags(FLAG_CONJURE)) {
    // if conjure is enabled then load the current mode index from storage
    cur_mode = m_storage.read_current_mode();
  }
  // read the global brightness from index 2 config
  uint8_t saved_brightness = m_storage.read_brightness();
  // Check if flags are valid (FLAGS_INVALID is inverse mask of valid flags)
  // and brightness is set in storage
  bool is_valid = !has_any_flags(FLAGS_INVALID) && saved_brightness > 0;
  if (is_valid) {
    m_led.setBrightness(saved_brightness);
  }

  if (!is_valid) {
    // if the brightness was 0 and the flags are invalid then the storage was likely
    // uninitialized or corrupt so write out the defaults
    factory_reset();
  }
}

void Helios::save_global_flags()
{
  m_storage.write_global_flags(global_flags);
  m_storage.write_current_mode(cur_mode);
}

void Helios::set_mode_index(uint8_t mode_index)
{
  cur_mode = (uint8_t)mode_index % NUM_MODE_SLOTS;
  // now load current mode again
  load_cur_mode();
}

#ifdef HELIOS_CLI
void Helios::setCallbacks(HeliosCallbacks *callbacks)
{
  m_callbacks = callbacks ? callbacks : &m_defaultCallbacks;
}
#endif

void Helios::handle_state()
{
  // check for the force sleep button hold regardless of which state we're in
  if (m_button.holdDuration() > FORCE_SLEEP_TIME) {
    // when released the device will just sleep
    if (m_button.onRelease()) {
      enter_sleep();
      // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
      return;
    }
    // but as long as it's held past the sleep time it just turns off the led
    if (m_button.isPressed()) {
      m_led.clear();
      return;
    }
  }
  // otherwise just handle the state like normal
  switch (cur_state) {
    case STATE_MODES:
      handle_state_modes();
      break;
    case STATE_COLOR_SELECT_SLOT:
    case STATE_COLOR_SELECT_QUADRANT:
    case STATE_COLOR_SELECT_HUE:
    case STATE_COLOR_SELECT_SAT:
    case STATE_COLOR_SELECT_VAL:
      handle_state_col_select();
      break;
    case STATE_PATTERN_SELECT:
      handle_state_pat_select();
      break;
    case STATE_TOGGLE_CONJURE:
      handle_state_toggle_flag(FLAG_CONJURE);
      break;
    case STATE_TOGGLE_LOCK:
      handle_state_toggle_flag(FLAG_LOCKED);
      break;
    case STATE_SET_DEFAULTS:
      handle_state_set_defaults();
      break;
    case STATE_SET_GLOBAL_BRIGHTNESS:
      handle_state_set_global_brightness();
      break;
    case STATE_SHIFT_MODE:
      handle_state_shift_mode();
      break;
    case STATE_RANDOMIZE:
      handle_state_randomize();
      break;
#ifdef HELIOS_CLI
    case STATE_SLEEP:
      // simulate sleep in helios CLI
      if (m_button.onPress() || m_button.onShortClick() || m_button.onLongClick()) {
        wakeup();
      }
      break;
#endif
  }
}

void Helios::handle_state_modes()
{
  // whether they have released the button since turning on
  bool hasReleased = (m_button.releaseCount() > 0);

  if (m_button.releaseCount() > 1 && m_button.onShortClick()) {
    if (has_flags(FLAG_CONJURE)) {
      enter_sleep();
    } else {
      load_next_mode();
    }
    return;
  }

  // check for lock and go back to sleep
  if (has_flags(FLAG_LOCKED) && hasReleased && !m_button.onRelease()) {
    enter_sleep();
    // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
    return;
  }

  if (!has_flags(FLAG_LOCKED) && hasReleased) {
    // just play the current mode
    pat.play();
  }
  // check how long the button is held
  uint32_t holdDur = m_button.holdDuration();
  // calculate a magnitude which corresponds to how many times past the MENU_HOLD_TIME
  // the user has held the button, so 0 means haven't held fully past one yet, etc
  uint8_t magnitude = (uint8_t)(holdDur / MENU_HOLD_TIME);
  // whether the user has held the button longer than a short click
  bool heldPast = (holdDur > SHORT_CLICK_THRESHOLD);

  // flash red briefly when locked and short clicked
  if (has_flags(FLAG_LOCKED) && !heldPast) {
    m_led.set(RGB_RED_BRI_LOW);
  }
  // if the button is held for at least 1 second
  if (m_button.isPressed() && heldPast) {
    // if the button has been released before then show the on menu
    if (hasReleased) {
      switch (magnitude) {
        default:
        case 0: m_led.clear(); break;                                     // Turn off
        case 1: m_led.set(0, 0x3c, 0x31); break;                          // Color Selection
        case 2: m_led.set(0x3c, 0, 0x0e); break;                          // Pattern Selection
        case 3: m_led.set(0x3c, 0x1c, 0); break;                          // Conjure Mode
        case 4: m_led.set(0x3c, 0x3c, 0x3c); break;                       // Shift Mode
        case 5: m_led.set(HSVColor(m_time.getCurtime(), 255, 100)); break; // Randomizer
      }
    } else {
      if (has_flags(FLAG_LOCKED)) {
        switch (magnitude) {
          default:
          case 0: m_led.clear(); break;
          case TIME_TILL_GLOW_LOCK_UNLOCK: m_led.set(0x3c, 0, 0); break; // Exit
        }
      } else {
        switch (magnitude) {
          default:
          case 0: m_led.clear(); break;         // nothing
          case 1: m_led.set(0x3c, 0, 0); break; // Enter Glow Lock
          case 2: m_led.set(0, 0x3c, 0); break; // Global Brightness
          case 3: m_led.set(0, 0, 0x3c); break; // Master Reset
        }
      }
    }
  }
  // if this isn't a release tick there's nothing more to do
  if (m_button.onRelease()) {
    // Resets the menu selection before entering new state
    menu_selection = 0;
    if (heldPast && m_button.releaseCount() == 1) {
      handle_off_menu(magnitude, heldPast);
      return;
    }
    // otherwise if we have released it then we are in the 'on' menu
    handle_on_menu(magnitude, heldPast);
  }
}

void Helios::handle_off_menu(uint8_t mag, bool past)
{
  // if still locked then handle the unlocking menu which is just if mag == 5
  if (has_flags(FLAG_LOCKED)) {
    switch (mag) {
      case TIME_TILL_GLOW_LOCK_UNLOCK:  // red lock
        cur_state = STATE_TOGGLE_LOCK;
        break;
      default:
        // just go back to sleep in hold-past off menu
        enter_sleep();
        // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
    }
    // in this case we return either way, since we're locked
    return;
  }

  // otherwise if not locked handle the off menu
  switch (mag) {
    case 1:  // red lock
      cur_state = STATE_TOGGLE_LOCK;
      m_led.clear();
      return; // RETURN HERE
    case 2:  // green global brightness
      cur_state = STATE_SET_GLOBAL_BRIGHTNESS;
      return; // RETURN HERE
    case 3:  // blue reset defaults
      cur_state = STATE_SET_DEFAULTS;
      return; //RETURN HERE
    default:
      // just go back to sleep in hold-past off menu
      enter_sleep();
      // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
      return;
  }
}

void Helios::handle_on_menu(uint8_t mag, bool past)
{
  switch (mag) {
    case 0:  // off
      // but only if we held for more than a short click
      if (past) {
        enter_sleep();
        // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
        return;
      }
      break;
    case 1:  // color select
      cur_state = STATE_COLOR_SELECT_SLOT;
      // reset the menu selection
      menu_selection = 0;
#if ALTERNATIVE_HSV_RGB == 1
      // use the nice hue to rgb rainbow
      g_hsv_rgb_alg = HSV_TO_RGB_RAINBOW;
#endif
      break;
    case 2:  // pat select
      cur_state = STATE_PATTERN_SELECT;
      // reset the menu selection
      menu_selection = 0;
      break;
    case 3:  // conjure mode
      cur_state = STATE_TOGGLE_CONJURE;
      m_led.clear();
      break;
    case 4:  // shift mode down
      cur_state = STATE_SHIFT_MODE;
      break;
    case 5:  // randomizer
      cur_state = STATE_RANDOMIZE;
      break;
    default:  // hold past
      break;
  }
}

void Helios::handle_state_col_select()
{
  ColorSelectOption slot_option = OPTION_NONE;
  switch (cur_state) {
    case STATE_COLOR_SELECT_SLOT:
      // pick the target colorset slot
      handle_state_col_select_slot(slot_option);
      break;
    case STATE_COLOR_SELECT_QUADRANT:
      // pick the hue quadrant
      handle_state_col_select_quadrant();
      break;
    case STATE_COLOR_SELECT_HUE:
    case STATE_COLOR_SELECT_SAT:
    case STATE_COLOR_SELECT_VAL:
    default:
      // pick the hue sat or val
      handle_state_col_select_hue_sat_val();
      break;
  }
  // get the current color
  RGBColor cur = m_led.get();
  cur.red /= 2;
  cur.green /= 2;
  cur.blue /= 2;
  // this is a stupid override for when we're exiting color select
  // show a white selection instead
  if (slot_option != OPTION_NONE) {
    cur = RGB_WHITE_BRI_LOW;
  }
  // show selection in all of these menus
  show_selection(cur);
}

void Helios::handle_state_col_select_slot(ColorSelectOption &out_option)
{
  Colorset &set = pat.colorset();
  uint8_t num_cols = set.numColors();

  if (m_button.onShortClick()) {
    // the number of menus in slot selection = all colors + exit
    uint8_t num_menus = num_cols + 1;
    // except if the number of colors is less than total color slots
    if (num_cols < NUM_COLOR_SLOTS) {
      // then we have another menu: add color
      num_menus++;
    }
    menu_selection = (menu_selection + 1) % num_menus;
  }

  bool long_click = m_button.onLongClick();

  // Reset the color selection variables, these are the hue/sat/val that have been selected
  // in the following menus, this is a weird place to reset these but it ends up being the only
  // place where it can be written once and still handle all the possible cases it needs to run
  selected_sat = 255;
  selected_val = 255;

  if (num_cols < NUM_COLOR_SLOTS && menu_selection == num_cols) {
    // add color
    out_option = SELECTED_ADD;
    m_led.strobe(100, 100, RGB_WHITE_BRI_LOW, RGB_OFF);
    if (long_click) {
      selected_slot = menu_selection;
    }
  } else if (menu_selection == num_cols + 1 || (num_cols == NUM_COLOR_SLOTS && menu_selection == num_cols)) {
    // exit
    out_option = SELECTED_EXIT;
    m_led.strobe(60, 40, RGB_RED_BRI_LOW, RGB_OFF);
    if (long_click) {
#if ALTERNATIVE_HSV_RGB == 1
      // restore hsv to rgb algorithm type, done color selection
      g_hsv_rgb_alg = HSV_TO_RGB_GENERIC;
#endif
      save_cur_mode();
      cur_state = STATE_MODES;
      return;
    }
  } else {
    out_option = SELECTED_SLOT;
    selected_slot = menu_selection;
    // render current selection
    RGBColor col = set.get(selected_slot);
    if (col.empty()) {
      m_led.strobe(1, 30, RGB_OFF, RGB_WHITE_BRI_LOW);
    } else {
      m_led.strobe(3, 30, RGB_OFF, col);
    }
    if (m_button.holdPressing()) {
      // flash red
      m_led.strobe(150, 150, RGB_RED_BRI_LOW, col);
    }
    if (m_button.onHoldClick()){
      set.removeColor(selected_slot);
      return;
    }
  }
  if (long_click) {
    cur_state = (State)(cur_state + 1);
    // reset the menu selection
    menu_selection = 0;
  }
}

struct ColorsMenuData {
  uint8_t hues[4];
};
// array of hues for selection
static const ColorsMenuData color_menu_data[4] = {
  // hue0           hue1              hue2          hue3
  // ==================================================================================
  { HUE_RED,        HUE_CORAL_ORANGE, HUE_ORANGE,   HUE_YELLOW },
  { HUE_LIME_GREEN, HUE_GREEN,        HUE_SEAFOAM,  HUE_TURQUOISE },
  { HUE_ICE_BLUE,   HUE_LIGHT_BLUE,   HUE_BLUE,     HUE_ROYAL_BLUE },
  { HUE_PURPLE,     HUE_PINK,         HUE_HOT_PINK, HUE_MAGENTA },
};

void Helios::handle_state_col_select_quadrant()
{
  if (m_button.onShortClick()) {
    menu_selection = (menu_selection + 1) % NUM_MENUS_QUADRANT;
  }

  uint8_t hue_quad = (menu_selection - 2) % 4;
  if (menu_selection > 5) {
    menu_selection = 0;
  }

  if (m_button.onLongClick()) {
    // select hue/sat/val
    switch (menu_selection) {
      case 0:  // selected blank
        // add blank to set
        pat.colorset().set(selected_slot, RGB_OFF);
        // Return to the slot you were editing
        menu_selection = selected_slot;
        // go to slot selection - 1 because we will increment outside here
        cur_state = STATE_COLOR_SELECT_SLOT;
        // RETURN HERE
        return;
      case 1:  // selected white
        // adds white, skip hue/sat to brightness
        selected_sat = 0;
        menu_selection = 0;
        cur_state = STATE_COLOR_SELECT_VAL;
        // RETURN HERE
        return;
      default:  // 2-5
        selected_base_quad = hue_quad;
        break;
    }
  }

  // default col1/col2 to off and white for the first two options
  RGBColor col1 = RGB_OFF;
  RGBColor col2;
  uint16_t on_dur, off_dur;

  switch (menu_selection) {
    case 0: // Blank Option
      col2 = RGB_WHITE_BRI_LOW;
      on_dur = 1;
      off_dur = 30;
      break;
    case 1: // White Option
      col2 = RGB_WHITE;
      on_dur = 9;
      off_dur = 0;
      break;
    default: // Color options
      col1 = HSVColor(color_menu_data[hue_quad].hues[0], 255, 255);
      col2 = HSVColor(color_menu_data[hue_quad].hues[2], 255, 255);
      on_dur = 500;
      off_dur = 500;
      break;
  }
  m_led.strobe(on_dur, off_dur, col1, col2);
  // show a white flash for the first two menus
  if (menu_selection <= 1) {
    show_selection(RGB_WHITE_BRI_LOW);
  } else {
    // dim the color for the quad menus
    RGBColor cur = m_led.get();
    cur.red /= 2;
    cur.green /= 2;
    cur.blue /= 2;
    show_selection(RGB_WHITE_BRI_LOW);
  }
  if (m_button.onLongClick()) {
    cur_state = (State)(cur_state + 1);
    // reset the menu selection
    menu_selection = 0;
  }
}

void Helios::handle_state_col_select_hue_sat_val()
{
  // handle iterating to the next option
  if (m_button.onShortClick()) {
    menu_selection = (menu_selection + 1) % NUM_MENUS_HUE_SAT_VAL;
  }
  // in the sat/val selection a longclick is next and hold is save but in
  // the final val selection a longclick is save and there's no next
  bool gotoNextMenu = m_button.onLongClick();
  bool saveAndFinish = m_button.onHoldClick();
  switch (cur_state) {
    default:
    case STATE_COLOR_SELECT_HUE:
      selected_hue = color_menu_data[selected_base_quad].hues[menu_selection];
      break;
    case STATE_COLOR_SELECT_SAT:
      static const uint8_t saturation_values[4] = {HSV_SAT_HIGH, HSV_SAT_MEDIUM, HSV_SAT_LOW, HSV_SAT_LOWEST};
      selected_sat = saturation_values[menu_selection];
      break;
    case STATE_COLOR_SELECT_VAL:
      static const uint8_t hsv_values[4] = {HSV_VAL_HIGH, HSV_VAL_MEDIUM, HSV_VAL_LOW, HSV_VAL_LOWEST};
      selected_val = hsv_values[menu_selection];
      // longclick becomes save and there is no next
      saveAndFinish = gotoNextMenu;
      break;
  }
  // render current selection
  m_led.set(HSVColor(selected_hue, selected_sat, selected_val));
  // show the long selection flash
  if (m_button.holdPressing()) {
    m_led.strobe(150, 150, RGB_CORAL_ORANGE_SAT_LOWEST, m_led.get());
  }
  // check to see if we are holding to save and skip
  if (saveAndFinish) {
    cur_state = STATE_COLOR_SELECT_SLOT;
    pat.updateColor(selected_slot, HSVColor(selected_hue, selected_sat, selected_val));
    save_cur_mode();
    // Return to the slot you were editing
    menu_selection = selected_slot;
    return;
  }
  if (gotoNextMenu) {
    cur_state = (State)(cur_state + 1);
    // reset the menu selection
    menu_selection = 0;
  }
}

void Helios::handle_state_pat_select()
{
  if (m_button.onLongClick()) {
    save_cur_mode();
    cur_state = STATE_MODES;
  }
  if (m_button.onShortClick()) {
    Patterns::make_pattern((PatternID)menu_selection, pat);
    menu_selection = (menu_selection + 1) % PATTERN_COUNT;
    pat.init();
  }
  pat.play();
  show_selection(RGB_MAGENTA_BRI_LOW);
}

void Helios::handle_state_toggle_flag(Flags flag)
{
  // toggle the conjure flag
  toggle_flags(flag);
  // write out the new global flags and the current mode
  save_global_flags();
  // switch back to modes
  cur_state = STATE_MODES;
}

void Helios::handle_state_set_defaults()
{
  if (m_button.onShortClick()) {
    menu_selection = !menu_selection;
  }
  // show low white for exit or red for select
  if (menu_selection) {
    m_led.strobe(80, 20, RGB_RED_BRI_LOW, RGB_OFF);
  } else {
    m_led.strobe(20, 10, RGB_WHITE_BRI_LOWEST, RGB_OFF);
  }
  // when the user long clicks a selection
  if (m_button.onLongClick()) {
    // if the user actually selected 'yes'
    if (menu_selection == 1) {
      factory_reset();
    }
    cur_state = STATE_MODES;
  }
  show_selection(RGB_WHITE_BRI_LOW);
}

void Helios::factory_reset()
{
  for (uint8_t i = 0; i < NUM_MODE_SLOTS; ++i) {
    Patterns::make_default(i, pat);
    m_storage.write_pattern(i, pat);
  }
  // Reset global brightness to default
  m_led.setBrightness(DEFAULT_BRIGHTNESS);
  m_storage.write_brightness(DEFAULT_BRIGHTNESS);
  // reset global flags
  global_flags = FLAG_NONE;
  cur_mode = 0;
  // save global flags
  save_global_flags();
  // re-load current mode
  load_cur_mode();
}

void Helios::handle_state_set_global_brightness()
{
  if (m_button.onShortClick()) {
    menu_selection = (menu_selection + 1) % NUM_BRIGHTNESS_OPTIONS;
  }
  // show different levels of green for each selection
  uint8_t col = 0;
  uint8_t brightness = 0;
  switch (menu_selection) {
    case 0:
      col = 0xFF;
      brightness = BRIGHTNESS_HIGH;
      break;
    case 1:
      col = 0x78;
      brightness = BRIGHTNESS_MEDIUM;
      break;
    case 2:
      col = 0x3c;
      brightness = BRIGHTNESS_LOW;
      break;
    case 3:
      col = 0x28;
      brightness = BRIGHTNESS_LOWEST;
      break;
  }
  m_led.set(0, col, 0);
  // when the user long clicks a selection
  if (m_button.onLongClick()) {
    // set the brightness based on the selection
    m_led.setBrightness(brightness);
    m_storage.write_brightness(brightness);
    cur_state = STATE_MODES;
  }
  show_selection(RGB_WHITE_BRI_LOW);
}

void Helios::handle_state_shift_mode()
{
  uint8_t new_mode = (cur_mode > 0) ? (uint8_t)(cur_mode - 1) : (uint8_t)(NUM_MODE_SLOTS - 1);
  // copy the storage from the new position into our current position
  m_storage.copy_slot(new_mode, cur_mode);
  // point at the new position
  cur_mode = new_mode;
  // write out the current mode to the newly updated position
  save_cur_mode();
  cur_state = STATE_MODES;
}

void Helios::handle_state_randomize()
{
  if (m_button.onShortClick()) {
    Colorset &cur_set = pat.colorset();
    Random ctx(pat.crc32());
    uint8_t randVal = ctx.next8();
    cur_set.randomizeColors(ctx, (randVal + 1) % NUM_COLOR_SLOTS, Colorset::COLOR_MODE_RANDOMLY_PICK);
    Patterns::make_pattern((PatternID)(randVal % PATTERN_COUNT), pat);
    pat.init();
  }
  if (m_button.onLongClick()) {
    save_cur_mode();
    cur_state = STATE_MODES;
  }
  pat.play();
  show_selection(RGB_WHITE_BRI_LOW);
}

void Helios::show_selection(RGBColor color)
{
  // only show selection while pressing the button
  if (!m_button.isPressed()) {
    return;
  }
  uint16_t holdDur = (uint16_t)m_button.holdDuration();
  // if the hold duration is outside the flashing range do nothing
  if (holdDur < SHORT_CLICK_THRESHOLD || holdDur >= HOLD_CLICK_START) {
    return;
  }
  m_led.set(color);
}
