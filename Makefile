# Unified Makefile for Helios Engine
# This Makefile runs commands in subdirectories

# List all make targets which are not filenames
.PHONY: all clean lib cli embedded tests wasm docs help package setup_dirs

# Directories containing Makefiles
HELIOS_LIB_DIR = HeliosLib
HELIOS_CLI_DIR = HeliosCLI
HELIOS_EMBEDDED_DIR = HeliosEmbedded

# Architecture directories for object files
ARCH_DIRS = build/avr build/x64 build/wasm

# Setup build directories
setup_dirs:
	@echo "Setting up architecture-specific build directories..."
	@mkdir -p $(ARCH_DIRS)

# Default target - Build the embedded version
embedded: setup_dirs
	@echo "Building Helios Embedded..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) ARCH=avr BUILD_DIR=../build/avr
	@cp $(HELIOS_EMBEDDED_DIR)/helios.hex ./helios_firmware.hex

# CLI builds lib implicitly
cli: setup_dirs
	@echo "Building Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) ARCH=x64 BUILD_DIR=../build/x64
	@cp $(HELIOS_CLI_DIR)/helios ./helios_cli

# Build everything (truly "all") - but build them separately to avoid conflicts
all:
	@echo "Building all components..."
	@$(MAKE) embedded
	@$(MAKE) cli
	@$(MAKE) lib
	@$(MAKE) wasm
	@echo "All components built successfully"

# Build the Helios library
lib: setup_dirs
	@echo "Building Helios library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) ARCH=x64 BUILD_DIR=../build/x64
	@cp $(HELIOS_LIB_DIR)/helios.a ./helios_lib.a

# Build WebAssembly version - HeliosLib will check for compiler availability
wasm: setup_dirs
	@echo "Building WebAssembly library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) wasm ARCH=wasm BUILD_DIR=../build/wasm
	@cp -f $(HELIOS_LIB_DIR)/HeliosLib.js ./helios_wasm.js 2>/dev/null || true
	@cp -f $(HELIOS_LIB_DIR)/HeliosLib.wasm ./helios_wasm.wasm 2>/dev/null || true

# Upload embedded firmware
upload:
	@echo "Uploading firmware to device..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) upload

# Run tests
tests: setup_dirs
	@echo "Running tests for Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) tests ARCH=x64 BUILD_DIR=../build/x64
	@echo "Running tests for Helios Library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) tests ARCH=x64 BUILD_DIR=../build/x64

# Generate PNGs for documentation
pngs:
	@echo "Generating PNG files..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) pngs
	@mkdir -p ./assets
	@cp $(HELIOS_CLI_DIR)/*.png ./assets/

# Generate BMPs for documentation
bmps:
	@echo "Generating BMP files..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) bmps
	@mkdir -p ./assets
	@cp $(HELIOS_CLI_DIR)/*.bmp ./assets/

# Clean all build artifacts and storage files
clean:
	@echo "Cleaning Helios library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) clean
	@echo "Cleaning Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) clean
	@echo "Cleaning Helios Embedded..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) clean
	@echo "Cleaning storage files..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) clean_storage
	@echo "Cleaning root directory artifacts..."
	@rm -f helios_cli helios_lib.a helios_firmware.hex helios_wasm.js helios_wasm.wasm
	@rm -rf assets helios_package helios_package.zip build

# Package all compiled binaries into a zip file
package:
	@echo "Packaging Helios binaries..."
	@$(MAKE) embedded
	@$(MAKE) cli
	@$(MAKE) lib
	@$(MAKE) wasm
	@mkdir -p helios_package
	@cp helios_cli helios_package/
	@cp helios_lib.a helios_package/
	@cp helios_firmware.hex helios_package/
	@cp helios_wasm.js helios_package/ 2>/dev/null || true
	@cp helios_wasm.wasm helios_package/ 2>/dev/null || true
	@cp -r assets helios_package/ 2>/dev/null || true
	@zip -r helios_package.zip helios_package
	@rm -rf helios_package
	@echo "Package created: helios_package.zip"

# Help information
help:
	@echo "Helios Engine Unified Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  embedded       - default, Build the embedded firmware"
	@echo "  cli            - Build the Helios CLI tool (implicitly builds lib)"
	@echo "  all            - Build everything: CLI, embedded firmware, and WebAssembly (if available)"
	@echo "  lib            - Build just the Helios library"
	@echo "  wasm           - Build WebAssembly version of library (requires Emscripten)"
	@echo "  upload         - Upload firmware to ATtiny device"
	@echo "  tests          - Run all tests"
	@echo "  pngs           - Generate PNG documentation files and copy to ./assets"
	@echo "  bmps           - Generate BMP files and copy to ./assets"
	@echo "  clean          - Clean all build artifacts and storage files"
	@echo "  package        - Build all components and package into a zip file"
	@echo "  help           - Show this help information"

# Important note for component Makefiles:
# To compile with the correct architecture settings and object file location,
# the following environment variables need to be used in the component Makefiles:
#
# 1. ARCH     - Target architecture (avr, x64, wasm)
# 2. BUILD_DIR - Location for object files
#
# Object files should then be written to $(BUILD_DIR)/*.o instead of
# in the source directories. Source files in shared directories like
# ../Helios should create corresponding directories in $(BUILD_DIR).
#
# Example: ../Helios/Timer.cpp → $(BUILD_DIR)/Helios/Timer.o