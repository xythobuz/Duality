/*
 * sound.c
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * Based on examples from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/sound/sound.c
 * https://github.com/gbdk-2020/gbdk-2020/tree/develop/gbdk-lib/examples/gb/wav_sample
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
#include "timer.h"
#include "sound_menu.h"
#include "sound_game.h"
#include "sound_over.h"
#include "sound.h"

BANKREF(sound)

const uint16_t frequencies[SILENCE] = {
      44,  156,  262,  363,  457,  547,  631,  710,  786,  854,  923,  986, //  0 .. 11
    1046, 1102, 1155, 1205, 1253, 1297, 1339, 1379, 1417, 1452, 1486, 1517, // 12 .. 23
    1546, 1575, 1602, 1627, 1650, 1673, 1694, 1714, 1732, 1750, 1767, 1783, // 24 .. 35
    1798, 1812, 1825, 1837, 1849, 1860, 1871, 1881, 1890, 1899, 1907, 1915, // 36 .. 47
    1923, 1930, 1936, 1943, 1949, 1954, 1959, 1964, 1969, 1974, 1978, 1982, // 48 .. 59
    1985, 1988, 1992, 1995, 1998, 2001, 2004, 2006, 2009, 2011, 2013, 2015  // 60 .. 71
};

static struct music const * music = NULL;
static uint8_t bank;
static uint16_t off = 0;
static uint16_t last_t = 0;

static void play_note(enum notes note) NONBANKED {
    if (note < SILENCE) {
        START_ROM_BANK(BANK(sound));
            uint16_t freq = frequencies[note];
        END_ROM_BANK();

        NR11_REG = 0x80 | 0x3F; // 50% duty, shortest initial length
        NR12_REG = 0x70; // half volume, no change
        NR13_REG = freq & 0xFF; // given frequency
        NR14_REG = 0x80 | ((freq >> 8) & 0x07); // trigger, upper freq bits
    } else {
        NR11_REG = 0x80 | 0x3F; // 50% duty, shortest initial length
        NR12_REG = 0x10; // 'lowest' volume without pop, no change
        NR13_REG = 0x00; // lowest frequency
        NR14_REG = 0x80 | 0x40 | 0x00; // trigger, enable length, upper freq bits
    }
}

static void play_drum(enum drums drum) NONBANKED {
    switch (drum) {
        case dKick:
            NR41_REG = 0x2F; // length timer, higher value is shorter time (up to 0x3F)
            NR42_REG = 0xF0; // initially full volume, no volume changes over time
            NR43_REG = 0x11; // frequency distribution
            NR44_REG = 0xC0; // trigger and enable length
            break;

        case dSnare:
            NR41_REG = 0x00; // length timer, higher value is shorter time (up to 0x3F)
            NR42_REG = 0xF1; // initially full volume, then fade sound out
            NR43_REG = 0x46; // frequency distribution
            NR44_REG = 0xC0; // trigger and enable length
            break;

        default:
            break;
    }
}

void snd_init(void) BANKED {
    NR52_REG = 0x80; // sound on
    NR51_REG = 0xFF; // all channels on left and right

#ifdef DEBUG
    NR50_REG = 0x33; // left and right on half volume
#else
    NR50_REG = 0x77; // left and right on full volume
#endif
}

void snd_music_off(void) BANKED {
    play_note(SILENCE);
}

static void play_current_note(void) NONBANKED {
    if (!music) {
        return;
    }

    START_ROM_BANK(bank);
    if (music->notes) {
        if (music->notes[off] != END) {
            play_note(music->notes[off]);
        }
    }
    if (music->drums) {
        if (music->drums[off] != dEND) {
            play_drum(music->drums[off]);
        }
    }
    END_ROM_BANK();
}

void snd_menu_music(void) BANKED {
    music = &music_menu;
    bank = BANK(sound_menu);
    off = 0;
    last_t = timer_get();
    play_current_note();
}

void snd_game_music(void) BANKED {
    music = &music_game;
    bank = BANK(sound_game);
    off = 0;
    last_t = timer_get();
    play_current_note();
}

void snd_gameover_music(void) BANKED {
    music = &music_over;
    bank = BANK(sound_over);
    off = 0;
    last_t = timer_get();
    play_current_note();
}

void snd_play(void) NONBANKED {
    if (!music) {
        return;
    }

    START_ROM_BANK(bank);
        uint16_t diff = timer_get() - last_t;
        if (diff >= music->duration) {
            off++;
            last_t += music->duration;

            if (music->notes) {
                if (music->notes[off] != END) {
                    play_note(music->notes[off]);
                } else {
                    off = 0xFFFF;
                }
            }

            if (music->drums) {
                if (music->drums[off] != dEND) {
                    play_drum(music->drums[off]);
                } else {
                    off = 0xFFFF;
                }
            }
        }
    END_ROM_BANK();
}
