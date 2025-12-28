-- Kryon Canvas Plugin - Lua FFI Bindings
-- Love2D-inspired immediate mode canvas drawing API

local ffi = require("ffi")

-- Determine library path
local function findCanvasLibrary()
  -- Try environment variable first
  local custom_path = os.getenv("KRYON_CANVAS_LIB_PATH")
  if custom_path then
    return custom_path
  end

  -- Determine library name
  local lib_name
  if ffi.os == "OSX" then
    lib_name = "libkryon_canvas.dylib"
  elseif ffi.os == "Windows" then
    lib_name = "kryon_canvas.dll"
  else
    lib_name = "libkryon_canvas.so"
  end

  -- Try various paths
  local search_paths = {
    "/home/wao/Projects/kryon-canvas/build/" .. lib_name,
    "build/" .. lib_name,
    "/usr/local/lib/" .. lib_name,
    "/usr/lib/" .. lib_name,
    lib_name,
  }

  -- Check which one exists
  for _, path in ipairs(search_paths) do
    local f = io.open(path, "r")
    if f then
      f:close()
      return path
    end
  end

  -- Default fallback
  return lib_name
end

-- Test if a symbol is available in the global namespace
local function isSymbolAvailable(symbol_name)
  local status, result = pcall(function()
    local sym = ffi.C[symbol_name]
    return sym ~= nil
  end)
  return status and result
end

-- Load the canvas library
-- Priority: 1) Global symbols (pre-loaded), 2) Explicit loading
local C
local load_method = nil

-- First, try to use symbols from the global namespace
if isSymbolAvailable("kryon_canvas_init") then
  C = ffi.C
  load_method = "global"
else
  -- Fall back to explicit library loading
  local lib_path = findCanvasLibrary()
  local success, result = pcall(function()
    return ffi.load(lib_path)
  end)

  if success then
    C = result
    load_method = "explicit"
  else
    error(string.format(
      "Failed to load kryon_canvas plugin:\n" ..
      "  • Not found in global namespace (ffi.C)\n" ..
      "  • Failed to load from: %s\n" ..
      "  • Error: %s\n\n" ..
      "Troubleshooting:\n" ..
      "  • Ensure plugin is configured in kryon.toml [plugins] section\n" ..
      "  • Check that plugin is built: cd /home/wao/Projects/kryon-canvas && make\n" ..
      "  • Set KRYON_CANVAS_LIB_PATH environment variable if needed\n",
      lib_path, tostring(result)
    ))
  end
end

-- Optional debug output
if os.getenv("KRYON_DEBUG") then
  print(string.format("[canvas] Loaded via: %s", load_method))
end

-- C type definitions
ffi.cdef[[
  // Plugin initialization
  bool kryon_canvas_plugin_init(void);
  void kryon_canvas_plugin_shutdown(void);

  // Drawing modes
  typedef enum {
    KRYON_DRAW_FILL = 0,
    KRYON_DRAW_LINE = 1
  } kryon_draw_mode_t;

  // Line styles
  typedef enum {
    KRYON_LINE_SOLID = 0,
    KRYON_LINE_ROUGH = 1,
    KRYON_LINE_SMOOTH = 2
  } kryon_line_style_t;

  // Blend modes
  typedef enum {
    KRYON_BLEND_ALPHA = 0,
    KRYON_BLEND_ADDITIVE = 1,
    KRYON_BLEND_MULTIPLY = 2,
    KRYON_BLEND_SUBTRACT = 3,
    KRYON_BLEND_SCREEN = 4,
    KRYON_BLEND_REPLACE = 5
  } kryon_blend_mode_t;

  // Fixed-point type (assuming it's a float for simplicity)
  typedef float kryon_fp_t;

  // Canvas lifecycle
  void kryon_canvas_init(uint16_t width, uint16_t height);
  void kryon_canvas_shutdown(void);
  void kryon_canvas_resize(uint16_t width, uint16_t height);
  void kryon_canvas_set_offset(int16_t x, int16_t y);
  void kryon_canvas_clear(void);
  void kryon_canvas_clear_color(uint32_t color);

  // Color management
  void kryon_canvas_set_color(uint32_t color);
  void kryon_canvas_set_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
  void kryon_canvas_set_color_rgb(uint8_t r, uint8_t g, uint8_t b);
  uint32_t kryon_canvas_get_color(void);
  void kryon_canvas_set_background_color(uint32_t color);
  void kryon_canvas_set_background_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

  // Line properties
  void kryon_canvas_set_line_width(kryon_fp_t width);
  kryon_fp_t kryon_canvas_get_line_width(void);
  void kryon_canvas_set_line_style(kryon_line_style_t style);
  void kryon_canvas_set_blend_mode(kryon_blend_mode_t mode);

  // Font management
  void kryon_canvas_set_font(uint16_t font_id);
  uint16_t kryon_canvas_get_font(void);

  // Transform operations
  void kryon_canvas_origin(void);
  void kryon_canvas_translate(kryon_fp_t x, kryon_fp_t y);
  void kryon_canvas_rotate(kryon_fp_t angle);
  void kryon_canvas_scale(kryon_fp_t sx, kryon_fp_t sy);
  void kryon_canvas_push(void);
  void kryon_canvas_pop(void);

  // Drawing primitives
  void kryon_canvas_rectangle(kryon_draw_mode_t mode, kryon_fp_t x, kryon_fp_t y, kryon_fp_t width, kryon_fp_t height);
  void kryon_canvas_circle(kryon_draw_mode_t mode, kryon_fp_t x, kryon_fp_t y, kryon_fp_t radius);
  void kryon_canvas_ellipse(kryon_draw_mode_t mode, kryon_fp_t x, kryon_fp_t y, kryon_fp_t rx, kryon_fp_t ry);
  void kryon_canvas_polygon(kryon_draw_mode_t mode, const kryon_fp_t* vertices, uint16_t vertex_count);
  void kryon_canvas_line(kryon_fp_t x1, kryon_fp_t y1, kryon_fp_t x2, kryon_fp_t y2);
  void kryon_canvas_line_points(const kryon_fp_t* points, uint16_t point_count);
  void kryon_canvas_point(kryon_fp_t x, kryon_fp_t y);
  void kryon_canvas_points(const kryon_fp_t* points, uint16_t point_count);
  void kryon_canvas_arc(kryon_draw_mode_t mode, kryon_fp_t cx, kryon_fp_t cy, kryon_fp_t radius,
                       kryon_fp_t start_angle, kryon_fp_t end_angle);

  // Text rendering
  void kryon_canvas_print(const char* text, kryon_fp_t x, kryon_fp_t y);
  void kryon_canvas_printf(const char* text, kryon_fp_t x, kryon_fp_t y, kryon_fp_t wrap_limit, int32_t align);
  kryon_fp_t kryon_canvas_get_text_width(const char* text);
  kryon_fp_t kryon_canvas_get_text_height(void);
]]

-- Module table
local canvas = {}

-- Draw modes
canvas.DrawMode = {
  FILL = C.KRYON_DRAW_FILL,
  LINE = C.KRYON_DRAW_LINE,
}

-- Helper to convert draw mode string to enum
local function parseDrawMode(mode)
  if type(mode) == "string" then
    mode = mode:lower()
    if mode == "fill" then
      return C.KRYON_DRAW_FILL
    elseif mode == "line" then
      return C.KRYON_DRAW_LINE
    end
  end
  return mode or C.KRYON_DRAW_FILL
end

-- ============================================================================
-- Canvas Lifecycle
-- ============================================================================

function canvas.init(width, height)
  C.kryon_canvas_init(width or 800, height or 600)
end

function canvas.shutdown()
  C.kryon_canvas_shutdown()
end

function canvas.resize(width, height)
  C.kryon_canvas_resize(width, height)
end

function canvas.clear(r, g, b, a)
  if r then
    if type(r) == "table" then
      C.kryon_canvas_set_background_color_rgba(r[1] or 0, r[2] or 0, r[3] or 0, r[4] or 255)
    else
      C.kryon_canvas_set_background_color_rgba(r, g or 0, b or 0, a or 255)
    end
  end
  C.kryon_canvas_clear()
end

-- ============================================================================
-- Color Management (Love2D style)
-- ============================================================================

-- Set fill/stroke color
function canvas.fill(r, g, b, a)
  if type(r) == "table" then
    C.kryon_canvas_set_color_rgba(r[1] or 255, r[2] or 255, r[3] or 255, r[4] or 255)
  else
    if a then
      C.kryon_canvas_set_color_rgba(r, g, b, a)
    else
      C.kryon_canvas_set_color_rgb(r, g, b)
    end
  end
end

-- Alias for Love2D compatibility
canvas.setColor = canvas.fill

function canvas.getColor()
  local color = C.kryon_canvas_get_color()
  local r = bit.rshift(bit.band(color, 0xFF000000), 24)
  local g = bit.rshift(bit.band(color, 0x00FF0000), 16)
  local b = bit.rshift(bit.band(color, 0x0000FF00), 8)
  local a = bit.band(color, 0x000000FF)
  return r, g, b, a
end

-- ============================================================================
-- Line Properties
-- ============================================================================

function canvas.setLineWidth(width)
  C.kryon_canvas_set_line_width(width)
end

function canvas.getLineWidth()
  return C.kryon_canvas_get_line_width()
end

function canvas.setLineStyle(style)
  if style == "solid" then
    C.kryon_canvas_set_line_style(C.KRYON_LINE_SOLID)
  elseif style == "rough" then
    C.kryon_canvas_set_line_style(C.KRYON_LINE_ROUGH)
  elseif style == "smooth" then
    C.kryon_canvas_set_line_style(C.KRYON_LINE_SMOOTH)
  end
end

-- ============================================================================
-- Font Management
-- ============================================================================

local current_font_size = 12

function canvas.setFont(font_id)
  C.kryon_canvas_set_font(font_id or 0)
end

function canvas.setFontSize(size)
  current_font_size = size
  -- Font size is typically handled via font selection in this API
  -- Store it for later use if needed
end

function canvas.getFontSize()
  return current_font_size
end

-- ============================================================================
-- Transform Operations
-- ============================================================================

function canvas.origin()
  C.kryon_canvas_origin()
end

function canvas.translate(x, y)
  C.kryon_canvas_translate(x, y)
end

function canvas.rotate(angle)
  C.kryon_canvas_rotate(angle)
end

function canvas.scale(sx, sy)
  C.kryon_canvas_scale(sx, sy or sx)
end

function canvas.push()
  C.kryon_canvas_push()
end

function canvas.pop()
  C.kryon_canvas_pop()
end

-- ============================================================================
-- Drawing Primitives (Love2D style)
-- ============================================================================

function canvas.rectangle(mode, x, y, width, height)
  C.kryon_canvas_rectangle(parseDrawMode(mode), x, y, width, height)
end

function canvas.circle(mode, x, y, radius)
  C.kryon_canvas_circle(parseDrawMode(mode), x, y, radius)
end

function canvas.ellipse(mode, x, y, rx, ry)
  C.kryon_canvas_ellipse(parseDrawMode(mode), x, y, rx, ry or rx)
end

function canvas.polygon(mode, points)
  -- Convert table of points to array
  -- Points can be either {x=1, y=2}, {x=3, y=4} or a flat array {1, 2, 3, 4}
  local vertices = ffi.new("kryon_fp_t[?]", #points)

  if type(points[1]) == "table" then
    -- Array of {x, y} tables
    for i, point in ipairs(points) do
      vertices[(i-1)*2] = point.x
      vertices[(i-1)*2 + 1] = point.y
    end
    C.kryon_canvas_polygon(parseDrawMode(mode), vertices, #points)
  else
    -- Flat array [x1, y1, x2, y2, ...]
    for i, v in ipairs(points) do
      vertices[i-1] = v
    end
    C.kryon_canvas_polygon(parseDrawMode(mode), vertices, #points / 2)
  end
end

function canvas.line(...)
  local args = {...}
  if #args == 4 then
    -- Single line: x1, y1, x2, y2
    C.kryon_canvas_line(args[1], args[2], args[3], args[4])
  elseif #args >= 2 then
    -- Multiple points: x1, y1, x2, y2, x3, y3, ...
    local points = ffi.new("kryon_fp_t[?]", #args)
    for i, v in ipairs(args) do
      points[i-1] = v
    end
    C.kryon_canvas_line_points(points, #args / 2)
  end
end

function canvas.point(x, y)
  C.kryon_canvas_point(x, y)
end

function canvas.points(...)
  local args = {...}
  local points = ffi.new("kryon_fp_t[?]", #args)
  for i, v in ipairs(args) do
    points[i-1] = v
  end
  C.kryon_canvas_points(points, #args / 2)
end

function canvas.arc(mode, cx, cy, radius, start_angle, end_angle)
  C.kryon_canvas_arc(parseDrawMode(mode), cx, cy, radius, start_angle, end_angle)
end

-- ============================================================================
-- Text Rendering
-- ============================================================================

function canvas.print(text, x, y)
  C.kryon_canvas_print(tostring(text), x or 0, y or 0)
end

function canvas.printf(text, x, y, wrap_limit, align)
  align = align or 0  -- 0 = left, 1 = center, 2 = right
  C.kryon_canvas_printf(tostring(text), x, y, wrap_limit, align)
end

function canvas.getTextWidth(text)
  return C.kryon_canvas_get_text_width(tostring(text))
end

function canvas.getTextHeight()
  return C.kryon_canvas_get_text_height()
end

-- ============================================================================
-- Blend Modes
-- ============================================================================

function canvas.setBlendMode(mode)
  if mode == "alpha" then
    C.kryon_canvas_set_blend_mode(C.KRYON_BLEND_ALPHA)
  elseif mode == "additive" or mode == "add" then
    C.kryon_canvas_set_blend_mode(C.KRYON_BLEND_ADDITIVE)
  elseif mode == "multiply" then
    C.kryon_canvas_set_blend_mode(C.KRYON_BLEND_MULTIPLY)
  elseif mode == "subtract" then
    C.kryon_canvas_set_blend_mode(C.KRYON_BLEND_SUBTRACT)
  elseif mode == "screen" then
    C.kryon_canvas_set_blend_mode(C.KRYON_BLEND_SCREEN)
  elseif mode == "replace" then
    C.kryon_canvas_set_blend_mode(C.KRYON_BLEND_REPLACE)
  end
end

-- ============================================================================
-- Plugin System Integration
-- ============================================================================

-- Initialize the canvas plugin (registers component renderer)
function canvas.init()
  local success = C.kryon_canvas_plugin_init()
  if not success then
    error("Failed to initialize canvas plugin")
  end
  print("[canvas] Plugin initialized successfully")
  return true
end

-- Shutdown the canvas plugin
function canvas.shutdown()
  C.kryon_canvas_plugin_shutdown()
end

-- Auto-initialize the plugin when the module is loaded
canvas.init()

return canvas
