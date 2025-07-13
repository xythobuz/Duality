/*
 * map_data.h
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

#ifndef __MAP__DATA_H__
#define __MAP__DATA_H__

#include <gbdk/platform.h>
#include <stdint.h>

#include "maps.h"

// background tiles are loaded into VRAM starting at tile number 0
#define BG_TILE_NUM_START 0

#define BG_COPY_NONE 0xFFU

#define BG_LOAD_GAME     (0U << 0)
#define BG_LOAD_SPLASH   (1U << 0)
#define BG_LOAD_ALL      (1U << 6)
#define BG_LOAD_GBC_ONLY (1U << 7)

struct maps {
    uint8_t width;
    uint8_t height;
    const uint8_t * map;
    const uint8_t * tiles;
    const palette_color_t * palettes;
    uint8_t palette_count;
    uint8_t palette_index;
    uint16_t map_count;
    uint8_t tile_count;
    uint8_t tile_offset;
    uint8_t tile_copy;
    uint8_t bank;
    uint8_t load;
};

extern struct maps maps[MAP_COUNT];

BANKREF_EXTERN(map_data)

#define INV_PALETTE_COUNT 1
extern const palette_color_t num_pal_inv[INV_PALETTE_COUNT * 4];

#endif // __MAP__DATA_H__
