## Canvas Patterns Demo
## Demonstrates drawing patterns and compositions

import ../../bindings/nim/canvas
import ../../bindings/nim/canvas_draw
import kryon_dsl
import runtime
import ir_desktop
import math

proc drawPatterns() =
  clear(20, 20, 25)

  # Grid pattern
  setColor(60, 60, 70)
  for x in 0..16:
    line(float(x) * 50, 0, float(x) * 50, 800)
  for y in 0..16:
    line(0, float(y) * 50, 800, float(y) * 50)

  # Checkerboard pattern
  setColor("white")
  print("Checkerboard", 10, 10)
  for y in 0..<5:
    for x in 0..<5:
      if (x + y) mod 2 == 0:
        setColor(200, 200, 200)
      else:
        setColor(80, 80, 80)
      rectangle(FILL, 10.0 + float(x) * 30, 30.0 + float(y) * 30, 30, 30)

  # Concentric circles
  setColor("white")
  print("Concentric Circles", 300, 10)
  for i in countdown(10, 1):
    if i mod 2 == 0:
      setColor(100, 150, 255)
    else:
      setColor(255, 150, 100)
    circle(FILL, 400, 100, float(i) * 10)

  # Spiral pattern with circles
  setColor("white")
  print("Spiral", 550, 10)
  for i in 0..<30:
    let angle = float(i) * 0.5
    let radius = float(i) * 3.0
    let x = 650.0 + cos(angle) * radius
    let y = 100.0 + sin(angle) * radius
    setColor(uint8(i * 8), uint8(255 - i * 8), 200)
    circle(FILL, x, y, 8)

  # Radial lines (sun burst)
  setColor("white")
  print("Radial Pattern", 10, 210)
  let centerX = 120.0
  let centerY = 320.0
  for i in 0..<24:
    let angle = float(i) * (PI * 2.0 / 24.0)
    let x2 = centerX + cos(angle) * 80.0
    let y2 = centerY + sin(angle) * 80.0
    setColor(uint8(i * 10), uint8(255 - i * 10), 150)
    line(centerX, centerY, x2, y2)

  # Wave pattern
  setColor("white")
  print("Sine Wave", 300, 210)
  setColor(100, 200, 255)
  for x in 0..<200:
    let y = sin(float(x) * 0.1) * 40.0 + 320.0
    point(300.0 + float(x), y)

  # Multiple sine waves
  for wave in 0..<5:
    let phase = float(wave) * 0.5
    for x in 0..<200:
      let y = sin(float(x) * 0.1 + phase) * 20.0 + 320.0 + float(wave) * 10.0
      setColor(uint8(50 + wave * 40), uint8(100 + wave * 30), 255)
      point(300.0 + float(x), y)

  # Polygon star pattern
  setColor("white")
  print("Star Pattern", 550, 210)
  setColor(255, 215, 0)  # Gold color
  let starX = 650.0
  let starY = 320.0
  let outerRadius = 60.0
  let innerRadius = 25.0
  var starPoints: seq[float] = @[]
  for i in 0..<10:
    let angle = float(i) * (PI * 2.0 / 10.0) - PI / 2.0
    let radius = if i mod 2 == 0: outerRadius else: innerRadius
    starPoints.add(starX + cos(angle) * radius)
    starPoints.add(starY + sin(angle) * radius)
  polygon(FILL, starPoints)

  # Hexagon tessellation
  setColor("white")
  print("Hexagons", 10, 410)
  let hexSize = 25.0
  for row in 0..<3:
    for col in 0..<4:
      let x = 80.0 + float(col) * hexSize * 1.8
      let y = 480.0 + float(row) * hexSize * 1.6 + (if col mod 2 == 1: hexSize * 0.8 else: 0.0)
      var hexPoints: seq[float] = @[]
      for i in 0..<6:
        let angle = float(i) * (PI * 2.0 / 6.0)
        hexPoints.add(x + cos(angle) * hexSize)
        hexPoints.add(y + sin(angle) * hexSize)
      setColor(uint8(50 + col * 50), uint8(100 + row * 50), uint8(150))
      polygon(FILL, hexPoints)
      setColor("white")
      polygon(LINE, hexPoints)

  # Triangle pattern
  setColor("white")
  print("Triangles", 350, 410)
  for row in 0..<4:
    for col in 0..<4:
      let x = 400.0 + float(col) * 40
      let y = 450.0 + float(row) * 40
      setColor(uint8(col * 60), uint8(row * 60), 200)
      polygon(FILL, [x, y, x + 35, y, x + 17.5, y + 30])

  # Random dots pattern
  setColor("white")
  print("Scattered Points", 550, 410)
  # Use deterministic pattern instead of random
  for i in 0..<100:
    let x = 550.0 + float((i * 37) mod 200)
    let y = 450.0 + float((i * 71) mod 150)
    setColor(uint8((i * 17) mod 256), uint8((i * 31) mod 256), uint8((i * 47) mod 256))
    circle(FILL, x, y, 3)

let app = kryonApp:
  Header:
    windowWidth = 800
    windowHeight = 650
    windowTitle = "Canvas Patterns Demo"

  Body:
    backgroundColor = "#141419"

    Canvas:
      width = 800
      height = 650
      backgroundColor = "#141419"

echo "✓ Canvas patterns demo ready!"
echo "  Shows: grids, checkerboards, spirals, waves, stars, tessellations"
