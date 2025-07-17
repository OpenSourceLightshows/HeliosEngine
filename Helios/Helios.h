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
    FLAG_NONE      = 0,
    FLAG_LOCKED    = (1 << 0),
    FLAG_CONJURE   = (1 << 1),
    FLAG_AUTOPLAY  = (1 << 2),

    FLAG_END,
    FLAG_INVALID   = (uint8_t)(~((1 << (FLAG_END - 1)) - 1))
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

  // the slot selection returns this info for internal menu logic
  enum ColorSelectOption {
    OPTION_NONE = 0,

    SELECTED_ADD,
    SELECTED_EXIT,
    SELECTED_SLOT
  };

  static void handle_off_menu(uint8_t mag, bool past);
  static void handle_on_menu(uint8_t mag, bool past);
  static void handle_state_color_selection();
  static void handle_state_color_group_selection();
  static void handle_state_color_variant_selection();
  static void handle_state_pat_select();
  static void handle_state_toggle_flag(Flags flag);
  static void handle_state_set_defaults();
  static void show_selection(RGBColor color);
  static void factory_reset();

  enum State : uint8_t {
    STATE_MODES,
    STATE_COLOR_GROUP_SELECTION,
    STATE_COLOR_VARIANT_SELECTION,
    STATE_PATTERN_SELECT,
    STATE_TOGGLE_CONJURE,
    STATE_TOGGLE_LOCK,
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
  // the group that was selected in color select
  static uint8_t selected_base_group;
  static uint8_t num_colors_selected;  // Track number of colors selected in current session
  static Pattern pat;
  static bool keepgoing;
  static uint32_t last_mode_switch_time;
  static Colorset new_colorset;

#ifdef HELIOS_CLI
  static bool sleeping;  // Only used in CLI mode
#endif
};
