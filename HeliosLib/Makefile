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

# source files
# local source files first, other sources after
ifeq ($(OS),Windows_NT)
    SRC = $(shell find ../Helios/ -type f -name \\*.cpp) HeliosLib.cpp
else
    SRC = $(shell find ../Helios/ -type f -name '*.cpp') HeliosLib.cpp
endif

# object files are source files with .c replaced with .o
OBJS=\
	$(SRC:.cpp=.o) \

# dependency files are source files with .c replaced with .d
DFILES=\
	$(SRC:.cpp=.d) \

# target dependencies
# this includes any script generated c/h files,
# the $(LLIBS) list, and the $(OBJS) list
DEPS=\
	$(LLIBS) \
	$(OBJS) \

# unit tests
TESTS=\

# target files
ifdef WASM
TARGETS=HeliosLib.js \
	helios.a
else
TARGETS=helios.a
endif

# Default target for 'make' command
all: $(TARGETS)

# unit test target
tests: $(TESTS)

# force sub-build of wasm
wasm: FORCE
	env WASM=1 $(MAKE)

# target for helios lib
helios.a: compute_version $(DEPS)
	$(AR) $@ $(DEPS)

HeliosLib.js: compute_version $(DEPS)
	$(CC) $(LDFLAGS) $(DEPS) -o $@ $(LLIBS)

# catch-all make target to generate .o and .d files
%.o: %.cpp
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# Empty rule that forces %.a to run all the time
FORCE:

# generic clean target
clean:
	@$(RM) $(DFILES) $(OBJS) $(TARGETS) $(TESTS) HeliosLib.js HeliosLib.wasm

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
