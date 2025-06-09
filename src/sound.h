/*
 * sound.h
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

#ifndef __SOUND_H__
#define __SOUND_H__

#include <gbdk/platform.h>

enum notes {
    C0, Cd0, D0, Dd0, E0, F0, Fd0, G0, Gd0, A0, Ad0, B0, //  0 .. 11
    C1, Cd1, D1, Dd1, E1, F1, Fd1, G1, Gd1, A1, Ad1, B1, // 12 .. 23
    C2, Cd2, D2, Dd2, E2, F2, Fd2, G2, Gd2, A2, Ad2, B2, // 24 .. 35
    C3, Cd3, D3, Dd3, E3, F3, Fd3, G3, Gd3, A3, Ad3, B3, // 36 .. 47
    C4, Cd4, D4, Dd4, E4, F4, Fd4, G4, Gd4, A4, Ad4, B4, // 48 .. 59
    C5, Cd5, D5, Dd5, E5, F5, Fd5, G5, Gd5, A5, Ad5, B5, // 60 .. 71
    SILENCE, END                                         // 72 .. 73
};

struct music {
    enum notes * const notes;
    uint16_t duration;
};

void snd_init(void) BANKED;

void snd_music_off(void) BANKED;
void snd_menu_music(void) BANKED;
void snd_game_music(void) BANKED;
void snd_gameover_music(void) BANKED;
void snd_play(void);

void snd_shot(void) BANKED;
void snd_explode(void) BANKED;

BANKREF_EXTERN(sound)

#endif // __SOUND_H__
