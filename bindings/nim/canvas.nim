## AUTO-GENERATED from bindings.json - DO NOT EDIT
## Kryon Plugin DSL Bindings

import macros, strutils, os
import canvas_draw

# Import Kryon IR core types and functions
# Use KRYON_ROOT environment variable to locate Kryon installation
const KRYON_ROOT = getEnv("HOME") & "/Projects/kryon"
{.passC: "-I" & KRYON_ROOT & "/ir".}
{.passL: "-L" & KRYON_ROOT & "/build -lkryon_ir".}

# Import IR types from Kryon's bindings
import ir_core
import canvas_nim_bridge  # For registerCanvasHandler

# Color helper type for parseColor functions
type IRColor* = object
  r*, g*, b*, a*: uint8

# IR Size Units
const
  IR_SIZE_UNIT_PIXELS* = 0'u32
  IR_SIZE_UNIT_PERCENT* = 1'u32
  IR_SIZE_UNIT_AUTO* = 2'u32

# Helper Functions
proc parseColorToRGBA(value: string): IRColor =
  ## Convert color string to IRColor components
  # Try hex color first (#RGB, #RRGGBB, #RRGGBBAA)
  if value.len > 0 and value[0] == '#':
    let hex = value[1..^1]
    try:
      case hex.len
      of 3:  # #RGB -> #RRGGBB
        let r = uint8(parseHexInt(hex[0..0] & hex[0..0]))
        let g = uint8(parseHexInt(hex[1..1] & hex[1..1]))
        let b = uint8(parseHexInt(hex[2..2] & hex[2..2]))
        return IRColor(r: r, g: g, b: b, a: 255)
      of 6:  # #RRGGBB
        let r = uint8(parseHexInt(hex[0..1]))
        let g = uint8(parseHexInt(hex[2..3]))
        let b = uint8(parseHexInt(hex[4..5]))
        return IRColor(r: r, g: g, b: b, a: 255)
      of 8:  # #RRGGBBAA
        let r = uint8(parseHexInt(hex[0..1]))
        let g = uint8(parseHexInt(hex[2..3]))
        let b = uint8(parseHexInt(hex[4..5]))
        let a = uint8(parseHexInt(hex[6..7]))
        return IRColor(r: r, g: g, b: b, a: a)
      else: discard
    except: discard

  # Named colors
  case value.toLowerAscii()
  of "yellow": IRColor(r: 255, g: 255, b: 0, a: 255)
  of "white": IRColor(r: 255, g: 255, b: 255, a: 255)
  of "blue": IRColor(r: 0, g: 0, b: 255, a: 255)
  of "cyan": IRColor(r: 0, g: 255, b: 255, a: 255)
  of "green": IRColor(r: 0, g: 255, b: 0, a: 255)
  of "grey": IRColor(r: 128, g: 128, b: 128, a: 255)
  of "magenta": IRColor(r: 255, g: 0, b: 255, a: 255)
  of "purple": IRColor(r: 128, g: 0, b: 128, a: 255)
  of "gray": IRColor(r: 128, g: 128, b: 128, a: 255)
  of "red": IRColor(r: 255, g: 0, b: 0, a: 255)
  of "black": IRColor(r: 0, g: 0, b: 0, a: 255)
  of "orange": IRColor(r: 255, g: 165, b: 0, a: 255)
  else: IRColor(r: 255, g: 255, b: 255, a: 255)

# DSL Macros

macro Canvas*(props: untyped): untyped =
  ## Auto-generated DSL macro for Canvas component
  ## Immediate mode drawing canvas with Love2D-style API

  var width = 800
  var hasWidth = false
  var height = 600
  var hasHeight = false
  var backgroundColor = "#000000"
  var hasBackgroundColor = false
  var onDrawCallback: NimNode = nil
  var hasOnDraw = false

  # Parse properties from DSL block
  if props.kind == nnkStmtList:
    for stmt in props:
      if stmt.kind == nnkAsgn:
        let propName = stmt[0].strVal
        let propValue = stmt[1]
        case propName
        of "width":
          if propValue.kind == nnkIntLit:
            width = propValue.intVal
            hasWidth = true
        of "height":
          if propValue.kind == nnkIntLit:
            height = propValue.intVal
            hasHeight = true
        of "backgroundColor":
          if propValue.kind in {nnkStrLit, nnkTripleStrLit, nnkRStrLit}:
            backgroundColor = propValue.strVal
            hasBackgroundColor = true
        of "onDraw":
          # Store the callback expression
          onDrawCallback = propValue
          hasOnDraw = true
        else:
          warning("Unknown Canvas property: " & propName)

  # Generate component creation code
  if hasOnDraw and onDrawCallback != nil:
    result = quote do:
      let canvasComp = ir_create_component(IRComponentType(10))
      let canvasCompStyle = ir_create_style()
      if `hasWidth`:
        ir_set_width(canvasCompStyle, IR_DIMENSION_PX, cfloat(`width`))
      if `hasHeight`:
        ir_set_height(canvasCompStyle, IR_DIMENSION_PX, cfloat(`height`))
      if `hasBackgroundColor`:
        block:
          let colorBackgroundColor = parseColorToRGBA(`backgroundColor`)
          ir_set_background_color(canvasCompStyle, colorBackgroundColor.r, colorBackgroundColor.g, colorBackgroundColor.b, colorBackgroundColor.a)
      ir_set_style(canvasComp, canvasCompStyle)

      # Register drawing callback
      block:
        let callback = `onDrawCallback`
        registerCanvasHandler(canvasComp, callback)

      canvasComp
  else:
    result = quote do:
      let canvasComp = ir_create_component(IRComponentType(10))
      let canvasCompStyle = ir_create_style()
      if `hasWidth`:
        ir_set_width(canvasCompStyle, IR_DIMENSION_PX, cfloat(`width`))
      if `hasHeight`:
        ir_set_height(canvasCompStyle, IR_DIMENSION_PX, cfloat(`height`))
      if `hasBackgroundColor`:
        block:
          let colorBackgroundColor = parseColorToRGBA(`backgroundColor`)
          ir_set_background_color(canvasCompStyle, colorBackgroundColor.r, colorBackgroundColor.g, colorBackgroundColor.b, colorBackgroundColor.a)
      ir_set_style(canvasComp, canvasCompStyle)
      canvasComp

export Canvas

