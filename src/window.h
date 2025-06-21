/*
 * window.h
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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <gbdk/platform.h>
#include <stdint.h>

#include "score.h"
#include "gbprinter.h"

void win_init(uint8_t is_splash);
void win_splash_draw(int32_t lowest, int32_t highest) BANKED;
void win_splash_mp(void) BANKED;
void win_score_clear(uint8_t is_black, uint8_t no_bg) BANKED;
void win_score_draw(struct scores score, uint8_t off, uint8_t is_black) BANKED;
void win_score_print(enum PRN_STATUS status) BANKED;
void win_about(void) BANKED;
void win_about_mp(void) BANKED;
void win_conf(void) BANKED;
void win_debug(void) BANKED;
void win_name(int32_t score) BANKED;
void win_name_draw(uint16_t name, uint8_t is_black, uint8_t pos) BANKED;
uint8_t win_game_draw(int32_t score) BANKED;

void win_str_center(const char *s, uint8_t y_off, uint8_t is_black);

BANKREF_EXTERN(window)

#endif // __WINDOW_H__
