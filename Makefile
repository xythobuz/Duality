# Adapted from GBDK-2020 examples

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
GB_EMU := gearboy

LCCFLAGS := -Wa-l -Wl-m -Wm"-yn Duality" -I$(BUILD_DIR)/$(DATA_DIR) -Wm-yc
EMUFLAGS := $(BIN)

ifndef GBDK_RELEASE
	LCCFLAGS += -debug -Wa-j -Wa-y -Wa-s -Wl-j -Wl-y -Wl-u -Wm-yS
	EMUFLAGS += $(BUILD_DIR)/$(BIN:.gb=.sym)
	BUILD_TYPE = Debug
else
	BUILD_TYPE = Release
endif

$(info BUILD_TYPE is $(BUILD_TYPE))

.PHONY: all run $(BIN) clean compile_commands.json

all: $(BIN)

compile_commands.json:
	@echo "Cleaning old build"
	@make clean
	@echo "Preparing bear.cfg"
	@echo '{"compilation":{"compilers_to_recognize":[{"executable":"$(GBDK_HOME)/bin/sdcc","flags_to_add":[""],"flags_to_remove":[""]}]}}' > bear.cfg
	@echo "Running full build within bear"
	@bear --config bear.cfg -- make -j4
	@rm -rf bear.cfg

run: $(BIN)
	@echo Emulating $<
	@$(GB_EMU) $(EMUFLAGS)

.PRECIOUS: $(BUILD_DIR)/$(DATA_DIR)/%.c $(BUILD_DIR)/$(DATA_DIR)/%.h
$(BUILD_DIR)/$(DATA_DIR)/%.c $(BUILD_DIR)/$(DATA_DIR)/%.h: $(DATA_DIR)/%.png
	@mkdir -p $(@D)
	$(if $(findstring _map,$<),           \
		@echo "Converting map $<" &&  \
		$(PNGA) $< -o $@ -spr8x8 -map \
	,                                     \
		@echo "Converting tile $<" && \
		$(PNGA) $< -o $@ -spr8x8      \
	)

$(BUILD_DIR)/%.o: %.c $(SPRITES)
	@mkdir -p $(@D)
	@echo Compiling $<
	@$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.c
	@mkdir -p $(@D)
	@echo Compiling $<
	@$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(@D)
	@echo Assembling $<
	@$(LCC) $(LCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/$(BIN): $(OBJS)
	@echo Linking $@
	@$(LCC) $(LCCFLAGS) -o $@ $(OBJS)

$(BIN): $(BUILD_DIR)/$(BIN)
	@cp $< $@

clean:
	rm -rf $(BUILD_DIR) $(BIN)
