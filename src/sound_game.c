/*
 * sound_game.c
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * Based on examples from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/sound/sound.c
 *
 * And the docs for the DMG APU:
 * https://gbdev.io/pandocs/Audio_Registers.html
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

#include "banks.h"
#include "sound_game.h"

BANKREF(sound_game)

#define NOTE_LOOP \
    F1,  F1,  F1,  G1, Gd2, Gd1, Cd1,  D1, \
    D2,  D2,  D2, Fd1, Fd1,  D2,  D2,  D2

// TODO
#define DIFF_LOOP \
    F1,  F1,  F1,  G1, Gd2, Gd1, Cd1,  D1, \
    D2,  D2,  D2, Fd1, Fd1,  D2,  D2,  D2

static const enum notes game_music[] = {
    NOTE_LOOP, NOTE_LOOP, NOTE_LOOP, NOTE_LOOP,
    NOTE_LOOP, NOTE_LOOP, NOTE_LOOP, NOTE_LOOP,

    NOTE_LOOP, NOTE_LOOP, NOTE_LOOP, NOTE_LOOP,

    DIFF_LOOP, DIFF_LOOP, DIFF_LOOP, DIFF_LOOP,

    NOTE_LOOP, NOTE_LOOP, NOTE_LOOP, NOTE_LOOP,

    END
};

#define DRUM_LOOP \
dSn, dSI, dSn, dSI, dKi, dSI, dSI, dSI, \
dSn, dSI, dSn, dSI, dKi, dSI, dSI, dSI  \

#define SILE_LOOP \
dSI, dSI, dSI, dSI, dSI, dSI, dSI, dSI, \
dSI, dSI, dSI, dSI, dSI, dSI, dSI, dSI  \

static const enum drums game_drums[] = {
    DRUM_LOOP, DRUM_LOOP, DRUM_LOOP, DRUM_LOOP,
    DRUM_LOOP, DRUM_LOOP, DRUM_LOOP, DRUM_LOOP,

    SILE_LOOP, SILE_LOOP, SILE_LOOP, SILE_LOOP,

    DRUM_LOOP, DRUM_LOOP, DRUM_LOOP, DRUM_LOOP,
    DRUM_LOOP, DRUM_LOOP, DRUM_LOOP, DRUM_LOOP,

    dEND
};

const struct music music_game = {
    .notes = game_music,
    .notes2 = NULL,
    .drums = game_drums,
    .duration = 160,
    .repeat = 0,
};
