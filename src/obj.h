/*
 * obj.h
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

#ifndef __OBJ_H__
#define __OBJ_H__

#include <gbdk/platform.h>
#include <stdint.h>

#include "sprites.h"

/*
 * sprite budget:
 *
 * fixed:
 * status bars: 8
 * ship + thruster: 7
 * --> 15 fixed
 *
 * hardware tiles: 40 - 15 = 25
 *
 * dynamic:
 * shot / small: 1
 * expl: 4
 * light: 4
 * dark: 4
 * --> 2x dark & 2x light & 1x expl = 20
 * --> 2x shot & 4x small = 6
 * --> 20 + 6 = 26
 *
 * TODO we will sometimes have glitches
 * 1 sprite tile too much
 */
#define MAX_DARK 2
#define MAX_LIGHT 2
#define MAX_SHOT 2
#define MAX_SHOT_DARK 2
#define MAX_SHOT_LIGHT 2
#define MAX_OBJ ((4 * MAX_DARK) + (4 * MAX_LIGHT) + MAX_SHOT + MAX_SHOT_DARK + MAX_SHOT_LIGHT)

struct obj {
    uint8_t active;
    enum SPRITES sprite;
    int16_t off_x, off_y;
    int16_t spd_x, spd_y;
    uint8_t travel;
    uint8_t frame;
    uint8_t frame_index;
    uint8_t frame_count;
    uint8_t frame_duration;
};

struct obj_state {
    struct obj objs[MAX_OBJ];
    uint8_t obj_cnt[SPRITE_COUNT];
};

enum OBJ_STATE {
    OBJ_ADDED = 0,
    OBJ_LIST_FULL,
    OBJ_TYPE_FULL,
};

void obj_spawn(void) BANKED;

enum OBJ_STATE obj_add(enum SPRITES sprite,
                       int16_t off_x, int16_t off_y,
                       int16_t spd_x, int16_t spd_y) BANKED;

int16_t obj_do(int16_t *spd_off_x, int16_t *spd_off_y,
               int32_t *score, uint8_t *hiwater, uint8_t is_splash) BANKED;

extern struct obj_state obj_state;

#endif // __OBJ_H__
