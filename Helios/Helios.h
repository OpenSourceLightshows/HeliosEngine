#include <stdint.h>

#include "HeliosConfig.h"
#include "Colorset.h"
#include "Pattern.h"
#include "Storage.h"
#include "Led.h"
#include "TimeControl.h"
#include "Button.h"
#include "HeliosCallbacks.h"

class Helios
{
public:
  Helios();
  bool init();
  void tick();

  void enter_sleep();
  void wakeup();

  bool keep_going() const { return keepgoing; }
  void terminate() { keepgoing = false; }

  void load_next_mode();
  void load_cur_mode();
  void save_cur_mode();
  void load_global_flags();
  void save_global_flags();
  void set_mode_index(uint8_t mode_index);

#ifdef HELIOS_CLI
  bool is_asleep() const { return sleeping; }
#endif
  Pattern &cur_pattern() { return pat; }
  const Pattern &cur_pattern() const { return pat; }
  Storage &storage() { return m_storage; }
  Led &led() { return m_led; }
  const Led &led() const { return m_led; }
  Time &time() { return m_time; }
  const Time &time() const { return m_time; }
  Button &button() { return m_button; }
  const Button &button() const { return m_button; }
  void setCallbacks(HeliosCallbacks *callbacks);
  HeliosCallbacks &callbacks() { return *m_callbacks; }
  const HeliosCallbacks &callbacks() const { return *m_callbacks; }

  enum Flags : uint8_t {
    // No flags are set
    FLAG_NONE     = 0,

    // The device is locked and must be unlocked to turn on
    FLAG_LOCKED   = (1 << 0),
    // Conjure mode is enabled, one click will toggle power
    FLAG_CONJURE  = (1 << 1),
    // Autoplay is enabled, modes will automatically cycle
    FLAG_AUTOPLAY = (1 << 2),
    // Add new flags here, max 8 flags

    // ==============================================
    // Auto increment to count the number of flags
    INTERNAL_FLAGS_END,
    // Calculate mask for invalid Flags based on the
    // inverse of all flags listed above here
    FLAGS_INVALID = (uint8_t)(~((1 << (INTERNAL_FLAGS_END - 1)) - 1))
  };

  // get/set global flags
  void set_flags(Flags flag) { global_flags = (Flags)(global_flags | flag); }
  bool has_flags(Flags flag) const { return (global_flags & flag) == flag; }
  bool has_any_flags(Flags flag) const { return (global_flags & flag) != FLAG_NONE; }
  void clear_flags(Flags flag) { global_flags = (Flags)(global_flags & ~flag); }
  void toggle_flags(Flags flag) { global_flags = (Flags)(global_flags ^ flag); }

private:
  // initialize the various components of helios
  bool init_components();

  void handle_state();
  void handle_state_modes();

  // the slot selection returns this info for internal menu logic
  enum ColorSelectOption {
    OPTION_NONE = 0,

    SELECTED_ADD,
    SELECTED_EXIT,
    SELECTED_SLOT
  };

  void handle_off_menu(uint8_t mag, bool past);
  void handle_on_menu(uint8_t mag, bool past);
  void handle_state_col_select();
  void handle_state_col_select_slot(ColorSelectOption &out_option);
  void handle_state_col_select_quadrant();
  void handle_state_col_select_hue_sat_val();
  void handle_state_pat_select();
  void handle_state_toggle_flag(Flags flag);
  void handle_state_set_defaults();
  void handle_state_set_global_brightness();
  void handle_state_shift_mode();
  void handle_state_randomize();
  void show_selection(RGBColor color);
  void factory_reset();

  enum State : uint8_t {
    STATE_MODES,
    STATE_COLOR_SELECT_SLOT,
    STATE_COLOR_SELECT_QUADRANT,
    STATE_COLOR_SELECT_HUE,
    STATE_COLOR_SELECT_SAT,
    STATE_COLOR_SELECT_VAL,
    STATE_PATTERN_SELECT,
    STATE_TOGGLE_CONJURE,
    STATE_TOGGLE_LOCK,
    STATE_SET_DEFAULTS,
    STATE_SET_GLOBAL_BRIGHTNESS,
    STATE_SHIFT_MODE,
    STATE_RANDOMIZE,
#ifdef HELIOS_CLI
    STATE_SLEEP,
#endif
  };

  // the current state of the system
  State cur_state;
  // global flags for the entire system
  Flags global_flags;
  uint8_t menu_selection;
  uint8_t cur_mode;
  // the quadrant that was selected in color select
  uint8_t selected_slot;
  uint8_t selected_base_quad;
  uint8_t selected_hue;
  uint8_t selected_sat;
  uint8_t selected_val;
  PatternArgs default_args[6];
  Colorset default_colorsets[6];
  Pattern pat;
  Storage m_storage;
  Led m_led;
  Time m_time;
  Button m_button;
  HeliosCallbacks m_defaultCallbacks;
  HeliosCallbacks *m_callbacks;
  bool keepgoing;

#ifdef HELIOS_CLI
  bool sleeping;
#endif
};

#ifdef HELIOS_EMBEDDED
extern Helios helios;
#endif
