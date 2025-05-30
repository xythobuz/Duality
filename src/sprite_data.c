/*
 * sprite_data.c
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

#include "sprite_data.h"

#include "rockshp_0.h"
#include "rockshp_90.h"
#include "thrust_0.h"
#include "thrust_90.h"
#include "light.h"
#include "dark.h"
#include "shoot.h"
#include "bar_1.h"
#include "bar_2.h"
#include "bar_3.h"
#include "bar_4.h"
#include "bar_5.h"
#include "bar_6.h"
#include "bar_7.h"
#include "bar_8.h"

const palette_color_t power_palettes[4] = {
  //RGB8(  0,  0,  0), RGB8(240,  0,  0), RGB8(196,  0,  0), RGB8(116,  0,  0)
    RGB8(  0,  0,  0), RGB8(  0,240,  0), RGB8(  0,196,  0), RGB8(  0,116,  0)

};

struct sprites metasprites[SPRITE_COUNT] = {
    { // SPR_SHIP_0
        .ms = rockshp_0_metasprites,
        .ti = rockshp_0_tiles,
        .pa = rockshp_0_palettes,
        .pa_i = OAMF_CGB_PAL0,
        .cnt = rockshp_0_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_SHIP_90
        .ms = rockshp_90_metasprites,
        .ti = rockshp_90_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL0,
        .cnt = rockshp_90_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_THRUST_0
        .ms = thrust_0_metasprites,
        .ti = thrust_0_tiles,
        .pa = thrust_0_palettes,
        .pa_i = OAMF_CGB_PAL1,
        .cnt = thrust_0_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_THRUST_90
        .ms = thrust_90_metasprites,
        .ti = thrust_90_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL1,
        .cnt = thrust_90_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_LIGHT
        .ms = light_metasprites,
        .ti = light_tiles,
        .pa = light_palettes,
        .pa_i = OAMF_CGB_PAL2,
        .cnt = light_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_DARK
        .ms = dark_metasprites,
        .ti = dark_tiles,
        .pa = dark_palettes,
        .pa_i = OAMF_CGB_PAL3,
        .cnt = dark_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_SHOT
        .ms = shoot_metasprites,
        .ti = shoot_tiles,
        .pa = shoot_palettes,
        .pa_i = OAMF_CGB_PAL4,
        .cnt = shoot_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_SHOT_LIGHT
        .ms = shoot_metasprites,
        .ti = shoot_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL2,
        .cnt = shoot_TILE_COUNT,
        .off = SPR_SHOT
    },
    { // SPR_SHOT_DARK
        .ms = shoot_metasprites,
        .ti = shoot_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL3,
        .cnt = shoot_TILE_COUNT,
        .off = SPR_SHOT
    },
    { // SPR_HEALTH_1
        .ms = bar_1_metasprites,
        .ti = bar_1_tiles,
        .pa = bar_1_palettes,
        .pa_i = OAMF_CGB_PAL5,
        .cnt = bar_1_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_HEALTH_2
        .ms = bar_2_metasprites,
        .ti = bar_2_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL5,
        .cnt = bar_2_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_HEALTH_3
        .ms = bar_3_metasprites,
        .ti = bar_3_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL5,
        .cnt = bar_3_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_HEALTH_4
        .ms = bar_4_metasprites,
        .ti = bar_4_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL5,
        .cnt = bar_4_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_HEALTH_5
        .ms = bar_5_metasprites,
        .ti = bar_5_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL5,
        .cnt = bar_5_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_HEALTH_6
        .ms = bar_6_metasprites,
        .ti = bar_6_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL5,
        .cnt = bar_6_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_HEALTH_7
        .ms = bar_7_metasprites,
        .ti = bar_7_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL5,
        .cnt = bar_7_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_HEALTH_8
        .ms = bar_8_metasprites,
        .ti = bar_8_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL5,
        .cnt = bar_8_TILE_COUNT,
        .off = TILE_NUM_START
    },
    { // SPR_POWER_1
        .ms = bar_1_metasprites,
        .ti = bar_1_tiles,
        .pa = power_palettes,
        .pa_i = OAMF_CGB_PAL6,
        .cnt = bar_1_TILE_COUNT,
        .off = SPR_HEALTH_1
    },
    { // SPR_POWER_2
        .ms = bar_2_metasprites,
        .ti = bar_2_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL6,
        .cnt = bar_2_TILE_COUNT,
        .off = SPR_HEALTH_2
    },
    { // SPR_POWER_3
        .ms = bar_3_metasprites,
        .ti = bar_3_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL6,
        .cnt = bar_3_TILE_COUNT,
        .off = SPR_HEALTH_3
    },
    { // SPR_POWER_4
        .ms = bar_4_metasprites,
        .ti = bar_4_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL6,
        .cnt = bar_4_TILE_COUNT,
        .off = SPR_HEALTH_4
    },
    { // SPR_POWER_5
        .ms = bar_5_metasprites,
        .ti = bar_5_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL6,
        .cnt = bar_5_TILE_COUNT,
        .off = SPR_HEALTH_5
    },
    { // SPR_POWER_6
        .ms = bar_6_metasprites,
        .ti = bar_6_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL6,
        .cnt = bar_6_TILE_COUNT,
        .off = SPR_HEALTH_6
    },
    { // SPR_POWER_7
        .ms = bar_7_metasprites,
        .ti = bar_7_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL6,
        .cnt = bar_7_TILE_COUNT,
        .off = SPR_HEALTH_7
    },
    { // SPR_POWER_8
        .ms = bar_8_metasprites,
        .ti = bar_8_tiles,
        .pa = NULL,
        .pa_i = OAMF_CGB_PAL6,
        .cnt = bar_8_TILE_COUNT,
        .off = SPR_HEALTH_8
    }
};
