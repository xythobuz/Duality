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
    C0 = 0, Cd0, D0, Dd0, E0, F0, Fd0, G0, Gd0, A0, Ad0, B0, //  0 .. 11
    C1,     Cd1, D1, Dd1, E1, F1, Fd1, G1, Gd1, A1, Ad1, B1, // 12 .. 23
    C2,     Cd2, D2, Dd2, E2, F2, Fd2, G2, Gd2, A2, Ad2, B2, // 24 .. 35
    C3,     Cd3, D3, Dd3, E3, F3, Fd3, G3, Gd3, A3, Ad3, B3, // 36 .. 47
    C4,     Cd4, D4, Dd4, E4, F4, Fd4, G4, Gd4, A4, Ad4, B4, // 48 .. 59
    C5,     Cd5, D5, Dd5, E5, F5, Fd5, G5, Gd5, A5, Ad5, B5, // 60 .. 71
    SILENCE, END, SIL = SILENCE,                             // 72 .. 73

              Db0 = Cd0, Eb0 = Dd0, Ed0 = F0,
    Fb0 = E0, Gb0 = Fd0, Ab0 = Gd0, Bb0 = Ad0,

    Cb1 = B0, Db1 = Cd1, Eb1 = Dd1, Ed1 = F1,
    Fb1 = E1, Gb1 = Fd1, Ab1 = Gd1, Bb1 = Ad1,

    Cb2 = B1, Db2 = Cd2, Eb2 = Dd2, Ed2 = F2,
    Fb2 = E2, Gb2 = Fd2, Ab2 = Gd2, Bb2 = Ad2,

    Cb3 = B2, Db3 = Cd3, Eb3 = Dd3, Ed3 = F3,
    Fb3 = E3, Gb3 = Fd3, Ab3 = Gd3, Bb3 = Ad3,

    Cb4 = B3, Db4 = Cd4, Eb4 = Dd4, Ed4 = F4,
    Fb4 = E4, Gb4 = Fd4, Ab4 = Gd4, Bb4 = Ad4,

    Cb5 = B4, Db5 = Cd5, Eb5 = Dd5, Ed5 = F5,
    Fb5 = E5, Gb5 = Fd5, Ab5 = Gd5, Bb5 = Ad5,
};

enum drums {
    dKick = 0, dSnare,
    dSilence,

    dKi = dKick, dSn = dSnare,
    dSI = dSilence,
    dEND,
};

#define MUSIC_NO_REPEAT 0xFFFF

struct music {
    enum notes * const notes;
    enum notes * const notes2;
    enum drums * const drums;
    uint16_t duration;
    uint16_t repeat;
};

enum SOUNDS {
    SND_MENU = 0,
    SND_GAME,
    SND_GAMEOVER,

    SND_COUNT
};

void snd_init(void) BANKED;
void snd_music_off(void) BANKED;
void snd_note_off(void) BANKED;
void snd_music(enum SOUNDS snd) BANKED;

void snd_play(void);

BANKREF_EXTERN(sound)

#endif // __SOUND_H__
