/*
 * maps.c
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

#include <gbdk/platform.h>

#include "maps.h"

#include "title_map.h"
#include "bg_map.h"
#include "numbers.h"

#define MAX_DIGITS 7

// TODO inverted score color not visible on DMG

const unsigned char num_attr_1[40] = {
    0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
    0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
};

const unsigned char num_attr_2[40] = {
    0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,
    0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,
};
const palette_color_t num_pal_inv[4] = {
  //RGB8(  0,  0,  0), RGB8(248,252,248), RGB8(  0,  0,  0), RGB8(  0,  0,  0)
    RGB8(  0,  0,  0), RGB8(  0,  0,  0), RGB8(248,252,248), RGB8(  0,  0,  0)
};

void map_title(void) {
    set_bkg_palette(OAMF_CGB_PAL0, title_map_PALETTE_COUNT, title_map_palettes);
    set_bkg_data(0, title_map_TILE_COUNT, title_map_tiles);
    set_bkg_attributes(0, 0, title_map_MAP_ATTRIBUTES_WIDTH, title_map_MAP_ATTRIBUTES_HEIGHT, title_map_MAP_ATTRIBUTES);
    set_bkg_tiles(0, 0, title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H, title_map_map);
}

void map_game(void) {
    set_bkg_palette(OAMF_CGB_PAL0, bg_map_PALETTE_COUNT, bg_map_palettes);
    set_bkg_data(0, bg_map_TILE_COUNT, bg_map_tiles);
    set_bkg_attributes(0, 0, bg_map_MAP_ATTRIBUTES_WIDTH, bg_map_MAP_ATTRIBUTES_HEIGHT, bg_map_MAP_ATTRIBUTES);
    set_bkg_tiles(0, 0, bg_map_WIDTH / bg_map_TILE_W, bg_map_HEIGHT / bg_map_TILE_H, bg_map_map);
}

inline void set_win_based(uint8_t x, uint8_t y, uint8_t w, uint8_t h, const uint8_t *tiles, uint8_t base_tile, const uint8_t *attributes)
{
    VBK_REG = VBK_ATTRIBUTES;
    set_win_tiles(x, y, w, h, attributes);
    VBK_REG = VBK_TILES;
    set_win_based_tiles(x, y, w, h, tiles, base_tile);
}

static void digit(uint8_t val, uint8_t digit, uint8_t x_off, uint8_t is_black) {
    uint8_t off = val * numbers_WIDTH / numbers_TILE_W;

    set_win_based(x_off + (digit * numbers_WIDTH / numbers_TILE_W), 0,
                  numbers_WIDTH / numbers_TILE_W, 1,
                  numbers_map + off, bg_map_TILE_COUNT,
                  (is_black ? num_attr_2 : num_attr_1) + off);

    set_win_based(x_off + (digit * numbers_WIDTH / numbers_TILE_W), 1,
                  numbers_WIDTH / numbers_TILE_W, 1,
                  numbers_map + off + (sizeof(numbers_map) / 2), bg_map_TILE_COUNT,
                  (is_black ? num_attr_2 : num_attr_1) + off);
}


void win_game_load(void) {
    set_bkg_palette(OAMF_CGB_PAL0 + bg_map_PALETTE_COUNT, numbers_PALETTE_COUNT, numbers_palettes);
    set_bkg_palette(OAMF_CGB_PAL0 + bg_map_PALETTE_COUNT + 1, numbers_PALETTE_COUNT, num_pal_inv);
    set_win_data(bg_map_TILE_COUNT, numbers_TILE_COUNT, numbers_tiles);
}

void win_game_draw(int32_t score) {
    // TODO can not set numbers larger than int16 max?!
    //score = 32767 + 1; // wtf?!

    uint8_t is_black = 0;
    if (score < 0) {
        score = -score;
        is_black = 1;
    }

    uint8_t len = 0;
    uint8_t digits[MAX_DIGITS];
    do {
        digits[len++] = score % 10L;
        score = score / 10L;
        if (len >= MAX_DIGITS) {
            break;
        }
    } while (score > 0);

    // if the number was too large for our buffer don't draw anything
    if (score > 0) {
        return;
    }

    for (uint8_t i = 0; i < len; i++) {
        digit(digits[len - i - 1], i, 10 - len, is_black);
    }
}
