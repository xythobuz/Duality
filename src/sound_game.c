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

const enum notes game_music[] = {
    C3, C3, G3, G3, A3, A3, G3, SILENCE,
    F3, F3, E3, E3, D3, D3, C3, SILENCE,
    G3, G3, F3, F3, E3, E3, D3, D3,
    G3, G3, F3, F3, E3, E3, D3, D3,
    C3, C3, G3, G3, A3, A3, G3, SILENCE,
    F3, F3, E3, E3, D3, D3, C3, SILENCE,
    SILENCE, SILENCE, END
};

const struct music music_game = {
    .notes = game_music,
    .notes2 = NULL,
    .drums = NULL,
    .duration = 200,
};
