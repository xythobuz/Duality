/*
 * map_data.c
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * See <http://www.gnu.org/licenses/>.
 */

#include <assert.h>

#include "map_data.h"

#include "title_map.h"
#include "bg_map.h"
#include "bg2_map.h"
#include "numbers_fnt16.h"
#include "text_fnt16.h"
#include "vincent_fnt8.h"

BANKREF(map_data)

/*
 * BCP0: Title map
 * BCP1: Game map 1
 * BCP2: Game map 2
 * BCP3: Text Font 16
 * BCP4: Text Font 16 inverted
 * BCP5: Num Font 16 (same as 3)
 * BCP6: Font Ascii 8
 * BCP7: (unused)
 *
 * Explosion uses OCP0 to OCP3 at end of game.
 * Pause is flipped in-place for animating the pause screen colors.
 */

const palette_color_t num_pal_inv[INV_PALETTE_COUNT * 4] = {
    //RGB8(  0,  0,  0), RGB8(248,252,248), RGB8(  0,  0,  0), RGB8(  0,  0,  0)
    RGB8(  0,  0,  0), RGB8(  0,  0,  0), RGB8(248,252,248), RGB8(  0,  0,  0)
};

// currently this assumption is hard-coded
static_assert(bg_map_WIDTH == 256, "bg_map needs to be 256x256");
static_assert(bg_map_HEIGHT == 256, "bg_map needs to be 256x256");
static_assert(bg2_map_WIDTH == 256, "bg2_map needs to be 256x256");
static_assert(bg2_map_HEIGHT == 256, "bg2_map needs to be 256x256");

struct maps maps[MAP_COUNT] = {
    { // MAP_TITLE
        .width = title_map_WIDTH / title_map_TILE_W,
        .height = title_map_HEIGHT / title_map_TILE_H,
        .map = title_map_map,
        .tiles = title_map_tiles,
        .palettes = title_map_palettes,
        .palette_count = title_map_PALETTE_COUNT,
        .palette_index = BKGF_CGB_PAL0,
        .map_count = sizeof(title_map_map),
        .tile_count = title_map_TILE_COUNT,
        .tile_offset = BG_TILE_NUM_START,
        .tile_copy = BG_COPY_NONE,
        .bank = BANK(title_map),
        .load = BG_LOAD_SPLASH,
    },
    { // MAP_GAME_1
        .width = bg_map_WIDTH / bg_map_TILE_W,
        .height = bg_map_HEIGHT / bg_map_TILE_H,
        .map = bg_map_map,
        .tiles = bg_map_tiles,
        .palettes = bg_map_palettes,
        .palette_count = bg_map_PALETTE_COUNT,
        .palette_index = BKGF_CGB_PAL1,
        .map_count = sizeof(bg_map_map),
        .tile_count = bg_map_TILE_COUNT,
        .tile_offset = BG_TILE_NUM_START,
        .tile_copy = BG_COPY_NONE,
        .bank = BANK(bg_map),
        .load = BG_LOAD_GAME,
    },
    { // MAP_GAME_2
        .width = bg2_map_WIDTH / bg2_map_TILE_W,
        .height = bg2_map_HEIGHT / bg2_map_TILE_H,
        .map = bg2_map_map,
        .tiles = bg2_map_tiles,
        .palettes = bg2_map_palettes,
        .palette_count = bg2_map_PALETTE_COUNT,
        .palette_index = BKGF_CGB_PAL2,
        .map_count = sizeof(bg2_map_map),
        .tile_count = bg2_map_TILE_COUNT,
        .tile_offset = BG_TILE_NUM_START,
        .tile_copy = BG_COPY_NONE,
        .bank = BANK(bg2_map),
        .load = BG_LOAD_GAME,
    },
    { // FNT_TEXT_16
        .width = text_fnt16_WIDTH / text_fnt16_TILE_W,
        .height = text_fnt16_HEIGHT / text_fnt16_TILE_H,
        .map = text_fnt16_map,
        .tiles = text_fnt16_tiles,
        .palettes = text_fnt16_palettes,
        .palette_count = text_fnt16_PALETTE_COUNT,
        .palette_index = BKGF_CGB_PAL3,
        .map_count = sizeof(text_fnt16_map),
        .tile_count = text_fnt16_TILE_COUNT,
        .tile_offset = BG_TILE_NUM_START,
        .tile_copy = BG_COPY_NONE,
        .bank = BANK(text_fnt16),
        .load = BG_LOAD_SPLASH,
    },
    { // FNT_TEXT_16_INV
        .width = text_fnt16_WIDTH / text_fnt16_TILE_W,
        .height = text_fnt16_HEIGHT / text_fnt16_TILE_H,
        .map = text_fnt16_map,
        .tiles = text_fnt16_tiles,
        .palettes = num_pal_inv,
        .palette_count = INV_PALETTE_COUNT,
        .palette_index = BKGF_CGB_PAL4,
        .map_count = sizeof(text_fnt16_map),
        .tile_count = text_fnt16_TILE_COUNT,
        .tile_offset = BG_TILE_NUM_START,
        .tile_copy = FNT_TEXT_16,
        .bank = BANK(text_fnt16),
        .load = BG_LOAD_SPLASH,
    },
    { // FNT_NUM_16
        .width = numbers_fnt16_WIDTH / numbers_fnt16_TILE_W,
        .height = numbers_fnt16_HEIGHT / numbers_fnt16_TILE_H,
        .map = numbers_fnt16_map,
        .tiles = numbers_fnt16_tiles,
        .palettes = numbers_fnt16_palettes,
        .palette_count = numbers_fnt16_PALETTE_COUNT,
        .palette_index = BKGF_CGB_PAL5,
        .map_count = sizeof(numbers_fnt16_map),
        .tile_count = numbers_fnt16_TILE_COUNT,
        .tile_offset = BG_TILE_NUM_START,
        .tile_copy = BG_COPY_NONE,
        .bank = BANK(numbers_fnt16),
        .load = BG_LOAD_ALL,
    },
    { // FNT_NUM_16_INV
        .width = numbers_fnt16_WIDTH / numbers_fnt16_TILE_W,
        .height = numbers_fnt16_HEIGHT / numbers_fnt16_TILE_H,
        .map = numbers_fnt16_map,
        .tiles = numbers_fnt16_tiles,
        .palettes = NULL,
        .palette_count = INV_PALETTE_COUNT,
        .palette_index = BKGF_CGB_PAL4,
        .map_count = sizeof(numbers_fnt16_map),
        .tile_count = numbers_fnt16_TILE_COUNT,
        .tile_offset = BG_TILE_NUM_START,
        .tile_copy = FNT_NUM_16,
        .bank = BANK(numbers_fnt16),
        .load = BG_LOAD_ALL,
    },
    { // FNT_ASCII_8
        .width = vincent_fnt8_WIDTH / vincent_fnt8_TILE_W,
        .height = vincent_fnt8_HEIGHT / vincent_fnt8_TILE_H,
        .map = vincent_fnt8_map,
        .tiles = vincent_fnt8_tiles,
        .palettes = vincent_fnt8_palettes,
        .palette_count = vincent_fnt8_PALETTE_COUNT,
        .palette_index = BKGF_CGB_PAL6,
        .map_count = sizeof(vincent_fnt8_map),
        .tile_count = vincent_fnt8_TILE_COUNT,
        .tile_offset = BG_TILE_NUM_START,
        .tile_copy = BG_COPY_NONE,
        .bank = BANK(vincent_fnt8),
        .load = BG_LOAD_SPLASH | BG_LOAD_GBC_ONLY,
    },
};
