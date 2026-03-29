#!/usr/bin/env python3
"""
Analyze the specific colors used in Helios color_menu_data to see if they remain unique at 8-bit
"""

def rgb24_to_rgb8(r, g, b):
    """Convert 24-bit RGB to 8-bit RGB332"""
    r3 = (r >> 5) & 0x07  # 3 bits for red
    g3 = (g >> 5) & 0x07  # 3 bits for green
    b2 = (b >> 6) & 0x03  # 2 bits for blue
    return (r3 << 5) | (g3 << 2) | b2

def hue_to_rgb(hue):
    """Convert HSV to RGB (assuming full saturation and value)"""
    # This is a simplified conversion for the Helios hue values
    hue_normalized = hue / 255.0

    region = int(hue_normalized * 6)
    remainder = (hue_normalized * 6) - region

    if region == 0:
        r, g, b = 255, int(255 * remainder), 0
    elif region == 1:
        r, g, b = int(255 * (1 - remainder)), 255, 0
    elif region == 2:
        r, g, b = 0, 255, int(255 * remainder)
    elif region == 3:
        r, g, b = 0, int(255 * (1 - remainder)), 255
    elif region == 4:
        r, g, b = int(255 * remainder), 0, 255
    else:
        r, g, b = 255, 0, int(255 * (1 - remainder))

    return r, g, b

# The 16 hues from color_menu_data in Helios.cpp
menu_hues = {
    "HUE_RED": 0,
    "HUE_CORAL_ORANGE": 5,
    "HUE_ORANGE": 10,
    "HUE_YELLOW": 20,
    "HUE_LIME_GREEN": 70,
    "HUE_GREEN": 85,
    "HUE_SEAFOAM": 95,
    "HUE_TURQUOISE": 120,
    "HUE_ICE_BLUE": 142,
    "HUE_LIGHT_BLUE": 158,
    "HUE_BLUE": 170,
    "HUE_ROYAL_BLUE": 175,
    "HUE_PURPLE": 192,
    "HUE_PINK": 205,
    "HUE_HOT_PINK": 225,
    "HUE_MAGENTA": 245,
}

# The corresponding RGB values from ColorConstants.h
menu_colors = {
    "HUE_RED": 0xFF0000,           # 255, 0, 0
    "HUE_CORAL_ORANGE": 0xFF1E00,  # 255, 30, 0
    "HUE_ORANGE": 0xFF3C00,        # 255, 60, 0
    "HUE_YELLOW": 0xFF7800,        # 255, 120, 0
    "HUE_LIME_GREEN": 0x59FF00,    # 89, 255, 0
    "HUE_GREEN": 0x00FF00,         # 0, 255, 0
    "HUE_SEAFOAM": 0x00FF3C,       # 0, 255, 60
    "HUE_TURQUOISE": 0x00FFD1,     # 0, 255, 209
    "HUE_ICE_BLUE": 0x00A7FF,      # 0, 167, 255
    "HUE_LIGHT_BLUE": 0x0047FF,    # 0, 71, 255
    "HUE_BLUE": 0x0000FF,          # 0, 0, 255
    "HUE_ROYAL_BLUE": 0x1D00FF,    # 29, 0, 255
    "HUE_PURPLE": 0x8300FF,        # 131, 0, 255
    "HUE_PINK": 0xD200FF,          # 210, 0, 255
    "HUE_HOT_PINK": 0xFF00B4,      # 255, 0, 180
    "HUE_MAGENTA": 0xFF003C,       # 255, 0, 60
}

print("=" * 80)
print("HELIOS MENU COLOR ANALYSIS - 8-BIT COLOR DEPTH")
print("=" * 80)

# Convert all menu colors to 8-bit
color_map_8bit = {}
for name, rgb24 in menu_colors.items():
    r = (rgb24 >> 16) & 0xFF
    g = (rgb24 >> 8) & 0xFF
    b = rgb24 & 0xFF

    rgb8 = rgb24_to_rgb8(r, g, b)

    if rgb8 not in color_map_8bit:
        color_map_8bit[rgb8] = []
    color_map_8bit[rgb8].append((name, rgb24, r, g, b))

# Display the color groups
print("\nColor Groups (colors with same 8-bit value):")
print("-" * 80)

group_num = 1
collisions_found = False

for rgb8_val, colors in sorted(color_map_8bit.items()):
    if len(colors) > 1:
        collisions_found = True
        print(f"\nGroup {group_num}: {len(colors)} colors map to 8-bit value 0x{rgb8_val:02X}")
        for name, rgb24, r, g, b in colors:
            print(f"  {name:20s} = 0x{rgb24:06X} ({r:3d}, {g:3d}, {b:3d})")
        group_num += 1

if not collisions_found:
    print("\n✓ NO COLLISIONS - All 16 menu colors remain unique at 8-bit!")
else:
    print(f"\n✗ COLLISIONS DETECTED")

# Show the color groups organized by menu structure
print("\n" + "=" * 80)
print("COLOR GROUPS AS ORGANIZED IN MENU")
print("=" * 80)

groups = [
    ["HUE_RED", "HUE_CORAL_ORANGE", "HUE_ORANGE", "HUE_YELLOW"],
    ["HUE_LIME_GREEN", "HUE_GREEN", "HUE_SEAFOAM", "HUE_TURQUOISE"],
    ["HUE_ICE_BLUE", "HUE_LIGHT_BLUE", "HUE_BLUE", "HUE_ROYAL_BLUE"],
    ["HUE_PURPLE", "HUE_PINK", "HUE_HOT_PINK", "HUE_MAGENTA"],
]

for i, group in enumerate(groups, 1):
    print(f"\nGroup {i}:")
    rgb8_values = []
    for hue_name in group:
        rgb24 = menu_colors[hue_name]
        r = (rgb24 >> 16) & 0xFF
        g = (rgb24 >> 8) & 0xFF
        b = rgb24 & 0xFF
        rgb8 = rgb24_to_rgb8(r, g, b)
        rgb8_values.append(rgb8)
        print(f"  {hue_name:20s} = 0x{rgb24:06X} → 8-bit: 0x{rgb8:02X}")

    # Check for duplicates within this group
    if len(set(rgb8_values)) < len(rgb8_values):
        print("  ⚠️  WARNING: Colors in this group collide at 8-bit!")
    else:
        print("  ✓ All unique at 8-bit")

# Summary
print("\n" + "=" * 80)
print("SUMMARY")
print("=" * 80)
print(f"Total menu colors: {len(menu_colors)}")
print(f"Unique at 24-bit: {len(menu_colors)}")
print(f"Unique at 8-bit: {len(color_map_8bit)}")

if len(color_map_8bit) < len(menu_colors):
    print(f"Color loss: {len(menu_colors) - len(color_map_8bit)} colors lost")
    print(f"Percentage retained: {100 * len(color_map_8bit) / len(menu_colors):.1f}%")
else:
    print("✓ All menu colors remain unique at 8-bit depth!")

print("=" * 80)

