#!/usr/bin/env python3
"""
Analyze ColorConstants.h to check for color collisions at different bit depths
"""

def rgb24_to_rgb16(r, g, b):
    """Convert 24-bit RGB to 16-bit RGB565"""
    r5 = (r >> 3) & 0x1F  # 5 bits for red
    g6 = (g >> 2) & 0x3F  # 6 bits for green
    b5 = (b >> 3) & 0x1F  # 5 bits for blue
    return (r5 << 11) | (g6 << 5) | b5

def rgb24_to_rgb8(r, g, b):
    """Convert 24-bit RGB to 8-bit RGB332"""
    r3 = (r >> 5) & 0x07  # 3 bits for red
    g3 = (g >> 5) & 0x07  # 3 bits for green
    b2 = (b >> 6) & 0x03  # 2 bits for blue
    return (r3 << 5) | (g3 << 2) | b2

def parse_rgb(hex_val):
    """Parse RGB hex value to components"""
    r = (hex_val >> 16) & 0xFF
    g = (hex_val >> 8) & 0xFF
    b = hex_val & 0xFF
    return r, g, b

# Define all colors from ColorConstants.h
colors = {
    # Basic colors
    "RGB_OFF": 0x000000,
    "RGB_WHITE": 0xFFFFFF,
    "RGB_RED": 0xFF0000,
    "RGB_CORAL_ORANGE": 0xFF1E00,
    "RGB_ORANGE": 0xFF3C00,
    "RGB_YELLOW": 0xFF7800,
    "RGB_LIME_GREEN": 0x59FF00,
    "RGB_GREEN": 0x00FF00,
    "RGB_SEAFOAM": 0x00FF3C,
    "RGB_TURQUOISE": 0x00FFD1,
    "RGB_ICE_BLUE": 0x00A7FF,
    "RGB_LIGHT_BLUE": 0x0047FF,
    "RGB_BLUE": 0x0000FF,
    "RGB_ROYAL_BLUE": 0x1D00FF,
    "RGB_PURPLE": 0x8300FF,
    "RGB_PINK": 0xD200FF,
    "RGB_HOT_PINK": 0xFF00B4,
    "RGB_MAGENTA": 0xFF003C,
    "RGB_CREAM": 0xFF932A,
    "RGB_CORAL": 0xFF2813,
    "RGB_CYAN": 0x00E5FA,
    "RGB_MINT": 0x9DFF8C,
    "RGB_LUNA": 0x4050A0,

    # Medium brightness
    "RGB_WHITE_BRI_MEDIUM": 0x787878,
    "RGB_RED_BRI_MEDIUM": 0x780000,
    "RGB_CORAL_ORANGE_BRI_MEDIUM": 0x780E00,
    "RGB_ORANGE_BRI_MEDIUM": 0x781C00,
    "RGB_YELLOW_BRI_MEDIUM": 0x783800,
    "RGB_LIME_GREEN_BRI_MEDIUM": 0x297800,
    "RGB_GREEN_BRI_MEDIUM": 0x007800,
    "RGB_SEAFOAM_BRI_MEDIUM": 0x00781C,
    "RGB_TURQUOISE_BRI_MEDIUM": 0x007862,
    "RGB_ICE_BLUE_BRI_MEDIUM": 0x004E78,
    "RGB_LIGHT_BLUE_BRI_MEDIUM": 0x002178,
    "RGB_BLUE_BRI_MEDIUM": 0x000078,
    "RGB_ROYAL_BLUE_BRI_MEDIUM": 0x0D0078,
    "RGB_PURPLE_BRI_MEDIUM": 0x3D0078,
    "RGB_PINK_BRI_MEDIUM": 0x620078,
    "RGB_HOT_PINK_BRI_MEDIUM": 0x780054,
    "RGB_MAGENTA_BRI_MEDIUM": 0x78001C,

    # Low brightness
    "RGB_WHITE_BRI_LOW": 0x3C3C3C,
    "RGB_RED_BRI_LOW": 0x3C0000,
    "RGB_CORAL_ORANGE_BRI_LOW": 0x3C0700,
    "RGB_ORANGE_BRI_LOW": 0x3C0E00,
    "RGB_YELLOW_BRI_LOW": 0x3C1C00,
    "RGB_LIME_GREEN_BRI_LOW": 0x143C00,
    "RGB_GREEN_BRI_LOW": 0x003C00,
    "RGB_SEAFOAM_BRI_LOW": 0x003C0E,
    "RGB_TURQUOISE_BRI_LOW": 0x003C31,
    "RGB_ICE_BLUE_BRI_LOW": 0x00273C,
    "RGB_LIGHT_BLUE_BRI_LOW": 0x00103C,
    "RGB_BLUE_BRI_LOW": 0x00003C,
    "RGB_ROYAL_BLUE_BRI_LOW": 0x06003C,
    "RGB_PURPLE_BRI_LOW": 0x1E003C,
    "RGB_PINK_BRI_LOW": 0x31003C,
    "RGB_HOT_PINK_BRI_LOW": 0x3C002A,
    "RGB_MAGENTA_BRI_LOW": 0x3C000E,

    # Lowest brightness
    "RGB_WHITE_BRI_LOWEST": 0x0A0A0A,
    "RGB_RED_BRI_LOWEST": 0x0A0000,
    "RGB_CORAL_ORANGE_BRI_LOWEST": 0x0A0100,
    "RGB_ORANGE_BRI_LOWEST": 0x0A0200,
    "RGB_YELLOW_BRI_LOWEST": 0x0A0400,
    "RGB_LIME_GREEN_BRI_LOWEST": 0x030A00,
    "RGB_GREEN_BRI_LOWEST": 0x000A00,
    "RGB_SEAFOAM_BRI_LOWEST": 0x000A02,
    "RGB_TURQUOISE_BRI_LOWEST": 0x000A08,
    "RGB_ICE_BLUE_BRI_LOWEST": 0x00060A,
    "RGB_LIGHT_BLUE_BRI_LOWEST": 0x00020A,
    "RGB_BLUE_BRI_LOWEST": 0x00000A,
    "RGB_ROYAL_BLUE_BRI_LOWEST": 0x01000A,
    "RGB_PURPLE_BRI_LOWEST": 0x05000A,
    "RGB_PINK_BRI_LOWEST": 0x08000A,
    "RGB_HOT_PINK_BRI_LOWEST": 0x0A0007,
    "RGB_MAGENTA_BRI_LOWEST": 0x0A0002,

    # Medium saturation
    "RGB_RED_SAT_MEDIUM": 0xFF2222,
    "RGB_CORAL_ORANGE_SAT_MEDIUM": 0xFF3C22,
    "RGB_ORANGE_SAT_MEDIUM": 0xFF5622,
    "RGB_YELLOW_SAT_MEDIUM": 0xFF8A22,
    "RGB_LIME_GREEN_SAT_MEDIUM": 0x6FFF22,
    "RGB_GREEN_SAT_MEDIUM": 0x22FF22,
    "RGB_SEAFOAM_SAT_MEDIUM": 0x22FF56,
    "RGB_TURQUOISE_SAT_MEDIUM": 0x22FFD7,
    "RGB_ICE_BLUE_SAT_MEDIUM": 0x22B3FF,
    "RGB_LIGHT_BLUE_SAT_MEDIUM": 0x2260FF,
    "RGB_BLUE_SAT_MEDIUM": 0x2222FF,
    "RGB_ROYAL_BLUE_SAT_MEDIUM": 0x3C22FF,
    "RGB_PURPLE_SAT_MEDIUM": 0x9422FF,
    "RGB_PINK_SAT_MEDIUM": 0xD822FF,
    "RGB_HOT_PINK_SAT_MEDIUM": 0xFF22BE,
    "RGB_MAGENTA_SAT_MEDIUM": 0xFF2256,

    # Low saturation
    "RGB_RED_SAT_LOW": 0xFF5555,
    "RGB_CORAL_ORANGE_SAT_LOW": 0xFF6955,
    "RGB_ORANGE_SAT_LOW": 0xFF7D55,
    "RGB_YELLOW_SAT_LOW": 0xFFA555,
    "RGB_LIME_GREEN_SAT_LOW": 0x90FF55,
    "RGB_GREEN_SAT_LOW": 0x55FF55,
    "RGB_SEAFOAM_SAT_LOW": 0x55FF7D,
    "RGB_TURQUOISE_SAT_LOW": 0x55FFE0,
    "RGB_ICE_BLUE_SAT_LOW": 0x55C4FF,
    "RGB_LIGHT_BLUE_SAT_LOW": 0x5584FF,
    "RGB_BLUE_SAT_LOW": 0x5555FF,
    "RGB_ROYAL_BLUE_SAT_LOW": 0x6855FF,
    "RGB_PURPLE_SAT_LOW": 0xAC55FF,
    "RGB_PINK_SAT_LOW": 0xE055FF,
    "RGB_HOT_PINK_SAT_LOW": 0xFF55CD,
    "RGB_MAGENTA_SAT_LOW": 0xFF557D,

    # Lowest saturation
    "RGB_RED_SAT_LOWEST": 0xFF7D7D,
    "RGB_CORAL_ORANGE_SAT_LOWEST": 0xFF8C7D,
    "RGB_ORANGE_SAT_LOWEST": 0xFF9B7D,
    "RGB_YELLOW_SAT_LOWEST": 0xFFBA7D,
    "RGB_LIME_GREEN_SAT_LOWEST": 0xAAFF7D,
    "RGB_GREEN_SAT_LOWEST": 0x7DFF7D,
    "RGB_SEAFOAM_SAT_LOWEST": 0x7DFF9B,
    "RGB_TURQUOISE_SAT_LOWEST": 0x7DFFE7,
    "RGB_ICE_BLUE_SAT_LOWEST": 0x7DD2FF,
    "RGB_LIGHT_BLUE_SAT_LOWEST": 0x7DA1FF,
    "RGB_BLUE_SAT_LOWEST": 0x7D7DFF,
    "RGB_ROYAL_BLUE_SAT_LOWEST": 0x8B7DFF,
    "RGB_PURPLE_SAT_LOWEST": 0xBF7DFF,
    "RGB_PINK_SAT_LOWEST": 0xE87DFF,
    "RGB_HOT_PINK_SAT_LOWEST": 0xFF7DD8,
    "RGB_MAGENTA_SAT_LOWEST": 0xFF7D9B,
}

def analyze_collisions():
    print("=" * 80)
    print("COLOR COLLISION ANALYSIS")
    print("=" * 80)

    # Build maps for each bit depth
    rgb24_map = {}
    rgb16_map = {}
    rgb8_map = {}

    for name, hex_val in colors.items():
        r, g, b = parse_rgb(hex_val)

        # Store 24-bit
        rgb24_map[name] = (hex_val, r, g, b)

        # Convert and store 16-bit
        rgb16_val = rgb24_to_rgb16(r, g, b)
        if rgb16_val not in rgb16_map:
            rgb16_map[rgb16_val] = []
        rgb16_map[rgb16_val].append(name)

        # Convert and store 8-bit
        rgb8_val = rgb24_to_rgb8(r, g, b)
        if rgb8_val not in rgb8_map:
            rgb8_map[rgb8_val] = []
        rgb8_map[rgb8_val].append(name)

    # Find collisions in 16-bit
    print("\n16-BIT RGB565 COLLISIONS:")
    print("-" * 80)
    collisions_16 = [colors for colors in rgb16_map.values() if len(colors) > 1]
    if collisions_16:
        for group in sorted(collisions_16, key=lambda x: len(x), reverse=True):
            print(f"\n{len(group)} colors map to same 16-bit value:")
            for color_name in group:
                hex_val, r, g, b = rgb24_map[color_name]
                print(f"  {color_name:40s} = 0x{hex_val:06X} ({r:3d}, {g:3d}, {b:3d})")
        print(f"\nTotal: {len(collisions_16)} collision groups affecting {sum(len(g) for g in collisions_16)} colors")
    else:
        print("NO COLLISIONS - All colors remain unique at 16-bit depth")

    # Find collisions in 8-bit
    print("\n\n8-BIT RGB332 COLLISIONS:")
    print("-" * 80)
    collisions_8 = [colors for colors in rgb8_map.values() if len(colors) > 1]
    if collisions_8:
        for group in sorted(collisions_8, key=lambda x: len(x), reverse=True):
            print(f"\n{len(group)} colors map to same 8-bit value:")
            for color_name in group:
                hex_val, r, g, b = rgb24_map[color_name]
                print(f"  {color_name:40s} = 0x{hex_val:06X} ({r:3d}, {g:3d}, {b:3d})")
        print(f"\nTotal: {len(collisions_8)} collision groups affecting {sum(len(g) for g in collisions_8)} colors")
    else:
        print("NO COLLISIONS - All colors remain unique at 8-bit depth")

    # Summary statistics
    print("\n" + "=" * 80)
    print("SUMMARY")
    print("=" * 80)
    print(f"Total colors defined: {len(colors)}")
    print(f"Unique at 24-bit: {len(colors)}")
    print(f"Unique at 16-bit: {len(rgb16_map)}")
    print(f"Unique at 8-bit: {len(rgb8_map)}")
    print(f"Color loss at 16-bit: {len(colors) - len(rgb16_map)} ({100*(len(colors)-len(rgb16_map))/len(colors):.1f}%)")
    print(f"Color loss at 8-bit: {len(colors) - len(rgb8_map)} ({100*(len(colors)-len(rgb8_map))/len(colors):.1f}%)")
    print("=" * 80)

if __name__ == "__main__":
    analyze_collisions()

