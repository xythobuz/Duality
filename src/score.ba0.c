/*
 * score.ba0.c
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

#include "score.h"

static struct scores scores[SCORE_NUM * 2];
static uint32_t scores_crc;

static uint16_t convert_name(char a, char b, char c) {
    // convert to lowercase
    if ((a >= 'A') && (a <= 'Z')) a = a - 'A' + 'a';
    if ((b >= 'A') && (b <= 'Z')) b = b - 'A' + 'a';
    if ((c >= 'A') && (c <= 'Z')) c = c - 'A' + 'a';

    // skip invalid characters
    if ((a < 'a') || (a > 'z')) a = 'x';
    if ((b < 'a') || (b > 'z')) b = 'x';
    if ((c < 'a') || (c > 'z')) c = 'x';

    // zero offset in alphabet
    a -= 'a';
    b -= 'a';
    c -= 'a';

    return (a << 10) | (b << 5) | c;
}

static uint32_t calc_crc(void) NONBANKED {
    const uint8_t *d = (const uint8_t *)scores;

    uint32_t c = 0xFFFFFFFF;
    for (size_t i = 0; i < sizeof(scores); i++) {
        // adapted from "Hacker's Delight"
        c ^= d[i];
        for (size_t j = 0; j < 8; j++) {
            uint32_t mask = -(c & 1);
            c = (c >> 1) ^ (0xEDB88320 & mask);
        }
    }

    return ~c;
}

static uint8_t check_crc(void) NONBANKED {
    return (calc_crc() == scores_crc) ? 1 : 0;
}

static void score_init(void) NONBANKED {
    // TODO
    scores[0].name = convert_name('a', 'b', 'c');
    scores[0].score = 10000;

    scores[1].name = convert_name('a', 'b', 'c');
    scores[1].score = 8000;

    scores[2].name = convert_name('a', 'b', 'c');
    scores[2].score = 6000;

    scores[3].name = convert_name('a', 'b', 'c');
    scores[3].score = 4000;

    scores[4].name = convert_name('a', 'b', 'c');
    scores[4].score = 2000;

    scores[5].name = convert_name('a', 'b', 'c');
    scores[5].score = -2000;

    scores[6].name = convert_name('a', 'b', 'c');
    scores[6].score = -4000;

    scores[7].name = convert_name('a', 'b', 'c');
    scores[7].score = -6000;

    scores[8].name = convert_name('a', 'b', 'c');
    scores[8].score = -8000;

    scores[9].name = convert_name('a', 'b', 'c');
    scores[9].score = -10000;

    scores_crc = calc_crc();
}

void score_add(struct scores score) NONBANKED {
    ENABLE_RAM;
    SWITCH_RAM(0);

    // initialize score table when data is invalid
    if (!check_crc()) {
        score_init();
    }

    // TODO

    DISABLE_RAM;
}

struct scores score_highest(uint8_t off) NONBANKED {
    ENABLE_RAM;
    SWITCH_RAM(0);

    // initialize score table when data is invalid
    if (!check_crc()) {
        score_init();
    }

    if (off >= SCORE_NUM) {
        off = SCORE_NUM - 1;
    }
    struct scores r = scores[off];

    DISABLE_RAM;
    return r;
}

struct scores score_lowest(uint8_t off) NONBANKED {
    ENABLE_RAM;
    SWITCH_RAM(0);

    // initialize score table when data is invalid
    if (!check_crc()) {
        score_init();
    }

    if (off >= SCORE_NUM) {
        off = SCORE_NUM - 1;
    }
    struct scores r = scores[(SCORE_NUM * 2) - 1 - off];

    DISABLE_RAM;
    return r;
}
