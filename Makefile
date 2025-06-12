.SUFFIXES:
.PHONY: all lib cli embedded wasm clean

######################
### CONFIGURATION ###
######################

LIB_DIR      = HeliosLib
CLI_DIR      = HeliosCLI
EMBEDDED_DIR = HeliosEmbedded

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
### CLEAN TARGET ###
######################

clean:
	@echo "== Cleaning all builds =="
	$(MAKE) -C $(LIB_DIR) clean
	$(MAKE) -C $(CLI_DIR) clean
	$(MAKE) -C $(EMBEDDED_DIR) clean