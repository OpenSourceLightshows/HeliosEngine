.SUFFIXES:
.PHONY: all clean upload set_fuses extract_hex compute_version

TARGET = avr
BUILD_DIR = build/$(TARGET)
CORE_LIB = ../Helios/build/$(TARGET)/helios.a

AVR_CHIP = attiny85
CPU_SPEED = 8000000L
BINDIR = $(HOME)/atmel_setup/avr8-gnu-toolchain-linux_x86_64/bin/

CC = $(BINDIR)avr-g++
LD = $(CC)
AR = $(BINDIR)avr-gcc-ar
OBJCOPY = $(BINDIR)avr-objcopy
NM = $(BINDIR)avr-nm
OBJDUMP = $(BINDIR)avr-objdump
AVRDUDE = avrdude
AVRDUDE_FLAGS = -Cavrdude.conf -p$(AVR_CHIP) -cusbasp -v -B1

FIRMWARE = $(BUILD_DIR)/helios
ELF = $(FIRMWARE).elf
HEX = $(FIRMWARE).hex
BIN = $(FIRMWARE).bin
EEP = $(FIRMWARE).eep
LST = $(FIRMWARE).lst
MAP = $(FIRMWARE).map

CFLAGS = -Os -Wall -std=gnu++17 -MMD \
	-mmcu=$(AVR_CHIP) -DF_CPU=$(CPU_SPEED) \
	-flto -fno-exceptions -fdata-sections -ffunction-sections \
	-DHELIOS_EMBEDDED \
	-DHELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
	-DHELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
	-DHELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
	-DHELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER) \
	-I ../Helios

LDFLAGS = -Os -mmcu=$(AVR_CHIP) -flto -Wl,--gc-sections

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

all: compute_version $(HEX)
	@chmod +x ../HeliosEmbedded/avrsize.sh
	@../HeliosEmbedded/avrsize.sh $(ELF)
	@echo "== Success building HeliosEmbedded v$(HELIOS_VERSION_NUMBER) =="

$(HEX): $(ELF)
	$(OBJCOPY) -O binary -R .eeprom $< $(BIN)
	$(OBJCOPY) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $< $(EEP)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(ELF): $(OBJS) $(CORE_LIB)
	$(LD) $(LDFLAGS) $(OBJS) $(CORE_LIB) -o $@
	$(OBJDUMP) --disassemble --source --line-numbers --demangle --section=.text $@ > $(LST)
	$(NM) --numeric-sort --line-numbers --demangle --print-size --format=bsd $@ > $(MAP)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(CORE_LIB):
	$(MAKE) -C ../Helios TARGET=$(TARGET) \
		HELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
		HELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
		HELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
		HELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER)

upload: $(HEX)
	$(AVRDUDE) $(AVRDUDE_FLAGS) -Uflash:w:$(HEX):i

set_fuses:
	@echo "Setting fuses for $(CPU_SPEED)..."
ifeq ($(CPU_SPEED),16000000L)
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0xe1:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
else ifeq ($(CPU_SPEED),8000000L)
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0xE2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
else ifeq ($(CPU_SPEED),1000000L)
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
else
	@echo "Unsupported CPU_SPEED for fuse settings."
endif

extract_hex:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:r:firmware_extracted.hex:i

compute_version:
	$(eval LATEST_TAG ?= $(shell \
		git fetch --depth=1 origin +refs/tags/*:refs/tags/* &> /dev/null 2>&1 && \
		git tag --list | grep -E '^[0-9]+\.[0-9]+\.[0-9]+$$' | sort -V | tail -n1 \
	))
	$(eval HELIOS_VERSION_MAJOR ?= $(shell echo $(LATEST_TAG) | cut -d. -f1))
	$(eval HELIOS_VERSION_MINOR ?= $(shell echo $(LATEST_TAG) | cut -d. -f2))
	$(eval LAST_HELIOS_BUILD_NUMBER ?= $(shell echo $(LATEST_TAG) | cut -d. -f3))
	$(eval COMMITS_SINCE_TAG := $(shell git rev-list --count $(LATEST_TAG)..HEAD))
	$(eval HELIOS_BUILD_NUMBER := $(shell echo $$(( $(LAST_HELIOS_BUILD_NUMBER) + $(COMMITS_SINCE_TAG) ))))
	$(eval HELIOS_VERSION_MAJOR := $(if $(HELIOS_VERSION_MAJOR),$(HELIOS_VERSION_MAJOR),1))
	$(eval HELIOS_VERSION_MINOR := $(if $(HELIOS_VERSION_MINOR),$(HELIOS_VERSION_MINOR),0))
	$(eval HELIOS_BUILD_NUMBER := $(if $(HELIOS_BUILD_NUMBER),$(HELIOS_BUILD_NUMBER),0))
	$(eval HELIOS_VERSION_NUMBER := $(HELIOS_VERSION_MAJOR).$(HELIOS_VERSION_MINOR).$(HELIOS_BUILD_NUMBER))

clean:
	rm -rf build

-include $(DEPS)
