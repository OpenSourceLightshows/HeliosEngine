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
#define NUM_COLORS_PER_GROUP 4

#define NUM_COLOR_GROUPS 4
// the number of menus in group selection
#define NUM_MENUS_GROUP 8

Helios::State Helios::cur_state;
Helios::Flags Helios::global_flags;
uint8_t Helios::menu_selection;
uint8_t Helios::cur_mode;
uint8_t Helios::selected_base_group;
uint8_t Helios::selected_hue;
uint8_t Helios::selected_val;
uint8_t Helios::num_colors_selected;
Pattern Helios::pat;
bool Helios::keepgoing;
uint32_t Helios::last_mode_switch_time;
Colorset Helios::new_colorset;

#ifdef HELIOS_CLI
bool Helios::sleeping;
#endif

volatile char helios_version[] = HELIOS_VERSION_STR;


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
  if (!Time::init()) {
    return false;
  }
  if (!Led::init()) {
    return false;
  }
  if (!Storage::init()) {
    return false;
  }
  if (!Button::init()) {
    return false;
  }
  // initialize global variables
  cur_state = STATE_MODES;
  menu_selection = 0;
  cur_mode = 0;
  num_colors_selected = 0;
  selected_base_group = 0;
  keepgoing = true;
  last_mode_switch_time = 0;
#ifdef HELIOS_CLI
  sleeping = false;
#endif
  // load global flags, this includes for example conjure mode
  // and the mode index of the conjure mode if it is enabled
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
  Button::update();

  // handle the current state of the system, ie whatever state
  // we're in we check for the appropriate input events for that
  // state by checking button globals, then run the appropriate logic
  handle_state();

  // Update the Leds once per frame
  Led::update();

  // finally tick the clock forward and then sleep till the entire
  // tick duration has been consumed
  Time::tickClock();
}

void Helios::enter_sleep()
{
#ifdef HELIOS_EMBEDDED
  // clear the led colors
  Led::clear();
  // Set all pins to input
  DDRB = 0x00;
  // Disable pull-ups on all pins
  PORTB = 0x00;
  // Enable wake on interrupt for the button
  Button::enableWake();
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
  bool pressed = Button::isPressed();
  // re-initialize some stuff
  Time::init();
  Button::init();
  // so just re-press it
  if (pressed) {
    Button::doPress();
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
  if (!Storage::read_pattern(cur_mode, pat)) {
    // and just initialize default if it cannot be read
    Patterns::make_default(cur_mode, pat);
    // try to write it out because storage was corrupt
    Storage::write_pattern(cur_mode, pat);
  }
  // then re-initialize the pattern
  pat.init();
  // Update the last mode switch time when loading a mode
  last_mode_switch_time = Time::getCurtime();
}

void Helios::save_cur_mode()
{
  Storage::write_pattern(cur_mode, pat);
}

void Helios::load_global_flags()
{
  // read the global flags from index 0 config
  global_flags = (Flags)Storage::read_global_flags();
  if (has_flag(FLAG_CONJURE)) {
    // if conjure is enabled then load the current mode index from storage
    cur_mode = Storage::read_current_mode();
  }
}

void Helios::save_global_flags()
{
  Storage::write_global_flags(global_flags);
  Storage::write_current_mode(cur_mode);
}

void Helios::set_mode_index(uint8_t mode_index)
{
  cur_mode = (uint8_t)mode_index % NUM_MODE_SLOTS;
  // now load current mode again
  load_cur_mode();
}

void Helios::handle_state()
{
  // check for the force sleep button hold regardless of which state we're in
  if (Button::holdDuration() > FORCE_SLEEP_TIME) {
    // when released the device will just sleep
    if (Button::onRelease()) {
      enter_sleep();
      // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
      return;
    }
    // but as long as it's held past the sleep time it just turns off the led
    if (Button::isPressed()) {
      Led::clear();
      return;
    }
  }
  // otherwise just handle the state like normal
  switch (cur_state) {
    case STATE_MODES:
      handle_state_modes();
      break;
    case STATE_COLOR_GROUP_SELECTION:
    case STATE_COLOR_VARIANT_SELECTION:
    case STATE_COLOR_SELECT_BRIGHTNESS:
      handle_state_color_selection();
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
#ifdef HELIOS_CLI
    case STATE_SLEEP:
      // simulate sleep in helios CLI
      if (Button::onPress() || Button::onShortClick() || Button::onLongClick()) {
        wakeup();
      }
      break;
#endif
  }
}

void Helios::handle_state_modes()
{
  // whether they have released the button since turning on
  bool hasReleased = (Button::releaseCount() > 0);

  if (Button::releaseCount() > 1 && Button::onShortClick()) {
    if (has_flag(FLAG_CONJURE)) {
      enter_sleep();
    } else {
      load_next_mode();
    }
    return;
  }

  // This handles iterating the mode forward when the autoplay feature is
  // enabled. The modes automatically cycle forward every AUTOPLAY_DURATION ticks
  // but only if the button isn't pressed to avoid iterating while opening menus
  if (has_flag(FLAG_AUTOPLAY) && !Button::isPressed()) {
    uint32_t current_time = Time::getCurtime();
    if (current_time - last_mode_switch_time >= AUTOPLAY_DURATION) {
      // If a pattern has a single cycle that is longer than the autoplay duration,
      // prevent the mode switch from interrupting the pattern so the full cycle can be seen.
      if (pat.colorset().numColors() <= 1 || pat.colorset().onStart()) {
        load_next_mode();
      }
    }
  }

  // check for lock and go back to sleep
  if (has_flag(FLAG_LOCKED) && hasReleased && !Button::onRelease()) {
    enter_sleep();
    // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
    return;
  }

  if (!has_flag(FLAG_LOCKED) && hasReleased) {
    // just play the current mode
    pat.play();
  }
  // check how long the button is held
  uint32_t holdDur = Button::holdDuration();
  // calculate a magnitude which corresponds to how many times past the MENU_HOLD_TIME
  // the user has held the button, so 0 means haven't held fully past one yet, etc
  uint8_t magnitude = (uint8_t)(holdDur / MENU_HOLD_TIME);
  // whether the user has held the button longer than a short click
  bool heldPast = (holdDur > SHORT_CLICK_THRESHOLD);

  // flash red briefly when locked and short clicked
  if (has_flag(FLAG_LOCKED) && holdDur < SHORT_CLICK_THRESHOLD) {
    Led::set(RGB_RED_BRI_LOW);
  }
  // if the button is held for at least 1 second
  if (Button::isPressed() && heldPast) {
    // if the button has been released before then show the on menu
    if (hasReleased) {
      switch (magnitude) {
        default:
        case 0: Led::clear(); break;                                     // Turn off
        case 1: Led::set(RGB_TURQUOISE_BRI_LOW); break;                 // Color Selection
        case 2: Led::set(RGB_MAGENTA_BRI_LOW); break;                   // Pattern Selection
        case 3: Led::set(RGB_YELLOW_BRI_LOW); break;                    // Conjure Mode
      }
    } else {
      if (has_flag(FLAG_LOCKED)) {
        switch (magnitude) {
          default:
          case 0: Led::clear(); break;
          case TIME_TILL_GLOW_LOCK_UNLOCK: Led::set(RGB_RED_BRI_LOW); break; // Exit
        }
      } else {
        switch (magnitude) {
          default:
          case 0: Led::clear(); break;         // nothing
          case 1: Led::set(RGB_RED_BRI_LOW); break; // Enter Glow Lock
          case 2: Led::set(RGB_BLUE_BRI_LOW); break; // Master Reset
          case 3: Led::set(has_flag(FLAG_AUTOPLAY) ? RGB_ORANGE_BRI_LOW : RGB_PINK_BRI_LOW); break; // Autoplay Toggle
        }
      }
    }
  }
  // if this isn't a release tick there's nothing more to do
  if (Button::onRelease()) {
    // Resets the menu selection before entering new state
    menu_selection = 0;
    if (heldPast && Button::releaseCount() == 1) {
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
  if (has_flag(FLAG_LOCKED)) {
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
      Led::clear();
      return; // RETURN HERE
    case 2:  // blue reset defaults
      cur_state = STATE_SET_DEFAULTS;
      return; //RETURN HERE
    case 3:  // autoplay toggle
      toggle_flag(FLAG_AUTOPLAY);
      save_global_flags();
      last_mode_switch_time = Time::getCurtime(); // Reset the timer when enabling autoplay
      cur_state = STATE_MODES;
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
      cur_state = STATE_COLOR_GROUP_SELECTION;
      // reset the menu selection and colors selected
      menu_selection = 0;
      num_colors_selected = 0;
      // Store original colorset before clearing
      new_colorset = pat.colorset();
      // Clear existing colors in pattern
      new_colorset.clear();
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
      Led::clear();
      break;
    default:  // hold past
      break;
  }
}

void Helios::handle_state_color_selection()
{
  switch (cur_state) {
    case STATE_COLOR_GROUP_SELECTION:
      // pick the hue group
      handle_state_color_group_selection();
      break;
    case STATE_COLOR_VARIANT_SELECTION:
      // pick the hue
      handle_state_color_variant_selection();
      break;
    case STATE_COLOR_SELECT_BRIGHTNESS:
      handle_state_color_brightness_selection();
      break;
    default:
      break;
  }
  // get the current color
  RGBColor cur = Led::get();
  cur.red /= 2;
  cur.green /= 2;
  cur.blue /= 2;
  // show selection in all of these menus
  show_selection(cur);
}

struct ColorsMenuData {
  uint8_t hues[4];
};

// array of colors for selection
static const ColorsMenuData color_menu_data[NUM_COLOR_GROUPS] = {
  // color0           color1              color2          color3
  // ===================================================================
  { HUE_RED,        HUE_CORAL_ORANGE, HUE_ORANGE,   HUE_YELLOW },
  { HUE_LIME_GREEN, HUE_GREEN,        HUE_SEAFOAM,  HUE_TURQUOISE },
  { HUE_ICE_BLUE,   HUE_LIGHT_BLUE,   HUE_BLUE,     HUE_ROYAL_BLUE },
  { HUE_PURPLE,     HUE_PINK,         HUE_HOT_PINK, HUE_MAGENTA },
};

void Helios::handle_state_color_group_selection()
{
  if (Button::onShortClick()) {
    menu_selection = (menu_selection + 1) % NUM_MENUS_GROUP;
  }
  uint8_t color_group = (menu_selection - 2) % NUM_COLOR_GROUPS;
  if (menu_selection > 5) {
    menu_selection = 0;
  }
  if (Button::onLongClick()) {
    switch (menu_selection) {
      case 0:
        addColorAndSave(HSVColor(0, 255, 0), false); // blank as val=0
        break;
      case 1:
        selected_hue = 0; // arbitrary hue for white
        selected_val = 255;
        cur_state = STATE_COLOR_SELECT_BRIGHTNESS;
        menu_selection = 0;
        return;
      default:
        selected_base_group = color_group;
        cur_state = STATE_COLOR_VARIANT_SELECTION;
        menu_selection = 0;
        return;
    }
    menu_selection = 0;
  }
  // default col1/col2 to off and white for the first two options
  HSVColor col1 = RGB_OFF;
  HSVColor col2;
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
      col1 = HSVColor(color_menu_data[color_group].hues[0], 255, 255);
      col2 = HSVColor(color_menu_data[color_group].hues[2], 255, 255);
      on_dur = 500;
      off_dur = 500;
      break;
  }
  Led::strobe(on_dur, off_dur, col1, col2);
  // show a white flash for the first two menus
  if (menu_selection <= 1) {
    show_selection(RGB_WHITE_BRI_LOW);
  } else {
    // dim the color for the quad menus
    RGBColor cur = Led::get();
    cur.red /= 2;
    cur.green /= 2;
    cur.blue /= 2;
    show_selection(RGB_WHITE_BRI_LOW);
  }

  if (menu_selection == 0) {
    if (Button::holdPressing()) {
      Led::strobe(150, 150, RGB_RED_BRI_LOW, RGB_OFF);
    }
    if (Button::onHoldClick()) {
      cur_state = STATE_MODES;
      if (num_colors_selected > 0) {
        pat.setColorset(new_colorset);
        save_cur_mode();
      }
      num_colors_selected = 0;
    }
  }
  if (menu_selection == 1) {
    if (Button::holdPressing()) {
      Led::strobe(150, 150, RGB_CORAL_ORANGE_BRI_LOWEST, RGB_WHITE);
    }
    if (Button::onHoldClick()) {
      addColorAndSave(RGB_WHITE, false);
    }
  }
}

void Helios::handle_state_color_variant_selection()
{
  if (Button::onShortClick()) {
    menu_selection = (menu_selection + 1) % NUM_COLORS_PER_GROUP;
  }
  selected_hue = color_menu_data[selected_base_group].hues[menu_selection];
  Led::set(HSVColor(selected_hue, 255, selected_val));
  if (Button::onLongClick()) {
    cur_state = STATE_COLOR_SELECT_BRIGHTNESS;
    menu_selection = 0;
    return;
  }
  if (Button::holdPressing()) {
    Led::strobe(150, 150, RGB_CORAL_ORANGE_BRI_LOWEST, HSVColor(selected_hue, 255, 255));
  }
  if (Button::onHoldClick()) {
    selected_val = 255;
    addColorAndSave(HSVColor(selected_hue, 255, selected_val), true);
  }
}

void Helios::handle_state_color_brightness_selection()
{
  if (Button::onShortClick()) {
    menu_selection = (menu_selection + 1) % 4;
  }
  static const uint8_t brightness_values[4] = {HSV_VAL_HIGH, HSV_VAL_MEDIUM, HSV_VAL_LOW, HSV_VAL_LOWEST};
  selected_val = brightness_values[menu_selection];
  Led::set(HSVColor(selected_hue, (menu_selection == 1 ? 0 : 255), selected_val)); // sat=0 for white
  bool saveAndFinish = Button::onLongClick() || Button::onHoldClick();
  if (saveAndFinish) {
    addColorAndSave(HSVColor(selected_hue, (menu_selection == 1 ? 0 : 255), selected_val), true);
  }
}

void Helios::handle_state_pat_select()
{
  if (Button::onLongClick()) {
    save_cur_mode();
    cur_state = STATE_MODES;
  }
  if (Button::onShortClick()) {
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
  toggle_flag(flag);
  // write out the new global flags and the current mode
  save_global_flags();
  // switch back to modes
  cur_state = STATE_MODES;
}

void Helios::handle_state_set_defaults()
{
  if (Button::onShortClick()) {
    menu_selection = !menu_selection;
  }
  // show low white for exit or red for select
  if (menu_selection) {
    Led::strobe(80, 20, RGB_RED_BRI_LOW, RGB_OFF);
  } else {
    Led::strobe(20, 10, RGB_WHITE_BRI_LOWEST, RGB_OFF);
  }
  // when the user long clicks a selection
  if (Button::onLongClick()) {
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
    Storage::write_pattern(i, pat);
  }
  // reset global flags
  global_flags = FLAG_NONE;
  cur_mode = 0;
  // save global flags
  save_global_flags();
  // re-load current mode
  load_cur_mode();
}

void Helios::addColorAndSave(const HSVColor &hsv, bool returnToGroup)
{
  new_colorset.addColor(hsv_to_rgb_generic(hsv)); // if needed, or if Colorset takes HSV, adjust
  num_colors_selected++;
  if (num_colors_selected >= NUM_COLOR_SLOTS) {
    pat.setColorset(new_colorset);
    save_cur_mode();
#if ALTERNATIVE_HSV_RGB == 1
    g_hsv_rgb_alg = HSV_TO_RGB_GENERIC;
#endif
    cur_state = STATE_MODES;
    return;
  } else {
    menu_selection = 0;
    if (returnToGroup) {
      cur_state = STATE_COLOR_GROUP_SELECTION;
    }
  }
}


void Helios::show_selection(RGBColor color)
{
  // only show selection while pressing the button
  if (!Button::isPressed()) {
    return;
  }
  uint16_t holdDur = (uint16_t)Button::holdDuration();
  // if the hold duration is outside the flashing range do nothing
  if (holdDur < SHORT_CLICK_THRESHOLD || holdDur >= HOLD_CLICK_START) {
    return;
  }
  Led::set(color);
}

