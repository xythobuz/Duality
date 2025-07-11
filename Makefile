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

GIT_GEN := $(DATA_DIR)/git.c
OBJS += $(GIT_GEN:%.c=$(BUILD_DIR)/%.o)

GEN_SRCS := $(DATA_DIR)/table_speed_shot.c
GEN_SRCS += $(DATA_DIR)/table_speed_move.c
OBJS += $(GEN_SRCS:%.c=$(BUILD_DIR)/%.o)

IMAGES := $(wildcard $(DATA_DIR)/*.png)
SPRITES := $(IMAGES:%.png=$(BUILD_DIR)/%.c)
OBJS += $(SPRITES:%.c=%.o)

WAVES := $(wildcard $(DATA_DIR)/*.wav)
SOUNDS := $(WAVES:%.wav=$(BUILD_DIR)/%.c)
OBJS += $(SOUNDS:%.c=%.o)

ASSETS := $(SPRITES)
ASSETS += $(SOUNDS)
ASSETS += $(GEN_SRCS)

LCC := $(GBDK_HOME)/bin/lcc
PNGA := $(GBDK_HOME)/bin/png2asset
ROMU := $(GBDK_HOME)/bin/romusage
GB_EMU := gearboy
SGB_EMU := sameboy
BGB_EMU := ~/bin/bgb/bgb.exe
GBE_EMU := ~/bin/gbe/gbe_plus_qt.exe
FLASHER := flashgbx

LCCFLAGS := -Wa-l -Wl-m -Wp-MMD -Wf--opt-code-speed
LCCFLAGS += -I$(SRC_DIR) -I$(BUILD_DIR)/$(DATA_DIR) -I$(DATA_DIR)
LCCFLAGS += -Wm-ynDuality -Wm-yt0x1B -Wm-yoA -Wm-ya1 -Wm-yc -Wm-ys
LCCFLAGS += -autobank -Wb-ext=.rel -Wb-v -Wf-bo255

GB_EMUFLAGS := $(BUILD_DIR)/$(BIN)
SGB_EMUFLAGS := $(BUILD_DIR)/$(BIN)
BGB_EMUFLAGS := $(BUILD_DIR)/$(BIN)
GBE_EMUFLAGS := $(BUILD_DIR)/$(BIN)

ifndef GBDK_RELEASE
	LCCFLAGS += -debug -DDEBUG -Wa-j -Wa-y -Wa-s -Wl-j -Wl-y -Wl-u -Wm-yS
	GB_EMUFLAGS += $(BUILD_DIR)/$(BIN:.gb=.sym)
	BUILD_TYPE = Debug
else
	BUILD_TYPE = Release
endif

FLASHCART := "DIY cart with MX29LV640 @ WR"
FLASHFLAGS := --mode dmg --action flash-rom --flashcart-type $(FLASHCART)

$(info BUILD_TYPE is $(BUILD_TYPE))

DEPS=$(OBJS:%.o=%.d)
-include $(DEPS)

.PHONY: all run cloc sgb_run bgb_run gbe_run flash clean compile_commands.json usage $(GIT_GEN)
.PRECIOUS: $(BUILD_DIR)/$(DATA_DIR)/%.c $(BUILD_DIR)/$(DATA_DIR)/%.h

all: $(BIN)

cloc:
	@cloc . --exclude-dir=$(BUILD_DIR)

compile_commands.json:
	@echo "Cleaning old build"
	@make clean
	@echo "Preparing bear.cfg"
	@echo '{"compilation":{"compilers_to_recognize":[{"executable":"$(GBDK_HOME)/bin/sdcc","flags_to_add":["-D__PORT_sm83", "-D__TARGET_gb", "-DBANK(x)=42", "-DBANKREF(x)=", "-DBANKREF_EXTERN(x)=" ],"flags_to_remove":[""]}]}}' > bear.cfg
	@echo "Running full build within bear"
	@bear --config bear.cfg -- make -j4
	@rm -rf bear.cfg

$(GIT_GEN): $(DATA_DIR)/git.c_template
	@mkdir -p $(@D)
	@echo Generating $@ from $<
	@sed 's|GIT_VERSION|"$(shell git describe --abbrev=7 --dirty --always --tags)"|g' $< > $@

$(DATA_DIR)/table_speed_shot.c: util/gen_angles.py Makefile
	@mkdir -p $(@D)
	@echo Generating $@
	@util/gen_angles.py -n table_speed_shot -d $(DATA_DIR) -s 16 -w 2 -f 0 -m 42 -t int8_t

$(DATA_DIR)/table_speed_move.c: util/gen_angles.py Makefile
	@mkdir -p $(@D)
	@echo Generating $@
	@util/gen_angles.py -n table_speed_move -d $(DATA_DIR) -s 16 -w 2 -f 0 -m 23 -t int8_t

usage: $(BUILD_DIR)/$(BIN)
	@echo Analyzing $<
	@$(ROMU) $(BUILD_DIR)/$(BIN:%.gb=%.map)

run: $(BIN)
	@echo Emulating $<
	@$(GB_EMU) $(GB_EMUFLAGS)

sgb_run: $(BIN)
	@echo Emulating $<
	@$(SGB_EMU) $(SGB_EMUFLAGS)

bgb_run: $(BIN)
	@echo Emulating $<
	@$(BGB_EMU) $(BGB_EMUFLAGS)

gbe_run: $(BIN)
	@echo Emulating $<
	@$(GBE_EMU) $(GBE_EMUFLAGS)

flash: $(BIN)
	@echo Flashing $<
	@$(FLASHER) $(FLASHFLAGS) $<

$(BUILD_DIR)/$(DATA_DIR)/%.c $(BUILD_DIR)/$(DATA_DIR)/%.h: $(DATA_DIR)/%.wav util/cvtsample.py Makefile
	@mkdir -p $(@D)
	@echo Converting sound $<
	@util/cvtsample.py $< "(None)" GBDK $(BUILD_DIR)/$(DATA_DIR)

$(BUILD_DIR)/$(DATA_DIR)/%.c $(BUILD_DIR)/$(DATA_DIR)/%.h: $(DATA_DIR)/%.png Makefile
	@mkdir -p $(@D)
	$(eval SPRFLAG = $(shell echo "$<" | sed -n 's/.*_spr\([0-9]\+\).*/\-sw \1 \-sh \1/p'))
	$(eval FNTFLAG = $(shell echo "$<" | sed -n 's/.*_fnt\([0-9]\+\).*/\-sw \1 \-sh \1/p'))
	$(if $(findstring _map,$<),                                                             \
		@echo "Converting map $<" &&                                                    \
		$(PNGA) $< -o $@ -spr8x8 -map -noflip                                           \
	,$(if $(findstring _fnt,$<),                                                            \
		@echo "Converting font $<" &&                                                   \
		$(PNGA) $< -o $@ -spr8x8 $(FNTFLAG) -map -noflip                                \
	,$(if $(findstring _spr,$<),                                                            \
		@echo "Converting 8x8 sprite $<" &&                                             \
		$(PNGA) $< -o $@ -spr8x8 $(SPRFLAG) -noflip                                     \
	,$(if $(findstring pause,$<),                                                           \
		@echo "Converting 40x16 sprite $<" &&                                           \
		$(PNGA) $< -o $@ -spr8x8 -sw 40 -sh 16 -noflip                                  \
	,$(if $(findstring _sgb,$<),                                                            \
		@echo "Converting sgb border $<" &&                                             \
		$(PNGA) $< -o $@ -map -bpp 4 -max_palettes 4 -pack_mode sgb -use_map_attributes \
	,                                                                                       \
		@echo "Converting tile $<" &&                                                   \
		$(PNGA) $< -o $@ -spr8x8                                                        \
	)))))

$(BUILD_DIR)/%.o: %.c $(ASSETS) Makefile
	@mkdir -p $(@D)
	@echo Compiling Code $<
	$(eval BAFLAG = $(shell echo "$<" | sed -n 's/.*\.ba\([0-9]\+\).*/\-Wf-ba\1/p'))
	@$(LCC) $(LCCFLAGS) $(BAFLAG) -c -o $@ $<

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c $(ASSETS) Makefile
	@mkdir -p $(@D)
	@echo Compiling Asset $<
	@$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.s $(ASSETS) Makefile
	@mkdir -p $(@D)
	@echo Assembling $<
	@$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(BIN): $(OBJS) Makefile
	@echo Linking $@
	@$(LCC) $(LCCFLAGS) -o $@ $(OBJS)

$(BIN): $(BUILD_DIR)/$(BIN) usage
	@cp $< $@

clean:
	rm -rf $(BUILD_DIR) $(BIN) $(DATA_DIR)/*.c $(DATA_DIR)/*.h
