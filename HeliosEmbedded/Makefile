#####################
### CONFIGURATION ###
#####################

.PHONY: all upload set_fuses set_default_fuses set_16mhz_fuses set_8mhz_fuses set_1mhz_fuses get_fuses extract_hex upload_hex extract_eeprom upload_eeprom clean compute_version extract_version

ifneq ($(OS),Windows_NT)
    OS = $(shell uname -s)
endif

#######################
### OS-SPECIFIC VARS ###
#######################

ifeq ($(OS),Windows_NT) # Windows
    BINDIR="C:/Program Files (x86)/Atmel/Studio/7.0/toolchain/avr8/avr8-gnu-toolchain/bin/"
    AVRDUDEDIR="$(shell echo "$$LOCALAPPDATA")/Arduino15/packages/DxCore/tools/avrdude/6.3.0-arduino17or18/bin/"
    PYTHON="$(shell echo "$$LOCALAPPDATA")/Arduino15/packages/megaTinyCore/tools/python3/3.7.2-post1/python3"
    PYPROG="$(shell echo "$$LOCALAPPDATA")/Arduino15/packages/megaTinyCore/hardware/megaavr/2.6.5/tools/prog.py"
    DEVICE_DIR="C:/Program Files (x86)/Atmel/Studio/7.0/Packs/atmel/ATtiny_DFP/1.10.348/gcc/dev/attiny85/"
    INCLUDE_DIR="C:/Program Files (x86)/Atmel/Studio/7.0/Packs/atmel/ATtiny_DFP/1.10.348/include/"
else ifeq ($(OS),Linux)
    BINDIR=~/atmel_setup/avr8-gnu-toolchain-linux_x86_64/bin/
    DEVICE_DIR=~/atmel_setup/gcc/dev/attiny85
    INCLUDE_DIR=~/atmel_setup/include/
else ifeq ($(OS),Darwin)
		BINDIR=/opt/homebrew/bin/
    AVRDUDEDIR=/opt/homebrew/bin/
    INCLUDE_DIR=""
else
    # ??? unknown OS
endif

#######################
### TOOLCHAIN SETUP ###
#######################

CC = ${BINDIR}avr-g++
LD = ${BINDIR}avr-g++
OBJCOPY = ${BINDIR}avr-objcopy -v
AR = ${BINDIR}avr-gcc-ar
SIZE = ${BINDIR}avr-size
OBJDUMP = ${BINDIR}avr-objdump
NM = ${BINDIR}avr-nm
AVRDUDE = ${AVRDUDEDIR}avrdude

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


#######################
### COMPILER FLAGS ####
#######################

CPU_SPEED = 8000000L

# the port for serial upload
SERIAL_PORT = COM11

# compiler defines
DEFINES=\
	-DHELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
	-DHELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
	-DHELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
	-DHELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER) \
	-D__AVR_ATtiny85__ \
	-DF_CPU=$(CPU_SPEED) \
	-D HELIOS_EMBEDDED

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
	 -ffunction-sections\
	 -funsigned-bitfields \
	 -fno-threadsafe-statics \
	 -fno-rtti \
	 -mcall-prologues \
	 -D__AVR_ATtiny85__ \
	 -mmcu=$(AVRDUDE_CHIP) \
	 -DF_CPU=$(CPU_SPEED) \

LDFLAGS = -g \
	  -Wall \
	  -Os \
	  -flto \
	  -fuse-linker-plugin \
	  -Wl,--gc-sections \
	  -mrelax \
	  -lm \
	  -mmcu=$(AVRDUDE_CHIP)

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

CFLAGS+=$(INCLUDES)

#######################
### SOURCE FILES ######
#######################

# Source files
ifeq ($(OS),Windows_NT) # Windows
SRCS = \
       $(shell find ../Helios -maxdepth 1 -type f -name '\*.cpp') main.cpp
else # linux
SRCS = \
       $(shell find ../Helios -maxdepth 1 -type f -name \*.cpp) main.cpp
endif

OBJS = $(SRCS:.cpp=.o)

DFILES = $(SRCS:.cpp=.d)

#######################
### BUILD TARGETS #####
#######################

# Target name
TARGET = helios

all: compute_version $(TARGET).hex
	@echo Detected Operating System: $(OS)
	$(OBJDUMP) --disassemble --source --line-numbers --demangle --section=.text $(TARGET).elf > $(TARGET).lst
	$(NM) --numeric-sort --line-numbers --demangle --print-size --format=s $(TARGET).elf > $(TARGET).map
	chmod +x avrsize.sh
	./avrsize.sh $(TARGET).elf
	@echo "== Success building Helios v$(HELIOS_VERSION_NUMBER) =="

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O binary -R .eeprom $(TARGET).elf $(TARGET).bin
	$(OBJCOPY) -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --no-change-warnings --change-section-lma .eeprom=0 $(TARGET).elf $(TARGET).eep
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(TARGET).elf: compute_version $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $@

%.o: %.S
	$(CC) $(ASMFLAGS) -c $< -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

upload: set_fuses $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -Uflash:w:$(TARGET).hex:i

#######################
### LINUX SETUP #######
#######################

ifneq ($(OS),Windows_NT) # Linux
build: all
INSTALL_DIR=~/atmel_setup
# Name of the toolchain tarball
TOOLCHAIN_TAR=avr8-gnu-toolchain-3.7.0.1796-linux.any.x86_64.tar.gz
# Name of the ATtiny DFP zip
ATTINY_ZIP=Atmel.ATtiny_DFP.2.0.368.atpack
install:
	@echo "Setting up in directory $(INSTALL_DIR)"
	@mkdir -p $(INSTALL_DIR)
	@cd $(INSTALL_DIR) && \
	echo "Downloading and installing AVR 8-bit Toolchain..." && \
	wget -q https://ww1.microchip.com/downloads/aemDocuments/documents/DEV/ProductDocuments/SoftwareTools/$(TOOLCHAIN_TAR) && \
	tar -xf $(TOOLCHAIN_TAR) && \
	echo "Downloading and installing ATtiny DFP..." && \
	wget -q http://packs.download.atmel.com/$(ATTINY_ZIP) && \
	unzip $(ATTINY_ZIP)
	@echo "Download and extraction complete. You'll find the toolchain and pack files in $(INSTALL_DIR)"
endif

#####################
####### FUSES #######
#####################

set_fuses: # Low Fuse = Clock Speed | High Fuse = Brown Out Detection Off | Extended Fuse N/A
ifeq ($(CPU_SPEED),16000000L)
	@echo "Setting fuses for 16MHz..."
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0xe1:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
else ifeq ($(CPU_SPEED),8000000L)
	@echo "Setting fuses for 8MHz..."
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0xE2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
else ifeq ($(CPU_SPEED),1000000L)
	@echo "Setting fuses for 1MHz..."
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U lfuse:w:0x62:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
else
	@echo "Unsupported CPU_SPEED for fuse settings."
endif

# Low Fuse = Clock Speed | High Fuse = Brown Out Detection Off | Extended Fuse N/A
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
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:aeos_firmware.hex:i

extract_hex: aeos_firmware.hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:r:aeos_firmware.hex:i

#####################
####### CLEAN #######
#####################

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).hex $(DFILES) $(TARGET).bin $(TARGET).eep $(TARGET).lst $(TARGET).map

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

extract_version:
	@echo "Reading firmware version..."
	$(eval TEMP_HEX := $(shell mktemp))
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:r:$(TEMP_HEX):i > /dev/null 2>&1
	python ./find_version.py $(TEMP_HEX)
	@rm $(TEMP_HEX)

# include dependency files to ensure partial rebuilds work correctly
-include $(DFILES)
