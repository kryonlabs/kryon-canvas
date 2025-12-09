# Canvas Plugin Examples

This directory contains comprehensive examples demonstrating the Kryon Canvas plugin's Love2D-inspired drawing API.

## Examples Overview

### Basic Examples

1. **test_canvas_macro.nim** - Tests the Canvas DSL macro
   - Verifies component creation
   - Checks style properties
   - Validates width/height settings

2. **test_hex_colors.nim** - Tests hex color parsing
   - #RGB format (3 digits)
   - #RRGGBB format (6 digits)
   - #RRGGBBAA format (8 digits with alpha)
   - Named colors

3. **canvas_test.nim** - Full integration test
   - Creates a window with Canvas component
   - Tests background color
   - Verifies IR generation

### Drawing Demos

4. **shapes_demo.nim** - All basic drawing primitives
   - Rectangles (filled and outlined)
   - Circles (filled and outlined)
   - Ellipses
   - Lines
   - Polygons (triangles, pentagons)
   - Points
   - Text labels

5. **colors_demo.nim** - Color manipulation
   - Hue spectrum (HSV color wheel)
   - Brightness gradients
   - RGB channel gradients
   - Alpha blending demonstration
   - Named color palette

6. **patterns_demo.nim** - Complex compositions
   - Grid patterns
   - Checkerboard
   - Concentric circles
   - Spirals
   - Radial/sunburst patterns
   - Sine waves
   - Star polygons
   - Hexagon tessellation
   - Triangle patterns
   - Scattered points

## Running Examples

### Prerequisites

Ensure the canvas plugin is built:

```bash
cd /home/wao/Projects/kryon-plugin-canvas
make clean && make
```

### Running Tests

```bash
# Test the DSL macro
nim c -r test_canvas_macro.nim

# Test hex color support
nim c -r test_hex_colors.nim

# Test full integration (opens window)
env LD_LIBRARY_PATH=/home/wao/Projects/kryon/build:/home/wao/Projects/kryon-plugin-canvas/build:$LD_LIBRARY_PATH \
    KRYON_RENDERER=sdl3 timeout 2 \
    nim c -r -p:/home/wao/Projects/kryon/bindings/nim -p:../../bindings/nim \
    canvas_test.nim
```

### Running Drawing Demos

Drawing demos require callback support to be fully implemented. They demonstrate the drawing API but callbacks are not yet wired up to the rendering system.

```bash
# Shapes demo
nim c shapes_demo.nim

# Colors demo
nim c colors_demo.nim

# Patterns demo
nim c patterns_demo.nim
```

## Canvas Drawing API

The canvas plugin provides a Love2D-style immediate mode drawing API:

### Basic Shapes

```nim
import canvas_draw

# Set drawing color
setColor("red")
setColor(255, 0, 0)  # RGB
setColor(255, 0, 0, 128)  # RGBA

# Draw rectangles
rectangle(FILL, 100, 100, 200, 150)  # Filled
rectangle(LINE, 100, 100, 200, 150)  # Outlined

# Draw circles
circle(FILL, 200, 200, 50)
circle(LINE, 200, 200, 50)

# Draw ellipses
ellipse(FILL, 300, 200, 60, 40)

# Draw polygons
polygon(FILL, [100.0, 100.0, 150.0, 50.0, 200.0, 100.0])  # Triangle

# Draw lines
line(0, 0, 100, 100)

# Draw points
point(50, 50)
```

### Colors

```nim
# Named colors
setColor("red")
setColor("green")
setColor("blue")
setColor("white")
setColor("black")

# RGB/RGBA
setColor(255, 128, 0)  # Orange
setColor(255, 0, 255, 128)  # Magenta with 50% alpha

# Color constants
setColor(COLOR_RED)
setColor(COLOR_GREEN)
setColor(COLOR_BLUE)

# Create colors
let myColor = rgb(255, 128, 0)
let transparent = rgba(255, 0, 0, 128)
```

### Line Styles

```nim
# Set line width for outlined shapes
setLineWidth(2.0)
setLineWidth(5.0)

# Line styles (to be implemented)
# setLineStyle(SOLID)
# setLineStyle(ROUGH)
# setLineStyle(SMOOTH)
```

### Text

```nim
# Print text
print("Hello World", 100, 100)
```

### Transforms (Placeholder)

```nim
# Push/pop transform stack
push()
translate(100, 50)
rotate(0.5)  # Radians
scale(2.0, 2.0)
# ... draw something ...
pop()

# Reset transform
origin()
```

### Clearing

```nim
# Clear with background color
clear()

# Clear with specific color
clear(COLOR_BLACK)
clear(50, 50, 60)
```

## Implementation Status

### ✅ Implemented

- Canvas component DSL (width, height, backgroundColor)
- Hex color parsing (#RGB, #RRGGBB, #RRGGBBAA)
- C drawing functions (rectangle, circle, ellipse, polygon, line, point, arc)
- Nim drawing bindings (high-level wrappers)
- Color management (RGB, RGBA, named colors)
- Line width control
- Text rendering (basic)
- Clear operations

### 🔜 TODO

- Drawing callbacks (onDraw property)
- Component-level command buffer management
- Transform system (translate, rotate, scale)
- Line join styles
- Blend modes
- Image/texture support
- Advanced text (wrapping, alignment)
- Bezier curves
- Path operations

## Architecture

```
User Code (Nim)
    ↓
canvas_draw.nim (High-level API)
    ↓
kryon_canvas.h (C API)
    ↓
canvas.c (Implementation)
    ↓
Command Buffer
    ↓
Desktop Renderer (SDL3/Terminal)
```

## Next Steps

1. **Add onDraw callback support** to Canvas component schema
2. **Wire up callbacks** in canvas_plugin.c renderer
3. **Set command buffer** when entering canvas drawing context
4. **Integrate transform system** for push/pop/translate/rotate
5. **Add more examples** (animations, games, data visualization)

## Contributing

When adding new drawing functions:

1. Add C function to `src/canvas.c`
2. Add C declaration to `include/kryon_canvas.h`
3. Add Nim import to `bindings/nim/canvas_draw.nim`
4. Add high-level Nim wrapper to `bindings/nim/canvas_draw.nim`
5. Create example demonstrating the new function
6. Update this README

## License

0BSD - Public domain equivalent
