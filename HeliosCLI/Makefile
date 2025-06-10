.SUFFIXES:
.PHONY: all clean compute_version

TARGET = desktop
BUILD_DIR = build/$(TARGET)
CORE_LIB = ../Helios/build/$(TARGET)/helios.a
BIN = $(BUILD_DIR)/helios

CC = g++
CFLAGS = -O2 -Wall -std=c++11 -g -MMD \
	-DHELIOS_CLI \
	-DHELIOS_VERSION_MAJOR=$(HELIOS_VERSION_MAJOR) \
	-DHELIOS_VERSION_MINOR=$(HELIOS_VERSION_MINOR) \
	-DHELIOS_BUILD_NUMBER=$(HELIOS_BUILD_NUMBER) \
	-DHELIOS_VERSION_NUMBER=$(HELIOS_VERSION_NUMBER) \
	-I ../Helios

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

# Final executable depends on obj + helios
all: compute_version $(BIN)
	@echo "== Success building HeliosCLI v$(HELIOS_VERSION_NUMBER) =="

$(BIN): $(OBJS) $(CORE_LIB)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(CORE_LIB)

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

# Compute version variables
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
