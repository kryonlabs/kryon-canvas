# Canvas Plugin Examples Makefile

KRYON_ROOT := $(HOME)/Projects/kryon
PLUGIN_ROOT := ../..

# Compiler and flags
NIM := nim
NIM_FLAGS := --hints:off -p:$(KRYON_ROOT)/bindings/nim
NIM_LIBS := --passL:"-L$(PLUGIN_ROOT)/build -lkryon_canvas" \
            --passL:"-L$(KRYON_ROOT)/build -lkryon_desktop -lkryon_ir" \
            --passL:"-L/nix/store/s17z2ddnsf44kgkzm8amkznn9f1nqs7j-sdl3-3.2.20-lib/lib -lSDL3 -lSDL3_ttf"

# Runtime library path
export LD_LIBRARY_PATH := $(PLUGIN_ROOT)/build:$(KRYON_ROOT)/build:$(LD_LIBRARY_PATH)

# Examples
EXAMPLES := simple_drawing colors_demo patterns_demo shapes_demo

# Default target
all: $(EXAMPLES)

# Build individual examples
%: %.nim
	@echo "Building $@..."
	@$(NIM) c $(NIM_FLAGS) $(NIM_LIBS) -o:$@ $<

# Run an example
run-%: %
	@echo "Running $*..."
	@./$*

# Clean
clean:
	@echo "Cleaning examples..."
	@rm -f $(EXAMPLES)
	@rm -rf nimcache

.PHONY: all clean run-%
