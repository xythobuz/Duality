# Adapted from GBDK-2020 examples
#
# Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
#
# https://gbdk.org/docs/api/docs_toolchain_settings.html
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# See <http://www.gnu.org/licenses/>.

ifndef GBDK_HOME
	GBDK_HOME = ../../../
endif

BIN := duality.gb
SRC_DIR := src
BUILD_DIR := build
DATA_DIR := data

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)

ASSETS := $(wildcard $(DATA_DIR)/*.png)
SPRITES := $(ASSETS:%.png=$(BUILD_DIR)/%.c)
OBJS += $(SPRITES:%.c=%.o)

LCC := $(GBDK_HOME)/bin/lcc
PNGA := $(GBDK_HOME)/bin/png2asset
ROMU := $(GBDK_HOME)/bin/romusage
GB_EMU := gearboy

LCCFLAGS := -Wa-l -Wl-m -Wp-MMD
LCCFLAGS += -I$(BUILD_DIR)/$(DATA_DIR)
LCCFLAGS += -Wm"-yn Duality" -Wm-yc

EMUFLAGS := $(BIN)

ifndef GBDK_RELEASE
	LCCFLAGS += -debug -DDEBUG -Wa-j -Wa-y -Wa-s -Wl-j -Wl-y -Wl-u -Wm-yS
	EMUFLAGS += $(BUILD_DIR)/$(BIN:.gb=.sym)
	BUILD_TYPE = Debug
else
	BUILD_TYPE = Release
endif

$(info BUILD_TYPE is $(BUILD_TYPE))

# TODO this is not working. why?!
#DEPS=$(OBJS:%.o=%.d)
#-include $(DEPS)

.PHONY: all run clean compile_commands.json usage
.PRECIOUS: $(BUILD_DIR)/$(DATA_DIR)/%.c $(BUILD_DIR)/$(DATA_DIR)/%.h

all: $(BIN)

compile_commands.json:
	@echo "Cleaning old build"
	@make clean
	@echo "Preparing bear.cfg"
	@echo '{"compilation":{"compilers_to_recognize":[{"executable":"$(GBDK_HOME)/bin/sdcc","flags_to_add":[""],"flags_to_remove":[""]}]}}' > bear.cfg
	@echo "Running full build within bear"
	@bear --config bear.cfg -- make -j4
	@rm -rf bear.cfg

usage: $(BUILD_DIR)/$(BIN)
	@echo Analyzing $<
	@$(ROMU) $(BUILD_DIR)/$(BIN:%.gb=%.map)

run: $(BIN)
	@echo Emulating $<
	@$(GB_EMU) $(EMUFLAGS)

$(BUILD_DIR)/$(DATA_DIR)/%.c $(BUILD_DIR)/$(DATA_DIR)/%.h: $(DATA_DIR)/%.png
	@mkdir -p $(@D)
	$(if $(findstring _map,$<),           \
		@echo "Converting map $<" &&  \
		$(PNGA) $< -o $@ -spr8x8 -map -use_map_attributes -noflip \
	,$(if $(findstring numbers,$<), \
		@echo "Converting font $<" && \
		$(PNGA) $< -o $@ -spr8x8 -sw 16 -sh 16 -map \
	,                                     \
		@echo "Converting tile $<" && \
		$(PNGA) $< -o $@ -spr8x8      \
	))

$(BUILD_DIR)/%.o: %.c $(SPRITES)
	@mkdir -p $(@D)
	@echo Compiling $<
	@$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c $(SPRITES)
	@mkdir -p $(@D)
	@echo Compiling $<
	@$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.s $(SPRITES)
	@mkdir -p $(@D)
	@echo Assembling $<
	@$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(BIN): $(OBJS)
	@echo Linking $@
	@$(LCC) $(LCCFLAGS) -o $@ $(OBJS)

$(BIN): $(BUILD_DIR)/$(BIN)
	@cp $< $@
	@make usage

clean:
	rm -rf $(BUILD_DIR) $(BIN)
