/*
 * text.c
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
#include <stdio.h>

#include "banks.h"
#include "maps.h"
#include "map_data.h"
#include "text.h"

// TODO inverted score color not visible on DMG
// TODO 8x8 font only available on GBC

#define TEXT_PALETTE_WHITE BKGF_CGB_PAL3
#define TEXT_PALETTE_BLACK BKGF_CGB_PAL4

#define TEXT_ATTR_WHITE    (BKGF_PRI | TEXT_PALETTE_WHITE)
#define TEXT_ATTR_BLACK    (BKGF_PRI | TEXT_PALETTE_BLACK)

#define ASCI_ATTR_DARK     (BKGF_PRI | BKGF_BANK1 | BKGF_CGB_PAL6) // TODO?
#define ASCI_ATTR_LIGHT    (BKGF_PRI | BKGF_BANK1 | BKGF_CGB_PAL5) // TODO?

BANKREF(text)

static void digit(uint8_t val, uint8_t pos, uint8_t x_off, uint8_t y_off, uint8_t is_black);

static void set_win_based(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                          const uint8_t *tiles, uint8_t base_tile, uint8_t tile_bank,
                          const uint8_t *attributes, uint8_t attr_bank) NONBANKED {
    if (attributes != NULL) {
        START_ROM_BANK(attr_bank) {
            VBK_REG = VBK_ATTRIBUTES;
            set_win_tiles(x, y, w, h, attributes);
        } END_ROM_BANK
    } else {
        VBK_REG = VBK_ATTRIBUTES;
        fill_win_rect(x, y, w, h, 0x00);
    }

    START_ROM_BANK(tile_bank) {
        VBK_REG = VBK_TILES;
        set_win_based_tiles(x, y, w, h, tiles, base_tile);
    } END_ROM_BANK
}

static void set_win_based_attr(uint8_t x, uint8_t y, uint8_t w, uint8_t h,
                               const uint8_t *tiles, uint8_t base_tile, uint8_t tile_bank,
                               const uint8_t attr) NONBANKED {
    VBK_REG = VBK_ATTRIBUTES;
    fill_win_rect(x, y, w, h, attr);

    START_ROM_BANK(tile_bank) {
        VBK_REG = VBK_TILES;
        set_win_based_tiles(x, y, w, h, tiles, base_tile);
    } END_ROM_BANK
}

// ----------------------------------------------------------------------------
// Characters 16x16 (for menus)
// ----------------------------------------------------------------------------

static void character(uint8_t c, uint8_t pos, uint8_t x_off, uint8_t y_off, uint8_t is_black) {
    uint8_t off = c * maps[FNT_TEXT_16].width;

    set_win_based_attr(x_off + (pos * maps[FNT_TEXT_16].width), y_off,
                       maps[FNT_TEXT_16].width, 1,
                       maps[FNT_TEXT_16].map + off,
                       maps[FNT_TEXT_16].tile_offset,
                       maps[FNT_TEXT_16].bank, is_black ? TEXT_ATTR_BLACK : TEXT_ATTR_WHITE);

    set_win_based_attr(x_off + (pos * maps[FNT_TEXT_16].width), y_off + 1,
                       maps[FNT_TEXT_16].width, 1,
                       maps[FNT_TEXT_16].map + off + (maps[FNT_TEXT_16].map_count / 2),
                       maps[FNT_TEXT_16].tile_offset,
                       maps[FNT_TEXT_16].bank, is_black ? TEXT_ATTR_BLACK : TEXT_ATTR_WHITE);
}

void str3(uint16_t name, uint8_t x_off, uint8_t y_off,
          uint8_t is_black_a, uint8_t is_black_b, uint8_t is_black_c) BANKED {
    character((name >> 10) & 0x1F, 0, x_off, y_off, is_black_a);
    character((name >>  5) & 0x1F, 1, x_off, y_off, is_black_b);
    character((name >>  0) & 0x1F, 2, x_off, y_off, is_black_c);
}

void str_l(const char *s, uint8_t len, uint8_t x_off, uint8_t y_off, uint8_t is_black) BANKED {
    for (uint8_t n = 0; (*s) && (n < TEXT_LINE_WIDTH) && (n < len); n++) {
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

void str(const char *s, uint8_t x_off, uint8_t y_off, uint8_t is_black) BANKED {
    str_l(s, 0xFF, x_off, y_off, is_black);
}

void str_center(const char *s, uint8_t y_off, uint8_t is_black) BANKED {
    uint8_t n = strlen(s);
    if (n > TEXT_LINE_WIDTH) n = TEXT_LINE_WIDTH;
    str(s, TEXT_LINE_WIDTH - n, y_off, is_black);
}

void str_lines(const char *s, uint8_t y_off, uint8_t is_black) BANKED {
    if (strlen(s) > 10) {
        str(s, 0, y_off, is_black);
        str_center(s + 10, y_off + 2, is_black);
    } else {
        str_center(s, y_off, is_black);
    }
}

// ----------------------------------------------------------------------------
// Numbers 16x16 (for scores)
// ----------------------------------------------------------------------------

static void digit(uint8_t val, uint8_t pos, uint8_t x_off, uint8_t y_off, uint8_t is_black) {
    uint8_t off = val * maps[FNT_NUM_16].width;

    set_win_based_attr(x_off + (pos * maps[FNT_NUM_16].width), y_off,
                       maps[FNT_NUM_16].width, 1,
                       maps[FNT_NUM_16].map + off,
                       maps[FNT_NUM_16].tile_offset,
                       maps[FNT_NUM_16].bank, is_black ? TEXT_ATTR_BLACK : TEXT_ATTR_WHITE);

    set_win_based_attr(x_off + (pos * maps[FNT_NUM_16].width), y_off + 1,
                       maps[FNT_NUM_16].width, 1,
                       maps[FNT_NUM_16].map + off + (maps[FNT_NUM_16].map_count / 2),
                       maps[FNT_NUM_16].tile_offset,
                       maps[FNT_NUM_16].bank, is_black ? TEXT_ATTR_BLACK : TEXT_ATTR_WHITE);
}

uint8_t number(int32_t score, uint8_t x_off, uint8_t y_off, uint8_t is_black) BANKED {
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

    uint8_t off = (x_off == 0xFF) ? (TEXT_LINE_WIDTH - len)
               : ((x_off == 0xFE) ? ((TEXT_LINE_WIDTH * 2) - (len * 2)) : x_off);
    for (uint8_t i = 0; i < len; i++) {
        digit(digits[len - i - 1], i, off, y_off, is_black);
    }

    return 8 * len * 2;
}

// ----------------------------------------------------------------------------
// GBC-only ASCII 8x8 font (for detailed / debug output)
// ----------------------------------------------------------------------------

static void char_ascii(uint8_t c, uint8_t pos, uint8_t x_off, uint8_t y_off, uint8_t light) {
    set_win_based_attr(x_off + pos, y_off, 1, 1,
                       maps[FNT_ASCII_8].map + c, 0,
                       maps[FNT_ASCII_8].bank, light ? ASCI_ATTR_LIGHT : ASCI_ATTR_DARK);
}

void str_ascii_l(const char *s, uint8_t len, uint8_t x_off, uint8_t y_off, uint8_t light) BANKED {
    for (uint8_t n = 0; (*s) && (n < (2 * TEXT_LINE_WIDTH)) && (n < len); n++) {
        char c = *(s++);
        char_ascii(c, n, x_off, y_off, light);
    }
}

void str_ascii(const char *s, uint8_t x_off, uint8_t y_off, uint8_t light) BANKED {
    str_ascii_l(s, 0xFF, x_off, y_off, light);
}

void str_ascii_lines(const char *s, uint8_t y_off, uint8_t light) BANKED {
    const char *nl = s;
    uint8_t lines = 0;
    do {
        // find next newline
        while (*nl && (*nl != '\n')) nl++;
        str_ascii_l(s, nl - s, 0, y_off + lines, light);
        lines++;
        if (*nl) nl++;
        s = nl;
    } while (*nl);
}
