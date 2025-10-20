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

CC = sdcc
LD = sdcc
OBJCOPY = objcopy
FLASHER = stm8flash

########################
### MCU SETTINGS #######
########################

# STM8S001J3M3TR specs:
# - 8KB Flash
# - 128 bytes EEPROM
# - 1KB RAM
# - 16MHz internal RC oscillator
# - Package: SO-8
MCU = stm8s001j3
MCU_FAMILY = STM8S001
CPU_SPEED = 16000000L

# STM8S001J3M3TR pinout (SO-8) - Based on schematic:
# Pin 1: PD1/SWIM - Programming
# Pin 2: VSS - Ground
# Pin 3: VCAP - Internal regulator capacitor (1µF to ground)
# Pin 4: VDD/VDDA - Power (6V from 2x CR1620 batteries)
# Pin 5: PB4 - LED Blue (TIM1_CH1N complementary output)
# Pin 6: PB5 - Button Input (active HIGH with external pull-down)
# Pin 7: VDD - Power (connected to Pin 4)
# Pin 8: PD6/AIN6 - Available for expansion

# Pin assignments for RGB LED and button (from schematic)
# RGB LED Pins (PWM capable via alternate functions):
LED_R_PORT = C
LED_R_PIN = 5
LED_G_PORT = C
LED_G_PIN = 4
LED_B_PORT = B
LED_B_PIN = 4

# Button Pin:
BUTTON_PORT = B
BUTTON_PIN = 5

########################
### PROGRAMMER #########
########################

# For stm8flash programmer
PROGRAMMER = stlinkv2
PROGRAMMER_PORT = usb

#######################
### COMPILER FLAGS ####
#######################

# compiler defines
DEFINES = \
	-DHELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
	-DHELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
	-DHELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
	-DHELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER) \
	-D$(MCU_FAMILY) \
	-DF_CPU=$(CPU_SPEED) \
	-DHELIOS_EMBEDDED \
	-DHELIOS_STM8 \
	-DLED_R_PORT_$(LED_R_PORT) \
	-DLED_R_PIN=$(LED_R_PIN) \
	-DLED_G_PORT_$(LED_G_PORT) \
	-DLED_G_PIN=$(LED_G_PIN) \
	-DLED_B_PORT_$(LED_B_PORT) \
	-DLED_B_PIN=$(LED_B_PIN) \
	-DBUTTON_PORT_$(BUTTON_PORT) \
	-DBUTTON_PIN=$(BUTTON_PIN)

# SDCC compiler flags for STM8
CFLAGS = \
	-mstm8 \
	--std-sdcc11 \
	--opt-code-size \
	--stack-auto \
	--fverbose-asm \
	--float-reent \
	--no-peep \
	$(DEFINES)

# Linker flags
LDFLAGS = \
	-mstm8 \
	--out-fmt-ihx \
	--opt-code-size

INCLUDES = \
	-I. \
	-I../Helios

CFLAGS += $(INCLUDES)

#######################
### SOURCE FILES ######
#######################

# Core Helios source files (using .cpp extension but C-compatible)
HELIOS_SRCS = \
	../Helios/Helios.cpp \
	../Helios/Button.cpp \
	../Helios/Led.cpp \
	../Helios/Pattern.cpp \
	../Helios/Patterns.cpp \
	../Helios/Colorset.cpp \
	../Helios/Colortypes.cpp \
	../Helios/TimeControl.cpp \
	../Helios/Storage.cpp \
	../Helios/Timer.cpp \
	../Helios/Random.cpp

# STM8-specific source files
STM8_SRCS = \
	main.c \
	stm8_init.c

SRCS = $(HELIOS_SRCS) $(STM8_SRCS)

# Object files (handle both .c and .cpp extensions)
OBJS = $(patsubst %.cpp,%.rel,$(patsubst %.c,%.rel,$(SRCS)))

#######################
### BUILD TARGETS #####
#######################

# Target name
TARGET = helios

all: compute_version $(TARGET).ihx
	@echo "== Success building Helios v$(HELIOS_VERSION_NUMBER) for STM8S001J3M3TR =="
	@echo "Flash size: 8KB"
	@echo "RAM size: 1KB"
	@echo "EEPROM size: 128 bytes"

$(TARGET).ihx: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@
	@if [ -f $(TARGET).map ]; then \
		echo "Memory usage:"; \
		grep "ROM" $(TARGET).map || true; \
		grep "RAM" $(TARGET).map || true; \
	fi

%.rel: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# For .cpp files, tell SDCC to treat them as C
%.rel: %.cpp
	$(CC) $(CFLAGS) -x c -c $< -o $@

# Flash the firmware to STM8
flash: $(TARGET).ihx
	$(FLASHER) -c $(PROGRAMMER) -p $(MCU) -w $(TARGET).ihx

# Upload is alias for flash
upload: flash

#######################
### VERSION COMPUTE ###
#######################

compute_version:
	$(eval LATEST_TAG ?= $(shell git fetch --depth=1 origin +refs/tags/*:refs/tags/* 2>/dev/null && git tag --list | grep -E '^[0-9]+\.[0-9]+\.[0-9]+$$' | sort -V | tail -n1))
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
	rm -f $(OBJS) $(TARGET).ihx $(TARGET).map $(TARGET).lk $(TARGET).mem
	rm -f ../Helios/*.rel ../Helios/*.asm ../Helios/*.lst ../Helios/*.rst ../Helios/*.sym
	rm -f *.asm *.lst *.rst *.sym *.rel
