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

#define OBJ_ADDED 0
#define OBJ_LIST_FULL -1

void obj_init(void);
int8_t obj_add(enum SPRITES sprite, int16_t off_x, int16_t off_y, int16_t spd_x, int16_t spd_y);
void obj_draw(int16_t spd_x, int16_t spd_y, uint8_t *hiwater);

#endif // __OBJ_H__
