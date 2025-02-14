#include <stdint.h>

#include "HeliosConfig.h"
#include "Colorset.h"
#include "Pattern.h"

class Helios
{
public:
  static bool init();
  static void tick();

  static void enter_sleep();
  static void wakeup();

  static bool keep_going() { return keepgoing; }
  static void terminate() { keepgoing = false; }

  static void load_next_mode();
  static void load_cur_mode();
  static void save_cur_mode();
  static void load_global_flags();
  static void save_global_flags();
  static void set_mode_index(uint8_t mode_index);

#ifdef HELIOS_CLI
  static bool is_asleep() { return sleeping; }
  static Pattern &cur_pattern() { return pat; }
#endif

  enum Flags : uint8_t {
    FLAG_NONE = 0,
    FLAG_LOCKED = (1 << 0),
    FLAG_CONJURE = (1 << 1),
  };

  // get/set global flags
  static void set_flag(Flags flag) { global_flags = (Flags)(global_flags | flag); }
  static bool has_flag(Flags flag) { return (global_flags & flag) == flag; }
  static void clear_flag(Flags flag) { global_flags = (Flags)(global_flags & ~flag); }
  static void toggle_flag(Flags flag) { global_flags = (Flags)(global_flags ^ flag); }

private:
  // initialize the various components of helios
  static bool init_components();

  static void handle_state();
  static void handle_state_modes();
  static void handle_state_color_select();
  static void handle_state_pat_select();
  static void handle_state_toggle_flag(Flags flag);
  static void handle_state_set_defaults();
  static void show_selection(RGBColor color);
  static void factory_reset();

  static void handle_off_menu(uint8_t mag, bool past);
  static void handle_on_menu(uint8_t mag, bool past);

  enum State : uint8_t {
    STATE_MODES,
    STATE_COLOR_SELECT,
    STATE_PATTERN_SELECT,
    STATE_TOGGLE_CONJURE,
    STATE_SET_DEFAULTS,
#ifdef HELIOS_CLI
    STATE_SLEEP,
#endif
  };

  // the current state of the system
  static State cur_state;
  // global flags for the entire system
  static Flags global_flags;
  static uint8_t menu_selection;
  static uint8_t cur_mode;
  // the quadrant that was selected in color select
  static uint8_t selected_slot;
  static uint8_t selected_base_quad;
  static uint8_t selected_hue;
  static uint8_t selected_sat;
  static uint8_t selected_val;
  static RGBColor current_color_set[NUM_COLOR_SLOTS];
  static PatternArgs default_args[6];
  static Colorset default_colorsets[6];
  static Pattern pat;
  static bool keepgoing;

#ifdef HELIOS_CLI
  static bool sleeping;
#endif
};
