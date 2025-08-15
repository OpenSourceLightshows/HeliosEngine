.SUFFIXES:
.PHONY: all clean compute_version wasm

TARGET ?= desktop
BUILD_DIR = build/$(TARGET)
CORE_LIB = ../Helios/build/$(TARGET)/helios.a

ifeq ($(TARGET),wasm)
	CC = em++
	AR = emar
	LIB_JS = $(BUILD_DIR)/HeliosLib.js
	EXPORT_FLAGS = --bind -s EXPORT_ES6=1 -s EXPORT_NAME='HeliosLib'
else
	CC = g++
	AR = ar
endif

CFLAGS = -O2 -Wall -MMD -I ../Helios -I . \
	-DHELIOS_CLI \
	-DHELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
	-DHELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
	-DHELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
	-DHELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER)

ifeq ($(TARGET),desktop)
	CFLAGS += -g
endif

SRCS = HeliosLib.cpp
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

LIB_A = $(BUILD_DIR)/HeliosLib.a
ifeq ($(TARGET),wasm)
TARGETS = $(LIB_JS)
else
TARGETS = $(LIB_A)
endif

all: compute_version $(TARGETS)
	@echo "== Success building HeliosLib v$(HELIOS_VERSION_NUMBER) =="

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(CORE_LIB):
	$(MAKE) -C ../Helios TARGET=$(TARGET) \
		HELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
		HELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
		HELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
		HELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER)

$(LIB_A): $(OBJS) $(CORE_LIB)
	$(AR) rcs $@ $(OBJS)

$(LIB_JS): $(OBJS) $(CORE_LIB)
	$(CC) $(EXPORT_FLAGS) $(OBJS) $(CORE_LIB) -o $@

wasm:
	$(MAKE) TARGET=wasm

clean:
	rm -rf build

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

-include $(DEPS)

