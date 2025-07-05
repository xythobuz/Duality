/*
 * sound_menu.c
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
#include "sound_menu.h"

BANKREF(sound_menu)

static const enum notes menu_music[] = {
    // intro

     G2,  G2,  D2,  D2,  F1,  F1,  G1,  G1, // 1
     G1,  G1, SIL, SIL, SIL, SIL, SIL,  G1, // 1
     G1,  G1,  G1,  G1, SIL, SIL, SIL, SIL, // 2
    SIL, SIL, Bb1, Bb1, Bb1, Bb1, Ab2, Ab2, // 2

     G2,  G2,  D2,  D2,  F1,  F1,  G1,  G1, // 3
     G1,  G1, SIL, SIL, SIL, SIL, SIL,  G1, // 3
     G1,  G1,  G1,  G1, SIL, SIL, SIL, SIL, // 4
    SIL, SIL, Bb1, Bb1, Bb1, Bb1, Ab2, Ab2, // 4

    // repeat 1

     G2,  G2,  G3,  G3,  G4,  G4, Eb4, Eb4, // 5
     D4, Eb4,  D4,  C4,  D4,  D4,  D4,  G1, // 5
     G1,  G1, Bb3, Bb3,  C4, Bb3,  C4,  C4, // 6
     D4,  D4,  G3,  G3,  C4,  C4, Bb3,  G3, // 6

     G2,  G2,  D2,  D2,  G4,  G4, Eb4, Eb4, // 7
     D4, Eb4,  D4,  C4,  D4,  D4,  D4,  G1, // 7
     G1,  G1, Bb3, Bb3,  C4, Bb3,  C4,  C4, // 8
     D4,  D4,  G3,  G3,  C4,  C4, Bb3,  G3, // 8

     G2,  G2,  G3,  G3,  G4,  G4, Eb4, Eb4, // 9
     D4, Eb4,  D4,  C4,  D4,  D4,  D4,  G1, // 9
     G1,  G1, Bb3, Bb3,  C4, Bb3,  C4,  C4, // 10
     D4,  D4,  G3,  G3,  C4,  C4, Bb3,  G3, // 10

     G2,  G2,  D2,  D2,  G4,  G4, Eb4, Eb4, // 11
     D4, Eb4,  D4,  C4,  D4,  D4,  D4,  G1, // 11
     G1,  G1, Bb3, Bb3,  C4, Bb3,  C4,  C4, // 12
     D4,  D4,  G3,  G3,  C4,  C4, Bb3,  G3, // 12

    // bridge

     G2,  G2,  D2,  D2,  F1,  F1,  G1,  G1, // 13
     G1,  G1, SIL, SIL, SIL, SIL, SIL,  G1, // 13
     G1,  G1,  G3,  D4,  D4,  G3, Db4, Db4, // 14
     G3,  C4, Bb1, Bb1, SIL, SIL,  G3,  G3, // 14

     G2,  G2,  D2,  D2,  F1,  F1,  G1,  G1, // 15
     G1,  G1, SIL, SIL, SIL, SIL, SIL,  G1, // 15
     G1,  G1,  G3,  D4,  D4,  G3, Db4, Db4, // 16
     G3,  C4, Bb1,  G3, Bb3, Bb3,  G3,  G3, // 16

    // repeat 2

     G2,  G2,  D2,  D2,  G4,  G4, Eb4, Eb4, // 17
     D4, Eb4,  D4,  C4,  D4,  D4,  D4,  G1, // 17
     G1,  G1, Bb3, Bb3,  C4, Bb3,  C4,  C4, // 18
     D4,  D4,  G3,  G3,  C4,  C4, Bb3,  G3, // 18

     G2,  G2,  D2,  D2,  G4,  G4, Eb4, Eb4, // 19
     D4, Eb4,  D4,  C4,  D4,  D4,  D4,  G1, // 19
     G1,  G1, Bb3, Bb3,  C4, Bb3,  C4,  C4, // 20
     D4,  D4,  G3,  G3,  C4,  C4, Bb3,  G3, // 20

    // bridge

     G2,  G2,  D2,  D2,  F1,  F1,  G1,  G1, // 21
     G1,  G1, SIL, SIL, SIL, SIL, SIL,  G1, // 21
     G1,  G1,  G3,  D4,  D4,  G3, Db4, Db4, // 22
     G3,  C4, Bb1, Bb1, SIL, SIL,  G3,  G3, // 22

     G2,  G2,  D2,  D2,  F1,  F1,  G1,  G1, // 23
     G1,  G1, SIL, SIL, SIL, SIL, SIL,  G1, // 23
     G1,  G1,  G3,  D4,  D4,  G3, Db4, Db4, // 24
     G3,  C4, Bb1,  G3, Bb3, Bb3,  G3,  G3, // 24

    // end

     G2,  G2,  G2,  G2, SIL, SIL,  G1,  G1, // 25
     G1,  G1,  G1,  G1,  G1,  G1,  G1,  G1, // 25
     G1,  G1,  G1,  G1, SIL, SIL, SIL, SIL, // 26

    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    END
};

static const enum notes menu_music2[] = {
    // intro

    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,

    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,

    // repeat 1

    SIL, SIL,  D2,  D2,  F1,  F1,  G1,  G1,
     G1,  G1, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, Bb1, Bb1, Bb1, Bb1, Ab2, SIL,

    SIL, SIL, SIL, SIL,  F1,  F1,  G1,  G1,
    G1,  G1, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, Bb1, Bb1, Bb1, Bb1, Ab2, SIL,

    SIL, SIL,  D2,  D2,  F1,  F1,  G1,  G1,
    G1,  G1, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, Bb1, Bb1, Bb1, Bb1, Ab2, SIL,

    SIL, SIL, SIL, SIL,  F1,  F1,  G1,  G1,
    G1,  G1, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, Bb1, Bb1, Bb1, Bb1, Ab2, SIL,

    // bridge

    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL,  G3, Bb3, Bb3, Ab2, Ab2,

    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, Ab2, Ab2,

    // repeat 2

    SIL, SIL, SIL, SIL,  F1,  F1,  G1,  G1,
    G1,  G1, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, Bb1, Bb1, Bb1, Bb1, Ab2, SIL,

    SIL, SIL, SIL, SIL,  F1,  F1,  G1,  G1,
    G1,  G1, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, Bb1, Bb1, Bb1, Bb1, Ab2, SIL,

    // bridge

    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL,  G3, Bb3, Bb3, Ab2, Ab2,

    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, Ab2, Ab2,

    // end

    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,
    SIL, SIL, SIL, SIL, SIL, SIL, SIL, SIL,

    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    SILENCE, SILENCE, SILENCE, SILENCE,
    END
};

#define DRUM_LOOP \
    dSI, dSI, dSn, dSI, dKi, dSI, dSI, dSI, \
    dSn, dSI, dSI, dSI, dKi, dSI, dSI, dSI  \

static const enum drums menu_drums[] = {
    // intro

    DRUM_LOOP,
    DRUM_LOOP,

    DRUM_LOOP,
    DRUM_LOOP,

    // repeat 1

    DRUM_LOOP,
    DRUM_LOOP,

    DRUM_LOOP,
    DRUM_LOOP,

    DRUM_LOOP,
    DRUM_LOOP,

    DRUM_LOOP,
    DRUM_LOOP,

    // bridge

    DRUM_LOOP,
    DRUM_LOOP,

    DRUM_LOOP,
    DRUM_LOOP,

    // repeat 2

    DRUM_LOOP,
    DRUM_LOOP,

    DRUM_LOOP,
    DRUM_LOOP,

    // bridge

    DRUM_LOOP,
    DRUM_LOOP,

    DRUM_LOOP,
    DRUM_LOOP,

    // end

    dSI, dSI, dSI, dSI, dSI, dSI, dSI, dSI,
    dSI, dSI, dSI, dSI, dSI, dSI, dSI, dSI,
    dSI, dSI, dSI, dSI, dSI, dSI, dSI, dSI,

    dSI, dSI, dSI, dSI, dSI, dSI, dSI, dSI,
    dSI, dSI, dSI, dSI, dSI, dSI, dSI, dSI,
    dEND
};

const struct music music_menu = {
    .notes = menu_music,
    .notes2 = menu_music2,
    .drums = menu_drums,
    .duration = 150,
    .repeat = 0,
};
