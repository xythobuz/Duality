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

#define SPR_NUM_START 0
#define SHIP_OFF (8 + 4)

enum SPRITES {
    SPR_SHIP_0 = 0,
    SPR_SHIP_90,
    SPR_THRUST_0,
    SPR_THRUST_90,
    SPR_LIGHT,
    SPR_DARK,
    SPR_SHOT,
    SPR_SHOT_LIGHT,
    SPR_SHOT_DARK,
    SPR_HEALTH_1,
    SPR_HEALTH_2,
    SPR_HEALTH_3,
    SPR_HEALTH_4,
    SPR_HEALTH_5,
    SPR_HEALTH_6,
    SPR_HEALTH_7,
    SPR_HEALTH_8,
    SPR_POWER_1,
    SPR_POWER_2,
    SPR_POWER_3,
    SPR_POWER_4,
    SPR_POWER_5,
    SPR_POWER_6,
    SPR_POWER_7,
    SPR_POWER_8,

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
void spr_draw(enum SPRITES sprite, enum SPRITE_FLIP flip, int8_t x_off, int8_t y_off, uint8_t frame, uint8_t *hiwater);
void spr_ship(enum SPRITE_ROT rot, uint8_t moving, uint8_t *hiwater);

#endif // __SPRITES_H__
