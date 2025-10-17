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
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#endif

#ifdef HELIOS_CLI
#include <stdio.h>
#endif

#include <stdlib.h>

/* Internal macros */
#define NUM_MENUS_HUE_SAT_VAL 4
#define NUM_MENUS_QUADRANT 7

/* Color select options for internal menu logic */
enum color_select_option {
  OPTION_NONE = 0,
  SELECTED_ADD,
  SELECTED_EXIT,
  SELECTED_SLOT
};

/* Global state variables */
static enum helios_state g_cur_state;
static enum helios_flags g_global_flags;
static uint8_t g_menu_selection;
static uint8_t g_cur_mode;
static uint8_t g_selected_slot;
static uint8_t g_selected_base_quad;
static uint8_t g_selected_hue;
static uint8_t g_selected_sat;
static uint8_t g_selected_val;
static pattern_t g_pat;
static uint8_t g_keepgoing;

#ifdef HELIOS_CLI
static uint8_t g_sleeping;
#endif

volatile char helios_version[] = HELIOS_VERSION_STR;

/* Forward declarations for internal helper functions */
static uint8_t helios_init_components(void);
static void helios_handle_state(void);
static void helios_handle_state_modes(void);
static void helios_handle_off_menu(uint8_t mag, uint8_t past);
static void helios_handle_on_menu(uint8_t mag, uint8_t past);
static void helios_handle_state_col_select(void);
static void helios_handle_state_col_select_slot(enum color_select_option *out_option);
static void helios_handle_state_col_select_quadrant(void);
static void helios_handle_state_col_select_hue_sat_val(void);
static void helios_handle_state_pat_select(void);
static void helios_handle_state_toggle_flag(enum helios_flags flag);
static void helios_handle_state_set_defaults(void);
static void helios_factory_reset(void);
static void helios_handle_state_set_global_brightness(void);
static void helios_handle_state_shift_mode(void);
static void helios_handle_state_randomize(void);
static void helios_show_selection(rgb_color_t color);

uint8_t helios_init(void)
{
  // first initialize all the components of helios
  if (!helios_init_components()) {
    return 0;
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
  g_cur_state = STATE_MODES;
  g_menu_selection = 0;
  g_cur_mode = 0;
  g_selected_slot = 0;
  g_selected_base_quad = 0;
  g_keepgoing = 1;
#ifdef HELIOS_CLI
  g_sleeping = 0;
#endif
  // load global flags, and brightness from storage, this
  // includes for example conjure mode and the mode index
  // of the conjure mode if it is enabled
  helios_load_global_flags();
  // finally load whatever current mode index is selected
  // this might be mode 0, or for example a separate index
  // if conjure mode is enabled
  helios_load_cur_mode();
  return 1;
}

void helios_tick(void)
{
  // sample the button and re-calculate all button globals
  // the button globals should not change anywhere else
  button_update();

  // handle the current state of the system, ie whatever state
  // we're in we check for the appropriate input events for that
  // state by checking button globals, then run the appropriate logic
  helios_handle_state();

  // Update the Leds once per frame
  led_update();

  // finally tick the clock forward and then sleep till the entire
  // tick duration has been consumed
  time_tick_clock();
}

void helios_enter_sleep(void)
{
#ifdef HELIOS_EMBEDDED
  // clear the led colors
  led_clear();
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
#else
  g_cur_state = STATE_SLEEP;
  // enable the sleep uint8_t
  g_sleeping = 1;
#endif
}

void helios_wakeup(void)
{
#ifdef HELIOS_EMBEDDED
  // nothing needed here, this interrupt firing will make the mainthread resume
#else
  // if the button was held down then they are entering off-menus
  // but if we re-initialize the button it will clear this state
  uint8_t pressed = button_is_pressed();
  // re-initialize some stuff
  time_init();
  button_init();
  // so just re-press it
  if (pressed) {
    button_do_press();
  }
  g_cur_state = STATE_MODES;
  // turn off the sleeping flag that only CLI has
  g_sleeping = 0;
#endif
}

void helios_load_next_mode(void)
{
  // increment current mode and wrap around
  g_cur_mode = (uint8_t)(g_cur_mode + 1) % NUM_MODE_SLOTS;
  // now load current mode again
  helios_load_cur_mode();
}

void helios_load_cur_mode(void)
{
  // read pattern from storage at cur mode index
  if (!storage_read_pattern(g_cur_mode, &g_pat)) {
    // and just initialize default if it cannot be read
    patterns_make_default(g_cur_mode, &g_pat);
    // try to write it out because storage was corrupt
    storage_write_pattern(g_cur_mode, &g_pat);
  }
  // then re-initialize the pattern
  pattern_init_state(&g_pat);
}

void helios_save_cur_mode(void)
{
  storage_write_pattern(g_cur_mode, &g_pat);
}

void helios_load_global_flags(void)
{
  // read the global flags from index 0 config
  g_global_flags = (enum helios_flags)storage_read_global_flags();
  if (helios_has_flags(FLAG_CONJURE)) {
    // if conjure is enabled then load the current mode index from storage
    g_cur_mode = storage_read_current_mode();
  }
  // read the global brightness from index 2 config
  uint8_t saved_brightness = storage_read_brightness();
  // Check if flags are valid (FLAGS_INVALID is inverse mask of valid flags)
  // and brightness is set in storage
  uint8_t is_valid = !helios_has_any_flags(FLAGS_INVALID) && saved_brightness > 0;
  if (is_valid) {
    led_set_brightness(saved_brightness);
  }

  if (!is_valid) {
    // if the brightness was 0 and the flags are invalid then the storage was likely
    // uninitialized or corrupt so write out the defaults
    helios_factory_reset();
  }
}

void helios_save_global_flags(void)
{
  storage_write_global_flags(g_global_flags);
  storage_write_current_mode(g_cur_mode);
}

void helios_set_mode_index(uint8_t mode_index)
{
  g_cur_mode = (uint8_t)mode_index % NUM_MODE_SLOTS;
  // now load current mode again
  helios_load_cur_mode();
}

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
  switch (g_cur_state) {
    case STATE_MODES:
      helios_handle_state_modes();
      break;
    case STATE_COLOR_SELECT_SLOT:
    case STATE_COLOR_SELECT_QUADRANT:
    case STATE_COLOR_SELECT_HUE:
    case STATE_COLOR_SELECT_SAT:
    case STATE_COLOR_SELECT_VAL:
      helios_handle_state_col_select();
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
    case STATE_SET_DEFAULTS:
      helios_handle_state_set_defaults();
      break;
    case STATE_SET_GLOBAL_BRIGHTNESS:
      helios_handle_state_set_global_brightness();
      break;
    case STATE_SHIFT_MODE:
      helios_handle_state_shift_mode();
      break;
    case STATE_RANDOMIZE:
      helios_handle_state_randomize();
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
    if (helios_has_flags(FLAG_CONJURE)) {
      helios_enter_sleep();
    } else {
      helios_load_next_mode();
    }
    return;
  }

  // check for lock and go back to sleep
  if (helios_has_flags(FLAG_LOCKED) && hasReleased && !button_on_release()) {
    helios_enter_sleep();
    // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
    return;
  }

  if (!helios_has_flags(FLAG_LOCKED) && hasReleased) {
    // just play the current mode
    pattern_play(&g_pat);
  }
  // check how long the button is held
  uint32_t holdDur = button_hold_duration();
  // calculate a magnitude which corresponds to how many times past the MENU_HOLD_TIME
  // the user has held the button, so 0 means haven't held fully past one yet, etc
  uint8_t magnitude = (uint8_t)(holdDur / MENU_HOLD_TIME);
  // whether the user has held the button longer than a short click
  uint8_t heldPast = (holdDur > SHORT_CLICK_THRESHOLD);

  // flash red briefly when locked and short clicked
  if (helios_has_flags(FLAG_LOCKED) && !heldPast) {
    rgb_color_t temp;
    rgb_init_from_raw(&temp, RGB_RED_BRI_LOW);
    led_set_rgb(&temp);
  }
  // if the button is held for at least 1 second
  if (button_is_pressed() && heldPast) {
    // if the button has been released before then show the on menu
    if (hasReleased) {
      switch (magnitude) {
        default:
        case 0: led_clear(); break;                                     // Turn off
        case 1: led_set_rgb3(0, 0x3c, 0x31); break;                     // Color Selection
        case 2: led_set_rgb3(0x3c, 0, 0x0e); break;                     // Pattern Selection
        case 3: led_set_rgb3(0x3c, 0x1c, 0); break;                     // Conjure Mode
        case 4: led_set_rgb3(0x3c, 0x3c, 0x3c); break;                  // Shift Mode
        case 5: {  // Randomizer
          hsv_color_t hsv_temp;
          rgb_color_t rgb_temp;
          hsv_init3(&hsv_temp, (uint8_t)time_get_current_time(), 255, 100);
          rgb_init_from_hsv(&rgb_temp, &hsv_temp);
          led_set_rgb(&rgb_temp);
        } break;
      }
    } else {
      if (helios_has_flags(FLAG_LOCKED)) {
        switch (magnitude) {
          default:
          case 0: led_clear(); break;
          case TIME_TILL_GLOW_LOCK_UNLOCK: led_set_rgb3(0x3c, 0, 0); break; // Exit
        }
      } else {
        switch (magnitude) {
          default:
          case 0: led_clear(); break;                  // nothing
          case 1: led_set_rgb3(0x3c, 0, 0); break;     // Enter Glow Lock
          case 2: led_set_rgb3(0, 0x3c, 0); break;     // Global Brightness
          case 3: led_set_rgb3(0, 0, 0x3c); break;     // Master Reset
        }
      }
    }
  }
  // if this isn't a release tick there's nothing more to do
  if (button_on_release()) {
    // Resets the menu selection before entering new state
    g_menu_selection = 0;
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
  // if still locked then handle the unlocking menu which is just if mag == 5
  if (helios_has_flags(FLAG_LOCKED)) {
    switch (mag) {
      case TIME_TILL_GLOW_LOCK_UNLOCK:  // red lock
        g_cur_state = STATE_TOGGLE_LOCK;
        break;
      default:
        // just go back to sleep in hold-past off menu
        helios_enter_sleep();
        // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
    }
    // in this case we return either way, since we're locked
    return;
  }

  // otherwise if not locked handle the off menu
  switch (mag) {
    case 1:  // red lock
      g_cur_state = STATE_TOGGLE_LOCK;
      led_clear();
      return; // RETURN HERE
    case 2:  // green global brightness
      g_cur_state = STATE_SET_GLOBAL_BRIGHTNESS;
      return; // RETURN HERE
    case 3:  // blue reset defaults
      g_cur_state = STATE_SET_DEFAULTS;
      return; //RETURN HERE
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
    case 0:  // off
      // but only if we held for more than a short click
      if (past) {
        helios_enter_sleep();
        // ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE!
        return;
      }
      break;
    case 1:  // color select
      g_cur_state = STATE_COLOR_SELECT_SLOT;
      // reset the menu selection
      g_menu_selection = 0;
#if ALTERNATIVE_HSV_RGB == 1
      // use the nice hue to rgb rainbow
      g_hsv_rgb_alg = HSV_TO_RGB_RAINBOW;
#endif
      break;
    case 2:  // g_pat select
      g_cur_state = STATE_PATTERN_SELECT;
      // reset the menu selection
      g_menu_selection = 0;
      break;
    case 3:  // conjure mode
      g_cur_state = STATE_TOGGLE_CONJURE;
      led_clear();
      break;
    case 4:  // shift mode down
      g_cur_state = STATE_SHIFT_MODE;
      break;
    case 5:  // randomizer
      g_cur_state = STATE_RANDOMIZE;
      break;
    default:  // hold past
      break;
  }
}

static void helios_handle_state_col_select(void)
{
  enum color_select_option slot_option = OPTION_NONE;
  switch (g_cur_state) {
    case STATE_COLOR_SELECT_SLOT:
      // pick the target colorset slot
      helios_handle_state_col_select_slot(&slot_option);
      break;
    case STATE_COLOR_SELECT_QUADRANT:
      // pick the hue quadrant
      helios_handle_state_col_select_quadrant();
      break;
    case STATE_COLOR_SELECT_HUE:
    case STATE_COLOR_SELECT_SAT:
    case STATE_COLOR_SELECT_VAL:
    default:
      // pick the hue sat or val
      helios_handle_state_col_select_hue_sat_val();
      break;
  }
  // get the current color
  rgb_color_t cur = led_get();
  cur.red /= 2;
  cur.green /= 2;
  cur.blue /= 2;
  // this is a stupid override for when we're exiting color select
  // show a white selection instead
  if (slot_option != OPTION_NONE) {
    rgb_init_from_raw(&cur, RGB_WHITE_BRI_LOW);
  }
  // show selection in all of these menus
  helios_show_selection(cur);
}

static void helios_handle_state_col_select_slot(enum color_select_option *out_option)
{
  colorset_t *set = &g_pat.m_colorset;
  uint8_t num_cols = colorset_num_colors(set);

  if (button_on_short_click()) {
    // the number of menus in slot selection = all colors + exit
    uint8_t num_menus = num_cols + 1;
    // except if the number of colors is less than total color slots
    if (num_cols < NUM_COLOR_SLOTS) {
      // then we have another menu: add color
      num_menus++;
    }
    g_menu_selection = (g_menu_selection + 1) % num_menus;
  }

  uint8_t long_click = button_on_long_click();

  // Reset the color selection variables, these are the hue/sat/val that have been selected
  // in the following menus, this is a weird place to reset these but it ends up being the only
  // place where it can be written once and still handle all the possible cases it needs to run
  g_selected_sat = 255;
  g_selected_val = 255;

  if (num_cols < NUM_COLOR_SLOTS && g_menu_selection == num_cols) {
    // add color
    *out_option = SELECTED_ADD;
    rgb_color_t temp_col1, temp_col2;
    rgb_init_from_raw(&temp_col1, RGB_WHITE_BRI_LOW);
    rgb_init_from_raw(&temp_col2, RGB_OFF);
    led_strobe(100, 100, &temp_col1, &temp_col2);
    if (long_click) {
      g_selected_slot = g_menu_selection;
    }
  } else if (g_menu_selection == num_cols + 1 || (num_cols == NUM_COLOR_SLOTS && g_menu_selection == num_cols)) {
    // exit
    *out_option = SELECTED_EXIT;
    rgb_color_t temp_col1, temp_col2;
    rgb_init_from_raw(&temp_col1, RGB_RED_BRI_LOW);
    rgb_init_from_raw(&temp_col2, RGB_OFF);
    led_strobe(60, 40, &temp_col1, &temp_col2);
    if (long_click) {
#if ALTERNATIVE_HSV_RGB == 1
      // restore hsv to rgb algorithm type, done color selection
      g_hsv_rgb_alg = HSV_TO_RGB_GENERIC;
#endif
      helios_save_cur_mode();
      g_cur_state = STATE_MODES;
      return;
    }
  } else {
    *out_option = SELECTED_SLOT;
    g_selected_slot = g_menu_selection;
    // render current selection
    rgb_color_t col = colorset_get(set, g_selected_slot);
    rgb_color_t empty_col;
    rgb_init_from_raw(&empty_col, RGB_OFF);
    if (rgb_equals(&col, &empty_col)) {
      rgb_color_t temp_col1, temp_col2;
      rgb_init_from_raw(&temp_col1, RGB_OFF);
      rgb_init_from_raw(&temp_col2, RGB_WHITE_BRI_LOW);
      led_strobe(1, 30, &temp_col1, &temp_col2);
    } else {
      rgb_color_t temp_col1;
      rgb_init_from_raw(&temp_col1, RGB_OFF);
      led_strobe(3, 30, &temp_col1, &col);
    }
    if (button_hold_pressing()) {
      // flash red
      rgb_color_t temp_col1;
      rgb_init_from_raw(&temp_col1, RGB_RED_BRI_LOW);
      led_strobe(150, 150, &temp_col1, &col);
    }
    if (button_on_hold_click()){
      colorset_remove_color(set, g_selected_slot);
      return;
    }
  }
  if (long_click) {
    g_cur_state = (enum helios_state)(g_cur_state + 1);
    // reset the menu selection
    g_menu_selection = 0;
  }
}

struct colors_menu_data {
  uint8_t hues[4];
};
// array of hues for selection
static const struct colors_menu_data color_menu_data[4] = {
  // hue0           hue1              hue2          hue3
  // ==================================================================================
  { { HUE_RED,        HUE_CORAL_ORANGE, HUE_ORANGE,   HUE_YELLOW } },
  { { HUE_LIME_GREEN, HUE_GREEN,        HUE_SEAFOAM,  HUE_TURQUOISE } },
  { { HUE_ICE_BLUE,   HUE_LIGHT_BLUE,   HUE_BLUE,     HUE_ROYAL_BLUE } },
  { { HUE_PURPLE,     HUE_PINK,         HUE_HOT_PINK, HUE_MAGENTA } },
};

static void helios_handle_state_col_select_quadrant(void)
{
  if (button_on_short_click()) {
    g_menu_selection = (g_menu_selection + 1) % NUM_MENUS_QUADRANT;
  }

  uint8_t hue_quad = (g_menu_selection - 2) % 4;
  if (g_menu_selection > 5) {
    g_menu_selection = 0;
  }

  if (button_on_long_click()) {
    // select hue/sat/val
    switch (g_menu_selection) {
      case 0: {  // selected blank
        // add blank to set
        rgb_color_t blank_col;
        rgb_init_from_raw(&blank_col, RGB_OFF);
        colorset_set(&g_pat.m_colorset, g_selected_slot, blank_col);
        // Return to the slot you were editing
        g_menu_selection = g_selected_slot;
        // go to slot selection - 1 because we will increment outside here
        g_cur_state = STATE_COLOR_SELECT_SLOT;
        // RETURN HERE
        return;
      }
      case 1:  // selected white
        // adds white, skip hue/sat to brightness
        g_selected_sat = 0;
        g_menu_selection = 0;
        g_cur_state = STATE_COLOR_SELECT_VAL;
        // RETURN HERE
        return;
      default:  // 2-5
        g_selected_base_quad = hue_quad;
        break;
    }
  }

  // default col1/col2 to off and white for the first two options
  rgb_color_t col1, col2;
  uint16_t on_dur, off_dur;
  rgb_init_from_raw(&col1, RGB_OFF);

  switch (g_menu_selection) {
    case 0: // Blank Option
      rgb_init_from_raw(&col2, RGB_WHITE_BRI_LOW);
      on_dur = 1;
      off_dur = 30;
      break;
    case 1: // White Option
      rgb_init_from_raw(&col2, RGB_WHITE);
      on_dur = 9;
      off_dur = 0;
      break;
    default: { // Color options
      hsv_color_t temp_hsv1, temp_hsv2;
      hsv_init3(&temp_hsv1, color_menu_data[hue_quad].hues[0], 255, 255);
      rgb_init_from_hsv(&col1, &temp_hsv1);
      hsv_init3(&temp_hsv2, color_menu_data[hue_quad].hues[2], 255, 255);
      rgb_init_from_hsv(&col2, &temp_hsv2);
      on_dur = 500;
      off_dur = 500;
    } break;
  }
  led_strobe(on_dur, off_dur, &col1, &col2);
  // show a white flash for the first two menus
  if (g_menu_selection <= 1) {
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
  if (button_on_long_click()) {
    g_cur_state = (enum helios_state)(g_cur_state + 1);
    // reset the menu selection
    g_menu_selection = 0;
  }
}

static void helios_handle_state_col_select_hue_sat_val(void)
{
  // handle iterating to the next option
  if (button_on_short_click()) {
    g_menu_selection = (g_menu_selection + 1) % NUM_MENUS_HUE_SAT_VAL;
  }
  // in the sat/val selection a longclick is next and hold is save but in
  // the final val selection a longclick is save and there's no next
  uint8_t gotoNextMenu = button_on_long_click();
  uint8_t saveAndFinish = button_on_hold_click();
  switch (g_cur_state) {
    default:
    case STATE_COLOR_SELECT_HUE:
      g_selected_hue = color_menu_data[g_selected_base_quad].hues[g_menu_selection];
      break;
    case STATE_COLOR_SELECT_SAT: {
      static const uint8_t saturation_values[4] = {HSV_SAT_HIGH, HSV_SAT_MEDIUM, HSV_SAT_LOW, HSV_SAT_LOWEST};
      g_selected_sat = saturation_values[g_menu_selection];
    } break;
    case STATE_COLOR_SELECT_VAL: {
      static const uint8_t hsv_values[4] = {HSV_VAL_HIGH, HSV_VAL_MEDIUM, HSV_VAL_LOW, HSV_VAL_LOWEST};
      g_selected_val = hsv_values[g_menu_selection];
      // longclick becomes save and there is no next
      saveAndFinish = gotoNextMenu;
    } break;
  }
  // render current selection
  hsv_color_t hsv_sel;
  hsv_init3(&hsv_sel, g_selected_hue, g_selected_sat, g_selected_val);
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
    g_cur_state = STATE_COLOR_SELECT_SLOT;
    hsv_color_t hsv_final;
    hsv_init3(&hsv_final, g_selected_hue, g_selected_sat, g_selected_val);
    rgb_color_t rgb_final;
    rgb_init_from_hsv(&rgb_final, &hsv_final);
    pattern_update_color(&g_pat, g_selected_slot, &rgb_final);
    helios_save_cur_mode();
    // Return to the slot you were editing
    g_menu_selection = g_selected_slot;
    return;
  }
  if (gotoNextMenu) {
    g_cur_state = (enum helios_state)(g_cur_state + 1);
    // reset the menu selection
    g_menu_selection = 0;
  }
}

static void helios_handle_state_pat_select(void)
{
  if (button_on_long_click()) {
    helios_save_cur_mode();
    g_cur_state = STATE_MODES;
  }
  if (button_on_short_click()) {
    patterns_make_pattern((enum pattern_id)g_menu_selection, &g_pat);
    g_menu_selection = (g_menu_selection + 1) % PATTERN_COUNT;
    pattern_init_state(&g_pat);
  }
  pattern_play(&g_pat);
  rgb_color_t temp;
  rgb_init_from_raw(&temp, RGB_MAGENTA_BRI_LOW);
  helios_show_selection(temp);
}

static void helios_handle_state_toggle_flag(enum helios_flags flag)
{
  // toggle the conjure flag
  helios_toggle_flags(flag);
  // write out the new global flags and the current mode
  helios_save_global_flags();
  // switch back to modes
  g_cur_state = STATE_MODES;
}

static void helios_handle_state_set_defaults(void)
{
  if (button_on_short_click()) {
    g_menu_selection = !g_menu_selection;
  }
  // show low white for exit or red for select
  rgb_color_t temp_col1, temp_col2;
  if (g_menu_selection) {
    rgb_init_from_raw(&temp_col1, RGB_RED_BRI_LOW);
    rgb_init_from_raw(&temp_col2, RGB_OFF);
    led_strobe(80, 20, &temp_col1, &temp_col2);
  } else {
    rgb_init_from_raw(&temp_col1, RGB_WHITE_BRI_LOWEST);
    rgb_init_from_raw(&temp_col2, RGB_OFF);
    led_strobe(20, 10, &temp_col1, &temp_col2);
  }
  // when the user long clicks a selection
  if (button_on_long_click()) {
    // if the user actually selected 'yes'
    if (g_menu_selection == 1) {
      helios_factory_reset();
    }
    g_cur_state = STATE_MODES;
  }
  rgb_color_t temp;
  rgb_init_from_raw(&temp, RGB_WHITE_BRI_LOW);
  helios_show_selection(temp);
}

static void helios_factory_reset(void)
{
  for (uint8_t i = 0; i < NUM_MODE_SLOTS; ++i) {
    patterns_make_default(i, &g_pat);
    storage_write_pattern(i, &g_pat);
  }
  // Reset global brightness to default
  led_set_brightness(DEFAULT_BRIGHTNESS);
  storage_write_brightness(DEFAULT_BRIGHTNESS);
  // reset global flags
  g_global_flags = FLAG_NONE;
  g_cur_mode = 0;
  // save global flags
  helios_save_global_flags();
  // re-load current mode
  helios_load_cur_mode();
}

static void helios_handle_state_set_global_brightness(void)
{
  if (button_on_short_click()) {
    g_menu_selection = (g_menu_selection + 1) % NUM_BRIGHTNESS_OPTIONS;
  }
  // show different levels of green for each selection
  uint8_t col = 0;
  uint8_t brightness = 0;
  switch (g_menu_selection) {
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
  led_set_rgb3(0, col, 0);
  // when the user long clicks a selection
  if (button_on_long_click()) {
    // set the brightness based on the selection
    led_set_brightness(brightness);
    storage_write_brightness(brightness);
    g_cur_state = STATE_MODES;
  }
  rgb_color_t temp;
  rgb_init_from_raw(&temp, RGB_WHITE_BRI_LOW);
  helios_show_selection(temp);
}

static void helios_handle_state_shift_mode(void)
{
  uint8_t new_mode = (g_cur_mode > 0) ? (uint8_t)(g_cur_mode - 1) : (uint8_t)(NUM_MODE_SLOTS - 1);
  // copy the storage from the new position into our current position
  storage_copy_slot(new_mode, g_cur_mode);
  // point at the new position
  g_cur_mode = new_mode;
  // write out the current mode to the newly updated position
  helios_save_cur_mode();
  g_cur_state = STATE_MODES;
}

static void helios_handle_state_randomize(void)
{
  if (button_on_short_click()) {
    colorset_t *cur_set = &g_pat.m_colorset;
    random_t ctx;
    random_init_seed(&ctx, pattern_crc32(&g_pat));
    uint8_t randVal = random_next8(&ctx, 0, 255);
    colorset_randomize_colors(cur_set, &ctx, (randVal + 1) % NUM_COLOR_SLOTS, COLOR_MODE_RANDOMLY_PICK);
    patterns_make_pattern((enum pattern_id)(randVal % PATTERN_COUNT), &g_pat);
    pattern_init_state(&g_pat);
  }
  if (button_on_long_click()) {
    helios_save_cur_mode();
    g_cur_state = STATE_MODES;
  }
  pattern_play(&g_pat);
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

/* Flag manipulation functions */
void helios_set_flags(enum helios_flags flag)
{
  g_global_flags = (enum helios_flags)(g_global_flags | flag);
}

uint8_t helios_has_flags(enum helios_flags flag)
{
  return (g_global_flags & flag) == flag;
}

uint8_t helios_has_any_flags(enum helios_flags flag)
{
  return (g_global_flags & flag) != 0;
}

void helios_clear_flags(enum helios_flags flag)
{
  g_global_flags = (enum helios_flags)(g_global_flags & ~flag);
}

void helios_toggle_flags(enum helios_flags flag)
{
  g_global_flags = (enum helios_flags)(g_global_flags ^ flag);
}

uint8_t helios_keep_going(void)
{
  return g_keepgoing;
}

void helios_terminate(void)
{
  g_keepgoing = 0;
}

#ifdef HELIOS_CLI
uint8_t helios_is_asleep(void)
{
  return g_sleeping;
}

pattern_t *helios_cur_pattern(void)
{
  return &g_pat;
}
#endif
