## Simple Canvas Drawing Example
## Demonstrates immediate mode drawing with the canvas plugin

import ../../bindings/nim/canvas
import ../../bindings/nim/canvas_draw
import kryon_dsl
import runtime
import ir_desktop

let app = kryonApp:
  Header:
    windowWidth = 600
    windowHeight = 500
    windowTitle = "Simple Canvas Drawing"

  Body:
    backgroundColor = "#1e1e1e"

    Canvas:
      width = 600
      height = 500
      backgroundColor = "#1e1e28"
      onDraw = proc() =
        # Drawing callback - called each frame

        # Clear canvas
        clear(30, 30, 40)

        # Draw some shapes
        setColor(COLOR_RED)
        rectangle(FILL, 50, 50, 100, 80)

        setColor(COLOR_GREEN)
        circle(FILL, 300, 100, 50)

        setColor(COLOR_BLUE)
        polygon(FILL, [400.0, 50.0, 450.0, 20.0, 500.0, 50.0, 475.0, 100.0, 425.0, 100.0])

        setColor(COLOR_YELLOW)
        line(50, 200, 500, 200)

        setColor(COLOR_CYAN)
        for i in 0..10:
          circle(FILL, 50.0 + float(i) * 50, 300, 20)

        setColor(COLOR_WHITE)
        print("Canvas Drawing Test", 200, 400)

echo "✓ Canvas drawing example created with onDraw callback!"
