.SUFFIXES:
.PHONY: all upload set_fuses set_default_fuses set_16mhz_fuses set_8mhz_fuses set_1mhz_fuses get_fuses extract_hex upload_hex extract_eeprom upload_eeprom clean compute_version extract_version

#####################
### CONFIGURATION ###
#####################

TARGET = avr
BUILD_DIR = build/$(TARGET)
CORE_LIB = ../Helios/build/$(TARGET)/helios.a

AVR_CHIP = attiny85
CPU_SPEED = 8000000L

# OS Detection and toolchain paths
ifeq ($(OS),Windows_NT) # Windows
    BINDIR="C:/Program Files (x86)/Atmel/Studio/7.0/toolchain/avr8/avr8-gnu-toolchain/bin/"
    AVRDUDEDIR="$(shell echo "$$LOCALAPPDATA")/Arduino15/packages/DxCore/tools/avrdude/6.3.0-arduino17or18/bin/"
    PYTHON="$(shell echo "$$LOCALAPPDATA")/Arduino15/packages/megaTinyCore/tools/python3/3.7.2-post1/python3"
    PYPROG="$(shell echo "$$LOCALAPPDATA")/Arduino15/packages/megaTinyCore/hardware/megaavr/2.6.5/tools/prog.py"
    DEVICE_DIR="C:/Program Files (x86)/Atmel/Studio/7.0/Packs/atmel/ATtiny_DFP/1.10.348/gcc/dev/attiny85/"
    INCLUDE_DIR="C:/Program Files (x86)/Atmel/Studio/7.0/Packs/atmel/ATtiny_DFP/1.10.348/include/"
else ifeq ($(shell uname),Linux)
    # Use system-installed AVR toolchain (from apt-get install gcc-avr avr-libc)
    BINDIR=
    DEVICE_DIR=
    INCLUDE_DIR=
else ifeq ($(shell uname),Darwin)
    BINDIR=/opt/homebrew/bin/
    AVRDUDEDIR=/opt/homebrew/bin/
    INCLUDE_DIR=""
else
    $(error Unknown operating system)
endif

#######################
### TOOLCHAIN SETUP ###
#######################

CC = ${BINDIR}avr-g++
LD = ${BINDIR}avr-g++
AR = ${BINDIR}avr-gcc-ar
OBJCOPY = ${BINDIR}avr-objcopy -v
NM = ${BINDIR}avr-nm
OBJDUMP = ${BINDIR}avr-objdump
AVRDUDE = ${AVRDUDEDIR}avrdude
SIZE = ${BINDIR}avr-size

########################
### AVRDUDE SETTINGS ###
########################

AVRDUDE_CONF = /opt/homebrew/etc/avrdude.conf
AVRDUDE_CHIP = attiny85

ifeq ($(OS),Windows_NT) # Windows
    AVRDUDE_PROGRAMMER = atmelice_isp
    AVRDUDE_BAUDRATE = 115200
    AVRDUDE_PORT = usb
else ifeq ($(OS),Darwin)
    AVRDUDE_PROGRAMMER = usbtiny
    AVRDUDE_BAUDRATE = 19200
    AVRDUDE_PORT = usb
endif

AVRDUDE_FLAGS = -C$(AVRDUDE_CONF) \
		-p$(AVRDUDE_CHIP) \
		-c$(AVRDUDE_PROGRAMMER) \
		-P$(AVRDUDE_PORT) \
		-b$(AVRDUDE_BAUDRATE) \
		-v \
		-B1

# -v -- Verbose output - display detailed progress
# -B1 -- Bit clock period (in microseconds) - sets programming speed
# -V -- Disable automatic verify check when uploading
# -D -- Disable auto erase for flash memory

FIRMWARE = $(BUILD_DIR)/helios
ELF = $(FIRMWARE).elf
HEX = $(FIRMWARE).hex
BIN = $(FIRMWARE).bin
EEP = $(FIRMWARE).eep
LST = $(FIRMWARE).lst
MAP = $(FIRMWARE).map

#######################
### COMPILER FLAGS ####
#######################

CFLAGS = -g \
	 -Os \
	 -MMD \
	 -Wall \
	 -flto \
	 -mrelax \
	 -std=gnu++17 \
	 -fshort-enums \
	 -fpack-struct \
	 -fno-exceptions \
	 -fdata-sections \
	 -funsigned-char \
	 -ffunction-sections \
	 -funsigned-bitfields \
	 -mcall-prologues \
	 -fno-threadsafe-statics \
	 -mcall-prologues \
	 -D__AVR_ATtiny85__ \
	 -mmcu=$(AVR_CHIP) \
	 -DF_CPU=$(CPU_SPEED) \
	 -DHELIOS_EMBEDDED \
	 -DHELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
	 -DHELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
	 -DHELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
	 -DHELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER)

LDFLAGS = -g \
	  -Wall \
	  -Os \
	  -flto \
	  -fuse-linker-plugin \
	  -Wl,--gc-sections \
	  -mrelax \
	  -lm \
	  -mmcu=$(AVR_CHIP)

ifeq ($(OS),Windows_NT) # Windows
    CFLAGS+=-B $(DEVICE_DIR)
    LDFLAGS+=-B $(DEVICE_DIR)
endif

ifneq ($(DEFINES),)
    CFLAGS+=$(DEFINES)
endif

INCLUDES= \
	-I $(INCLUDE_DIR) \
	-I ../Helios

#######################
### SOURCE FILES ######
#######################

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

#######################
### BUILD TARGETS #####
#######################

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
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(CORE_LIB):
	$(MAKE) -C ../Helios TARGET=$(TARGET) \
		HELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
		HELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
		HELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
		HELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER)

#####################
####### FUSES #######
#####################

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

set_16mhz_fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0xe1:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

set_8mhz_fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0xE2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

set_1mhz_fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m

set_8mhz_fuses_bod_on:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0xE2:m -U hfuse:w:0xde:m -U efuse:w:0xff:m

get_fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

######################
####### EEPROM #######
######################

upload_with_default_eeprom: upload eeprom_data.eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U eeprom:w:eeprom_data.eep:i

extract_eeprom:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U eeprom:r:eeprom_data.eep:i

upload_eeprom: eeprom_data.eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U eeprom:w:eeprom_data.eep:i

extract_eeprom_thumbs:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U eeprom:r:eeprom_thumbs.eep:i

upload_eeprom_thumbs: eeprom_thumbs.eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U eeprom:w:eeprom_thumbs.eep:i

extract_eeprom_fingers:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U eeprom:r:eeprom_fingers.eep:i

upload_eeprom_fingers: eeprom_fingers.eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U eeprom:w:eeprom_fingers.eep:i

###################
####### HEX #######
###################

upload_hex:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$(HEX):i

extract_hex:
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:r:firmware_extracted.hex:i

##########################
### VERSION MANAGEMENT ###
##########################

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

extract_version:
	@echo "Reading firmware version..."
	$(eval TEMP_HEX := $(shell mktemp))
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:r:$(TEMP_HEX):i > /dev/null 2>&1
	python ./find_version.py $(TEMP_HEX)
	@rm $(TEMP_HEX)

#####################
####### CLEAN #######
#####################

clean:
	rm -rf build

-include $(DEPS)

