# Unified Makefile for Helios Engine
# This Makefile runs commands in subdirectories

# List all make targets which are not filenames
.PHONY: all clean lib cli embedded tests wasm docs help version

# Directories containing Makefiles
HELIOS_LIB_DIR = HeliosLib
HELIOS_CLI_DIR = HeliosCLI
HELIOS_EMBEDDED_DIR = HeliosEmbedded

# Default target
all: lib cli

# Version information
version:
	@echo "Computing Helios version across all components..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) compute_version
	@$(MAKE) -C $(HELIOS_CLI_DIR) compute_version
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) compute_version

# Build the Helios library
lib:
	@echo "Building Helios library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR)

# Build the Helios CLI
cli:
	@echo "Building Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR)

# Build the embedded version
embedded:
	@echo "Building Helios Embedded..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR)

# Build WebAssembly version
wasm:
	@echo "Building WebAssembly library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) wasm

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

# Generate BMPs for documentation
bmps:
	@echo "Generating BMP files..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) bmps

# Clean all build artifacts
clean:
	@echo "Cleaning Helios library..."
	@$(MAKE) -C $(HELIOS_LIB_DIR) clean
	@echo "Cleaning Helios CLI..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) clean
	@echo "Cleaning Helios Embedded..."
	@$(MAKE) -C $(HELIOS_EMBEDDED_DIR) clean

# Clean storage files
clean_storage:
	@echo "Cleaning storage files..."
	@$(MAKE) -C $(HELIOS_CLI_DIR) clean_storage

# Help information
help:
	@echo "Helios Engine Unified Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all            - Build library and CLI (default)"
	@echo "  lib            - Build the Helios library"
	@echo "  cli            - Build the Helios CLI tool"
	@echo "  embedded       - Build the embedded firmware"
	@echo "  wasm           - Build WebAssembly version of library"
	@echo "  upload         - Upload firmware to ATtiny device"
	@echo "  tests          - Run all tests"
	@echo "  pngs           - Generate PNG documentation files"
	@echo "  bmps           - Generate BMP files"
	@echo "  clean          - Clean all build artifacts"
	@echo "  clean_storage  - Clean Helios storage files"
	@echo "  version        - Compute version information"
	@echo "  help           - Show this help information"