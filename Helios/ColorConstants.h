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

#define RGB_WARM_WHITE    (uint32_t)0xFFD700 // 255, 215, 0
#define RGB_MINT_GREEN   (uint32_t)0x00FFD7 // 0, 255, 215
#define RGB_SILVER      (uint32_t)0xC0C0C0 // 192, 192, 192
#define RGB_LUNAR_GRAY  (uint32_t)0x787878 // 120, 120, 120

// Helios Medium Brightness Colors
#define RGB_WHITE_BRI_MEDIUM         (uint32_t)0x787878 // 120 120 120
#define RGB_RED_BRI_MEDIUM           (uint32_t)0x780000 // 120, 0, 0
#define RGB_CORAL_ORANGE_BRI_MEDIUM  (uint32_t)0x780E00 // 120, 14, 0
#define RGB_ORANGE_BRI_MEDIUM        (uint32_t)0x781C00 // 120, 28, 0
#define RGB_YELLOW_BRI_MEDIUM        (uint32_t)0x783800 // 120, 56, 0
#define RGB_LIME_GREEN_BRI_MEDIUM    (uint32_t)0x297800 // 41, 120, 0
#define RGB_GREEN_BRI_MEDIUM         (uint32_t)0x007800 // 0, 120, 0
#define RGB_SEAFOAM_BRI_MEDIUM       (uint32_t)0x00781C // 0, 120, 28
#define RGB_TURQUOISE_BRI_MEDIUM     (uint32_t)0x007862 // 0, 120, 98
#define RGB_ICE_BLUE_BRI_MEDIUM      (uint32_t)0x004E78 // 0, 78, 120
#define RGB_LIGHT_BLUE_BRI_MEDIUM    (uint32_t)0x002178 // 0, 33, 120
#define RGB_BLUE_BRI_MEDIUM          (uint32_t)0x000078 // 0, 0, 120
#define RGB_ROYAL_BLUE_BRI_MEDIUM    (uint32_t)0x0D0078 // 13, 0, 120
#define RGB_PURPLE_BRI_MEDIUM        (uint32_t)0x3D0078 // 61, 0, 120
#define RGB_PINK_BRI_MEDIUM          (uint32_t)0x620078 // 98, 0, 120
#define RGB_HOT_PINK_BRI_MEDIUM      (uint32_t)0x780054 // 120, 0, 84
#define RGB_MAGENTA_BRI_MEDIUM       (uint32_t)0x78001C // 120, 0, 28

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
#define RGB_RED_BRI_LOWEST           (uint32_t)0x0A0000 // 10, 0, 0
#define RGB_CORAL_ORANGE_BRI_LOWEST  (uint32_t)0x0A0100 // 10, 1, 0
#define RGB_ORANGE_BRI_LOWEST        (uint32_t)0x0A0200 // 10, 2, 0
#define RGB_YELLOW_BRI_LOWEST        (uint32_t)0x0A0400 // 10, 4, 0
#define RGB_LIME_GREEN_BRI_LOWEST    (uint32_t)0x030A00 // 3, 10, 0
#define RGB_GREEN_BRI_LOWEST         (uint32_t)0x000A00 // 0, 10, 0
#define RGB_SEAFOAM_BRI_LOWEST       (uint32_t)0x000A02 // 0, 10, 2
#define RGB_TURQUOISE_BRI_LOWEST     (uint32_t)0x000A08 // 0, 10, 8
#define RGB_ICE_BLUE_BRI_LOWEST      (uint32_t)0x00060A // 0, 6, 10
#define RGB_LIGHT_BLUE_BRI_LOWEST    (uint32_t)0x00020A // 0, 2, 10
#define RGB_BLUE_BRI_LOWEST          (uint32_t)0x00000A // 0, 0, 10
#define RGB_ROYAL_BLUE_BRI_LOWEST    (uint32_t)0x01000A // 1, 0, 10
#define RGB_PURPLE_BRI_LOWEST        (uint32_t)0x05000A // 5, 0, 10
#define RGB_PINK_BRI_LOWEST          (uint32_t)0x08000A // 8, 0, 10
#define RGB_HOT_PINK_BRI_LOWEST      (uint32_t)0x0A0007 // 10, 0, 7
#define RGB_MAGENTA_BRI_LOWEST       (uint32_t)0x0A0002 // 10, 0, 2

// Helios Medium Saturation Colors
#define RGB_RED_SAT_MEDIUM           (uint32_t)0xFF2222 // 255, 34, 34
#define RGB_CORAL_ORANGE_SAT_MEDIUM  (uint32_t)0xFF3C22 // 255, 60, 34
#define RGB_ORANGE_SAT_MEDIUM        (uint32_t)0xFF5622 // 255, 86, 34
#define RGB_YELLOW_SAT_MEDIUM        (uint32_t)0xFF8A22 // 255, 138, 34
#define RGB_LIME_GREEN_SAT_MEDIUM    (uint32_t)0x6FFF22 // 111, 255, 34
#define RGB_GREEN_SAT_MEDIUM         (uint32_t)0x22FF22 // 34, 255, 34
#define RGB_SEAFOAM_SAT_MEDIUM       (uint32_t)0x22FF56 // 34, 255, 86
#define RGB_TURQUOISE_SAT_MEDIUM     (uint32_t)0x22FFD7 // 34, 255, 215
#define RGB_ICE_BLUE_SAT_MEDIUM      (uint32_t)0x22B3FF // 34, 179, 255
#define RGB_LIGHT_BLUE_SAT_MEDIUM    (uint32_t)0x2260FF // 34, 96, 255
#define RGB_BLUE_SAT_MEDIUM          (uint32_t)0x2222FF // 34, 34, 255
#define RGB_ROYAL_BLUE_SAT_MEDIUM    (uint32_t)0x3C22FF // 60, 34, 255
#define RGB_PURPLE_SAT_MEDIUM        (uint32_t)0x9422FF // 148, 34, 255
#define RGB_PINK_SAT_MEDIUM          (uint32_t)0xD822FF // 216, 34, 255
#define RGB_HOT_PINK_SAT_MEDIUM      (uint32_t)0xFF22BE // 255, 34, 190
#define RGB_MAGENTA_SAT_MEDIUM       (uint32_t)0xFF2256 // 255, 34, 86

// Helios Low Saturation Colors
#define RGB_RED_SAT_LOW              (uint32_t)0xFF5555 // 255, 85, 85
#define RGB_CORAL_ORANGE_SAT_LOW     (uint32_t)0xFF6955 // 255, 105, 85
#define RGB_ORANGE_SAT_LOW           (uint32_t)0xFF7D55 // 255, 125, 85
#define RGB_YELLOW_SAT_LOW           (uint32_t)0xFFA555 // 255, 165, 85
#define RGB_LIME_GREEN_SAT_LOW       (uint32_t)0x90FF55 // 144, 255, 85
#define RGB_GREEN_SAT_LOW            (uint32_t)0x55FF55 // 85, 255, 85
#define RGB_SEAFOAM_SAT_LOW          (uint32_t)0x55FF7D // 85, 255, 125
#define RGB_TURQUOISE_SAT_LOW        (uint32_t)0x55FFE0 // 85, 255, 224
#define RGB_ICE_BLUE_SAT_LOW         (uint32_t)0x55C4FF // 85, 196, 255
#define RGB_LIGHT_BLUE_SAT_LOW       (uint32_t)0x5584FF // 85, 132, 255
#define RGB_BLUE_SAT_LOW             (uint32_t)0x5555FF // 85, 85, 255
#define RGB_ROYAL_BLUE_SAT_LOW       (uint32_t)0x6855FF // 104, 85, 255
#define RGB_PURPLE_SAT_LOW           (uint32_t)0xAC55FF // 172, 85, 255
#define RGB_PINK_SAT_LOW             (uint32_t)0xE055FF // 224, 85, 255
#define RGB_HOT_PINK_SAT_LOW         (uint32_t)0xFF55CD // 255, 85, 205
#define RGB_MAGENTA_SAT_LOW          (uint32_t)0xFF557D // 255, 85, 125

// Helios Lowest Saturation Colors
#define RGB_RED_SAT_LOWEST           (uint32_t)0xFF7D7D // 255, 125, 125
#define RGB_CORAL_ORANGE_SAT_LOWEST  (uint32_t)0xFF8C7D // 255, 140, 125
#define RGB_ORANGE_SAT_LOWEST        (uint32_t)0xFF9B7D // 255, 155, 125
#define RGB_YELLOW_SAT_LOWEST        (uint32_t)0xFFBA7D // 255, 186, 125
#define RGB_LIME_GREEN_SAT_LOWEST    (uint32_t)0xAAFF7D // 170, 255, 125
#define RGB_GREEN_SAT_LOWEST         (uint32_t)0x7DFF7D // 125, 255, 125
#define RGB_SEAFOAM_SAT_LOWEST       (uint32_t)0x7DFF9B // 125, 255, 155
#define RGB_TURQUOISE_SAT_LOWEST     (uint32_t)0x7DFFE7 // 125, 255, 231
#define RGB_ICE_BLUE_SAT_LOWEST      (uint32_t)0x7DD2FF // 125, 210, 255
#define RGB_LIGHT_BLUE_SAT_LOWEST    (uint32_t)0x7DA1FF // 125, 161, 255
#define RGB_BLUE_SAT_LOWEST          (uint32_t)0x7D7DFF // 125, 125, 255
#define RGB_ROYAL_BLUE_SAT_LOWEST    (uint32_t)0x8B7DFF // 139, 125, 255
#define RGB_PURPLE_SAT_LOWEST        (uint32_t)0xBF7DFF // 191, 125, 255
#define RGB_PINK_SAT_LOWEST          (uint32_t)0xE87DFF // 232, 125, 255
#define RGB_HOT_PINK_SAT_LOWEST      (uint32_t)0xFF7DD8 // 255, 125, 216
#define RGB_MAGENTA_SAT_LOWEST       (uint32_t)0xFF7D9B // 255, 125, 155
