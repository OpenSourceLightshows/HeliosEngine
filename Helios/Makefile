.SUFFIXES:
.PHONY: all clean

TARGET ?= desktop

ifeq ($(TARGET),desktop)
	CC = g++
	AR = ar cru
	CFLAGS = -O2 -Wall -std=c++11 -MMD -g
	DEFINES = -D HELIOS_CLI
else ifeq ($(TARGET),avr)
	AVR_CHIP = attiny85
	# OS Detection for AVR toolchain paths
	ifeq ($(OS),Windows_NT) # Windows
		BINDIR = "C:/Program Files (x86)/Atmel/Studio/7.0/toolchain/avr8/avr8-gnu-toolchain/bin/"
	else ifeq ($(shell uname),Linux)
		# Use system-installed AVR toolchain (from apt-get install gcc-avr avr-libc)
		BINDIR =
	else ifeq ($(shell uname),Darwin)
		BINDIR = /Applications/Arduino.app/Contents/Java/hardware/tools/avr/bin/
	else
		$(error Unknown operating system for AVR build)
	endif
	CC = $(BINDIR)avr-g++
	AR = $(BINDIR)avr-gcc-ar rcs
	CFLAGS = -Os -Wall -std=gnu++17 -MMD -mmcu=$(AVR_CHIP) -flto -fno-exceptions
	DEFINES = -DHELIOS_EMBEDDED -D__AVR_ATtiny85__ -DF_CPU=8000000L
else ifeq ($(TARGET),wasm)
	CC = em++
	AR = emar rcs
	CFLAGS = -O2 -std=c++17 -Wall -MMD
	DEFINES = -DWASM -DHELIOS_CLI
else
	$(error Unknown TARGET '$(TARGET)')
endif

OUTDIR = build/$(TARGET)
OUTFILE = $(OUTDIR)/helios.a

SRC = $(wildcard *.cpp)
OBJ = $(SRC:%.cpp=$(OUTDIR)/%.o)
DEP = $(OBJ:.o=.d)

all: $(OUTFILE)

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/%.o: %.cpp | $(OUTDIR)
	$(CC) $(CFLAGS) $(DEFINES) -c $< -o $@

$(OUTFILE): $(OBJ)
	$(AR) $@ $^

clean:
	rm -rf build

-include $(DEP)
