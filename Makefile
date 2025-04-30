# Unified Makefile for Helios Engine
# This Makefile runs commands in subdirectories

# List all make targets which are not filenames
.PHONY: all clean lib cli embedded tests wasm docs help package setup_root_output

# Directories containing Makefiles
HELIOS_LIB_DIR = HeliosLib
HELIOS_CLI_DIR = HeliosCLI
HELIOS_EMBEDDED_DIR = HeliosEmbedded

# Root output directory for aggregated artifacts
ROOT_OUTPUT_DIR = output

# Setup root output directory
setup_root_output:
	@echo "Setting up root output directory..."
	@mkdir -p $(ROOT_OUTPUT_DIR)

# Default target - Build the embedded version
embedded: setup_root_output
	@echo "Building Helios Embedded..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) ARCH=avr

# CLI builds lib implicitly
cli: setup_root_output
	@echo "Building Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) ARCH=x64
	@cp $(HELIOS_CLI_DIR)/output/helios_cli $(ROOT_OUTPUT_DIR)/

# Build everything (truly "all") - but build them separately
all:
	@echo "Building all components..."
	@$(MAKE) embedded
	@$(MAKE) cli
	@$(MAKE) lib
	@$(MAKE) wasm
	@echo "All components built successfully"

# Build the Helios library
lib: setup_root_output
	@echo "Building Helios library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) ARCH=x64
	@cp $(HELIOS_LIB_DIR)/output/helios_lib.a $(ROOT_OUTPUT_DIR)/

# Build WebAssembly version - HeliosLib will check for compiler availability
wasm: setup_root_output
	@echo "Building WebAssembly library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) wasm ARCH=wasm
	@cp -f $(HELIOS_LIB_DIR)/output/helios_wasm.js $(ROOT_OUTPUT_DIR)/ 2>/dev/null || true
	@cp -f $(HELIOS_LIB_DIR)/output/helios_wasm.wasm $(ROOT_OUTPUT_DIR)/ 2>/dev/null || true

# Upload embedded firmware
upload:
	@echo "Uploading firmware to device..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) upload

# Run tests
tests: setup_root_output
	@echo "Running tests for Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) tests ARCH=x64
	@echo "Running tests for Helios Library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) tests ARCH=x64

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
	@echo "Cleaning root directories..."
	@rm -rf assets helios_package helios_package.zip $(ROOT_OUTPUT_DIR)

# Package all compiled binaries into a zip file
package:
	@echo "Packaging Helios binaries..."
	@$(MAKE) all
	@mkdir -p helios_package
	@cp $(ROOT_OUTPUT_DIR)/* helios_package/ 2>/dev/null || true
	@cp -r assets helios_package/ 2>/dev/null || true
	@zip -r helios_package.zip helios_package
	@rm -rf helios_package
	@echo "Package created: helios_package.zip"

# Help information
help:
	@echo "Helios Engine Unified Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  embedded       - default, Build embedded firmware (output in HeliosEmbedded/output & ./output)"
	@echo "  cli            - Build CLI tool (output in HeliosCLI/output & ./output)"
	@echo "  all            - Build everything (outputs in component dirs & ./output)"
	@echo "  lib            - Build library (output in HeliosLib/output & ./output)"
	@echo "  wasm           - Build WebAssembly (output in HeliosLib/output & ./output)"
	@echo "  upload         - Upload firmware from HeliosEmbedded/output/helios_firmware.hex"
	@echo "  tests          - Run all tests"
	@echo "  pngs           - Generate PNG documentation files and copy to ./assets"
	@echo "  bmps           - Generate BMP files and copy to ./assets"
	@echo "  clean          - Clean all build artifacts, output files, and storage files"
	@echo "  package        - Build all components and package ./output/* into a zip file"
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