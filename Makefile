# Configuration

BIN_DEBUG := main
BIN_RELEASE := gm
BIN_DIR := out
BUILD_DIR := build
MAKEFLAGS += -j
DEBUG_FLAGS := -ggdb
RELEASE_FLAGS := -O3
CXXFLAGS := -std=c++23

# Inner workings
MODE := debug
SOURCES := $(wildcard *.cc)
OBJECTS := $(patsubst %.cc,$(BUILD_DIR)/$(MODE)/%.o,$(SOURCES))

TOTAL := $(words $(OBJECTS))
CURRENT = 0
RECURSIVE := 0

ifeq ($(MODE), debug)
	CXXFLAGS := $(CXXFLAGS) $(DEBUG_FLAGS)
	BIN := $(BIN_DIR)/$(BIN_DEBUG)
endif

ifeq ($(MODE), release)
	CXXFLAGS := $(CXXFLAGS) $(RELEASE_FLAGS)
	BIN := $(BIN_DIR)/$(BIN_RELEASE)
endif

define comp_prog
    $(eval CURRENT=$(shell echo $$(( $(CURRENT) + 1 ))))
	$(info [$(CURRENT)/$(TOTAL)] $(1): $@)
endef

define compiling_log
	$(call comp_prog,Compiling object)
endef

define linking_log
	$(call comp_prog,Linking executable)
endef

debug: build
release:
	@$(MAKE) --no-print-directory MODE=release build

build: calculate_total $(BIN)
ifeq ($(RECURSIVE), 0)
calculate_total:
	$(eval TOTAL := $(shell \
		$(MAKE) RECURSIVE=1 -n $(MODE) | grep '$(CXX)' | wc -l))
endif

$(BIN): $(OBJECTS) | $(BIN_DIR)/
	$(call linking_log)
	@$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJECTS)

$(BUILD_DIR)/$(MODE)/%.o: %.cc %.hh | $(BUILD_DIR)/$(MODE)/
	$(call compiling_log)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(MODE)/%.o: %.cc | $(BUILD_DIR)/$(MODE)/
	$(call compiling_log)
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

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

.PHONY: debug release build clean calculate_total
