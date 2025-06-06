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

#include <stdint.h>
#include "sprites.h"

enum OBJ_STATE {
    OBJ_ADDED = 0,
    OBJ_LIST_FULL,
    OBJ_TYPE_FULL,
};

void obj_init(void);
enum OBJ_STATE obj_add(enum SPRITES sprite, int16_t off_x, int16_t off_y, int16_t spd_x, int16_t spd_y);
int16_t obj_act(int16_t *spd_off_x, int16_t *spd_off_y, int32_t *score);
void obj_draw(uint8_t *hiwater);

#endif // __OBJ_H__
