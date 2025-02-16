#ifndef HELIOS_CONFIG_H
#define HELIOS_CONFIG_H

// Version Configurations
//
// The engine major version indicates the state of the save file,
// if changes to the save format occur then the major version
// must increment so that the savefiles will not be loaded
#ifndef HELIOS_VERSION_MAJOR
#define HELIOS_VERSION_MAJOR 0
#endif

// A minor version simply indicates a bugfix or minor change that
// will not affect the save files produced by the engine. This means
// a savefile produced by 1.1 should be loadable by an engine on 1.2
// and vice versa, but an engine on 2.0 cannot share savefiles with
// either of the engines on version 1.1 or 1.2
#ifndef HELIOS_VERSION_MINOR
#define HELIOS_VERSION_MINOR 0
#endif

// The build or patch number based on the major.minor version, this is
// set by the build system using the number of commits since last version
#ifndef HELIOS_BUILD_NUMBER
#define HELIOS_BUILD_NUMBER 0
#endif

// Produces a number like 1.3.0
#ifndef HELIOS_VERSION_NUMBER
#define HELIOS_VERSION_NUMBER HELIOS_VERSION_MAJOR.HELIOS_VERSION_MINOR.HELIOS_BUILD_NUMBER
#endif


// Helios Version String
//
// This is the string literal equivalent of HELIOS_VERSION_NUMBER above
#define ADD_QUOTES(str) #str
#define EXPAND_AND_QUOTE(str) ADD_QUOTES(str)
#define HELIOS_VERSION_STR    EXPAND_AND_QUOTE(HELIOS_VERSION_NUMBER)

// Short Click Threshold
//
// The length of time in milliseconds for a click to
// be considered either a short or long click
#define SHORT_CLICK_THRESHOLD 400

// Selection Flash Duration
//
// How long the led flashes when selecting something
#define TIME_TILL_LONG_CLICK_FLASH 1000

// Unlock Glow Lock Duration
//
// How long the hold the button to unlock chip
#define TIME_TILL_GLOW_LOCK_UNLOCK 2

// Hold Click Start Threshold
//
// The minimum length a hold click can be
#define HOLD_CLICK_START (SHORT_CLICK_THRESHOLD + TIME_TILL_LONG_CLICK_FLASH)

// Hold Click End Threshold
//
// The maximum length a long click can be
#define HOLD_CLICK_END (HOLD_CLICK_START + TIME_TILL_LONG_CLICK_FLASH)

// Max Color Slots
//
// The number of slots in a colorset
#define NUM_COLOR_SLOTS 3

// Mode Slots
//
// The number of modes on the device
#define NUM_MODE_SLOTS 3


// Default Brightness
//
// The default brightness of the led
#define DEFAULT_BRIGHTNESS 255

// Tickrate
//
// The number of engine ticks per second
#define TICKRATE 1000

// Menu Hold Time
//
// How long the button must be held for the menus to open and cycle
// note this is a measure of ticks, but if the tickrate is 1000 then
// it is a measure of milliseconds
#define MENU_HOLD_TIME 1000

// Force Sleep Time
//
// The duration in ms/ticks to hold the button to force the chip to
// sleep at any location in the menus
#define FORCE_SLEEP_TIME 7000

// Autoplay Duration
//
// The duration in ms/ticks before switching to the next mode in autoplay
#define AUTOPLAY_DURATION 20000

// Alternative HSV to RGB
//
// This enabled the alternative HSV to RGB algorithm to be used in the
// color selection menu and provide a slightly different range of colors
#define ALTERNATIVE_HSV_RGB 0

// ============================================================================
//  Storage Constants
//
//  These are various storage sizes of data and some math to help
//  calculate sizes or space requirements, note these will produce
//  compiler errors unless you include the respective headers


// Storage Name
//
// This is mainly used by the CLI tool as a filename for simulated eeprom
#define STORAGE_FILENAME "Helios.storage"

// Storage Size
//
// The total size of storage where modes and global settings are saved.
// The EEPROM on attiny85 is 512 bytes, but due to limitations on flash
// only the lower half of the eeprom is being used
#define STORAGE_SIZE 256

// Colorset Size
//
// the colorset is just an array of colors but it also has a num colors val
#define COLORSET_SIZE ((sizeof(RGBColor) * NUM_COLOR_SLOTS) + 1)

// Pattern Args Size
//
// There is currently 6 args for a pattern: on, off, gap, dash, group, blend
// Each takes up 1 byte currently
#define PAT_ARGS_SIZE (sizeof(PatternArgs))

// Pattern Size
//
// The actual pattern storage size is the size of the colorset + params + 1 pat flags
#define PATTERN_SIZE (COLORSET_SIZE + PAT_ARGS_SIZE + 1)

// Slot Size
//
// the slot stores the pattern + 1 byte CRC
#define SLOT_SIZE (PATTERN_SIZE + 1)

// Some math to calculate storage sizes:
// 3 * 6 = 18 for the colorset
// 1 + 7 + 1 + 1 = 10 for the rest
//  = 28 bytes total for a pattern including CRC
//    -> 8 slots = 8 * 28 = 224
//      = 31 bytes left
//    -> 9 slots = 9 * 28 = 252
//      = 3 bytes left

// forbidden constant:
// #define HELIOS_ARDUINO 1



#endif
