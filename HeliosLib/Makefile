# clear out all default make targets
.SUFFIXES:

# List all make targets which are not filenames
.PHONY: all tests clean wasm compute_version

# compiler tool definitions
ifdef WASM
CC=em++
AR=emar cru
else
CC=g++
AR=ar cru
endif

MAKE=make
RM=rm -rf
RANLIB=ranlib

CFLAGS=-O2 -Wall

# add -g flag for debug symbols if we're not using wasm
ifndef WASM
CFLAGS += -g
endif

# compiler defines
DEFINES=\
	-D HELIOS_CLI \
	-D HELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
	-D HELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
	-D HELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
	-D HELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER)

# compiler include paths
INCLUDES=\
	-I ../Helios/ \
	-I ./ \

ifdef WASM
DEFINES += -D WASM
# this was used before but I think only the above is necessary
LDFLAGS += --bind -s EXPORT_ES6=1 -s EXPORT_NAME='HeliosLib'
endif

# only set them if they're not empty to prevent unnecessary whitespace
ifneq ($(DEFINES),)
    CFLAGS+=$(DEFINES)
endif
ifneq ($(INCLUDES),)
    CFLAGS+=$(INCLUDES)
endif

# local NONSTANDARD libraries to link with
# these MUST be exact filenames, cannot be -l syntax
# for example:
#   ../path/to/libname.a
# NOT THIS:
#   -L../path/to -lname
# Youheliosshould NOT need to add a make target to build
# this library if you have added it correctly.
LLIBS=\

# STANDARD libraries to link with (-l is fine here)
# MUST have LLIBS BEFORE the standard libraries
LIBS=\
	$(LLIBS) \

# Architecture passed from parent Makefile (default to x64 if run standalone)
ARCH ?= x64

# Build and Output directories relative to this Makefile
BUILD_DIR = $(ARCH)
OUTPUT_DIR = output
FINAL_LIB_TARGET = $(OUTPUT_DIR)/helios_lib.a
FINAL_WASM_TARGET = $(OUTPUT_DIR)/helios_wasm.js
FINAL_WASM_BIN_TARGET = $(OUTPUT_DIR)/helios_wasm.wasm

# source files
ifeq ($(OS),Windows_NT)
    HELIOS_SRC = $(shell find ../Helios/ -type f -name \\*.cpp)
    LOCAL_SRC = HeliosLib.cpp
else
    HELIOS_SRC = $(shell find ../Helios/ -type f -name '*.cpp')
    LOCAL_SRC = HeliosLib.cpp
endif

# Generate object file paths using BUILD_DIR
HELIOS_OBJS = $(patsubst ../Helios/%.cpp,$(BUILD_DIR)/Helios/%.o,$(HELIOS_SRC))
LOCAL_OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(LOCAL_SRC))
OBJS = $(HELIOS_OBJS) $(LOCAL_OBJS)

# Generate dependency file paths
HELIOS_DFILES = $(patsubst ../Helios/%.cpp,$(BUILD_DIR)/Helios/%.d,$(HELIOS_SRC))
LOCAL_DFILES = $(patsubst %.cpp,$(BUILD_DIR)/%.d,$(LOCAL_SRC))
DFILES = $(HELIOS_DFILES) $(LOCAL_DFILES)

# target dependencies
DEPS=\
	$(LLIBS) \
	$(OBJS) \

# unit tests
TESTS=\

# target files
ifdef WASM
TARGETS=$(FINAL_WASM_TARGET) $(FINAL_LIB_TARGET)
else
TARGETS=$(FINAL_LIB_TARGET)
endif

# Default target for 'make' command
all: $(TARGETS)

# unit test target
tests: $(TESTS)

# force sub-build of wasm
wasm: FORCE
	@if command -v em++; then \
		echo "WebAssembly compiler found, building WASM..."; \
		env ARCH=wasm WASM=1 $(MAKE) $(FINAL_WASM_TARGET) $(FINAL_LIB_TARGET); \
	else \
		echo "WebAssembly compiler (em++) not found, skipping WASM build."; \
		mkdir -p $(OUTPUT_DIR); \
		touch $(FINAL_WASM_TARGET); \
		touch $(FINAL_WASM_BIN_TARGET); \
	fi

# target for helios lib
$(FINAL_LIB_TARGET): compute_version $(DEPS)
	@mkdir -p $(dir $@)
	$(AR) $@ $(DEPS)
	@echo "Created $@"

# target for HeliosLib.js
$(FINAL_WASM_TARGET): compute_version $(DEPS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(DEPS) -o $@ $(LLIBS)

# Rules for building object files
$(BUILD_DIR)/Helios/%.o: ../Helios/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Empty rule that forces .a to run all the time
FORCE:

# generic clean target
clean:
	@echo "Cleaning up HeliosLib..."
	@$(RM) -rf $(BUILD_DIR) $(OUTPUT_DIR)
	@$(RM) -f $(DFILES) $(OBJS) $(TESTS) HeliosLib.js HeliosLib.wasm # Remove old possible output names
	@echo "Cleaned HeliosLib!"

compute_version:
	$(eval LATEST_TAG ?= $(shell git fetch --depth=1 origin +refs/tags/*:refs/tags/* &> /dev/null && git tag --list | sort -V | tail -n1))
	$(eval HELIOS_VERSION_MAJOR ?= $(shell echo $(LATEST_TAG) | cut -d. -f1))
	$(eval HELIOS_VERSION_MINOR ?= $(shell echo $(LATEST_TAG) | cut -d. -f2))
	$(eval LAST_HELIOS_BUILD_NUMBER ?= $(shell echo $(LATEST_TAG) | cut -d. -f3))
	$(eval COMMITS_SINCE_TAG := $(shell git rev-list --count $(LATEST_TAG)..HEAD))
	$(eval HELIOS_BUILD_NUMBER := $(shell echo $$(( $(LAST_HELIOS_BUILD_NUMBER) + $(COMMITS_SINCE_TAG) ))))
	$(eval HELIOS_VERSION_MAJOR := $(if $(HELIOS_VERSION_MAJOR),$(HELIOS_VERSION_MAJOR),1))
	$(eval HELIOS_VERSION_MINOR := $(if $(HELIOS_VERSION_MINOR),$(HELIOS_VERSION_MINOR),0))
	$(eval HELIOS_BUILD_NUMBER := $(if $(HELIOS_BUILD_NUMBER),$(HELIOS_BUILD_NUMBER),0))
	$(eval HELIOS_VERSION_NUMBER := $(HELIOS_VERSION_MAJOR).$(HELIOS_VERSION_MINOR).$(HELIOS_BUILD_NUMBER))

# Now include our target dependency files
# the hyphen means ignore non-existent files
-include $(DFILES)
