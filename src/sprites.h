/*
 * sprites.h
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

#ifndef __SPRITES_H__
#define __SPRITES_H__

#include <stdint.h>

#define SHIP_OFF (8 + 4)

enum SPRITES {
    SPR_SHIP_0 = 0,
    SPR_SHIP_90,
    SPR_THRUST_0,
    SPR_THRUST_90,
    SPR_LIGHT,
    SPR_DARK,
    SPR_SHOT,

    SPRITE_COUNT
};

enum SPRITE_FLIP {
    FLIP_NONE = 0,
    FLIP_X,
    FLIP_Y,
    FLIP_XY
};

enum SPRITE_ROT {
    ROT_0 = 0,
    ROT_90,
    ROT_180,
    ROT_270,

    ROT_INVALID
};

void spr_init(void);
void spr_draw(enum SPRITES sprite, uint8_t *hiwater, enum SPRITE_FLIP flip, int8_t x_off, int8_t y_off);
void spr_ship(enum SPRITE_ROT rot, uint8_t moving, uint8_t *hiwater);

#endif // __SPRITES_H__
