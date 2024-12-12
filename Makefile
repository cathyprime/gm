BIN_DEBUG := main
BIN_RELEASE := gm
BIN_DIR := out
BUILD_DIR := build

DEBUG_FLAGS := -ggdb
RELEASE_FLAGS := -O3
CXXFLAGS := -std=c++23

MODE := debug
SOURCES := $(wildcard *.cc)
OBJECTS := $(patsubst %.cc,$(BUILD_DIR)/$(MODE)/%.o,$(SOURCES))

ifeq ($(MODE), debug)
	CXXFLAGS := $(CXXFLAGS) $(DEBUG_FLAGS)
	BIN := $(BIN_DIR)/$(BIN_DEBUG)
endif

ifeq ($(MODE), release)
	CXXFLAGS := $(CXXFLAGS) $(RELEASE_FLAGS)
	BIN := $(BIN_DIR)/$(BIN_RELEASE)
endif

build: $(BIN)

release:
	make MODE=release build

$(BIN): $(OBJECTS) | $(BIN_DIR)/
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJECTS)

$(BUILD_DIR)/$(MODE)/%.o: %.cc %.hh | $(BUILD_DIR)/$(MODE)/
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(MODE)/%.o: %.cc | $(BUILD_DIR)/$(MODE)/
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/:
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/$(MODE)/: $(BUILD_DIR)/
	@mkdir -p $(BUILD_DIR)/$(MODE)

$(BIN_DIR)/:
	@mkdir -p $(BIN_DIR)

clean:
	$(info Cleaning $(BIN_DIR) and $(BUILD_DIR)...)
	@if [ -d "$(BIN_DIR)" ] && [ "$(wildcard $(BIN_DIR)*)" ]; then rm -r $(BIN_DIR)*; fi
	@if [ -d "$(BUILD_DIR)" ] && [ "$(wildcard $(BUILD_DIR)*)" ]; then rm -r $(BUILD_DIR)*; fi

.PHONY: release build clean
