/*
 * window.c
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
#include <string.h>
#include <assert.h>

#include "banks.h"
#include "config.h"
#include "score.h"
#include "title_map.h"
#include "bg_map.h"
#include "numbers_fnt.h"
#include "text_fnt.h"
#include "git.h"
#include "main.h"
#include "window.h"

#define MAX_DIGITS 7
#define LINE_WIDTH 10

// TODO inverted score color not visible on DMG

BANKREF(window)

const palette_color_t num_pal_inv[4] = {
  //RGB8(  0,  0,  0), RGB8(248,252,248), RGB8(  0,  0,  0), RGB8(  0,  0,  0)
    RGB8(  0,  0,  0), RGB8(  0,  0,  0), RGB8(248,252,248), RGB8(  0,  0,  0)
};

static uint8_t fnt_off = 0;

void win_init(uint8_t is_splash) NONBANKED {
    fnt_off = is_splash ? title_map_TILE_COUNT : bg_map_TILE_COUNT;

    START_ROM_BANK(BANK(numbers_fnt));
        set_bkg_palette(OAMF_CGB_PAL0 + bg_map_PALETTE_COUNT, numbers_fnt_PALETTE_COUNT, numbers_fnt_palettes);
        set_win_data(fnt_off, numbers_fnt_TILE_COUNT, numbers_fnt_tiles);
    END_ROM_BANK();

    START_ROM_BANK_2(BANK(window));
        set_bkg_palette(OAMF_CGB_PAL0 + bg_map_PALETTE_COUNT + numbers_fnt_PALETTE_COUNT, numbers_fnt_PALETTE_COUNT, num_pal_inv);
    END_ROM_BANK();

    if (is_splash) {
        START_ROM_BANK_2(BANK(text_fnt));
            set_win_data(fnt_off + numbers_fnt_TILE_COUNT, text_fnt_TILE_COUNT, text_fnt_tiles);
        END_ROM_BANK();
    }
}

static void set_win_based(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                          const uint8_t *tiles, uint8_t base_tile, uint8_t tile_bank,
                          const uint8_t *attributes, uint8_t attr_bank) NONBANKED {
    if (attributes != NULL) {
        START_ROM_BANK(attr_bank);
            VBK_REG = VBK_ATTRIBUTES;
            set_win_tiles(x, y, w, h, attributes);
        END_ROM_BANK();
    } else {
        VBK_REG = VBK_ATTRIBUTES;
        fill_win_rect(x, y, w, h, 0x00);
    }

    START_ROM_BANK(tile_bank);
        VBK_REG = VBK_TILES;
        set_win_based_tiles(x, y, w, h, tiles, base_tile);
    END_ROM_BANK();
}

static void set_win_based_attr(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                               const uint8_t *tiles, uint8_t base_tile, uint8_t tile_bank,
                               const uint8_t attr) NONBANKED {
    VBK_REG = VBK_ATTRIBUTES;
    fill_win_rect(x, y, w, h, attr);

    START_ROM_BANK(tile_bank);
        VBK_REG = VBK_TILES;
        set_win_based_tiles(x, y, w, h, tiles, base_tile);
    END_ROM_BANK();
}

static void character(uint8_t c, uint8_t pos, uint8_t x_off, uint8_t y_off, uint8_t is_black) {
    uint8_t off = c * text_fnt_WIDTH / text_fnt_TILE_W;

    set_win_based_attr(x_off + (pos * text_fnt_WIDTH / text_fnt_TILE_W), y_off,
                       text_fnt_WIDTH / text_fnt_TILE_W, 1,
                       text_fnt_map + off, fnt_off + numbers_fnt_TILE_COUNT,
                       BANK(text_fnt), is_black ? 0x82 : 0x81);

    set_win_based_attr(x_off + (pos * text_fnt_WIDTH / text_fnt_TILE_W), y_off + 1,
                       text_fnt_WIDTH / text_fnt_TILE_W, 1,
                       text_fnt_map + off + (sizeof(text_fnt_map) / 2), fnt_off + numbers_fnt_TILE_COUNT,
                       BANK(text_fnt), is_black ? 0x82 : 0x81);
}

static void str3(uint16_t name, uint8_t x_off, uint8_t y_off,
                 uint8_t is_black_a, uint8_t is_black_b, uint8_t is_black_c) {
    character((name >> 10) & 0x1F, 0, x_off, y_off, is_black_a);
    character((name >>  5) & 0x1F, 1, x_off, y_off, is_black_b);
    character((name >>  0) & 0x1F, 2, x_off, y_off, is_black_c);
}

static void digit(uint8_t val, uint8_t pos, uint8_t x_off, uint8_t y_off, uint8_t is_black) {
    uint8_t off = val * numbers_fnt_WIDTH / numbers_fnt_TILE_W;

    set_win_based_attr(x_off + (pos * numbers_fnt_WIDTH / numbers_fnt_TILE_W), y_off,
                       numbers_fnt_WIDTH / numbers_fnt_TILE_W, 1,
                       numbers_fnt_map + off, fnt_off,
                       BANK(numbers_fnt), is_black ? 0x82 : 0x81);

    set_win_based_attr(x_off + (pos * numbers_fnt_WIDTH / numbers_fnt_TILE_W), y_off + 1,
                       numbers_fnt_WIDTH / numbers_fnt_TILE_W, 1,
                       numbers_fnt_map + off + (sizeof(numbers_fnt_map) / 2), fnt_off,
                       BANK(numbers_fnt), is_black ? 0x82 : 0x81);
}

static void str_l(const char *s, uint8_t len, uint8_t x_off, uint8_t y_off, uint8_t is_black) {
    for (uint8_t n = 0; (*s) && (n < LINE_WIDTH) && (n < len); n++) {
        char c = *(s++);
        if ((c >= 'A') && (c <= 'Z')) {
            c = c - 'A' + 'a';
        }
        if ((c >= '0') && (c <= '9')) {
            digit(c - '0', n, x_off, y_off, is_black);
        } else if ((c >= 'a') && (c <= 'z')) {
            character(c - 'a', n, x_off, y_off, is_black);
        }
    }
}

static void str(const char *s, uint8_t x_off, uint8_t y_off, uint8_t is_black) {
    str_l(s, 0xFF, x_off, y_off, is_black);
}

static void str_center(const char *s, uint8_t y_off, uint8_t is_black) {
    uint8_t n = strlen(s);
    if (n > LINE_WIDTH) n = LINE_WIDTH;
    str(s, LINE_WIDTH - n, y_off, is_black);
}

static void str_lines(const char *s, uint8_t y_off, uint8_t is_black) {
    if (strlen(s) > 10) {
        str(s, 0, y_off, is_black);
        str_center(s + 10, y_off + 2, is_black);
    } else {
        str_center(s, y_off, is_black);
    }
}

static uint8_t number(int32_t score, uint8_t x_off, uint8_t y_off, uint8_t is_black) {
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

    uint8_t off = (x_off == 0xFF) ? (LINE_WIDTH - len) : ((x_off == 0xFE) ? ((LINE_WIDTH * 2) - (len * 2)) : x_off);
    for (uint8_t i = 0; i < len; i++) {
        digit(digits[len - i - 1], i, off, y_off, is_black);
    }

    return 8 * len * 2;
}

static void fill_win(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tile, uint8_t attr) {
    VBK_REG = VBK_ATTRIBUTES;
    fill_win_rect(x, y, w, h, attr);
    VBK_REG = VBK_TILES;
    fill_win_rect(x, y, w, h, tile);
}

void win_splash_draw(int32_t lowest, int32_t highest) BANKED {
    // reuse full black and white tiles at 0 and 1 from splash bg
    fill_win(0, 0, 10, 4, 0, 0x00);
    fill_win(10, 0, 10, 4, 1, 0x00);

    // only show on splash if they fit
    if ((lowest <= 99999) && (highest <= 99999)) {
        number(lowest, 0, 0, 1);
        number(highest, 0xFE, 0, 0);

        str("top", 0, 2, 1);
        str("score", 10, 2, 0);
    }
}

void win_score_clear(uint8_t is_black) BANKED {
    set_win_based(0, 0,
                  title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H,
                  title_map_map, 0, BANK(title_map), title_map_MAP_ATTRIBUTES, BANK(title_map));

    str_center(is_black ? "black" : "white", 1, is_black);
}

void win_score_draw(struct scores score, uint8_t off, uint8_t is_black) BANKED {
    str3(score.name, 0, 4 + off * 3, is_black, is_black, is_black);
    number(is_black ? -score.score : score.score, 7, 4 + off * 3, is_black);
}

static void get_git(char *line_buff) NONBANKED {
    START_ROM_BANK(BANK(git));
        strncpy(line_buff, git_version, 2 * LINE_WIDTH);
    END_ROM_BANK();
}

void win_about(void) BANKED {
    set_win_based(0, 0,
                  title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H,
                  title_map_map, 0, BANK(title_map), title_map_MAP_ATTRIBUTES, BANK(title_map));

    str_center("Duality", 0, 1);
    str_center("xythobuz", 2, 1);

    char line_buff[2 * LINE_WIDTH + 1] = {0};
    get_git(line_buff);

    str_lines(line_buff, 7, 0);

    str_l(&__DATE__[7], 4,           0, 14, 1); // year (4)
    str_l(&__DATE__[0], 3, (4 * 2) + 1, 14, 1); // month (3)
    str_l(&__DATE__[4], 2, (7 * 2) + 2, 14, 1); // day (2)

    str(__TIME__, 4, 16, 0);
}

static uint8_t get_debug(char *name_buff, uint8_t i) NONBANKED {
    START_ROM_BANK(BANK(main));
        strncpy(name_buff, debug_entries[i].name, ENTRY_NAME_LEN + 1);

        uint8_t n_len = strlen(name_buff);
        name_buff[n_len] = ' ';
        if (debug_entries[i].flag == DBG_NONE) {
            if (debug_menu_index == i) {
                name_buff[n_len + 1] = debug_special_value + '0';
            } else {
                name_buff[n_len + 1] = '0';
            }
        } else {
            name_buff[n_len + 1] = (conf_get()->debug_flags & debug_entries[i].flag) ? '1' : '0';
        }
        name_buff[n_len + 2] = '\0';
        n_len += 2;
    END_ROM_BANK();
    return n_len;
}

void win_debug(void) BANKED {
    set_win_based(0, 0,
                  title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H,
                  title_map_map, 0, BANK(title_map), title_map_MAP_ATTRIBUTES, BANK(title_map));

    // TODO paging when more options added
    static_assert(DEBUG_ENTRY_COUNT <= 8, "too many debug menu entries");
    uint8_t off = (10 - DEBUG_ENTRY_COUNT) / 2;

    str_center("Debug Menu", 0, 0);

    for (uint8_t i = 0; (i < DEBUG_ENTRY_COUNT) && (i < 8); i++) {
        char name_buff[ENTRY_NAME_LEN + 2 + 1] = {0};
        uint8_t n_len = get_debug(name_buff, i);
        str(name_buff, (LINE_WIDTH - n_len) * 2, (i * 2) + 3 + off, (debug_menu_index == i) ? 1 : 0);
    }
}

static uint8_t get_conf(char *name_buff, uint8_t i) NONBANKED {
    START_ROM_BANK(BANK(main));
        strncpy(name_buff, conf_entries[i].name, ENTRY_NAME_LEN + 1);

        uint8_t n_len = strlen(name_buff);
        name_buff[n_len] = ' ';
        if (*conf_entries[i].var < 10) {
            name_buff[n_len + 1] = *conf_entries[i].var + '0';
        } else {
            name_buff[n_len + 1] = *conf_entries[i].var - 10 + 'A';
        }
        name_buff[n_len + 2] = '\0';
        n_len += 2;
    END_ROM_BANK();
    return n_len;
}

void win_conf(void) BANKED {
    set_win_based(0, 0,
                  title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H,
                  title_map_map, 0, BANK(title_map), title_map_MAP_ATTRIBUTES, BANK(title_map));

    // TODO paging when more options added
    static_assert(CONF_ENTRY_COUNT <= 8, "too many conf menu entries");
    uint8_t off = (10 - CONF_ENTRY_COUNT) / 2;

    str_center("Conf Menu", 0, 0);

    for (uint8_t i = 0; (i < CONF_ENTRY_COUNT) && (i < 8); i++) {
        char name_buff[ENTRY_NAME_LEN + 2 + 1] = {0};
        uint8_t n_len = get_conf(name_buff, i);
        str(name_buff, (LINE_WIDTH - n_len) * 2, (i * 2) + 3 + off, (debug_menu_index == i) ? 1 : 0);
    }
}

void win_name(int32_t score) BANKED {
    set_win_based(0, 0,
                  title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H,
                  title_map_map, 0, BANK(title_map), title_map_MAP_ATTRIBUTES, BANK(title_map));

    str_center("score", 1, score < 0);
    number(score < 0 ? -score : score, 0xFF, 3, score < 0);

    str_center("enter", 6, score < 0);
    str_center("name", 8, score < 0);

    str_center("start ok", 16, score < 0);
}

void win_name_draw(uint16_t name, uint8_t is_black, uint8_t pos) BANKED {
    str3(name, LINE_WIDTH - 3, 12,
         (pos == 0) ? !is_black : is_black,
         (pos == 1) ? !is_black : is_black,
         (pos == 2) ? !is_black : is_black);
}

uint8_t win_game_draw(int32_t score) BANKED {
    fill_win(0, 0, 10, 2, fnt_off + numbers_fnt_TILE_COUNT, 0x81);

    uint8_t is_black = 0;
    if (score < 0) {
        score = -score;
        is_black = 1;
    }

    return number(score, 0, 0, is_black);
}
