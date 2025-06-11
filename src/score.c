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

#include <string.h>

#include "banks.h"
#include "config.h"
#include "score.h"

BANKREF(score)

#define NAME(a, b, c) (((uint16_t)(a - 'a') << 10) | ((uint16_t)(b - 'a') << 5) | (uint16_t)(c - 'a'))

static const struct scores initial_scores[SCORE_NUM * 2] = {
    //{ .name = NAME('a', 'd', 'z'), .score = 10000 },
    //{ .name = NAME('c', 'a', 'n'), .score = 7500 },
    //{ .name = NAME('i', 'm', 'y'), .score = 5000 },
    //{ .name = NAME('w', 'i', 'l'), .score = 2500 },
    { .name = NAME('d', 'b', 'p'), .score = 1000 },
    { .name = NAME('d', 'a', 'v'), .score = 750 },
    { .name = NAME('d', 'o', 'd'), .score = 500 },
    //{ .name = NAME('n', 'f', '.'), .score = 250 },
    { .name = NAME('k', 'm', 'b'), .score = 175 },
    { .name = NAME('s', 'j', 'l'), .score = 100 },

    { .name = NAME('j', 'u', 'd'), .score = -100 },
    //{ .name = NAME('1', '0', '1'), .score = -175 },
    { .name = NAME('g', 'a', 'z'), .score = -250 },
    { .name = NAME('n', 'o', 'n'), .score = -500 },
    { .name = NAME('l', 'r', 'g'), .score = -750 },
    { .name = NAME('d', 'a', 'n'), .score = -1000 },
    //{ .name = NAME('w', 'd', 'y'), .score = -2500 },
    //{ .name = NAME('s', 'i', 's'), .score = -5000 },
    //{ .name = NAME('k', 'r', 'y'), .score = -7500 },
    //{ .name = NAME('d', 'j', '.'), .score = -10000 },
};

uint16_t convert_name(char a, char b, char c) BANKED {
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

static uint8_t score_pos(int32_t score) {
    if (score > 0) {
        for (uint8_t i = 0; i < SCORE_NUM; i++) {
            if (score > conf_scores()[i].score) {
                return i;
            }
        }
    } else if (score < 0) {
        for (uint8_t i = (SCORE_NUM * 2) - 1; i >= 5; i--) {
            if (score < conf_scores()[i].score) {
                return i;
            }
        }
    }

    return 0xFF;
}

uint8_t score_ranking(int32_t score) BANKED {
    return (score_pos(score) < (SCORE_NUM * 2)) ? 1 : 0;
}

void score_add(struct scores score) BANKED {
    uint8_t new = score_pos(score.score);
    if (new < (SCORE_NUM * 2)) {
        // move old scores out of the way
        if ((score.score > 0) && (new < (SCORE_NUM - 1))) {
            memmove(conf_scores() + new + 1, conf_scores() + new, sizeof(struct scores) * (SCORE_NUM - 1 - new));
        } else if ((score.score < 0) && (new > SCORE_NUM)) {
            memmove(conf_scores() + new - 1, conf_scores() + new, sizeof(struct scores) * (new - SCORE_NUM));
        }

        conf_scores()[new] = score;
        conf_write_crc();

#ifdef DEBUG
        EMU_printf("%s: add %li at %hu for %x\n",
                   __func__, (int32_t)score.score,
                   (uint8_t)new, (uint16_t)score.name);
#endif // DEBUG
    }
}

void score_highest(uint8_t off, struct scores *t) BANKED {
    if (off >= SCORE_NUM) {
        off = SCORE_NUM - 1;
    }
    *t = conf_scores()[off];
}

void score_lowest(uint8_t off, struct scores *t) BANKED {
    if (off >= SCORE_NUM) {
        off = SCORE_NUM - 1;
    }
    *t = conf_scores()[(SCORE_NUM * 2) - 1 - off];
}

void score_reset(void) NONBANKED {
    START_ROM_BANK(BANK(score));
        memcpy(conf_scores(), initial_scores, sizeof(struct scores) * SCORE_NUM * 2);
    END_ROM_BANK();
    conf_write_crc();
}

void score_zero(void) NONBANKED {
    memset(conf_scores(), 0, sizeof(struct scores) * SCORE_NUM * 2);
    conf_write_crc();
}
