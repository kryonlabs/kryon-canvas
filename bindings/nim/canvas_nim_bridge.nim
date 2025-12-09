## Canvas Callback Bridge - Connects C plugin to Nim user callbacks
## This bridge allows the canvas plugin to invoke user-defined onDraw callbacks

import tables, os

# Import Kryon IR core types
const KRYON_ROOT = getEnv("HOME") & "/Projects/kryon"
{.passC: "-I" & KRYON_ROOT & "/ir".}
{.passL: "-L" & KRYON_ROOT & "/build -lkryon_ir".}

import ir_core

# ============================================================================
# Callback Table
# ============================================================================

# Callback table - maps component ID to Nim proc
var canvasHandlers {.global.} = initTable[uint32, proc()]()

# ============================================================================
# Registration Functions
# ============================================================================

proc registerCanvasHandler*(canvas: ptr IRComponent, handler: proc()) =
  ## Register a canvas onDraw handler
  ## Called from the Canvas DSL macro when a component is created with onDraw property
  if canvas.isNil:
    return
  canvasHandlers[canvas.id] = handler

proc cleanupCanvasHandler*(componentId: uint32) =
  ## Remove canvas handler for a component
  ## Called when a canvas component is destroyed
  if canvasHandlers.hasKey(componentId):
    canvasHandlers.del(componentId)

proc cleanupAllCanvasHandlers*() =
  ## Clear all canvas handlers
  ## Called during plugin shutdown
  canvasHandlers.clear()

# ============================================================================
# C Bridge Function (exported)
# ============================================================================

proc canvas_nim_callback_bridge*(componentId: uint32) {.exportc, cdecl, dynlib.} =
  ## Bridge function called from C plugin - invokes the Nim callback
  ## This is called by the canvas plugin's component renderer during rendering
  if canvasHandlers.hasKey(componentId):
    canvasHandlers[componentId]()

# Export public API
export registerCanvasHandler, cleanupCanvasHandler
