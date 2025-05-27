/*
 * obj.c
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

#include <gbdk/platform.h>
#include <string.h>
#include <stdlib.h>

#include "obj.h"
#include "sprites.h"

#define MAX_OBJ 10
#define MAX_TRAVEL 250

struct obj {
    uint8_t active;
    enum SPRITES sprite;
    int16_t off_x;
    int16_t off_y;
    int16_t spd_x;
    int16_t spd_y;
    uint8_t travel;
};

static struct obj objs[MAX_OBJ];

void obj_init(void) {
    memset(objs, 0, sizeof(objs));
}

int8_t obj_add(enum SPRITES sprite, int16_t off_x, int16_t off_y, int16_t spd_x, int16_t spd_y) {
    uint8_t obj_cnt = 0xFF;
    for (uint8_t i = 0; i < MAX_OBJ; i++) {
        if (!objs[i].active) {
            obj_cnt = i;
            break;
        }
    }
    if (obj_cnt >= MAX_OBJ) {
        return OBJ_LIST_FULL;
    }

    objs[obj_cnt].active = 1;
    objs[obj_cnt].sprite = sprite;
    objs[obj_cnt].off_x = off_x << 4;
    objs[obj_cnt].off_y = off_y << 4;
    objs[obj_cnt].spd_x = spd_x;
    objs[obj_cnt].spd_y = spd_y;
    objs[obj_cnt].travel = 0;

    obj_cnt += 1;
    return OBJ_ADDED;
}

void obj_draw(int16_t spd_x, int16_t spd_y, uint8_t *hiwater) {
    for (uint8_t i = 0; i < MAX_OBJ; i++) {
        if (!objs[i].active) {
            continue;
        }

        spr_draw(objs[i].sprite, FLIP_NONE, objs[i].off_x >> 4, objs[i].off_y >> 4, hiwater);

        // move objects by their speed and compensate for movement of the background / ship
        objs[i].off_x += objs[i].spd_x - spd_x;
        objs[i].off_y += objs[i].spd_y - spd_y;

        // only update travel time if we're actually moving
        if ((objs[i].spd_x != 0) || (objs[i].spd_y != 0)) {
            objs[i].travel += 1;
        }

        // remove objects that have traveled for too long
        if (objs[i].travel >= MAX_TRAVEL) {
            objs[i].active = 0;
        }
    }
}
