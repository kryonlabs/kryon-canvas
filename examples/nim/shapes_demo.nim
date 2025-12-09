## Canvas Shapes Demo
## Demonstrates all basic drawing primitives

import ../../bindings/nim/canvas
import ../../bindings/nim/canvas_draw
import kryon_dsl
import runtime
import ir_desktop

# Simple callback for canvas drawing
proc drawShapes() =
  # Clear with dark background
  clear(30, 30, 40)

  # Draw filled rectangles
  setColor(COLOR_RED)
  rectangle(FILL, 50, 50, 100, 80)

  setColor("green")
  rectangle(FILL, 200, 50, 100, 80)

  setColor("blue")
  rectangle(FILL, 350, 50, 100, 80)

  # Draw rectangle outlines
  setColor("white")
  setLineWidth(2.0)
  rectangle(LINE, 50, 160, 100, 80)
  rectangle(LINE, 200, 160, 100, 80)
  rectangle(LINE, 350, 160, 100, 80)

  # Draw filled circles
  setColor(COLOR_YELLOW)
  circle(FILL, 100, 330, 40)

  setColor(COLOR_CYAN)
  circle(FILL, 250, 330, 40)

  setColor(COLOR_MAGENTA)
  circle(FILL, 400, 330, 40)

  # Draw circle outlines
  setColor("white")
  circle(LINE, 100, 450, 40)
  circle(LINE, 250, 450, 40)
  circle(LINE, 400, 450, 40)

  # Draw ellipses
  setColor(COLOR_ORANGE)
  ellipse(FILL, 550, 100, 60, 40)

  setColor(COLOR_PURPLE)
  ellipse(FILL, 550, 220, 40, 60)

  setColor("white")
  ellipse(LINE, 550, 350, 50, 30)

  # Draw lines
  setColor(COLOR_RED)
  line(50, 550, 200, 550)

  setColor(COLOR_GREEN)
  line(200, 550, 200, 600)

  setColor(COLOR_BLUE)
  line(200, 600, 350, 550)

  # Draw polygon (triangle)
  setColor(COLOR_CYAN)
  polygon(FILL, [500.0, 550.0, 450.0, 600.0, 550.0, 600.0])

  # Draw polygon outline (pentagon)
  setColor("white")
  polygon(LINE, [
    650.0, 550.0,
    680.0, 570.0,
    670.0, 600.0,
    630.0, 600.0,
    620.0, 570.0
  ])

  # Draw points
  setColor(COLOR_WHITE)
  for i in 0..20:
    point(50.0 + float(i) * 10, 650.0)

  # Labels
  setColor("white")
  print("Filled Rects", 60, 30)
  print("Outlined Rects", 60, 140)
  print("Filled Circles", 60, 260)
  print("Outlined Circles", 60, 430)
  print("Ellipses", 550, 30)
  print("Lines", 50, 530)
  print("Polygons", 450, 530)
  print("Points", 50, 630)

let app = kryonApp:
  Header:
    windowWidth = 800
    windowHeight = 700
    windowTitle = "Canvas Shapes Demo"

  Body:
    backgroundColor = "#2a2a2a"

    Canvas:
      width = 800
      height = 700
      backgroundColor = "#1e1e28"
      # TODO: Add onDraw callback support

echo "✓ Canvas shapes demo ready!"
echo "  Shows: rectangles, circles, ellipses, lines, polygons, points"
