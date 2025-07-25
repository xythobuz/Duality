/*
 * maps.h
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

#ifndef __MAPS_H__
#define __MAPS_H__

#include <gbdk/platform.h>
#include <stdint.h>

enum MAPS {
    MAP_TITLE = 0,
    MAP_GAME_1,
    MAP_GAME_2,

    FNT_TEXT_16,
    FNT_TEXT_16_INV,

    FNT_NUM_16,
    FNT_NUM_16_INV,

    FNT_ASCII_8,

    MAP_COUNT
};

void map_load(uint8_t is_splash) BANKED;
void map_fill(enum MAPS map, uint8_t bkg);
void map_move(int16_t delta_x, int16_t delta_y);

void map_dbg_reset(void);

BANKREF_EXTERN(maps)

#endif // __MAPS_H__
