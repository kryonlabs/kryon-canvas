## Canvas Colors Demo
## Demonstrates color manipulation and gradients

import ../../bindings/nim/canvas
import ../../bindings/nim/canvas_draw
import kryon_dsl
import runtime
import ir_desktop
import math

proc drawColors() =
  # Clear with black
  clear(0, 0, 0)

  # Color spectrum - Hue gradient
  print("Color Spectrum (Hue)", 10, 10)
  for i in 0..<360:
    let h = float(i)
    # Simple HSV to RGB conversion for demonstration
    let c = 1.0
    let x = 1.0 - abs((h / 60.0) mod 2.0 - 1.0)
    let (r1, g1, b1) = if h < 60: (c, x, 0.0)
                       elif h < 120: (x, c, 0.0)
                       elif h < 180: (0.0, c, x)
                       elif h < 240: (0.0, x, c)
                       elif h < 300: (x, 0.0, c)
                       else: (c, 0.0, x)

    setColor(uint8(r1 * 255), uint8(g1 * 255), uint8(b1 * 255))
    line(10.0 + float(i) * 2, 30, 10.0 + float(i) * 2, 80)

  # Brightness gradient
  print("Brightness Gradient", 10, 100)
  for i in 0..<256:
    setColor(uint8(i), uint8(i), uint8(i))
    line(10.0 + float(i) * 2.8, 120, 10.0 + float(i) * 2.8, 170)

  # Red gradient
  print("Red Gradient", 10, 190)
  for i in 0..<256:
    setColor(uint8(i), 0, 0)
    line(10.0 + float(i) * 2.8, 210, 10.0 + float(i) * 2.8, 260)

  # Green gradient
  print("Green Gradient", 10, 280)
  for i in 0..<256:
    setColor(0, uint8(i), 0)
    line(10.0 + float(i) * 2.8, 300, 10.0 + float(i) * 2.8, 350)

  # Blue gradient
  print("Blue Gradient", 10, 370)
  for i in 0..<256:
    setColor(0, 0, uint8(i))
    line(10.0 + float(i) * 2.8, 390, 10.0 + float(i) * 2.8, 440)

  # Alpha gradient (transparency)
  print("Alpha Gradient (over white)", 10, 460)
  # White background
  setColor(255, 255, 255)
  rectangle(FILL, 10, 480, 720, 50)
  # Red with varying alpha
  for i in 0..<256:
    setColor(255, 0, 0, uint8(i))
    line(10.0 + float(i) * 2.8, 480, 10.0 + float(i) * 2.8, 530)

  # Color palette
  print("Named Colors", 10, 550)
  let colors = ["red", "green", "blue", "yellow", "cyan", "magenta", "orange", "purple", "white", "gray"]
  let colorNames = ["Red", "Green", "Blue", "Yellow", "Cyan", "Magenta", "Orange", "Purple", "White", "Gray"]

  for i, col in colors:
    setColor(col)
    rectangle(FILL, 10.0 + float(i) * 75, 570, 60, 60)
    setColor("white")
    print(colorNames[i], 12.0 + float(i) * 75, 638)

let app = kryonApp:
  Header:
    windowWidth = 800
    windowHeight = 700
    windowTitle = "Canvas Colors Demo"

  Body:
    backgroundColor = "#000000"

    Canvas:
      width = 800
      height = 700
      backgroundColor = "#000000"

echo "✓ Canvas colors demo ready!"
echo "  Shows: color gradients, hue spectrum, alpha blending, named colors"
