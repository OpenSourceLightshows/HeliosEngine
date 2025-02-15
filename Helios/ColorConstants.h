#pragma once

#include <inttypes.h>

// ====================================================================================================
//  Color Constants
//
// This file defines constants for colors as we best see them for the Helios
// Engine

// Pre-defined hue values
#define HUE_RED           0
#define HUE_CORAL_ORANGE  5
#define HUE_ORANGE        10
#define HUE_YELLOW        20

#define HUE_LIME_GREEN    70
#define HUE_GREEN         85
#define HUE_SEAFOAM       95
#define HUE_TURQUOISE     120

#define HUE_ICE_BLUE      142
#define HUE_LIGHT_BLUE    158
#define HUE_BLUE          170
#define HUE_ROYAL_BLUE    175

#define HUE_PURPLE        192
#define HUE_PINK          205
#define HUE_HOT_PINK      225
#define HUE_MAGENTA       245

// Helios Colors
#define RGB_OFF          (uint32_t)0x000000 // 0 0 0
#define RGB_WHITE        (uint32_t)0xFFFFFF // 255 255 255
#define RGB_RED          (uint32_t)0xFF0000 // 255, 0, 0
#define RGB_CORAL_ORANGE (uint32_t)0xFF1E00 // 255, 30, 0
#define RGB_ORANGE       (uint32_t)0xFF3C00 // 255, 60, 0
#define RGB_YELLOW       (uint32_t)0xFF7800 // 255, 120, 0
#define RGB_LIME_GREEN   (uint32_t)0x59FF00 // 89, 255, 0
#define RGB_GREEN        (uint32_t)0x00FF00 // 0, 255, 0
#define RGB_SEAFOAM      (uint32_t)0x00FF3C // 0, 255, 60
#define RGB_TURQUOISE    (uint32_t)0x00FFD1 // 0, 255, 209
#define RGB_ICE_BLUE     (uint32_t)0x00A7FF // 0, 167, 255
#define RGB_LIGHT_BLUE   (uint32_t)0x0047FF // 0, 71, 255
#define RGB_BLUE         (uint32_t)0x0000FF // 0, 0, 255
#define RGB_ROYAL_BLUE   (uint32_t)0x1D00FF // 29, 0, 255
#define RGB_PURPLE       (uint32_t)0x8300FF // 131, 0, 255
#define RGB_PINK         (uint32_t)0xD200FF // 210, 0, 255
#define RGB_HOT_PINK     (uint32_t)0xFF00B4 // 255, 0, 180
#define RGB_MAGENTA      (uint32_t)0xFF003C // 255, 0, 60


#define RGB_CREAM        (uint32_t)0xFF932A // 255, 147, 42  - Warm cream with golden undertones
#define RGB_CORAL        (uint32_t)0xFF2813 // 255, 40, 19   - Deep coral with rich red base
#define RGB_CYAN         (uint32_t)0x00E5FA // 0, 255, 255   - Pure cyan, equal mix of green and blue

#define RGB_MINT         (uint32_t)0x40C090 // 64, 192, 144  - More green-blue tint, less white
#define RGB_LUNA         (uint32_t)0x4050A0 // 64, 80, 160   - Deeper blue tint, less gray

// Helios Low Brightness Colors
#define RGB_WHITE_BRI_LOW            (uint32_t)0x3C3C3C // 60 60 60
#define RGB_RED_BRI_LOW              (uint32_t)0x3C0000 // 60, 0, 0
#define RGB_CORAL_ORANGE_BRI_LOW     (uint32_t)0x3C0700 // 60, 7, 0
#define RGB_ORANGE_BRI_LOW           (uint32_t)0x3C0E00 // 60, 14, 0
#define RGB_YELLOW_BRI_LOW           (uint32_t)0x3C1C00 // 60, 28, 0
#define RGB_LIME_GREEN_BRI_LOW       (uint32_t)0x143C00 // 20, 60, 0
#define RGB_GREEN_BRI_LOW            (uint32_t)0x003C00 // 0, 60, 0
#define RGB_SEAFOAM_BRI_LOW          (uint32_t)0x003C0E // 0, 60, 14
#define RGB_TURQUOISE_BRI_LOW        (uint32_t)0x003C31 // 0, 60, 49
#define RGB_ICE_BLUE_BRI_LOW         (uint32_t)0x00273C // 0, 39, 60
#define RGB_LIGHT_BLUE_BRI_LOW       (uint32_t)0x00103C // 0, 16, 60
#define RGB_BLUE_BRI_LOW             (uint32_t)0x00003C // 0, 0, 60
#define RGB_ROYAL_BLUE_BRI_LOW       (uint32_t)0x06003C // 6, 0, 60
#define RGB_PURPLE_BRI_LOW           (uint32_t)0x1E003C // 30, 0, 60
#define RGB_PINK_BRI_LOW             (uint32_t)0x31003C // 49, 0, 60
#define RGB_HOT_PINK_BRI_LOW         (uint32_t)0x3C002A // 60, 0, 42
#define RGB_MAGENTA_BRI_LOW          (uint32_t)0x3C000E // 60, 0, 14

// Helios Lowest Brightness Colors
#define RGB_WHITE_BRI_LOWEST         (uint32_t)0x0A0A0A // 10 10 10