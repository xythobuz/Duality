/*
 * score.h
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

#ifndef __SCORE_H__
#define __SCORE_H__

#include <gbdk/platform.h>
#include <stdint.h>

#define SCORE_NUM 5

struct scores {
    uint16_t name;
    int32_t score;
};

uint16_t convert_name(char a, char b, char c) BANKED;
uint8_t score_ranking(int32_t score) BANKED;
void score_add(struct scores score) BANKED;
void score_highest(uint8_t off, struct scores *t) BANKED;
void score_lowest(uint8_t off, struct scores *t) BANKED;
void score_reset(void);
void score_zero(void);

BANKREF_EXTERN(score)

#endif // __SCORE_H__
