# Kryon Canvas Plugin Makefile

# Detect Kryon installation
KRYON_ROOT ?= $(HOME)/Projects/kryon

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -O2 -fPIC -DENABLE_SDL3 -I$(KRYON_ROOT)/ir -I$(KRYON_ROOT)/core/include -Iinclude -I/nix/store/rcyjzcv5f4naqq53lsczm9dqal4bwmws-sdl3-3.2.20-dev/include
LDFLAGS := -shared

# Source and build directories
SRC_DIR := src
BUILD_DIR := build
BINDINGS_DIR := bindings

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Nim bridge files
NIM_BRIDGE := bindings/nim/canvas_nim_bridge.nim
NIM_BRIDGE_SENTINEL := $(BUILD_DIR)/.nim_bridge_compiled

# Library output
LIB_STATIC := $(BUILD_DIR)/libkryon_canvas.a
LIB_SHARED := $(BUILD_DIR)/libkryon_canvas.so

# Default target
all: $(LIB_STATIC) $(LIB_SHARED)
	@echo "✓ Canvas plugin built successfully"
	@echo "  Static:  $(LIB_STATIC)"
	@echo "  Shared:  $(LIB_SHARED)"

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Compile object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile Nim bridge to object files (Nim compiles to C and then to .o automatically)
$(NIM_BRIDGE_SENTINEL): $(NIM_BRIDGE) | $(BUILD_DIR)
	@echo "Compiling Nim bridge..."
	@nim c --noMain --noLinking --nimcache:$(BUILD_DIR) \
		-p:$(KRYON_ROOT)/bindings/nim \
		--passC:"-I$(KRYON_ROOT)/ir" \
		--passL:"-L$(KRYON_ROOT)/build -lkryon_ir" \
		$<
	@touch $@

# Build static library (include all Nim-generated .o files)
$(LIB_STATIC): $(OBJS) $(NIM_BRIDGE_SENTINEL)
	@echo "Creating static library..."
	@ar rcs $@ $(OBJS) $(shell find $(BUILD_DIR) -name '*canvas_nim_bridge*.o')

# Build shared library (include all Nim-generated .o files)
$(LIB_SHARED): $(OBJS) $(NIM_BRIDGE_SENTINEL)
	@echo "Creating shared library..."
	@$(CC) $(LDFLAGS) -o $@ $(OBJS) $(shell find $(BUILD_DIR) -name '*canvas_nim_bridge*.o')

# Generate language bindings
bindings: $(LIB_STATIC)
	@echo "Generating language bindings..."
	@if [ -f "$(KRYON_ROOT)/build/generate_plugin_dsl" ]; then \
		$(KRYON_ROOT)/build/generate_plugin_dsl $(BINDINGS_DIR)/bindings.json $(BINDINGS_DIR)/nim/; \
	else \
		echo "  ⚠ DSL generator not found. Run 'make' in $(KRYON_ROOT) first."; \
	fi

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR)

# Rebuild everything
rebuild: clean all

.PHONY: all bindings clean rebuild
