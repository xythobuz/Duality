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

#include "banks.h"
#include "sprite_data.h"

void spr_init(void) NONBANKED {
    uint8_t off = TILE_NUM_START;
    for (uint8_t i = 0; i < SPRITE_COUNT; i++) {
        START_ROM_BANK(metasprites[i].bank);
            if (metasprites[i].off == TILE_NUM_START) {
                metasprites[i].off = off;
                off += metasprites[i].cnt;
                set_sprite_data(metasprites[i].off, metasprites[i].cnt, metasprites[i].ti);
            } else {
                metasprites[i].off = metasprites[metasprites[i].off].off;
            }
        END_ROM_BANK();
    }
}

void spr_init_pal(void) NONBANKED {
    for (uint8_t i = 0; i < SPRITE_COUNT; i++) {
        uint8_t bank = metasprites[i].bank;
        if (metasprites[i].pa == power_palettes) {
            bank = BANK(power_palettes);
        }

        START_ROM_BANK(bank);
            if ((metasprites[i].pa != NULL) && ((metasprites[i].pa_i & PALETTE_ALL_FLAGS) == PALETTE_PRELOAD)) {
                set_sprite_palette(metasprites[i].pa_i, metasprites[i].pa_n, metasprites[i].pa);
            }
        END_ROM_BANK();
    }
}

void spr_draw(enum SPRITES sprite, enum SPRITE_FLIP flip,
              int8_t x_off, int8_t y_off, uint8_t frame,
              uint8_t *hiwater) NONBANKED {
    if (sprite >= SPRITE_COUNT) {
        return;
    }

    START_ROM_BANK(metasprites[sprite].bank);

    if (frame >= metasprites[sprite].ms_n) {
        frame = 0;
    }

    uint8_t pa_off = 0;

    if ((metasprites[sprite].pa_i & PALETTE_ALL_FLAGS) == PALETTE_DYNAMIC_LOAD) {
        uint8_t pa_i = frame;
        if (pa_i >= metasprites[sprite].pa_n) {
            pa_i = 0;
        }

        set_sprite_palette((metasprites[sprite].pa_i & PALETTE_NO_FLAGS) + pa_i, 1, metasprites[sprite].pa + (pa_i * 4));
    } else if ((metasprites[sprite].pa_i & PALETTE_ALL_FLAGS) == PALETTE_DYNAMIC_LOAD_IP) {
        pa_off = frame;
        if (pa_off >= metasprites[sprite].pa_n) {
            pa_off = 0;
        }

        set_sprite_palette((metasprites[sprite].pa_i & PALETTE_NO_FLAGS), 1, metasprites[sprite].pa + (pa_off * 4));
    }

    switch (flip) {
        case FLIP_Y:
            *hiwater += move_metasprite_flipy(
                    metasprites[sprite].ms[frame], metasprites[sprite].off,
                    (metasprites[sprite].pa_i - pa_off) & PALETTE_NO_FLAGS, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_XY:
            *hiwater += move_metasprite_flipxy(
                    metasprites[sprite].ms[frame], metasprites[sprite].off,
                    (metasprites[sprite].pa_i - pa_off) & PALETTE_NO_FLAGS, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_X:
            *hiwater += move_metasprite_flipx(
                    metasprites[sprite].ms[frame], metasprites[sprite].off,
                    (metasprites[sprite].pa_i - pa_off) & PALETTE_NO_FLAGS, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;

        case FLIP_NONE:
        default:
            *hiwater += move_metasprite_ex(
                    metasprites[sprite].ms[frame], metasprites[sprite].off,
                    (metasprites[sprite].pa_i - pa_off) & PALETTE_NO_FLAGS, *hiwater,
                    DEVICE_SPRITE_PX_OFFSET_X + (DEVICE_SCREEN_PX_WIDTH / 2) + x_off,
                    DEVICE_SPRITE_PX_OFFSET_Y + (DEVICE_SCREEN_PX_HEIGHT / 2) + y_off);
            break;
    }

    END_ROM_BANK();
}

void spr_ship(enum SPRITE_ROT rot, uint8_t moving, uint8_t *hiwater) NONBANKED {
    switch (rot) {
        case ROT_0:
            spr_draw(SPR_SHIP, FLIP_NONE, -1, 4, moving ? 1 : 0, hiwater);
            break;

        case ROT_45:
            spr_draw(SPR_SHIP, FLIP_NONE, -6, 0, moving ? 3 : 2, hiwater);
            break;

        case ROT_90:
            spr_draw(SPR_SHIP, FLIP_NONE, -4, -1, moving ? 5 : 4, hiwater);
            break;

        case ROT_135:
            spr_draw(SPR_SHIP, FLIP_Y, -6, 0, moving ? 3 : 2, hiwater);
            break;

        case ROT_180:
            spr_draw(SPR_SHIP, FLIP_Y, 0, -4, moving ? 1 : 0, hiwater);
            break;

        case ROT_225:
            spr_draw(SPR_SHIP, FLIP_XY, 6, 0, moving ? 3 : 2, hiwater);
            break;

        case ROT_270:
            spr_draw(SPR_SHIP, FLIP_X, 4, 0, moving ? 5 : 4, hiwater);
            break;

        case ROT_315:
            spr_draw(SPR_SHIP, FLIP_X, 6, 0, moving ? 3 : 2, hiwater);
            break;

        default:
            break;
    }
}
