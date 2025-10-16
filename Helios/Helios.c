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

/* some internal macros that shouldn't change */
/* The number of menus in hue/sat/val selection */
#define NUM_COLORS_PER_GROUP 4
/* the number of color groups in the color selection menu */
#define NUM_COLOR_GROUPS 4
/* the number of menus in group selection */
#define NUM_MENUS_GROUP 8

/* Forward declarations for internal functions */
static uint8_t helios_init_components(void);
static void helios_handle_state(void);
static void helios_handle_state_modes(void);
static void helios_handle_off_menu(uint8_t mag, uint8_t past);
static void helios_handle_on_menu(uint8_t mag, uint8_t past);
static void helios_handle_state_color_selection(void);
static void helios_handle_state_color_group_selection(void);
static void helios_handle_state_color_variant_selection(void);
static void helios_handle_state_pat_select(void);
static void helios_handle_state_toggle_flag(enum helios_flags flag);
static void helios_handle_state_set_defaults(void);
static void helios_show_selection(rgb_color_t color);
static void helios_factory_reset(void);

/* the slot selection returns this info for internal menu logic */
enum helios_color_select_option {
  OPTION_NONE = 0,

  SELECTED_ADD,
  SELECTED_EXIT,
  SELECTED_SLOT
};

enum helios_state {
  STATE_MODES,
  STATE_COLOR_GROUP_SELECTION,
  STATE_COLOR_VARIANT_SELECTION,
  STATE_PATTERN_SELECT,
  STATE_TOGGLE_LOCK,
  STATE_SET_DEFAULTS,
#ifdef HELIOS_CLI
  STATE_SLEEP,
#endif
};

/* static members */
static enum helios_state cur_state;
static enum helios_flags global_flags;
static uint8_t menu_selection;
static uint8_t cur_mode;
static uint8_t selected_base_group;
static uint8_t num_colors_selected;  /* Track number of colors selected in current session */
static pattern_t pat;
static uint8_t keepgoing;
static uint32_t last_mode_switch_time;
static colorset_t new_colorset;

#ifdef HELIOS_CLI
static uint8_t sleeping;  /* Only used in CLI mode */
#endif

volatile char helios_version[] = HELIOS_VERSION_STR;

uint8_t helios_init(void)
{
  /* first initialize all the components of helios */
  if (!helios_init_components()) {
    return 0;
  }
  /* then initialize the hardware for embedded helios */
#ifdef HELIOS_EMBEDDED
  /* Set PB0, PB1, PB4 as output */
  DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB4);
  /* Timer0 Configuration for PWM */
  TCCR0A = (1 << WGM01) | (1 << WGM00) | (1 << COM0A1) | (1 << COM0B1);
  /* No prescaler */
  TCCR0B = (1 << CS00);
  /* Timer1 for PWM on PB4, Fast PWM, Non-inverting, No prescaler */
  TCCR1 = (1 << PWM1A) | (1 << COM1A1) | (1 << CS10);
  /* Enable PWM on OC1B */
  GTCCR = (1 << PWM1B) | (1 << COM1B1);
  /* Enable Timer0 overflow interrupt */
  TIMSK |= (1 << TOIE0);
  /* Enable interrupts */
  sei();
#endif
  return 1;
}

static uint8_t helios_init_components(void)
{
  /* initialize various components of Helios */
  if (!Time_init()) {
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
  /* initialize global variables */
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
  /* sample the button and re-calculate all button globals
   * the button globals should not change anywhere else */
  button_update();

  /* handle the current state of the system, ie whatever state
   * we're in we check for the appropriate input events for that
   * state by checking button globals, then run the appropriate logic */
  helios_handle_state();

  /* Update the Leds once per frame */
  led_update();

  /* finally tick the clock forward and then sleep till the entire
   * tick duration has been consumed */
  Time_tickClock();
}

void helios_enter_sleep(void)
{
#ifdef HELIOS_EMBEDDED
  /* clear the led colors */
  led_clear();
  /* Set all pins to input */
  DDRB = 0x00;
  /* Disable pull-ups on all pins */
  PORTB = 0x00;
  /* Enable wake on interrupt for the button */
  button_enable_wake();
  /* Set sleep mode to POWER DOWN mode */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  /* enter sleep */
  sleep_mode();
  /* ... interrupt will make us wake here */

  /* Set PB0, PB1, PB4 as output */
  DDRB |= (1 << DDB0) | (1 << DDB1) | (1 << DDB4);
  /* wakeup here, re-init */
  helios_init_components();
#else
  cur_state = STATE_SLEEP;
  /* enable the sleep bool */
  sleeping = 1;
#endif
}

void helios_wakeup(void)
{
#ifdef HELIOS_EMBEDDED
  /* nothing needed here, this interrupt firing will make the mainthread resume */
#else
  /* if the button was held down then they are entering off-menus
   * but if we re-initialize the button it will clear this state */
  uint8_t pressed = button_is_pressed();
  /* re-initialize some stuff */
  Time_init();
  button_init();
  /* so just re-press it */
  if (pressed) {
    button_do_press();
  }
  cur_state = STATE_MODES;
  /* turn off the sleeping flag that only CLI has */
  sleeping = 0;
#endif
}

void helios_load_next_mode(void)
{
  /* increment current mode and wrap around */
  cur_mode = (uint8_t)(cur_mode + 1) % NUM_MODE_SLOTS;
  /* now load current mode again */
  helios_load_cur_mode();
}

void helios_load_cur_mode(void)
{
  /* read pattern from storage at cur mode index */
  if (!storage_read_pattern(cur_mode, &pat)) {
    /* and just initialize default if it cannot be read */
    patterns_make_default(cur_mode, &pat);
    /* try to write it out because storage was corrupt */
    storage_write_pattern(cur_mode, &pat);
  }
  /* then re-initialize the pattern */
  pattern_init_state(&pat);
  /* Update the last mode switch time when loading a mode */
  last_mode_switch_time = Time_getCurtime();
}

void helios_save_cur_mode(void)
{
  storage_write_pattern(cur_mode, &pat);
}

void helios_load_global_flags(void)
{
  /* read the global flags from index 0 config */
  global_flags = (enum helios_flags)storage_read_global_flags();
  cur_mode = storage_read_current_mode();
}

void helios_save_global_flags(void)
{
  storage_write_global_flags(global_flags);
  storage_write_current_mode(cur_mode);
}

void helios_set_mode_index(uint8_t mode_index)
{
  cur_mode = (uint8_t)mode_index % NUM_MODE_SLOTS;
  /* now load current mode again */
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
  /* check for the force sleep button hold regardless of which state we're in */
  if (button_hold_duration() > FORCE_SLEEP_TIME) {
    /* when released the device will just sleep */
    if (button_on_release()) {
      helios_enter_sleep();
      /* ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE! */
      return;
    }
    /* but as long as it's held past the sleep time it just turns off the led */
    if (button_is_pressed()) {
      led_clear();
      return;
    }
  }
  /* otherwise just handle the state like normal */
  switch (cur_state) {
    case STATE_MODES:
      helios_handle_state_modes();
      break;
    case STATE_COLOR_GROUP_SELECTION:
    case STATE_COLOR_VARIANT_SELECTION:
      helios_handle_state_color_selection();
      break;
    case STATE_PATTERN_SELECT:
      helios_handle_state_pat_select();
      break;
    case STATE_TOGGLE_LOCK:
      helios_handle_state_toggle_flag(FLAG_LOCKED);
      break;
    case STATE_SET_DEFAULTS:
      helios_handle_state_set_defaults();
      break;
#ifdef HELIOS_CLI
    case STATE_SLEEP:
      /* simulate sleep in helios CLI */
      if (button_on_press() || button_on_short_click() || button_on_long_click()) {
        helios_wakeup();
      }
      break;
#endif
  }
}

static void helios_handle_state_modes(void)
{
  /* whether they have released the button since turning on */
  uint8_t hasReleased = (button_release_count() > 0);

  if (button_release_count() > 1 && button_on_short_click()) {
    helios_enter_sleep();
    return;
  }

  /* check for lock and go back to sleep */
  if (helios_has_flag(FLAG_LOCKED) && hasReleased && !button_on_release()) {
    helios_enter_sleep();
    /* ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE! */
    return;
  }

  if (!helios_has_flag(FLAG_LOCKED) && hasReleased) {
    /* just play the current mode */
    pattern_play(&pat);
  }
  /* check how long the button is held */
  uint32_t holdDur = button_hold_duration();
  /* calculate a magnitude which corresponds to how many times past the MENU_HOLD_TIME
   * the user has held the button, so 0 means haven't held fully past one yet, etc */
  uint8_t magnitude = (uint8_t)(holdDur / MENU_HOLD_TIME);
  /* whether the user has held the button longer than a short click */
  uint8_t heldPast = (holdDur > SHORT_CLICK_THRESHOLD);

  /* flash red briefly when locked and short clicked */
  if (helios_has_flag(FLAG_LOCKED) && holdDur < SHORT_CLICK_THRESHOLD) {
    rgb_color_t red;
    rgb_init_from_raw(&red, RGB_RED_BRI_LOW);
    led_set_rgb(&red);
  }
  /* if the button is held for at least 1 second */
  if (button_is_pressed() && heldPast) {
    rgb_color_t color;
    /* if the button has been released before then show the on menu */
    if (hasReleased) {
      switch (magnitude) {
        default:
        case 0: led_clear(); break;                                     /* Turn off */
        case 1: rgb_init_from_raw(&color, RGB_TURQUOISE_BRI_LOW); led_set_rgb(&color); break;  /* Color Selection */
        case 2: rgb_init_from_raw(&color, RGB_MAGENTA_BRI_LOW); led_set_rgb(&color); break;    /* Pattern Selection */
      }
    } else {
      if (helios_has_flag(FLAG_LOCKED)) {
        switch (magnitude) {
          default:
          case 0: led_clear(); break;
          case TIME_TILL_GLOW_LOCK_UNLOCK: rgb_init_from_raw(&color, RGB_RED_BRI_LOW); led_set_rgb(&color); break; /* Exit */
        }
      } else {
        switch (magnitude) {
          default:
          case 0: led_clear(); break;         /* nothing */
          case 1: rgb_init_from_raw(&color, RGB_RED_BRI_LOW); led_set_rgb(&color); break; /* Enter Glow Lock */
          case 2: rgb_init_from_raw(&color, RGB_BLUE_BRI_LOW); led_set_rgb(&color); break; /* Master Reset */
        }
      }
    }
  }
  /* if this isn't a release tick there's nothing more to do */
  if (button_on_release()) {
    /* Resets the menu selection before entering new state */
    menu_selection = 0;
    if (heldPast && button_release_count() == 1) {
      helios_handle_off_menu(magnitude, heldPast);
      return;
    }
    /* otherwise if we have released it then we are in the 'on' menu */
    helios_handle_on_menu(magnitude, heldPast);
  }
}

static void helios_handle_off_menu(uint8_t mag, uint8_t past)
{
  (void)past; /* unused */
  /* if still locked then handle the unlocking menu which is just if mag == 5 */
  if (helios_has_flag(FLAG_LOCKED)) {
    switch (mag) {
      case TIME_TILL_GLOW_LOCK_UNLOCK:  /* red lock */
        cur_state = STATE_TOGGLE_LOCK;
        break;
      default:
        /* just go back to sleep in hold-past off menu */
        helios_enter_sleep();
        /* ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE! */
    }
    /* in this case we return either way, since we're locked */
    return;
  }

  /* otherwise if not locked handle the off menu */
  switch (mag) {
    case 1:  /* red lock */
      cur_state = STATE_TOGGLE_LOCK;
      led_clear();
      return; /* RETURN HERE */
    case 2:  /* blue reset defaults */
      cur_state = STATE_SET_DEFAULTS;
      return; /* RETURN HERE */
    default:
      /* just go back to sleep in hold-past off menu */
      helios_enter_sleep();
      /* ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE! */
      return;
  }
}

static void helios_handle_on_menu(uint8_t mag, uint8_t past)
{
  switch (mag) {
    case 0:  /* off */
      /* but only if we held for more than a short click */
      if (past) {
        helios_enter_sleep();
        /* ALWAYS RETURN AFTER SLEEP! WE WILL WAKE HERE! */
        return;
      }
      break;
    case 1:  /* color select */
      cur_state = STATE_COLOR_GROUP_SELECTION;
      /* reset the menu selection and colors selected */
      menu_selection = 0;
      num_colors_selected = 0;
      /* Store original colorset before clearing */
      new_colorset = *pattern_colorset_ptr(&pat);
      /* Clear existing colors in pattern */
      colorset_clear(&new_colorset);
#if ALTERNATIVE_HSV_RGB == 1
      /* use the nice hue to rgb rainbow */
      g_hsv_rgb_alg = HSV_TO_RGB_RAINBOW;
#endif
      break;
    case 2:  /* pat select */
      cur_state = STATE_PATTERN_SELECT;
      /* reset the menu selection */
      menu_selection = 0;
      break;
    default:  /* hold past */
      break;
  }
}

struct colors_menu_data {
  uint32_t colors[4];
};

/* array of colors for selection */
static const struct colors_menu_data color_menu_data[NUM_COLOR_GROUPS] = {
  /* color0           color1              color2          color3 */
  /* =================================================================== */
  { {RGB_RED,        RGB_CORAL_ORANGE, RGB_ORANGE,   RGB_YELLOW} },
  { {RGB_LIME_GREEN, RGB_GREEN,        RGB_SEAFOAM,  RGB_TURQUOISE} },
  { {RGB_ICE_BLUE,   RGB_LIGHT_BLUE,   RGB_BLUE,     RGB_ROYAL_BLUE} },
  { {RGB_PURPLE,     RGB_PINK,         RGB_HOT_PINK, RGB_MAGENTA} },
};

static void helios_handle_state_color_selection(void)
{
  switch (cur_state) {
    case STATE_COLOR_GROUP_SELECTION:
      /* pick the hue group */
      helios_handle_state_color_group_selection();
      break;
    case STATE_COLOR_VARIANT_SELECTION:
      /* pick the hue */
      helios_handle_state_color_variant_selection();
      break;
    default:
      break;
  }
  /* get the current color */
  rgb_color_t cur = led_get();
  cur.red /= 2;
  cur.green /= 2;
  cur.blue /= 2;
  /* show selection in all of these menus */
  helios_show_selection(cur);
}

static void helios_handle_state_color_group_selection(void)
{
  rgb_color_t color;

  if (button_on_short_click()) {
    menu_selection = (menu_selection + 1) % NUM_COLOR_GROUPS;
  }

  /* Display a sample color from the selected group */
  rgb_init_from_raw(&color, color_menu_data[menu_selection].colors[0]);
  led_set_rgb(&color);

  if (button_on_long_click()) {
    selected_base_group = menu_selection;
    cur_state = STATE_COLOR_VARIANT_SELECTION;
    menu_selection = 0;
  }
}

static void helios_handle_state_color_variant_selection(void)
{
  rgb_color_t color;

  if (button_on_short_click()) {
    /* If we've selected max colors, next click exits */
    if (num_colors_selected >= NUM_COLOR_SLOTS) {
      /* Apply the newly built colorset */
      pattern_set_colorset(&pat, &new_colorset);
      /* Save and return to normal mode */
      helios_save_cur_mode();
      cur_state = STATE_MODES;
      menu_selection = 0;
#if ALTERNATIVE_HSV_RGB == 1
      g_hsv_rgb_alg = HSV_TO_RGB_GENERIC;
#endif
      return;
    }

    /* Cycle through colors in the group */
    menu_selection = (menu_selection + 1) % NUM_COLORS_PER_GROUP;
  }

  /* Display the currently selected color */
  rgb_init_from_raw(&color, color_menu_data[selected_base_group].colors[menu_selection]);
  led_set_rgb(&color);

  if (button_on_long_click()) {
    /* Add the selected color to the colorset */
    rgb_init_from_raw(&color, color_menu_data[selected_base_group].colors[menu_selection]);
    if (colorset_add_color(&new_colorset, color)) {
      num_colors_selected++;
    }

    /* If we've selected max colors, exit */
    if (num_colors_selected >= NUM_COLOR_SLOTS) {
      /* Apply the newly built colorset */
      pattern_set_colorset(&pat, &new_colorset);
      /* Save and return to normal mode */
      helios_save_cur_mode();
      cur_state = STATE_MODES;
      menu_selection = 0;
#if ALTERNATIVE_HSV_RGB == 1
      g_hsv_rgb_alg = HSV_TO_RGB_GENERIC;
#endif
      return;
    }

    /* Otherwise go back to group selection for next color */
    cur_state = STATE_COLOR_GROUP_SELECTION;
    menu_selection = 0;
  }
}

static void helios_handle_state_pat_select(void)
{
  rgb_color_t color;

  if (button_on_short_click()) {
    menu_selection = (menu_selection + 1) % PATTERN_COUNT;
  }

  /* show the menu selection */
  switch (menu_selection) {
    case 0: rgb_init_from_raw(&color, RGB_RED); break;
    case 1: rgb_init_from_raw(&color, RGB_GREEN); break;
    case 2: rgb_init_from_raw(&color, RGB_BLUE); break;
    case 3: rgb_init_from_raw(&color, RGB_YELLOW); break;
    case 4: rgb_init_from_raw(&color, RGB_MAGENTA); break;
    default: rgb_init_from_raw(&color, RGB_WHITE); break;
  }
  led_set_rgb(&color);

  if (button_on_long_click()) {
    /* make the selected pattern */
    patterns_make_pattern((enum pattern_id)(PATTERN_FIRST + menu_selection), &pat);
    /* reset the pattern to revert to on/off state */
    pattern_init_state(&pat);
    /* save and return to normal mode */
    helios_save_cur_mode();
    cur_state = STATE_MODES;
    menu_selection = 0;
  }
}

static void helios_handle_state_toggle_flag(enum helios_flags flag)
{
  rgb_color_t color;

  /* wait until button release then toggle the flag */
  if (button_on_release()) {
    helios_toggle_flag(flag);
    helios_save_global_flags();
    /* show feedback based on new state */
    if (helios_has_flag(flag)) {
      rgb_init_from_raw(&color, RGB_GREEN);
    } else {
      rgb_init_from_raw(&color, RGB_RED);
    }
    led_hold(&color);
    cur_state = STATE_MODES;
  }
}

static void helios_handle_state_set_defaults(void)
{
  rgb_color_t color;

  /* wait until button release then factory reset */
  if (button_on_release()) {
    helios_factory_reset();
    /* show feedback */
    rgb_init_from_raw(&color, RGB_BLUE);
    led_hold(&color);
    cur_state = STATE_MODES;
  }
}

static void helios_show_selection(rgb_color_t color)
{
  uint32_t time_since_click = Time_getCurtime();
  if (button_press_time() > 0) {
    time_since_click = Time_getCurtime() - button_press_time();
  }
  /* flash the selection color briefly after clicking */
  if (time_since_click < 150) {
    led_set_rgb(&color);
  }
}

static void helios_factory_reset(void)
{
  uint8_t slot;
  /* write default patterns to all slots */
  for (slot = 0; slot < NUM_MODE_SLOTS; ++slot) {
    patterns_make_default(slot, &pat);
    storage_write_pattern(slot, &pat);
  }
  /* clear all flags */
  global_flags = FLAG_NONE;
  helios_save_global_flags();
  /* reload current mode */
  helios_load_cur_mode();
}

