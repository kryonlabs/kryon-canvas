## Canvas Drawing API - Nim Bindings
## Love2D-inspired immediate mode drawing for Kryon

import os, strutils

# Link canvas library
const CANVAS_ROOT = getEnv("HOME") & "/Projects/kryon-plugin-canvas"
const KRYON_ROOT = getEnv("HOME") & "/Projects/kryon"
{.passL: "-L" & KRYON_ROOT & "/build -lkryon_ir".}
{.passL: "-L" & CANVAS_ROOT & "/build -lkryon_canvas".}
{.passC: "-I" & CANVAS_ROOT & "/include".}
{.passC: "-I" & KRYON_ROOT & "/ir".}

# ============================================================================
# Callback Support
# ============================================================================

type
  CanvasDrawCallback* = proc() {.cdecl.}

proc kryon_canvas_register_callback*(component_id: uint32, callback: CanvasDrawCallback) {.importc, cdecl.}
proc kryon_canvas_unregister_callback*(component_id: uint32) {.importc, cdecl.}
proc kryon_canvas_invoke_callback*(component_id: uint32): bool {.importc, cdecl.}
proc kryon_canvas_clear_callbacks*() {.importc, cdecl.}

# ============================================================================
# Types
# ============================================================================

type
  DrawMode* = enum
    FILL = 0    ## Fill the shape
    LINE = 1    ## Draw outline only

  LineStyle* = enum
    SOLID = 0
    ROUGH = 1
    SMOOTH = 2

  LineJoin* = enum
    MITER = 0
    BEVEL = 1
    ROUND = 2

  BlendMode* = enum
    ALPHA = 0        ## Standard alpha blending
    ADDITIVE = 1     ## Additive blending
    MULTIPLY = 2     ## Multiplicative blending
    SUBTRACT = 3     ## Subtractive blending
    SCREEN = 4       ## Screen blending
    REPLACE = 5      ## Replace (no blending)

# ============================================================================
# Color Constants
# ============================================================================

const
  COLOR_RED* = 0xFF0000FF'u32
  COLOR_GREEN* = 0x00FF00FF'u32
  COLOR_BLUE* = 0x0000FFFF'u32
  COLOR_WHITE* = 0xFFFFFFFF'u32
  COLOR_BLACK* = 0x000000FF'u32
  COLOR_YELLOW* = 0xFFFF00FF'u32
  COLOR_CYAN* = 0x00FFFFFF'u32
  COLOR_MAGENTA* = 0xFF00FFFF'u32
  COLOR_GRAY* = 0x808080FF'u32
  COLOR_ORANGE* = 0xFFA500FF'u32
  COLOR_PURPLE* = 0x800080FF'u32

# ============================================================================
# C Function Imports
# ============================================================================

# Canvas lifecycle
proc kryon_canvas_init*(width: uint16, height: uint16) {.importc, cdecl.}
proc kryon_canvas_shutdown*() {.importc, cdecl.}
proc kryon_canvas_resize*(width: uint16, height: uint16) {.importc, cdecl.}

# Color management
proc kryon_canvas_set_color*(color: uint32) {.importc, cdecl.}
proc kryon_canvas_set_color_rgba*(r, g, b, a: uint8) {.importc, cdecl.}
proc kryon_canvas_set_color_rgb*(r, g, b: uint8) {.importc, cdecl.}
proc kryon_canvas_get_color*(): uint32 {.importc, cdecl.}

proc kryon_canvas_set_background_color*(color: uint32) {.importc, cdecl.}
proc kryon_canvas_set_background_color_rgba*(r, g, b, a: uint8) {.importc, cdecl.}

# Line properties
proc kryon_canvas_set_line_width*(width: cfloat) {.importc, cdecl.}
proc kryon_canvas_get_line_width*(): cfloat {.importc, cdecl.}

# Basic drawing
proc kryon_canvas_rectangle*(mode: DrawMode, x, y, width, height: cfloat) {.importc, cdecl.}
proc kryon_canvas_circle*(mode: DrawMode, x, y, radius: cfloat) {.importc, cdecl.}
proc kryon_canvas_ellipse*(mode: DrawMode, x, y, rx, ry: cfloat) {.importc, cdecl.}
proc kryon_canvas_polygon*(mode: DrawMode, vertices: ptr cfloat, vertex_count: uint16) {.importc, cdecl.}
proc kryon_canvas_line*(x1, y1, x2, y2: cfloat) {.importc, cdecl.}
proc kryon_canvas_line_points*(points: ptr cfloat, point_count: uint16) {.importc, cdecl.}
proc kryon_canvas_point*(x, y: cfloat) {.importc, cdecl.}
proc kryon_canvas_points*(points: ptr cfloat, point_count: uint16) {.importc, cdecl.}
proc kryon_canvas_arc*(mode: DrawMode, cx, cy, radius, start_angle, end_angle: cfloat) {.importc, cdecl.}

# Text rendering
proc kryon_canvas_print*(text: cstring, x, y: cfloat) {.importc, cdecl.}
proc kryon_canvas_printf*(text: cstring, x, y, wrap_limit: cfloat, align: int32) {.importc, cdecl.}

# Clear operations
proc kryon_canvas_clear*() {.importc, cdecl.}
proc kryon_canvas_clear_color*(color: uint32) {.importc, cdecl.}

# Transform operations
proc kryon_canvas_origin*() {.importc, cdecl.}
proc kryon_canvas_translate*(x, y: cfloat) {.importc, cdecl.}
proc kryon_canvas_rotate*(angle: cfloat) {.importc, cdecl.}
proc kryon_canvas_scale*(sx, sy: cfloat) {.importc, cdecl.}
proc kryon_canvas_push*() {.importc, cdecl.}
proc kryon_canvas_pop*() {.importc, cdecl.}

# ============================================================================
# High-Level Nim Wrappers
# ============================================================================

# Color helpers
proc setColor*(r, g, b: uint8, a: uint8 = 255) =
  ## Set the current drawing color with RGB values
  kryon_canvas_set_color_rgba(r, g, b, a)

proc setColor*(color: uint32) =
  ## Set the current drawing color with a packed color value
  kryon_canvas_set_color(color)

proc setColor*(name: string) =
  ## Set color by name (red, green, blue, etc.)
  let color = case name.toLowerAscii()
    of "red": COLOR_RED
    of "green": COLOR_GREEN
    of "blue": COLOR_BLUE
    of "white": COLOR_WHITE
    of "black": COLOR_BLACK
    of "yellow": COLOR_YELLOW
    of "cyan": COLOR_CYAN
    of "magenta": COLOR_MAGENTA
    of "gray", "grey": COLOR_GRAY
    of "orange": COLOR_ORANGE
    of "purple": COLOR_PURPLE
    else: COLOR_WHITE
  kryon_canvas_set_color(color)

proc setBackgroundColor*(r, g, b: uint8, a: uint8 = 255) =
  kryon_canvas_set_background_color_rgba(r, g, b, a)

proc setBackgroundColor*(color: uint32) =
  kryon_canvas_set_background_color(color)

# Drawing helpers
proc rectangle*(mode: DrawMode, x, y, w, h: float) =
  ## Draw a rectangle
  kryon_canvas_rectangle(mode, cfloat(x), cfloat(y), cfloat(w), cfloat(h))

proc circle*(mode: DrawMode, x, y, radius: float) =
  ## Draw a circle
  kryon_canvas_circle(mode, cfloat(x), cfloat(y), cfloat(radius))

proc ellipse*(mode: DrawMode, x, y, rx, ry: float) =
  ## Draw an ellipse
  kryon_canvas_ellipse(mode, cfloat(x), cfloat(y), cfloat(rx), cfloat(ry))

proc polygon*(mode: DrawMode, points: openArray[float]) =
  ## Draw a polygon from an array of points [x1,y1, x2,y2, ...]
  assert points.len >= 6 and points.len mod 2 == 0, "Polygon needs at least 3 points (6 values)"
  var vertices = newSeq[cfloat](points.len)
  for i, p in points:
    vertices[i] = cfloat(p)
  kryon_canvas_polygon(mode, addr vertices[0], uint16(points.len div 2))

proc line*(x1, y1, x2, y2: float) =
  ## Draw a line between two points
  kryon_canvas_line(cfloat(x1), cfloat(y1), cfloat(x2), cfloat(y2))

proc point*(x, y: float) =
  ## Draw a single point
  kryon_canvas_point(cfloat(x), cfloat(y))

proc arc*(mode: DrawMode, cx, cy, radius, startAngle, endAngle: float) =
  ## Draw an arc
  kryon_canvas_arc(mode, cfloat(cx), cfloat(cy), cfloat(radius),
                   cfloat(startAngle), cfloat(endAngle))

# Text helpers
proc print*(text: string, x, y: float) =
  ## Print text at position
  kryon_canvas_print(cstring(text), cfloat(x), cfloat(y))

# Clear helpers
proc clear*() =
  ## Clear the canvas with background color
  kryon_canvas_clear()

proc clear*(color: uint32) =
  ## Clear the canvas with a specific color
  kryon_canvas_clear_color(color)

proc clear*(r, g, b: uint8, a: uint8 = 255) =
  ## Clear the canvas with RGB color
  let color = (uint32(r) shl 24) or (uint32(g) shl 16) or (uint32(b) shl 8) or uint32(a)
  kryon_canvas_clear_color(color)

# Line style helpers
proc setLineWidth*(width: float) =
  ## Set line width for stroked shapes
  kryon_canvas_set_line_width(cfloat(width))

# Transform helpers
proc push*() =
  ## Push current transform onto stack
  kryon_canvas_push()

proc pop*() =
  ## Pop transform from stack
  kryon_canvas_pop()

proc translate*(x, y: float) =
  ## Translate coordinate system
  kryon_canvas_translate(cfloat(x), cfloat(y))

proc rotate*(angle: float) =
  ## Rotate coordinate system (angle in radians)
  kryon_canvas_rotate(cfloat(angle))

proc scale*(sx, sy: float) =
  ## Scale coordinate system
  kryon_canvas_scale(cfloat(sx), cfloat(sy))

proc origin*() =
  ## Reset transform to identity
  kryon_canvas_origin()

# ============================================================================
# Utility Functions
# ============================================================================

proc rgb*(r, g, b: uint8): uint32 =
  ## Create a color from RGB components (alpha = 255)
  (uint32(r) shl 24) or (uint32(g) shl 16) or (uint32(b) shl 8) or 0xFF

proc rgba*(r, g, b, a: uint8): uint32 =
  ## Create a color from RGBA components
  (uint32(r) shl 24) or (uint32(g) shl 16) or (uint32(b) shl 8) or uint32(a)

# ============================================================================
# Callback Registration Helpers
# ============================================================================

proc registerDrawCallback*(componentId: uint32, callback: proc()) =
  ## Register a drawing callback for a canvas component
  ## The callback will be called each frame when the canvas is rendered

  # Wrap the Nim proc in a cdecl proc
  proc cdeclWrapper() {.cdecl.} =
    callback()

  kryon_canvas_register_callback(componentId, cdeclWrapper)

proc unregisterDrawCallback*(componentId: uint32) =
  ## Unregister a drawing callback for a canvas component
  kryon_canvas_unregister_callback(componentId)

# Export common items
export DrawMode
export LineStyle, LineJoin, BlendMode
export CanvasDrawCallback
export registerDrawCallback, unregisterDrawCallback
