.SUFFIXES:
.PHONY: all lib cli embedded wasm install clean package

######################
### CONFIGURATION ###
######################

LIB_DIR      = HeliosLib
CLI_DIR      = HeliosCLI
EMBEDDED_DIR = HeliosEmbedded
PACKAGE_DIR  = release_package
VERSION     ?= $(shell git describe --tags --always --dirty 2>/dev/null || echo "dev")

######################
### INSTALL TARGET ###
######################

install:
	@echo "== Installing dependencies =="
	@if command -v apt-get >/dev/null 2>&1; then \
		echo "Installing AVR toolchain via apt-get..."; \
		sudo apt-get update; \
		sudo apt-get install -y gcc-avr avr-libc binutils-avr; \
	elif command -v brew >/dev/null 2>&1; then \
		echo "Installing AVR toolchain via brew..."; \
		brew tap osx-cross/avr; \
		brew install avr-gcc; \
	elif command -v pacman >/dev/null 2>&1; then \
		echo "Installing AVR toolchain via pacman..."; \
		sudo pacman -S --noconfirm avr-gcc avr-libc; \
	else \
		echo "No supported package manager found. Please install AVR toolchain manually:"; \
		echo "  - gcc-avr"; \
		echo "  - avr-libc"; \
		echo "  - binutils-avr"; \
		exit 1; \
	fi
	@echo "== Dependencies installed successfully =="

######################
### BUILD TARGETS ###
######################

all: lib cli embedded wasm

lib:
	@echo "== Building libHelios.a =="
	$(MAKE) -C $(LIB_DIR)

cli:
	@echo "== Building HeliosCLI =="
	$(MAKE) -C $(CLI_DIR)

embedded:
	@echo "== Building HeliosEmbedded =="
	$(MAKE) -C $(EMBEDDED_DIR)

wasm:
	@echo "== Building WASM target =="
	$(MAKE) -C $(LIB_DIR) TARGET=wasm

######################
### PACKAGE TARGET ###
######################

package: all
	@echo "== Creating release package v$(VERSION) =="
	@rm -rf $(PACKAGE_DIR)
	@mkdir -p $(PACKAGE_DIR)
	@mkdir -p $(PACKAGE_DIR)/cli
	@mkdir -p $(PACKAGE_DIR)/embedded
	@mkdir -p $(PACKAGE_DIR)/wasm

	# Copy CLI binary and related files
	@if [ -f $(CLI_DIR)/build/desktop/helios ]; then \
		cp $(CLI_DIR)/build/desktop/helios $(PACKAGE_DIR)/cli/; \
		echo "✓ CLI binary packaged"; \
	else \
		echo "✗ CLI binary not found"; \
	fi

	# Copy embedded firmware files
	@if [ -d $(EMBEDDED_DIR)/build/avr ]; then \
		cp $(EMBEDDED_DIR)/build/avr/helios.hex $(PACKAGE_DIR)/embedded/ 2>/dev/null || true; \
		cp $(EMBEDDED_DIR)/build/avr/helios.bin $(PACKAGE_DIR)/embedded/ 2>/dev/null || true; \
		cp $(EMBEDDED_DIR)/build/avr/helios.elf $(PACKAGE_DIR)/embedded/ 2>/dev/null || true; \
		cp $(EMBEDDED_DIR)/build/avr/helios.map $(PACKAGE_DIR)/embedded/ 2>/dev/null || true; \
		echo "✓ Embedded firmware packaged"; \
	else \
		echo "✗ Embedded firmware not found"; \
	fi

	# Copy WASM files
	@if [ -d $(LIB_DIR)/build/wasm ]; then \
		cp $(LIB_DIR)/build/wasm/HeliosLib.js $(PACKAGE_DIR)/wasm/ 2>/dev/null || true; \
		cp $(LIB_DIR)/build/wasm/HeliosLib.wasm $(PACKAGE_DIR)/wasm/ 2>/dev/null || true; \
		echo "✓ WASM library packaged"; \
	else \
		echo "✗ WASM library not found"; \
	fi

	# Create version info file
	@echo "Helios Engine Release Package" > $(PACKAGE_DIR)/VERSION.txt
	@echo "Version: $(VERSION)" >> $(PACKAGE_DIR)/VERSION.txt
	@echo "Build Date: $(shell date)" >> $(PACKAGE_DIR)/VERSION.txt
	@echo "Git Commit: $(shell git rev-parse HEAD 2>/dev/null || echo 'unknown')" >> $(PACKAGE_DIR)/VERSION.txt

	# Create the final archive
	@cd $(PACKAGE_DIR) && zip -r ../helios-$(VERSION).zip .
	@echo "== Package created: helios-$(VERSION).zip =="
	@echo "Package contents:"
	@ls -la $(PACKAGE_DIR)/

######################
### CLEAN TARGET ###
######################

clean:
	@echo "== Cleaning all builds =="
	$(MAKE) -C $(LIB_DIR) clean
	$(MAKE) -C $(CLI_DIR) clean
	$(MAKE) -C $(EMBEDDED_DIR) clean
	@rm -rf $(PACKAGE_DIR)
	@rm -f helios-*.zip