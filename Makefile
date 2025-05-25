# Adapted from GBDK-2020 examples

ifndef GBDK_HOME
	GBDK_HOME = ../../../
endif

BIN := duality.gb
SRC_DIR := src
BUILD_DIR := build

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

LCC := $(GBDK_HOME)/bin/lcc
GB_EMU := gearboy

LCCFLAGS := -Wa-l -Wl-m -Wm"-yn Duality"
EMUFLAGS := $(BIN)

GBDK_DEBUG = ON
ifdef GBDK_DEBUG
	LCCFLAGS += -debug -Wa-j -Wa-y -Wa-s -Wl-j -Wl-y -Wl-u -Wm-yS
	EMUFLAGS += $(BUILD_DIR)/$(BIN:.gb=.sym)
endif

.PHONY: all run $(BIN) clean

all: $(BIN)

run: $(BIN)
	@echo Emulating $<
	@$(GB_EMU) $(EMUFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo Compiling $<
	@$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(@D)
	@echo Assembling $<
	@$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(BIN): $(OBJS)
	@echo Linking $@
	@$(LCC) $(LCCFLAGS) -o $@ $<

$(BIN): $(BUILD_DIR)/$(BIN)
	@cp $< $@

clean:
	rm -rf $(BUILD_DIR) $(BIN)
