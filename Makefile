# Unified Makefile for Helios Engine
# This Makefile runs commands in subdirectories

# List all make targets which are not filenames
.PHONY: all clean lib cli embedded tests wasm docs help package

# Directories containing Makefiles
HELIOS_LIB_DIR = HeliosLib
HELIOS_CLI_DIR = HeliosCLI
HELIOS_EMBEDDED_DIR = HeliosEmbedded

# # Default target - Build the embedded version
embedded:
	@echo "Building Helios Embedded..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR)
	@cp $(HELIOS_EMBEDDED_DIR)/helios.hex ./helios_firmware.hex

# CLI builds lib implicitly
cli:
	@echo "Building Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR)
	@cp $(HELIOS_CLI_DIR)/helios ./helios_cli

# Build everything (truly "all")
all: cli embedded wasm
	@echo "All components built successfully"

# Build the Helios library
lib:
	@echo "Building Helios library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR)
	@cp $(HELIOS_LIB_DIR)/helios.a ./helios_lib.a



# Build WebAssembly version
wasm:
	@echo "Building WebAssembly library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) wasm
	@cp $(HELIOS_LIB_DIR)/HeliosLib.js ./helios_wasm.js
	@cp $(HELIOS_LIB_DIR)/HeliosLib.wasm ./helios_wasm.wasm

# Upload embedded firmware
upload:
	@echo "Uploading firmware to device..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) upload

# Run tests
tests:
	@echo "Running tests for Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) tests
	@echo "Running tests for Helios Library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) tests

# Generate PNGs for documentation
pngs:
	@echo "Generating PNG files..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) pngs
	@mkdir -p ./assets
	@cp $(HELIOS_CLI_DIR)/*.png ./assets/ 2>/dev/null || true

# Generate BMPs for documentation
bmps:
	@echo "Generating BMP files..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) bmps
	@mkdir -p ./assets
	@cp $(HELIOS_CLI_DIR)/*.bmp ./assets/ 2>/dev/null || true

# Clean all build artifacts and storage files
clean:
	@echo "Cleaning Helios library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) clean
	@echo "Cleaning Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) clean
	@echo "Cleaning Helios Embedded..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) clean
	@echo "Cleaning storage files..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) clean_storage 2>/dev/null || true
	@echo "Cleaning root directory artifacts..."
	@rm -f helios_cli helios_lib.a helios_firmware.hex helios_wasm.js helios_wasm.wasm
	@rm -rf assets

# Package all compiled binaries into a zip file
package: all
	@echo "Packaging Helios binaries..."
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
	@echo "  all            - Build everything: CLI, embedded firmware, and WebAssembly"
	@echo "  lib            - Build just the Helios library"
	@echo "  wasm           - Build WebAssembly version of library"
	@echo "  upload         - Upload firmware to ATtiny device"
	@echo "  tests          - Run all tests"
	@echo "  pngs           - Generate PNG documentation files and copy to ./assets"
	@echo "  bmps           - Generate BMP files and copy to ./assets"
	@echo "  clean          - Clean all build artifacts and storage files"
	@echo "  package        - Build all components and package into a zip file"
	@echo "  help           - Show this help information"