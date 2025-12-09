# Kryon Canvas Plugin

Love2D-inspired immediate mode canvas drawing plugin for Kryon UI Framework.

## Features

- **Love2D-Style API**: Familiar immediate mode drawing API
- **Vector Graphics**: Shapes, lines, circles, polygons
- **Multi-Backend**: SDL3 and terminal rendering
- **Schema-Based DSL**: Automatic DSL generation from JSON schema
- **Zero Hardcoding**: Generic component renderer registration

## Installation

```bash
kryon plugin add github.com/kryon-plugins/canvas
```

Or for local development:

```bash
cd ~/Projects
git clone https://github.com/kryon-plugins/canvas kryon-plugin-canvas
kryon plugin add ./kryon-plugin-canvas
```

## Building

The plugin uses Make for building:

```bash
make        # Build plugin libraries
make clean  # Clean build artifacts
make test   # Run tests (TODO)
```

## Usage

### Nim

```nim
import kryon_dsl
import canvas  # From plugin bindings

kryonApp:
  Canvas:
    width = 800
    height = 600
    backgroundColor = "black"
    onDraw = proc() =
      # Love2D-style immediate mode drawing
      setColor("white")
      circle(FILL, 400, 300, 50)
      rectangle(LINE, 200, 200, 100, 100)
```

### Kry

```kry
Canvas {
  width: 800
  height: 600
  backgroundColor: "black"
}
```

## Schema-Based DSL Generation

The plugin uses **automatic DSL generation** from a JSON schema:

### How It Works

1. **Define schema** in `bindings/bindings.json`:

```json
{
  "components": [{
    "name": "Canvas",
    "type": 10,
    "properties": [
      {
        "name": "width",
        "type": "int",
        "default": "800",
        "ir_function": "ir_set_width"
      }
    ]
  }]
}
```

2. **Generate bindings**:

```bash
make bindings  # Calls generate_plugin_dsl
```

3. **Use in code**:

```nim
Canvas:
  width = 600
  height = 400
```

### Available Properties

- **width** (int, default: 800) - Canvas width in pixels
- **height** (int, default: 600) - Canvas height in pixels
- **backgroundColor** (color, default: "#000000") - Background color

## Development

### Directory Structure

```
kryon-plugin-canvas/
├── plugin.toml               # Plugin metadata
├── Makefile                  # Build system
├── src/
│   ├── canvas.c              # Canvas implementation
│   └── canvas_plugin.c       # Plugin registration
├── include/
│   ├── kryon_canvas.h        # Public API
│   └── canvas_plugin.h       # Plugin header
├── bindings/
│   ├── bindings.json         # Binding specification
│   └── nim/                  # Generated Nim bindings
├── examples/
│   └── nim/                  # Example applications
└── tests/
    └── test_canvas.c
```

### Requirements

- Kryon >= 0.3.0
- SDL3 (for desktop backend)
- FreeType, HarfBuzz (for text rendering)

## License

0BSD - Public domain equivalent
