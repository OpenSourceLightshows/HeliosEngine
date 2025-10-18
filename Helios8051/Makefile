#####################
### CONFIGURATION ###
#####################

.PHONY: all upload clean compute_version flash

ifneq ($(OS),Windows_NT)
    OS = $(shell uname -s)
endif

#######################
### TOOLCHAIN SETUP ###
#######################

# SDCC Compiler for 8051
CC = sdcc
OBJCOPY = objcopy
PACKIHX = packihx
SIZE = size

# Flash programmer - adjust for your programmer
# Options: stcgal, stc-isp, or custom programmer
PROGRAMMER = stcgal
PROGRAMMER_PORT = /dev/ttyUSB0
PROGRAMMER_BAUD = 115200

########################
### CHIP SETTINGS ######
########################

# CA51F152XX has 16KB flash, 256 bytes RAM
CHIP = CA51F152XX
CPU_SPEED = 8000000L

#######################
### COMPILER FLAGS ####
#######################

# Version information (computed from git)
DEFINES=\
	-DHELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
	-DHELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
	-DHELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
	-DHELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER) \
	-DHELIOS_8051 \
	-DF_CPU=$(CPU_SPEED) \
	-DHELIOS_EMBEDDED

# SDCC Compiler flags for 8051
CFLAGS = \
	-mmcs51 \
	--model-small \
	--xram-size 256 \
	--code-size 16384 \
	--iram-size 256 \
	--std-c11 \
	--opt-code-size \
	--xstack \
	--max-allocs-per-node 50000 \
	--allow-unsafe-read \
	--disable-warning 110 \
	--no-xinit-opt \
	$(DEFINES) \
	$(INCLUDES)

# --model-small: Use small memory model (256 bytes internal RAM)
# --xram-size 256: External RAM size (we have 256 bytes)
# --code-size 16384: Flash size (16KB)
# --iram-size 256: Internal RAM size
# --opt-code-size: Optimize for code size
# --xstack: Use external stack
# --no-xinit-opt: Don't optimize xdata initialization

LDFLAGS = \
	-mmcs51 \
	--model-small \
	--xram-size 256 \
	--code-size 16384 \
	--iram-size 256 \
	--xstack \
	--out-fmt-ihx \
	-L/opt/homebrew/share/sdcc/lib/small

INCLUDES= \
	-I ../Helios

#######################
### SOURCE FILES ######
#######################

# Find all C++ source files in Helios core directory (they contain C-compatible code)
HELIOS_SRCS = $(shell find ../Helios -maxdepth 1 -type f -name '*.cpp')
MAIN_SRC = main.c

# SDCC doesn't support .cpp extension, so we'll compile via temporary .c files
HELIOS_C_FILES = $(HELIOS_SRCS:.cpp=.c)
HELIOS_OBJS = $(HELIOS_SRCS:.cpp=.rel)
MAIN_OBJ = main.rel

OBJS = $(HELIOS_OBJS) $(MAIN_OBJ)

#######################
### BUILD TARGETS #####
#######################

TARGET = helios

all: compute_version $(TARGET).hex
	@echo "== Success building Helios v$(HELIOS_VERSION_NUMBER) for 8051 =="
	@echo "Flash usage:"
	@$(SIZE) $(TARGET).ihx 2>/dev/null || echo "Size information not available"

$(TARGET).hex: $(TARGET).ihx
	@echo "Converting to hex format..."
	$(PACKIHX) $(TARGET).ihx > $(TARGET).hex

$(TARGET).ihx: compute_version $(OBJS)
	@echo "Linking..."
	$(CC) $(LDFLAGS) $(OBJS) -o $@

# Compile .cpp files from Helios directory by creating temporary .c file
# SDCC only accepts .c extension but the code is C-compatible
../Helios/%.rel: ../Helios/%.cpp
	@echo "Compiling $< (as C)..."
	@cp $< $(<:.cpp=.c)
	@$(CC) $(CFLAGS) -c $(<:.cpp=.c) -o $@
	@rm -f $(<:.cpp=.c)

# Compile main.c
%.rel: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

#######################
### UPLOAD TARGET #####
#######################

upload: $(TARGET).hex
	@echo "Uploading to CA51F152XX via $(PROGRAMMER)..."
ifeq ($(PROGRAMMER),stcgal)
	stcgal -p $(PROGRAMMER_PORT) -b $(PROGRAMMER_BAUD) $(TARGET).hex
else
	@echo "Please configure your programmer in the Makefile"
	@echo "Default programmer is stcgal, adjust PROGRAMMER variable as needed"
endif

flash: upload

#######################
### VERSION COMPUTE ###
#######################

compute_version:
	$(eval LATEST_TAG ?= $(shell git fetch --depth=1 origin +refs/tags/*:refs/tags/* 2>/dev/null; git tag --list | grep -E '^[0-9]+\.[0-9]+\.[0-9]+$$' | sort -V | tail -n1))
	$(eval HELIOS_VERSION_MAJOR ?= $(shell echo $(LATEST_TAG) | cut -d. -f1))
	$(eval HELIOS_VERSION_MINOR ?= $(shell echo $(LATEST_TAG) | cut -d. -f2))
	$(eval LAST_HELIOS_BUILD_NUMBER ?= $(shell echo $(LATEST_TAG) | cut -d. -f3))
	$(eval COMMITS_SINCE_TAG := $(shell git rev-list --count $(LATEST_TAG)..HEAD 2>/dev/null || echo 0))
	$(eval HELIOS_BUILD_NUMBER := $(shell echo $$(( $(LAST_HELIOS_BUILD_NUMBER) + $(COMMITS_SINCE_TAG) ))))
	$(eval HELIOS_VERSION_MAJOR := $(if $(HELIOS_VERSION_MAJOR),$(HELIOS_VERSION_MAJOR),1))
	$(eval HELIOS_VERSION_MINOR := $(if $(HELIOS_VERSION_MINOR),$(HELIOS_VERSION_MINOR),0))
	$(eval HELIOS_BUILD_NUMBER := $(if $(HELIOS_BUILD_NUMBER),$(HELIOS_BUILD_NUMBER),0))
	$(eval HELIOS_VERSION_NUMBER := $(HELIOS_VERSION_MAJOR).$(HELIOS_VERSION_MINOR).$(HELIOS_BUILD_NUMBER))

#####################
####### CLEAN #######
#####################

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(OBJS)
	rm -f ../Helios/*.rel ../Helios/*.sym ../Helios/*.lst ../Helios/*.rst ../Helios/*.asm ../Helios/*.mem ../Helios/*.map
	rm -f *.rel *.sym *.lst *.rst *.asm *.mem *.map
	rm -f $(TARGET).ihx $(TARGET).hex $(TARGET).lk $(TARGET).map $(TARGET).mem

.PHONY: all upload flash clean compute_version
