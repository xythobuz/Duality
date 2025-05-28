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
#include "health.h"

// Metasprite tiles are loaded into VRAM starting at tile number 0
#define TILE_NUM_START 0

struct sprites {
    const metasprite_t * const * ms;
    const uint8_t * ti;
    const palette_color_t * pa;
    uint8_t pa_i;
    uint8_t cnt;
    uint8_t off;
};

const palette_color_t power_palettes[4] = {
  //RGB8(  0,  0,  0), RGB8(240,  0,  0), RGB8(196,  0,  0), RGB8(116,  0,  0)
    RGB8(  0,  0,  0), RGB8(  0,240,  0), RGB8(  0,196,  0), RGB8(  0,116,  0)

};

static struct sprites metasprites[SPRITE_COUNT] = {
    {
        .ms = rockshp_0_metasprites,
        .ti = rockshp_0_tiles,
        .pa = rockshp_0_palettes,
        .pa_i = OAMF_CGB_PAL0,
        .cnt = rockshp_0_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = rockshp_90_metasprites,
        .ti = rockshp_90_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL0,
        .cnt = rockshp_90_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = thrust_0_metasprites,
        .ti = thrust_0_tiles,
        .pa = thrust_0_palettes,
        .pa_i = OAMF_CGB_PAL1,
        .cnt = thrust_0_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = thrust_90_metasprites,
        .ti = thrust_90_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL1,
        .cnt = thrust_90_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = light_metasprites,
        .ti = light_tiles,
        .pa = light_palettes,
        .pa_i = OAMF_CGB_PAL2,
        .cnt = light_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = dark_metasprites,
        .ti = dark_tiles,
        .pa = dark_palettes,
        .pa_i = OAMF_CGB_PAL3,
        .cnt = dark_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = shoot_metasprites,
        .ti = shoot_tiles,
        .pa = shoot_palettes,
        .pa_i = OAMF_CGB_PAL4,
        .cnt = shoot_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = health_metasprites,
        .ti = health_tiles,
        .pa = health_palettes,
        .pa_i = OAMF_CGB_PAL5,
        .cnt = health_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = health_metasprites,
        .ti = health_tiles,
        .pa = power_palettes,
        .pa_i = OAMF_CGB_PAL6,
        .cnt = health_TILE_COUNT,
        .off = SPR_HEALTH
    }
};

void spr_init(void) {
    uint8_t off = TILE_NUM_START;
    for (uint8_t i = 0; i < SPRITE_COUNT; i++) {
        if (metasprites[i].pa != NULL) {
            set_sprite_palette(metasprites[i].pa_i, 1, metasprites[i].pa);
        }

        if (metasprites[i].off == TILE_NUM_START) {
            metasprites[i].off = off;
            off += metasprites[i].cnt;
            set_sprite_data(metasprites[i].off, metasprites[i].cnt, metasprites[i].ti);
        } else {
            metasprites[i].off = metasprites[metasprites[i].off].off;
        }
    }
}

void spr_draw(enum SPRITES sprite, enum SPRITE_FLIP flip, int8_t x_off, int8_t y_off, uint8_t *hiwater) {
    switch (flip) {
        case FLIP_Y:
            *hiwater += move_metasprite_flipy(
                    metasprites[sprite].ms[0], metasprites[sprite].off,
                    metasprites[sprite].pa_i, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_XY:
            *hiwater += move_metasprite_flipxy(
                    metasprites[sprite].ms[0], metasprites[sprite].off,
                    metasprites[sprite].pa_i, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_X:
            *hiwater += move_metasprite_flipx(
                    metasprites[sprite].ms[0], metasprites[sprite].off,
                    metasprites[sprite].pa_i, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_NONE:
        default:
            *hiwater += move_metasprite_ex(
                    metasprites[sprite].ms[0], metasprites[sprite].off,
                    metasprites[sprite].pa_i, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;
    }
}

void spr_ship(enum SPRITE_ROT rot, uint8_t moving, uint8_t *hiwater) {
    switch (rot) {
        case ROT_0:
            spr_draw(SPR_SHIP_0, FLIP_NONE, 0, 0, hiwater);
            if (moving) {
                spr_draw(SPR_THRUST_0, FLIP_NONE, 0, SHIP_OFF, hiwater);
            }
            break;

        case ROT_90:
            spr_draw(SPR_SHIP_90, FLIP_NONE, 0, 0, hiwater);
            if (moving) {
                spr_draw(SPR_THRUST_90, FLIP_NONE, -SHIP_OFF, 0, hiwater);
            }
            break;

        case ROT_180:
            spr_draw(SPR_SHIP_0, FLIP_Y, 0, 0, hiwater);
            if (moving) {
                spr_draw(SPR_THRUST_0, FLIP_Y, 0, -SHIP_OFF, hiwater);
            }
            break;

        case ROT_270:
            spr_draw(SPR_SHIP_90, FLIP_X, 0, 0, hiwater);
            if (moving) {
                spr_draw(SPR_THRUST_90, FLIP_X, SHIP_OFF, 0, hiwater);
            }
            break;

        default:
            break;
    }
}
