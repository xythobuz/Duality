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

#include "rockshp_spr24.h"
#include "light.h"
#include "dark.h"
#include "shoot.h"
#include "bar_spr8.h"
#include "expl_spr16.h"
#include "pause.h"
#include "status_spr8.h"
#include "debug_marker.h"
#include "debug_marker_spr32.h"

BANKREF(power_palettes)

/*
 * OCP0: Rocketship (1)
 * OCP1: Rocketship (2)
 * OCP2: Light
 * OCP3: Dark
 * OCP4: Shot
 * OCP5: Health
 * OCP6: Power
 * OCP7: Pause
 *
 * Explosion uses OCP0 to OCP3 at end of game.
 * Pause is flipped in-place for animating the pause screen colors.
 */

const palette_color_t power_palettes[4] = {
  //RGB8(  0,  0,  0), RGB8(240,  0,  0), RGB8(196,  0,  0), RGB8(116,  0,  0)
    RGB8(  0,  0,  0), RGB8(  0,240,  0), RGB8(  0,196,  0), RGB8(  0,116,  0)
};

struct sprites metasprites[SPRITE_COUNT] = {
    { // SPR_SHIP
        .ms = rockshp_spr24_metasprites,
        .ms_n = ARR_LEN(rockshp_spr24_metasprites),
        .ti = rockshp_spr24_tiles,
        .pa = rockshp_spr24_palettes,
        .pa_n = rockshp_spr24_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL0,
        .cnt = rockshp_spr24_TILE_COUNT,
        .off = TILE_NUM_START,
        .bank = BANK(rockshp_spr24),
    },
    { // SPR_LIGHT
        .ms = light_metasprites,
        .ms_n = ARR_LEN(light_metasprites),
        .ti = light_tiles,
        .pa = light_palettes,
        .pa_n = light_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL2,
        .cnt = light_TILE_COUNT,
        .off = TILE_NUM_START,
        .bank = BANK(light),
    },
    { // SPR_DARK
        .ms = dark_metasprites,
        .ms_n = ARR_LEN(dark_metasprites),
        .ti = dark_tiles,
        .pa = dark_palettes,
        .pa_n = dark_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL3,
        .cnt = dark_TILE_COUNT,
        .off = TILE_NUM_START,
        .bank = BANK(dark),
    },
    { // SPR_SHOT
        .ms = shoot_metasprites,
        .ms_n = ARR_LEN(shoot_metasprites),
        .ti = shoot_tiles,
        .pa = shoot_palettes,
        .pa_n = shoot_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL4,
        .cnt = shoot_TILE_COUNT,
        .off = TILE_NUM_START,
        .bank = BANK(shoot),
    },
    { // SPR_SHOT_LIGHT
        .ms = shoot_metasprites,
        .ms_n = ARR_LEN(shoot_metasprites),
        .ti = shoot_tiles,
        .pa = NULL,
        .pa_n = shoot_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL2,
        .cnt = shoot_TILE_COUNT,
        .off = SPR_SHOT,
        .bank = BANK(shoot),
    },
    { // SPR_SHOT_DARK
        .ms = shoot_metasprites,
        .ms_n = ARR_LEN(shoot_metasprites),
        .ti = shoot_tiles,
        .pa = NULL,
        .pa_n = shoot_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL3,
        .cnt = shoot_TILE_COUNT,
        .off = SPR_SHOT,
        .bank = BANK(shoot),
    },
    { // SPR_HEALTH
        .ms = bar_spr8_metasprites,
        .ms_n = ARR_LEN(bar_spr8_metasprites),
        .ti = bar_spr8_tiles,
        .pa = bar_spr8_palettes,
        .pa_n = bar_spr8_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL5,
        .cnt = bar_spr8_TILE_COUNT,
        .off = TILE_NUM_START,
        .bank = BANK(bar_spr8),
    },
    { // SPR_POWER
        .ms = bar_spr8_metasprites,
        .ms_n = ARR_LEN(bar_spr8_metasprites),
        .ti = bar_spr8_tiles,
        .pa = power_palettes,
        .pa_n = bar_spr8_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL6,
        .cnt = bar_spr8_TILE_COUNT,
        .off = SPR_HEALTH,
        .bank = BANK(bar_spr8),
    },
    { // SPR_EXPL
        .ms = expl_spr16_metasprites,
        .ms_n = ARR_LEN(expl_spr16_metasprites),
        .ti = expl_spr16_tiles,
        .pa = expl_spr16_palettes,
        .pa_n = expl_spr16_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL7 | PALETTE_DYNAMIC_LOAD_IP,
        .cnt = expl_spr16_TILE_COUNT,
        .off = TILE_NUM_START,
        .bank = BANK(expl_spr16),
    },
    { // SPR_PAUSE
        .ms = pause_metasprites,
        .ms_n = ARR_LEN(pause_metasprites),
        .ti = pause_tiles,
        .pa = pause_palettes,
        .pa_n = pause_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL7 | PALETTE_DYNAMIC_LOAD_IP,
        .cnt = pause_TILE_COUNT,
        .off = TILE_NUM_START,
        .bank = BANK(pause),
    },
    { // SPR_STATUS
        .ms = status_spr8_metasprites,
        .ms_n = ARR_LEN(status_spr8_metasprites),
        .ti = status_spr8_tiles,
        .pa = status_spr8_palettes,
        .pa_n = status_spr8_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL7 | PALETTE_DYNAMIC_LOAD_IP,
        .cnt = status_spr8_TILE_COUNT,
        .off = TILE_NUM_START,
        .bank = BANK(status_spr8),
    },
    { // SPR_DEBUG
        .ms = debug_marker_metasprites,
        .ms_n = ARR_LEN(debug_marker_metasprites),
        .ti = debug_marker_tiles,
        .pa = debug_marker_palettes,
        .pa_n = debug_marker_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL7 | PALETTE_DYNAMIC_LOAD_IP,
        .cnt = debug_marker_TILE_COUNT,
        .off = TILE_NUM_START,
        .bank = BANK(debug_marker),
    },
    { // SPR_DEBUG_LARGE
        .ms = debug_marker_spr32_metasprites,
        .ms_n = ARR_LEN(debug_marker_spr32_metasprites),
        .ti = debug_marker_spr32_tiles,
        .pa = debug_marker_spr32_palettes,
        .pa_n = debug_marker_spr32_PALETTE_COUNT,
        .pa_i = OAMF_CGB_PAL7 | PALETTE_DYNAMIC_LOAD_IP,
        .cnt = debug_marker_spr32_TILE_COUNT,
        .off = TILE_NUM_START,
        .bank = BANK(debug_marker_spr32),
    },
};
