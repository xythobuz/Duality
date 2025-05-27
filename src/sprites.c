/*
 * sprites.c
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

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#include "sprites.h"

#include "rockshp_0.h"
#include "rockshp_90.h"
#include "thrust_0.h"
#include "thrust_90.h"
#include "light.h"
#include "dark.h"
#include "shoot.h"

// Metasprite tiles are loaded into VRAM starting at tile number 0
#define TILE_NUM_START 0

struct sprites {
    const metasprite_t * const * ms;
    const uint8_t * ti;
    const palette_color_t * pa;
    uint8_t cnt;
    uint8_t off;
};

static struct sprites metasprites[SPRITE_COUNT] = {
    {
        .ms = rockshp_0_metasprites,
        .ti = rockshp_0_tiles,
        .pa = rockshp_0_palettes,
        .cnt = rockshp_0_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = rockshp_90_metasprites,
        .ti = rockshp_90_tiles,
        .pa = rockshp_90_palettes,
        .cnt = rockshp_90_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = thrust_0_metasprites,
        .ti = thrust_0_tiles,
        .pa = thrust_0_palettes,
        .cnt = thrust_0_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = thrust_90_metasprites,
        .ti = thrust_90_tiles,
        .pa = thrust_90_palettes,
        .cnt = thrust_90_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = light_metasprites,
        .ti = light_tiles,
        .pa = light_palettes,
        .cnt = light_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = dark_metasprites,
        .ti = dark_tiles,
        .pa = dark_palettes,
        .cnt = dark_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = shoot_metasprites,
        .ti = shoot_tiles,
        .pa = shoot_palettes,
        .cnt = shoot_TILE_COUNT,
        .off = TILE_NUM_START
    }
};

void spr_init(void) {
    uint8_t off = TILE_NUM_START;
    for (int i = 0; i < (sizeof(metasprites) / sizeof(metasprites[0])); i++) {
        metasprites[i].off = off;
        off += metasprites[i].cnt;

        set_sprite_palette(OAMF_CGB_PAL0 + i, 1, metasprites[i].pa);
        set_sprite_data(metasprites[i].off, metasprites[i].cnt, metasprites[i].ti);
    }
}

void spr_draw(enum SPRITES sprite, uint8_t *hiwater, enum SPRITE_FLIP flip, int8_t x_off, int8_t y_off) {
    switch (flip) {
        case FLIP_Y:
            *hiwater += move_metasprite_flipy(
                    metasprites[sprite].ms[0], metasprites[sprite].off,
                    OAMF_CGB_PAL0 + sprite, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_XY:
            *hiwater += move_metasprite_flipxy(
                    metasprites[sprite].ms[0], metasprites[sprite].off,
                    OAMF_CGB_PAL0 + sprite, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_X:
            *hiwater += move_metasprite_flipx(
                    metasprites[sprite].ms[0], metasprites[sprite].off,
                    OAMF_CGB_PAL0 + sprite, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_NONE:
        default:
            *hiwater += move_metasprite_ex(
                    metasprites[sprite].ms[0], metasprites[sprite].off,
                    OAMF_CGB_PAL0 + sprite, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;
    }
}

void spr_ship(enum SPRITE_ROT rot, uint8_t moving, uint8_t *hiwater) {
    switch (rot) {
        case ROT_0:
            spr_draw(SPR_SHIP_0, hiwater, FLIP_NONE, 0, 0);
            if (moving) {
                spr_draw(SPR_THRUST_0, hiwater, FLIP_NONE, 0, SHIP_OFF);
            }
            break;

        case ROT_90:
            spr_draw(SPR_SHIP_90, hiwater, FLIP_NONE, 0, 0);
            if (moving) {
                spr_draw(SPR_THRUST_90, hiwater, FLIP_NONE, -SHIP_OFF, 0);
            }
            break;

        case ROT_180:
            spr_draw(SPR_SHIP_0, hiwater, FLIP_Y, 0, 0);
            if (moving) {
                spr_draw(SPR_THRUST_0, hiwater, FLIP_Y, 0, -SHIP_OFF);
            }
            break;

        case ROT_270:
            spr_draw(SPR_SHIP_90, hiwater, FLIP_X, 0, 0);
            if (moving) {
                spr_draw(SPR_THRUST_90, hiwater, FLIP_X, SHIP_OFF, 0);
            }
            break;

        default:
            break;
    }
}
