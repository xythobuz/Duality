/*
 * maps.h
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

#ifndef __MAPS_H__
#define __MAPS_H__

#include <stdint.h>
#include "score.h"

void map_title(void);
void map_game(void);

void win_init(uint8_t is_splash);
void win_splash_draw(int32_t lowest, int32_t highest);
void win_score_clear(uint8_t is_black);
void win_score_draw(struct scores score, uint8_t off, uint8_t is_black);
void win_about(void);
void win_conf(void);
void win_debug(void);
void win_name(int32_t score);
void win_name_draw(uint16_t name, uint8_t is_black, uint8_t pos);
uint8_t win_game_draw(int32_t score);

BANKREF_EXTERN(maps)

#endif // __MAPS_H__
