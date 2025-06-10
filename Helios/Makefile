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
	BINDIR ?= $(HOME)/atmel_setup/avr8-gnu-toolchain-linux_x86_64/bin/
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
