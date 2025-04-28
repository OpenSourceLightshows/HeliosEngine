# Unified Makefile for Helios Engine
# This Makefile runs commands in subdirectories

# List all make targets which are not filenames
.PHONY: all clean lib cli embedded tests wasm docs help package setup_dirs

# Directories containing Makefiles
HELIOS_LIB_DIR = HeliosLib
HELIOS_CLI_DIR = HeliosCLI
HELIOS_EMBEDDED_DIR = HeliosEmbedded

# Build and Output directories
BUILD_BASE_DIR = build
OUTPUT_DIR = output
ARCH_DIRS = $(BUILD_BASE_DIR)/avr $(BUILD_BASE_DIR)/x64 $(BUILD_BASE_DIR)/wasm $(OUTPUT_DIR)

# Setup build directories
setup_dirs:
	@echo "Setting up build and output directories..."
	@mkdir -p $(ARCH_DIRS)

# Default target - Build the embedded version
embedded: setup_dirs
	@echo "Building Helios Embedded..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) ARCH=avr BUILD_DIR=../$(BUILD_BASE_DIR)/avr ROOT_OUTPUT_DIR=../$(OUTPUT_DIR)

# CLI builds lib implicitly
cli: setup_dirs
	@echo "Building Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) ARCH=x64 BUILD_DIR=../$(BUILD_BASE_DIR)/x64 ROOT_OUTPUT_DIR=../$(OUTPUT_DIR)

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
	@$(MAKE) -C $(HELIOS_LIB_DIR) ARCH=x64 BUILD_DIR=../$(BUILD_BASE_DIR)/x64 ROOT_OUTPUT_DIR=../$(OUTPUT_DIR)

# Build WebAssembly version - HeliosLib will check for compiler availability
wasm: setup_dirs
	@echo "Building WebAssembly library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) wasm ARCH=wasm BUILD_DIR=../$(BUILD_BASE_DIR)/wasm ROOT_OUTPUT_DIR=../$(OUTPUT_DIR)

# Upload embedded firmware
upload:
	@echo "Uploading firmware to device..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) upload ROOT_OUTPUT_DIR=../$(OUTPUT_DIR)

# Run tests
tests: setup_dirs
	@echo "Running tests for Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) tests ARCH=x64 BUILD_DIR=../$(BUILD_BASE_DIR)/x64 ROOT_OUTPUT_DIR=../$(OUTPUT_DIR)
	@echo "Running tests for Helios Library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) tests ARCH=x64 BUILD_DIR=../$(BUILD_BASE_DIR)/x64 ROOT_OUTPUT_DIR=../$(OUTPUT_DIR)

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
	@rm -rf assets helios_package helios_package.zip $(BUILD_BASE_DIR) $(OUTPUT_DIR)

# Package all compiled binaries into a zip file
package:
	@echo "Packaging Helios binaries..."
	@$(MAKE) all
	@mkdir -p helios_package
	@cp $(OUTPUT_DIR)/* helios_package/ 2>/dev/null || true
	@cp -r assets helios_package/ 2>/dev/null || true
	@zip -r helios_package.zip helios_package
	@rm -rf helios_package
	@echo "Package created: helios_package.zip"

# Help information
help:
	@echo "Helios Engine Unified Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  embedded       - default, Build the embedded firmware into ./output"
	@echo "  cli            - Build the Helios CLI tool into ./output (implicitly builds lib)"
	@echo "  all            - Build everything into ./output: CLI, embedded, lib, wasm (if available)"
	@echo "  lib            - Build just the Helios library into ./output"
	@echo "  wasm           - Build WebAssembly version into ./output (requires Emscripten)"
	@echo "  upload         - Upload firmware from ./output/helios_firmware.hex"
	@echo "  tests          - Run all tests"
	@echo "  pngs           - Generate PNG documentation files and copy to ./assets"
	@echo "  bmps           - Generate BMP files and copy to ./assets"
	@echo "  clean          - Clean all build artifacts, output files, and storage files"
	@echo "  package        - Build all components and package output/* into a zip file"
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