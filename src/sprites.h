/*
 * sprites.h
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
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
