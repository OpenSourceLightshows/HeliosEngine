from PIL import Image, ImageDraw, ImageFilter
import numpy as np
import argparse
import os

def map_pattern_to_circle(draw, pattern, center, outer_radius, thickness):
    num_rings = 3  # Number of rings
    gap_between_rings = thickness * 0.5  # Reduced gap between rings

    segment_width = 1  # Width of each color segment in degrees
    pattern_width = pattern.width

    for ring in range(num_rings):
        inner_radius = outer_radius - thickness - (ring * (thickness + gap_between_rings))
        outer_r = inner_radius + thickness

        for angle in range(0, 360, segment_width):
            start_angle = angle - 90  # Adjust start angle (PIL uses 3 o'clock as 0 degrees)
            end_angle = start_angle + segment_width

            pattern_index = (angle // segment_width) % pattern_width
            color = pattern.getpixel((pattern_index, 0))

            if len(color) == 3:  # RGB
                if color == (0, 0, 0):  # Black color
                    continue  # Skip this segment (make it transparent)
                fill_color = color + (255,)  # Add full opacity
            elif len(color) == 4:  # RGBA
                if color[:3] == (0, 0, 0):  # Black color
                    continue  # Skip this segment (make it transparent)
                fill_color = color

            # Draw a filled arc
            draw.arc([center[0] - outer_r, center[1] - outer_r,
                      center[0] + outer_r, center[1] + outer_r],
                     start=start_angle, end=end_angle, fill=fill_color, width=thickness)

def create_circular_pattern(path_to_bmp):
    print(f"-- Creating circular pattern for {path_to_bmp}")
    pattern_strip = Image.open(path_to_bmp).convert('RGBA')  # Convert to RGBA

    canvas_size = 1000  # Fixed canvas size for all patterns
    canvas = Image.new('RGBA', (canvas_size, canvas_size), (0, 0, 0, 0))  # Transparent background
    draw = ImageDraw.Draw(canvas)

    center = (canvas_size // 2, canvas_size // 2)
    outer_radius = int(canvas_size * 0.45)  # Adjusted for better fit
    thickness = 50  # Adjusted thickness for balance

    print(f"-- Canvas prepared with size {canvas_size}x{canvas_size}, center at {center}, outer radius {outer_radius}, and thickness {thickness}")
    map_pattern_to_circle(draw, pattern_strip, center, outer_radius, thickness)

    print("-- Pattern mapped successfully")

    # Apply smoothing effect
    smoothed_canvas = canvas.filter(ImageFilter.GaussianBlur(radius=2))
    print("-- Smoothing effect applied")

    return smoothed_canvas

def main():
    parser = argparse.ArgumentParser(description="Generate a circular pattern from a BMP file.")
    parser.add_argument("bmp_file", type=str, help="Path to the BMP file")
    parser.add_argument("output_file", type=str, help="Path to the output PNG file")
    args = parser.parse_args()

    circular_pattern = create_circular_pattern(args.bmp_file)
    circular_pattern.save(args.output_file)
    print(f"** Saved smoothed circular pattern to {args.output_file}")

if __name__ == "__main__":
    main()