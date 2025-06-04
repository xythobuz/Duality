/*
 * sprite_data.h
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * Based on examples from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/cross-platform/metasprites/src/metasprites.c
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

#ifndef __SPRITE__DATA_H
#define __SPRITE__DATA_H

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <stdint.h>

#include "sprites.h"

// Metasprite tiles are loaded into VRAM starting at tile number 0
#define TILE_NUM_START 0

#define PALETTE_DYNAMIC_LOAD 0xF0
#define PALETTE_NO_FLAGS 0x0F

#define ARR_LEN(x) (sizeof(x) / sizeof(x[0]))

struct sprites {
    const metasprite_t * const * ms;
    uint8_t ms_n;
    const uint8_t * ti;
    const palette_color_t * pa;
    uint8_t pa_n;
    uint8_t pa_i;
    uint8_t cnt;
    uint8_t off;
    uint8_t bank;
};

extern struct sprites metasprites[SPRITE_COUNT];

#endif // __SPRITE__DATA_H
