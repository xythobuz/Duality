/*
 * sound_over.c
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

#include <assert.h>

#include "banks.h"
#include "sound.h"
#include "sound_over.h"

BANKREF(sound_over)

static const enum notes over_notes[] = {
    // repeat 1

    Ab4,  C5, Ab4,  F4, Ab4, Ab4, Ab4,  C5,
    Ab4,  F4, Ab4, Bb4,  C5, Bb4, Ab4,  F4,

     A4,  C5,  A4,  F4,  A4,  A4,  A4,  C5,
     A4,  F4,  A4, Bb4,  C5, Bb4,  A4,  F4,

    Ab4,  C5, Ab4,  F4, Ab4, Ab4, Ab4,  C5,
    Ab4,  F4, Ab4, Bb4,  C5, Bb4, Ab4,  F4,

    Ab4,  C5, Ab4,  E4, Ab4, Ab4, Ab4,  C5,
    Ab4,  E4, Ab4, Bb4,  C5, Bb4, Ab4,  E4,

    Ab4,  C5, Ab4,  F4, Ab4, Ab4, Ab4,  C5,
    Ab4,  F4, Ab4, Bb4,  C5, Bb4, Ab4,  F4,

     A4,  C5,  A4,  F4,  A4,  A4,  A4,  C5,
     A4,  F4,  A4, Bb4,  C5, Bb4,  A4,  F4,

    Bb4, Db5, Bb4, Gb4, Bb4, Bb4, Bb4, Db5,
    Bb4, Gb4, Bb4,  C5, Db5,  C5, Bb4, Gb4,

    Bb4, Db5, Bb4, Gb4, Bb4, Bb4, Bb4, Db5,
    Bb4, Gb4, Bb4,  C5, Db5,  C5, Bb4, Gb4,

    Bb4,  D5, Bb4,  G4, Bb4, Bb4, Bb4, D5,
    Bb4,  G4, Bb4,  C5,  D5,  C5, Bb4, G4,

    Bb4,  D5, Bb4,  G4, Bb4, Bb4, Bb4, D5,
    Bb4,  G4, Bb4,  C5,  D5,  C5, Bb4, G4,

    // repeat 2

    Ab4,  C5, Ab4,  F4, Ab4, Ab4, Ab4,  C5,
    Ab4,  F4, Ab4, Bb4,  C5, Bb4, Ab4,  F4,

    A4,  C5,  A4,  F4,  A4,  A4,  A4,  C5,
    A4,  F4,  A4, Bb4,  C5, Bb4,  A4,  F4,

    Ab4,  C5, Ab4,  F4, Ab4, Ab4, Ab4,  C5,
    Ab4,  F4, Ab4, Bb4,  C5, Bb4, Ab4,  F4,

    Ab4,  C5, Ab4,  E4, Ab4, Ab4, Ab4,  C5,
    Ab4,  E4, Ab4, Bb4,  C5, Bb4, Ab4,  E4,

    Ab4,  C5, Ab4,  F4, Ab4, Ab4, Ab4,  C5,
    Ab4,  F4, Ab4, Bb4,  C5, Bb4, Ab4,  F4,

    A4,  C5,  A4,  F4,  A4,  A4,  A4,  C5,
    A4,  F4,  A4, Bb4,  C5, Bb4,  A4,  F4,

    Bb4, Db5, Bb4, Gb4, Bb4, Bb4, Bb4, Db5,
    Bb4, Gb4, Bb4,  C5, Db5,  C5, Bb4, Gb4,

    Bb4, Db5, Bb4, Gb4, Bb4, Bb4, Bb4, Db5,
    Bb4, Gb4, Bb4,  C5, Db5,  C5, Bb4, Gb4,

    Bb4,  D5, Bb4,  G4, Bb4, Bb4, Bb4, D5,
    Bb4,  G4, Bb4,  C5,  D5,  C5, Bb4, G4,

    Bb4, Dd5, Bb4,  G4, Bb4, Bb4, Bb4, D5,
    Bb4,  G4, Bb4,  C5,  D5,  C5, Bb4, G4,

    // end

    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    END
};

#define ADD_LOOP \
    Ab2, SIL, Ab2, SIL, Ab2, SIL, Ab2, SIL, \
    Ab2, SIL, Ab2, SIL, Ab2, SIL, Ab2, SIL

#define ADD_LOOP_2 \
     A2, SIL,  A2, SIL,  A2, SIL,  A2, SIL, \
     A2, SIL,  A2, SIL,  A2, SIL,  A2, SIL

#define ADD_LOOP_3 \
     E2, SIL,  E2, SIL,  E2, SIL,  E2, SIL, \
     E2, SIL,  E2, SIL,  E2, SIL,  E2, SIL

static const enum notes over_notes2[] = {
    // repeat 1

    ADD_LOOP, ADD_LOOP_2,
    ADD_LOOP, ADD_LOOP_3,
    ADD_LOOP, ADD_LOOP_2,

    Db3, Db3, Db3, Db3, Db3, Db3, Db3, Db3,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,

     D3,  D3,  D3,  D3,  D3,  D3,  D3,  D3,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,

    // repeat 2

    ADD_LOOP, ADD_LOOP_2,
    ADD_LOOP, ADD_LOOP_3,
    ADD_LOOP, ADD_LOOP_2,

    Db3, Db3, Db3, Db3, Db3, Db3, Db3, Db3,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,

    D3,  D3,  D3,  D3,  D3,  D3,  D3,  D3,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,

    // end

    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    END
};

#define DRUM_LOOP \
    dKi, dSI, dSI, dSI, dSI, dSI, dKi, dSI, \
    dSn, dSI, dSI, dSI, dKi, dSI, dSI, dSI  \

static const enum drums over_drums[] = {
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,

    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,
    DRUM_LOOP,

    dSI, dSI, dSI, dSI, dSI, dSI, dSI, dSI,
    dSI, dSI, dSI, dSI, dSI, dSI, dSI, dSI,
    dEND
};

static_assert(sizeof(over_notes)  == sizeof(over_notes2), "music loops need to be same length");
static_assert(sizeof(over_notes2) == sizeof(over_drums),  "music loops need to be same length");
static_assert(sizeof(over_notes)  == sizeof(over_drums),  "music loops need to be same length");

const struct music music_over = {
    .notes = over_notes,
    .notes2 = over_notes2,
    .drums = over_drums,
    .duration = 120,
    .repeat = MUSIC_NO_REPEAT,
};
