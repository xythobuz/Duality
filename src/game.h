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

#include <gbdk/platform.h>
#include <stdint.h>
#include "multiplayer.h"

#define HEALTH_MAX 0x1FF
#define HEALTH_SHIFT 1

#define SHOT_SPEED 42 //23
#define MAX_TRAVEL 64 //128

#define SHOT_SPEED_DIAG 30
#define SHOT_SPEED_D_HI 39
#define SHOT_SPEED_D_LO 16

enum GAME_MODE {
    GM_SINGLE = 0,
    GM_MULTI,
};

void game_get_mp_state(void) BANKED;
void game_set_mp_player2(struct mp_player_state *state) BANKED;
void game_set_mp_shot(struct mp_shot_state *state) BANKED;

uint8_t game_get_fps(void) BANKED;
uint16_t game_get_framecount(void) BANKED;
int32_t game(enum GAME_MODE mode) BANKED;

BANKREF_EXTERN(game)

#endif // __GAME_H__
