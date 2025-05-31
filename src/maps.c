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

#include "gb/gb.h"
#include "score.h"
#include "title_map.h"
#include "bg_map.h"
#include "numbers_fnt.h"
#include "text_fnt.h"

#define MAX_DIGITS 7

// TODO inverted score color not visible on DMG

const unsigned char num_attr_1[104] = {
    0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
    0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
    0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
    0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
    0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,0x81,
    0x81,0x81,0x81,0x81,
};

const unsigned char num_attr_2[104] = {
    0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,
    0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,
    0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,
    0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,
    0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,
    0x82,0x82,0x82,0x82,
};

const palette_color_t num_pal_inv[4] = {
  //RGB8(  0,  0,  0), RGB8(248,252,248), RGB8(  0,  0,  0), RGB8(  0,  0,  0)
    RGB8(  0,  0,  0), RGB8(  0,  0,  0), RGB8(248,252,248), RGB8(  0,  0,  0)
};

static uint8_t fnt_off = 0;

void map_title(void) NONBANKED {
    SWITCH_ROM(BANK(title_map));
    set_bkg_palette(OAMF_CGB_PAL0, title_map_PALETTE_COUNT, title_map_palettes);
    set_bkg_data(0, title_map_TILE_COUNT, title_map_tiles);
    set_bkg_attributes(0, 0, title_map_MAP_ATTRIBUTES_WIDTH, title_map_MAP_ATTRIBUTES_HEIGHT, title_map_MAP_ATTRIBUTES);
    set_bkg_tiles(0, 0, title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H, title_map_map);
}

void map_game(void) NONBANKED {
    SWITCH_ROM(BANK(bg_map));
    set_bkg_palette(OAMF_CGB_PAL0, bg_map_PALETTE_COUNT, bg_map_palettes);
    set_bkg_data(0, bg_map_TILE_COUNT, bg_map_tiles);
    set_bkg_attributes(0, 0, bg_map_MAP_ATTRIBUTES_WIDTH, bg_map_MAP_ATTRIBUTES_HEIGHT, bg_map_MAP_ATTRIBUTES);
    set_bkg_tiles(0, 0, bg_map_WIDTH / bg_map_TILE_W, bg_map_HEIGHT / bg_map_TILE_H, bg_map_map);
}

void win_init(uint8_t is_splash) NONBANKED {
    fnt_off = is_splash ? title_map_TILE_COUNT : bg_map_TILE_COUNT;

    SWITCH_ROM(BANK(numbers_fnt));
    set_bkg_palette(OAMF_CGB_PAL0 + bg_map_PALETTE_COUNT, numbers_fnt_PALETTE_COUNT, numbers_fnt_palettes);
    set_bkg_palette(OAMF_CGB_PAL0 + bg_map_PALETTE_COUNT + numbers_fnt_PALETTE_COUNT, numbers_fnt_PALETTE_COUNT, num_pal_inv);
    set_win_data(fnt_off, numbers_fnt_TILE_COUNT, numbers_fnt_tiles);

    if (is_splash) {
        SWITCH_ROM(BANK(text_fnt));
        set_win_data(fnt_off + numbers_fnt_TILE_COUNT, text_fnt_TILE_COUNT, text_fnt_tiles);
    }
}

static void set_win_based(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                          const uint8_t *tiles, uint8_t base_tile, const uint8_t *attributes) NONBANKED {
    VBK_REG = VBK_ATTRIBUTES;
    set_win_tiles(x, y, w, h, attributes);
    VBK_REG = VBK_TILES;
    set_win_based_tiles(x, y, w, h, tiles, base_tile);
}

static void character(uint8_t c, uint8_t pos, uint8_t x_off, uint8_t y_off, uint8_t is_black) NONBANKED {
    SWITCH_ROM(BANK(text_fnt));
    uint8_t off = c * text_fnt_WIDTH / text_fnt_TILE_W;

    set_win_based(x_off + (pos * text_fnt_WIDTH / text_fnt_TILE_W), y_off,
                  text_fnt_WIDTH / text_fnt_TILE_W, 1,
                  text_fnt_map + off, fnt_off + numbers_fnt_TILE_COUNT,
                  (is_black ? num_attr_2 : num_attr_1) + off);

    set_win_based(x_off + (pos * text_fnt_WIDTH / text_fnt_TILE_W), y_off + 1,
                  text_fnt_WIDTH / text_fnt_TILE_W, 1,
                  text_fnt_map + off + (sizeof(text_fnt_map) / 2), fnt_off + numbers_fnt_TILE_COUNT,
                  (is_black ? num_attr_2 : num_attr_1) + off);
}

static void str3(uint16_t name, uint8_t x_off, uint8_t y_off, uint8_t is_black) NONBANKED {
    character((name >> 10) & 0x1F, 0, x_off, y_off, is_black);
    character((name >>  5) & 0x1F, 1, x_off, y_off, is_black);
    character((name >>  0) & 0x1F, 2, x_off, y_off, is_black);
}

static void str(const char *s, uint8_t x_off, uint8_t y_off, uint8_t is_black) NONBANKED {
    uint8_t n = 0;
    while (*s) {
        char c = *(s++);
        if ((c >= 'A') && (c <= 'Z')) c = c - 'A' + 'a';
        if ((c < 'a') || (c > 'z')) c = 'x';
        character(c - 'a', n++, x_off, y_off, is_black);
    }
}

static void digit(uint8_t val, uint8_t pos, uint8_t x_off, uint8_t y_off, uint8_t is_black) NONBANKED {
    SWITCH_ROM(BANK(numbers_fnt));
    uint8_t off = val * numbers_fnt_WIDTH / numbers_fnt_TILE_W;

    set_win_based(x_off + (pos * numbers_fnt_WIDTH / numbers_fnt_TILE_W), y_off,
                  numbers_fnt_WIDTH / numbers_fnt_TILE_W, 1,
                  numbers_fnt_map + off, fnt_off,
                  (is_black ? num_attr_2 : num_attr_1) + off);

    set_win_based(x_off + (pos * numbers_fnt_WIDTH / numbers_fnt_TILE_W), y_off + 1,
                  numbers_fnt_WIDTH / numbers_fnt_TILE_W, 1,
                  numbers_fnt_map + off + (sizeof(numbers_fnt_map) / 2), fnt_off,
                  (is_black ? num_attr_2 : num_attr_1) + off);
}

static uint8_t number(int32_t score, uint8_t x_off, uint8_t y_off, uint8_t is_black) NONBANKED {
    // TODO can not set numbers larger than int16 max?!
    //score = 32767 + 1; // wtf?!

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
        return 0;
    }

    uint8_t off = (x_off == 0xFF) ? (10 - len) : ((x_off == 0xFE) ? (20 - (len * 2)) : x_off);
    for (uint8_t i = 0; i < len; i++) {
        digit(digits[len - i - 1], i, off, y_off, is_black);
    }

    return 8 * len * 2;
}

static void fill_win(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tile, uint8_t attr) NONBANKED {
    VBK_REG = VBK_ATTRIBUTES;
    fill_win_rect(x, y, w, h, attr);
    VBK_REG = VBK_TILES;
    fill_win_rect(x, y, w, h, tile);
}

void win_splash_draw(int32_t lowest, int32_t highest) NONBANKED {
    // reuse full black and white tiles at 0 and 1 from splash bg
    fill_win(0, 0, 10, 2, 0, 0x00);
    fill_win(10, 0, 10, 2, 1, 0x00);

    number(lowest, 0, 0, 1);
    number(highest, 0xFE, 0, 0);
}

void win_score_clear(uint8_t is_black) NONBANKED {
    SWITCH_ROM(BANK(title_map));
    set_win_based(0, 0,
                  title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H,
                  title_map_map, 0, title_map_MAP_ATTRIBUTES);

    str(is_black ? "black" : "white", 10 - 5, 1, is_black);
}

void win_score_draw(struct scores score, uint8_t off, uint8_t is_black) NONBANKED {
    str3(score.name, 0, 4 + off * 3, is_black);
    number(is_black ? -score.score : score.score, 7, 4 + off * 3, is_black);
}

uint8_t win_game_draw(int32_t score) NONBANKED {
    fill_win(0, 0, 10, 2, fnt_off + numbers_fnt_TILE_COUNT, 0x81);

    uint8_t is_black = 0;
    if (score < 0) {
        score = -score;
        is_black = 1;
    }

    return number(score, 0, 0, is_black);
}
