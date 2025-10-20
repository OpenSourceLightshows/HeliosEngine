#include <stdint.h>

#include "Helios.h"

#include "ColorConstants.h"
#include "TimeControl.h"
#include "Storage.h"
#include "Pattern.h"
#include "Patterns.h"
#include "Random.h"
#include "Button.h"
#include "Led.h"

#ifdef HELIOS_EMBEDDED
#ifndef HELIOS_STM8
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#endif
#endif

#ifdef HELIOS_CLI
#include <stdio.h>
#endif

#include <stdlib.h>

// some internal macros that shouldn't change
// The number of menus in hue/sat/val selection
#define NUM_COLORS_PER_GROUP 4
// the number of color groups
#define NUM_COLOR_GROUPS 4
// the number of menus in group selection
#define NUM_MENUS_GROUP 7

// Forward declarations for internal functions
static uint8_t helios_init_components(void);
static void helios_handle_state(void);
static void helios_handle_state_modes(void);
static void helios_handle_off_menu(uint8_t mag, uint8_t past);
static void helios_handle_on_menu(uint8_t mag, uint8_t past);
static void helios_handle_state_color_selection(void);
static void helios_handle_state_color_group_selection(void);
static void helios_handle_state_col_select_hue_val(void);
static void helios_handle_state_pat_select(void);
static void helios_handle_state_toggle_flag(enum helios_flags flag);
static void helios_handle_state_set_defaults(void);
static void helios_show_selection(rgb_color_t color);
static void helios_factory_reset(void);

// the slot selection returns this info for internal menu logic
enum helios_color_select_option {
  OPTION_NONE = 0,

  SELECTED_ADD,
  SELECTED_EXIT,
  SELECTED_SLOT
};

enum helios_state {
  STATE_MODES,
  STATE_COLOR_GROUP_SELECTION,
  STATE_COLOR_SELECT_HUE,
  STATE_COLOR_SELECT_VAL,
  STATE_PATTERN_SELECT,
  STATE_TOGGLE_CONJURE,
  STATE_TOGGLE_LOCK,
  STATE_TOGGLE_LOCK_ON,
  STATE_SET_DEFAULTS,
#ifdef HELIOS_CLI
  STATE_SLEEP,
#endif
};

// static members
static enum helios_state cur_state;
static enum helios_flags global_flags;
static uint8_t menu_selection;
static uint8_t cur_mode;
static uint8_t selected_base_group;
static uint8_t selected_hue;
static uint8_t selected_val;
static uint8_t selected_sat;
static uint8_t num_colors_selected;  // 
static pattern_t pat;
static uint8_t keepgoing;
static uint32_t last_mode_switch_time;
static colorset_t new_colorset;

#ifdef HELIOS_CLI
static uint8_t sleeping;  // 
#endif

volatile char helios_version[] = HELIOS_VERSION_STR;

#ifdef HELIOS_STM8
// STM8-specific helper to get color without struct return - use pointer
static void helios_get_color_ptr(const colorset_t *set, uint8_t index, rgb_color_t *out)
{
  if (index >= set->m_numColors) {
    rgb_init3(out, 0, 0, 0);
    return;
  }
  *out = set->m_palette[index];
}
#endif

uint8_t helios_init(void)
{
  // first initialize all the components of helios
  if (!helios_init_components()) {
    return 0;
  }
  // then initialize the hardware for embedded helios
#ifdef HELIOS_EMBEDDED
#ifndef HELIOS_STM8
  // AVR hardware initialization
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
  // STM8 hardware initialization is done in stm8_init functions before main()
#endif
  return 1;
}

static uint8_t helios_init_components(void)
{
  // initialize various components of Helios
  if (!time_init()) {
    return 0;
  }
  if (!led_init()) {
    return 0;
  }
  if (!storage_init()) {
    return 0;
  }
  if (!button_init()) {
    return 0;
  }
  // initialize global variables
  cur_state = STATE_MODES;
  menu_selection = 0;
  cur_mode = 0;
  num_colors_selected = 0;
  selected_base_group = 0;
  keepgoing = 1;
  last_mode_switch_time = 0;
#ifdef HELIOS_CLI
  sleeping = 0;
#endif
  helios_load_global_flags();
  helios_load_cur_mode();
  return 1;
}

void helios_tick(void)
{
  // 
  button_update();

  // 
  helios_handle_state();

  // Update the Leds once per frame
  led_update();

  // 
  time_tick_clock();
}

void helios_enter_sleep(void)
{
#ifdef HELIOS_EMBEDDED
  // clear the led colors
  led_clear();
#ifdef HELIOS_STM8
  // STM8 - Enter Wait For Interrupt mode
  button_enable_wake();
  __asm__("wfi");  // Wait for interrupt (low power mode)
  // ... interrupt will make us wake here
  helios_init_components();
#else
  // AVR - Full power down sleep
  // Set all pins to input
  DDRB = 0x00;
  // Disable pull-ups on all pins
  PORTB = 0x00;
  // Enable wake on interrupt for the button
  button_enable_wake();
  // Set sleep mode to POWER DOWN mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  // enter sleep
  sleep_mode();
  // ... interrupt will make us wake here

  // Set PB0, PB1, PB4 as output
  DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB4);
  // wakeup here, re-init
  helios_init_components();
#endif
#else
  cur_state = STATE_SLEEP;
  // enable the sleep bool
  sleeping = 1;
  // Enable wake on button press/click for CLI
  button_enable_wake();
#endif
}

void helios_wakeup(void)
{
#ifdef HELIOS_EMBEDDED
  // nothing needed here, this interrupt firing will make the mainthread resume
#else
  // 
  uint8_t pressed = button_is_pressed();
  // re-initialize some stuff
  time_init();
  button_init();
  // so just re-press it
  if (pressed) {
    button_do_press();
  }
  cur_state = STATE_MODES;
  // turn off the sleeping flag that only CLI has
  sleeping = 0;
#endif
}

void helios_load_next_mode(void)
{
  // increment current mode and wrap around
  cur_mode = (uint8_t)(cur_mode + 1) % NUM_MODE_SLOTS;
  // now load current mode again
  helios_load_cur_mode();
}

void helios_load_cur_mode(void)
{
  // read pattern from storage at cur mode index
  if (!storage_read_pattern(cur_mode, &pat)) {
    // and just initialize default if it cannot be read
    patterns_make_default(cur_mode, &pat);
    // try to write it out because storage was corrupt
    storage_write_pattern(cur_mode, &pat);
  }
  // then re-initialize the pattern
  pattern_init_state(&pat);
  // Update the last mode switch time when loading a mode
  last_mode_switch_time = time_get_current_time();
}

void helios_save_cur_mode(void)
{
  storage_write_pattern(cur_mode, &pat);
}

void helios_load_global_flags(void)
{
  // read the global flags from index 0 config
  global_flags = (enum helios_flags)storage_read_global_flags();
  if (helios_has_any_flags((enum helios_flags)(FLAG_CONJURE | FLAG_LOCK_ON))) {
    // if conjure or lock on is enabled then load the current mode index from storage
    cur_mode = storage_read_current_mode();
  }
  // read the global brightness from index 2 config
  uint8_t saved_brightness = storage_read_brightness();
  // 
  uint8_t is_valid = !helios_has_any_flags(FLAGS_INVALID) && saved_brightness > 0;
  if (is_valid) {
    led_set_brightness(saved_brightness);
  }

  if (!is_valid) {
    // 
    helios_factory_reset();
  }
}

void helios_save_global_flags(void)
{
  storage_write_global_flags(global_flags);
  storage_write_current_mode(cur_mode);
}

void helios_set_mode_index(uint8_t mode_index)
{
  cur_mode = (uint8_t)mode_index % NUM_MODE_SLOTS;
  // now load current mode again
  helios_load_cur_mode();
}

uint8_t helios_keep_going(void)
{
  return keepgoing;
}

void helios_terminate(void)
{
  keepgoing = 0;
}

void helios_set_flag(enum helios_flags flag)
{
  global_flags = (enum helios_flags)(global_flags | flag);
}

uint8_t helios_has_flag(enum helios_flags flag)
{
  return (global_flags & flag) == flag;
}

uint8_t helios_has_any_flags(enum helios_flags flag)
{
  return (global_flags & flag) != 0;
}

void helios_clear_flag(enum helios_flags flag)
{
  global_flags = (enum helios_flags)(global_flags & ~flag);
}

void helios_toggle_flag(enum helios_flags flag)
{
  global_flags = (enum helios_flags)(global_flags ^ flag);
}

#ifdef HELIOS_CLI
uint8_t helios_is_asleep(void)
{
  return sleeping;
}

pattern_t *helios_cur_pattern(void)
{
  return &pat;
}
#endif

static void helios_handle_state(void)
{
  // check for the force sleep button hold regardless of which state we're in
  if (button_hold_duration() > FORCE_SLEEP_TIME) {
    // when released the device will just sleep
    if (button_on_release()) {
      helios_enter_sleep();
      // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
      return;
    }
    // but as long as it's held past the sleep time it just turns off the led
    if (button_is_pressed()) {
      led_clear();
      return;
    }
  }
  // otherwise just handle the state like normal
  switch (cur_state) {
    case STATE_MODES:
      helios_handle_state_modes();
      break;
    case STATE_COLOR_GROUP_SELECTION:
    case STATE_COLOR_SELECT_HUE:
    case STATE_COLOR_SELECT_VAL:
      helios_handle_state_color_selection();
      break;
    case STATE_PATTERN_SELECT:
      helios_handle_state_pat_select();
      break;
    case STATE_TOGGLE_CONJURE:
      helios_handle_state_toggle_flag(FLAG_CONJURE);
      break;
    case STATE_TOGGLE_LOCK:
      helios_handle_state_toggle_flag(FLAG_LOCKED);
      break;
    case STATE_TOGGLE_LOCK_ON:
      helios_handle_state_toggle_flag(FLAG_LOCK_ON);
      break;
    case STATE_SET_DEFAULTS:
      helios_handle_state_set_defaults();
      break;
#ifdef HELIOS_CLI
    case STATE_SLEEP:
      // simulate sleep in helios CLI
      if (button_on_press() || button_on_short_click() || button_on_long_click()) {
        helios_wakeup();
      }
      break;
#endif
  }
}

static void helios_handle_state_modes(void)
{
  // whether they have released the button since turning on
  uint8_t hasReleased = (button_release_count() > 0);

  if (button_release_count() > 1 && button_on_short_click()) {
    if (helios_has_flag(FLAG_CONJURE)) {
      helios_enter_sleep();
    } else if (helios_has_flag(FLAG_LOCK_ON)) {
      // when lock on is enabled, short clicks do nothing
      return;
    } else {
      helios_load_next_mode();
    }
    return;
  }

  // 
  if (helios_has_flag(FLAG_AUTOPLAY) && !button_is_pressed()) {
    uint32_t current_time = time_get_current_time();
    if (current_time - last_mode_switch_time >= AUTOPLAY_DURATION) {
      // 
      colorset_t *colorset = pattern_colorset_ptr(&pat);
      if (colorset_num_colors(colorset) <= 1 || colorset_on_start(colorset)) {
        helios_load_next_mode();
      }
    }
  }

  // check for lock and go back to sleep
  if (helios_has_flag(FLAG_LOCKED) && hasReleased && !button_on_release()) {
    helios_enter_sleep();
    // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
    return;
  }

  // check for lock on - device stays on but locked
  if (helios_has_flag(FLAG_LOCK_ON) && hasReleased && !button_on_release()) {
    // For lock on mode, always play the pattern unless we're in a long hold (menu access)
    uint32_t holdDur = button_hold_duration();
    uint8_t heldPast = (holdDur > SHORT_CLICK_THRESHOLD);
    if (!button_is_pressed() || !heldPast) {
      pattern_play(&pat);
      return;
    }
  }

  if (!helios_has_any_flags((enum helios_flags)(FLAG_LOCKED | FLAG_LOCK_ON)) && hasReleased) {
    // just play the current mode
    pattern_play(&pat);
  }
  // check how long the button is held
  uint32_t holdDur = button_hold_duration();
  // 
  uint8_t magnitude = (uint8_t)(holdDur / MENU_HOLD_TIME);
  // whether the user has held the button longer than a short click
  uint8_t heldPast = (holdDur > SHORT_CLICK_THRESHOLD);

  // flash red briefly when locked and short clicked (only for glow lock, not lock on)
  if (helios_has_flag(FLAG_LOCKED) && holdDur < SHORT_CLICK_THRESHOLD) {
    rgb_color_t color;
    rgb_init_from_raw(&color, RGB_RED_BRI_LOW);
    led_set_rgb(&color);
  }
  // if the button is held for at least 1 second
  if (button_is_pressed() && heldPast) {
    rgb_color_t color;
    // if the button has been released before then show the on menu
    if (hasReleased) {
      switch (magnitude) {
        default:
        case 0: led_clear(); break;                                                 // 
        case 1: rgb_init_from_raw(&color, RGB_TURQUOISE_BRI_LOW); led_set_rgb(&color); break;     // 
        case 2: rgb_init_from_raw(&color, RGB_MAGENTA_BRI_LOW); led_set_rgb(&color); break;       // 
        case 3: rgb_init_from_raw(&color, RGB_YELLOW_BRI_LOW); led_set_rgb(&color); break;        // 
        case 4: rgb_init_from_raw(&color, RGB_WHITE_BRI_LOW); led_set_rgb(&color); break;    // 
      }
    } else {
      if (helios_has_flag(FLAG_LOCKED)) {
        switch (magnitude) {
          default:
          case 0: led_clear(); break;
          case TIME_TILL_GLOW_LOCK_UNLOCK: rgb_init_from_raw(&color, RGB_RED_BRI_LOW); led_set_rgb(&color); break; // 
        }
      } else {
        switch (magnitude) {
          default:
          case 0: led_clear(); break;         // 
          case 1: rgb_init_from_raw(&color, RGB_RED_BRI_LOW); led_set_rgb(&color); break; // 
          case 2: rgb_init_from_raw(&color, RGB_BLUE_BRI_LOW); led_set_rgb(&color); break; // 
          case 3: {
            uint8_t autoplay = helios_has_flag(FLAG_AUTOPLAY);
            rgb_init_from_raw(&color, autoplay ? RGB_ORANGE_BRI_LOW : RGB_PINK_BRI_LOW);
            led_set_rgb(&color);
          } break; // 
        }
      }
    }
  }
  // if this isn't a release tick there's nothing more to do
  if (button_on_release()) {
    // Resets the menu selection before entering new state
    menu_selection = 0;
    if (heldPast && button_release_count() == 1) {
      helios_handle_off_menu(magnitude, heldPast);
      return;
    }
    // otherwise if we have released it then we are in the 'on' menu
    helios_handle_on_menu(magnitude, heldPast);
  }
}

static void helios_handle_off_menu(uint8_t mag, uint8_t past)
{
  (void)past; // 
  // if still locked then handle the unlocking menu which is just if mag == 5
  if (helios_has_flag(FLAG_LOCKED)) {
    switch (mag) {
      case TIME_TILL_GLOW_LOCK_UNLOCK:  // 
        cur_state = STATE_TOGGLE_LOCK;
        break;
      default:
        // just go back to sleep in hold-past off menu
        helios_enter_sleep();
        // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
    }
    // in this case we return either way, since we're locked
    return;
  }

  // if lock on is enabled, handle the unlocking menu
  if (helios_has_flag(FLAG_LOCK_ON)) {
    switch (mag) {
      case TIME_TILL_GLOW_LOCK_UNLOCK:  // 
        cur_state = STATE_TOGGLE_LOCK_ON;
        break;
      default:
        // stay on but locked - do not sleep
        return;
    }
    // in this case we return either way, since we're locked on
    return;
  }

  // otherwise if not locked handle the off menu
  switch (mag) {
    case 1:  // 
      cur_state = STATE_TOGGLE_LOCK;
      led_clear();
      return; // 
    case 2:  // 
      cur_state = STATE_SET_DEFAULTS;
      return; // 
    case 3:  // 
      helios_handle_state_toggle_flag(FLAG_AUTOPLAY);
      return; // 
    default:
      // just go back to sleep in hold-past off menu
      helios_enter_sleep();
      // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
      return;
  }
}

static void helios_handle_on_menu(uint8_t mag, uint8_t past)
{
  switch (mag) {
    case 0:  // 
      // but only if we held for more than a short click
      if (past) {
        helios_enter_sleep();
        // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
        return;
      }
      break;
    case 1:  // 
      cur_state = STATE_COLOR_GROUP_SELECTION;
      // reset the menu selection and colors selected
      menu_selection = 0;
      num_colors_selected = 0;
      // Store original colorset before clearing
      new_colorset = *pattern_colorset_ptr(&pat);
      // Clear existing colors in pattern
      colorset_clear(&new_colorset);
#if ALTERNATIVE_HSV_RGB == 1
      // use the nice hue to rgb rainbow
      g_hsv_rgb_alg = HSV_TO_RGB_RAINBOW;
#endif
      break;
    case 2:  // 
      cur_state = STATE_PATTERN_SELECT;
      // reset the menu selection
      menu_selection = 0;
      break;
    case 3:  // 
      cur_state = STATE_TOGGLE_CONJURE;
      led_clear();
      break;
    case 4:  // 
      cur_state = STATE_TOGGLE_LOCK_ON;
      led_clear();
      break;
    default:  // 
      break;
  }
}

struct colors_menu_data {
  uint8_t hues[NUM_COLOR_GROUPS];
};

// array of colors for selection
static const struct colors_menu_data color_menu_data[NUM_COLOR_GROUPS] = {
  // hue0           hue1              hue2          hue3
  // ==================================================================================
  { {HUE_RED,        HUE_CORAL_ORANGE, HUE_ORANGE,   HUE_YELLOW} },
  { {HUE_LIME_GREEN, HUE_GREEN,        HUE_SEAFOAM,  HUE_TURQUOISE} },
  { {HUE_ICE_BLUE,   HUE_LIGHT_BLUE,   HUE_BLUE,     HUE_ROYAL_BLUE} },
  { {HUE_PURPLE,     HUE_PINK,         HUE_HOT_PINK, HUE_MAGENTA} },
};

static void helios_handle_state_color_selection(void)
{
  switch (cur_state) {
    case STATE_COLOR_GROUP_SELECTION:
      // pick the hue group
      helios_handle_state_color_group_selection();
      break;
    case STATE_COLOR_SELECT_HUE:
    case STATE_COLOR_SELECT_VAL:
    default:
      // pick the hue and val
      helios_handle_state_col_select_hue_val();
      break;
  }
  // get the current color
  rgb_color_t cur = led_get();
  cur.red /= 2;
  cur.green /= 2;
  cur.blue /= 2;
  // show selection in all of these menus
  helios_show_selection(cur);
}

static void helios_handle_state_color_group_selection(void)
{
  if (button_on_short_click()) {
    menu_selection = (menu_selection + 1) % NUM_MENUS_GROUP;
  }
  uint8_t color_group = (menu_selection - 2) % NUM_COLOR_GROUPS;
  if (menu_selection > 5) {
    menu_selection = 0;
  }
  if (button_on_long_click()) {
    // select hue/val
    switch (menu_selection) {
      case 0:  // 
        // add blank to set
        colorset_add_color_hsv(&new_colorset, 0, 0, 0);
        num_colors_selected++;
        // Check if we've reached the maximum number of colors
        if (num_colors_selected >= NUM_COLOR_SLOTS) {
          pattern_set_colorset(&pat, &new_colorset);
          helios_save_cur_mode();
          num_colors_selected = 0;
          last_mode_switch_time = time_get_current_time(); // 
          cur_state = STATE_MODES;
          return;
        }
        cur_state = STATE_COLOR_GROUP_SELECTION;
        // RETURN HERE
        return;
      case 1:  // 
        // adds white, skip hue/sat to brightness
        selected_hue = 0;
        selected_sat = 0;
        selected_val = 255;
        menu_selection = 0;
        cur_state = STATE_COLOR_SELECT_VAL;
        return;
      default:  // 
        selected_base_group = color_group;
        selected_sat = 255;
        selected_val = 255;  // 
        cur_state = STATE_COLOR_SELECT_HUE;
        menu_selection = 0;
        return;
    }
    menu_selection = 0;
  }
  // default col1/col2 to off and white for the first two options
  rgb_color_t col1, col2;
  uint16_t on_dur, off_dur;
  rgb_init_from_raw(&col1, RGB_OFF);

  switch (menu_selection) {
    case 0: // 
      rgb_init_from_raw(&col2, RGB_WHITE_BRI_LOW);
      on_dur = 1;
      off_dur = 30;
      break;
    case 1: // 
      rgb_init_from_raw(&col2, RGB_WHITE);
      on_dur = 9;
      off_dur = 0;
      break;
    default: { // 
      hsv_color_t temp_hsv1, temp_hsv2;
      hsv_init3(&temp_hsv1, color_menu_data[color_group].hues[0], 255, 255);
      rgb_init_from_hsv(&col1, &temp_hsv1);
      hsv_init3(&temp_hsv2, color_menu_data[color_group].hues[2], 255, 255);
      rgb_init_from_hsv(&col2, &temp_hsv2);
      on_dur = 500;
      off_dur = 500;
    } break;
  }
  led_strobe(on_dur, off_dur, &col1, &col2);
  // show a white flash for the first two menus
  if (menu_selection <= 1) {
    rgb_color_t temp;
    rgb_init_from_raw(&temp, RGB_WHITE_BRI_LOW);
    helios_show_selection(temp);
  } else {
    // dim the color for the quad menus
    rgb_color_t cur = led_get();
    cur.red /= 2;
    cur.green /= 2;
    cur.blue /= 2;
    rgb_color_t temp;
    rgb_init_from_raw(&temp, RGB_WHITE_BRI_LOW);
    helios_show_selection(temp);
  }

  if (menu_selection == 0) {
    // If the user is on the blank option (menu_selection == 0) and holding, flash red to indicate they can save with current colors
    if (button_hold_pressing()) {
      // flash red to indicate save action is available
      rgb_color_t red, off;
      rgb_init_from_raw(&red, RGB_RED_BRI_LOW);
      rgb_init_from_raw(&off, RGB_OFF);
      led_strobe(150, 150, &red, &off);
    }

    if (button_on_hold_click()) {
      cur_state = STATE_MODES;
      if (num_colors_selected > 0) {
        pattern_set_colorset(&pat, &new_colorset);
        // Save with current colors if at least one color is selected
        helios_save_cur_mode();
      }
      num_colors_selected = 0;
      last_mode_switch_time = time_get_current_time(); // 
    }
  }
  if (menu_selection == 1) {
    if (button_hold_pressing()) {
      rgb_color_t coral, white;
      rgb_init_from_raw(&coral, RGB_CORAL_ORANGE_BRI_LOWEST);
      rgb_init_from_raw(&white, RGB_WHITE);
      led_strobe(150, 150, &coral, &white);
    }
    if (button_on_hold_click()) {
      colorset_add_color_hsv(&new_colorset, 0, 0, 255);
      num_colors_selected++;
      // Check if we've reached the maximum number of colors
      if (num_colors_selected >= NUM_COLOR_SLOTS) {
        pattern_set_colorset(&pat, &new_colorset);
        helios_save_cur_mode();
        num_colors_selected = 0;
        last_mode_switch_time = time_get_current_time(); // 
        cur_state = STATE_MODES;
        return;
      }
      cur_state = STATE_COLOR_GROUP_SELECTION;
      menu_selection = 0;
      return;
    }
  }
}

static void helios_handle_state_col_select_hue_val(void)
{
  // handle iterating to the next option
  if (button_on_short_click()) {
    menu_selection = (menu_selection + 1) % NUM_COLORS_PER_GROUP;
  }
  // 
  uint8_t gotoNextMenu = button_on_long_click();
  uint8_t saveAndFinish = button_on_hold_click();
  switch (cur_state) {
    default:
    case STATE_COLOR_SELECT_HUE:
      selected_hue = color_menu_data[selected_base_group].hues[menu_selection];
      break;
    case STATE_COLOR_SELECT_VAL: {
      static const uint8_t hsv_values[4] = {HSV_VAL_HIGH, HSV_VAL_MEDIUM, HSV_VAL_LOW, HSV_VAL_LOWEST};
      selected_val = hsv_values[menu_selection];
      // longclick becomes save and there is no next
      saveAndFinish = gotoNextMenu;
    } break;
  }
  // render current selection
  hsv_color_t hsv_sel;
  hsv_init3(&hsv_sel, selected_hue, selected_sat, selected_val);
  rgb_color_t rgb_sel;
  rgb_init_from_hsv(&rgb_sel, &hsv_sel);
  led_set_rgb(&rgb_sel);
  // show the long selection flash
  if (button_hold_pressing()) {
    rgb_color_t temp_col = led_get();
    rgb_color_t temp_col1;
    rgb_init_from_raw(&temp_col1, RGB_CORAL_ORANGE_SAT_LOWEST);
    led_strobe(150, 150, &temp_col1, &temp_col);
  }
  // check to see if we are holding to save and skip
  if (saveAndFinish) {
    colorset_add_color_hsv(&new_colorset, selected_hue, selected_sat, selected_val);
    num_colors_selected++;
    // Check if we've reached the maximum number of colors
    if (num_colors_selected >= NUM_COLOR_SLOTS) {
      pattern_set_colorset(&pat, &new_colorset);
      helios_save_cur_mode();
      num_colors_selected = 0;
      last_mode_switch_time = time_get_current_time(); // 
      cur_state = STATE_MODES;
      return;
    }
    menu_selection = 0;
    cur_state = STATE_COLOR_GROUP_SELECTION;
    return;
  }
  if (gotoNextMenu) {
    cur_state = (enum helios_state)(cur_state + 1);
    // reset the menu selection
    menu_selection = 0;
  }
}

static void helios_handle_state_pat_select(void)
{
  if (button_on_long_click()) {
    helios_save_cur_mode();
    last_mode_switch_time = time_get_current_time(); // 
    cur_state = STATE_MODES;
  }
  if (button_on_short_click()) {
    patterns_make_pattern((enum pattern_id)menu_selection, &pat);
    menu_selection = (menu_selection + 1) % PATTERN_COUNT;
    pattern_init_state(&pat);
  }
  pattern_play(&pat);
  rgb_color_t temp;
  rgb_init_from_raw(&temp, RGB_MAGENTA_BRI_LOW);
  helios_show_selection(temp);
}

static void helios_handle_state_toggle_flag(enum helios_flags flag)
{
  // toggle the conjure flag
  helios_toggle_flag(flag);
  // write out the new global flags and the current mode
  helios_save_global_flags();
  // switch back to modes
  last_mode_switch_time = time_get_current_time(); // 
  cur_state = STATE_MODES;
}

static void helios_handle_state_set_defaults(void)
{
  if (button_on_short_click()) {
    menu_selection = !menu_selection;
  }
  // show low white for exit or red for select
  if (menu_selection) {
    rgb_color_t red, off;
    rgb_init_from_raw(&red, RGB_RED_BRI_LOW);
    rgb_init_from_raw(&off, RGB_OFF);
    led_strobe(80, 20, &red, &off);
  } else {
    rgb_color_t white, off;
    rgb_init_from_raw(&white, RGB_WHITE_BRI_LOWEST);
    rgb_init_from_raw(&off, RGB_OFF);
    led_strobe(20, 10, &white, &off);
  }
  // when the user long clicks a selection
  if (button_on_long_click()) {
    // if the user actually selected 'yes'
    if (menu_selection == 1) {
      helios_factory_reset();
    }
    last_mode_switch_time = time_get_current_time(); // 
    cur_state = STATE_MODES;
  }
  rgb_color_t temp;
  rgb_init_from_raw(&temp, RGB_WHITE_BRI_LOW);
  helios_show_selection(temp);
}

static void helios_show_selection(rgb_color_t color)
{
  // only show selection while pressing the button
  if (!button_is_pressed()) {
    return;
  }
  uint16_t holdDur = (uint16_t)button_hold_duration();
  // if the hold duration is outside the flashing range do nothing
  if (holdDur < SHORT_CLICK_THRESHOLD || holdDur >= HOLD_CLICK_START) {
    return;
  }
  led_set_rgb(&color);
}

static void helios_factory_reset(void)
{
  uint8_t i;
  for (i = 0; i < NUM_MODE_SLOTS; ++i) {
    patterns_make_default(i, &pat);
    storage_write_pattern(i, &pat);
  }
  // Reset global brightness to default
  led_set_brightness(DEFAULT_BRIGHTNESS);
  storage_write_brightness(DEFAULT_BRIGHTNESS);
  // set global flags to autoplay
  global_flags = FLAG_AUTOPLAY;
  cur_mode = 0;
  // save global flags
  helios_save_global_flags();
  // re-load current mode
  helios_load_cur_mode();
}

