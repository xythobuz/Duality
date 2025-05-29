/*
 * game.h
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

#ifndef __GAME_H__
#define __GAME_H__

#define SPEED_INC 1
#define SPEED_DEC 1

#define SPEED_MAX_ACC 23
#define SPEED_MAX_IDLE 16

#define POS_SCALE_OBJS 5
#define POS_SCALE_BG 6

#define POWER_MAX 0x1FF
#define POWER_SHIFT 1

#define POWER_INC 2
#define POWER_DEC 4

#define HEALTH_MAX 0x1FF
#define HEALTH_SHIFT 1

#define SHOT_SPEED 23

void game(void);

#endif // __GAME_H__
