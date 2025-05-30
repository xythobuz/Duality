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

#include "sprite_data.h"

void spr_init(void) NONBANKED {
    uint8_t off = TILE_NUM_START;
    for (uint8_t i = 0; i < SPRITE_COUNT; i++) {
        SWITCH_ROM(metasprites[i].bank);

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

void spr_draw(enum SPRITES sprite, enum SPRITE_FLIP flip,
              int8_t x_off, int8_t y_off, uint8_t frame,
              uint8_t *hiwater) NONBANKED {
    SWITCH_ROM(metasprites[sprite].bank);

    switch (flip) {
        case FLIP_Y:
            *hiwater += move_metasprite_flipy(
                    metasprites[sprite].ms[frame], metasprites[sprite].off,
                    metasprites[sprite].pa_i, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_XY:
            *hiwater += move_metasprite_flipxy(
                    metasprites[sprite].ms[frame], metasprites[sprite].off,
                    metasprites[sprite].pa_i, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_X:
            *hiwater += move_metasprite_flipx(
                    metasprites[sprite].ms[frame], metasprites[sprite].off,
                    metasprites[sprite].pa_i, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_NONE:
        default:
            *hiwater += move_metasprite_ex(
                    metasprites[sprite].ms[frame], metasprites[sprite].off,
                    metasprites[sprite].pa_i, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;
    }
}

void spr_ship(enum SPRITE_ROT rot, uint8_t moving, uint8_t *hiwater) NONBANKED {
    switch (rot) {
        case ROT_0:
            spr_draw(SPR_SHIP_0, FLIP_NONE, 0, 0, 0, hiwater);
            if (moving) {
                spr_draw(SPR_THRUST_0, FLIP_NONE, 0, SHIP_OFF, 0, hiwater);
            }
            break;

        case ROT_90:
            spr_draw(SPR_SHIP_90, FLIP_NONE, 0, 0, 0, hiwater);
            if (moving) {
                spr_draw(SPR_THRUST_90, FLIP_NONE, -SHIP_OFF, 0, 0, hiwater);
            }
            break;

        case ROT_180:
            spr_draw(SPR_SHIP_0, FLIP_Y, 0, 0, 0, hiwater);
            if (moving) {
                spr_draw(SPR_THRUST_0, FLIP_Y, 0, -SHIP_OFF, 0, hiwater);
            }
            break;

        case ROT_270:
            spr_draw(SPR_SHIP_90, FLIP_X, 0, 0, 0, hiwater);
            if (moving) {
                spr_draw(SPR_THRUST_90, FLIP_X, SHIP_OFF, 0, 0, hiwater);
            }
            break;

        default:
            break;
    }
}
