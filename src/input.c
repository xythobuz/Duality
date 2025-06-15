/*
 * input.c
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

#include <gbdk/platform.h>

#include "banks.h"
#include "input.h"

static uint8_t joyp = 0;
static uint8_t old_joyp = 0;
static int8_t debug_cnt = 0;

BANKREF(input)

static const uint8_t key_debug_sequence[] = {
    J_UP, J_UP, J_DOWN, J_DOWN,
    J_LEFT, J_RIGHT, J_LEFT, J_RIGHT,
    J_B, J_A, /* J_START */
};

#define DEBUG_SEQUENCE_COUNT (sizeof(key_debug_sequence) / sizeof(key_debug_sequence[0]))

void key_read(void) NONBANKED {
    old_joyp = joyp;
    joyp = joypad();

    if (debug_cnt < DEBUG_SEQUENCE_COUNT) {
        START_ROM_BANK(BANK(input)) {
            if (key_pressed(key_debug_sequence[debug_cnt])) {
                debug_cnt++;
            } else if (key_pressed(0xFF)) {
                debug_cnt = 0;
            }
        } END_ROM_BANK
    } else {
        if (key_pressed(0xFF ^ J_START)) {
            debug_cnt = 0;
        }
    }
}

uint8_t key_down(uint8_t key) NONBANKED {
    return joyp & key;
}

uint8_t key_pressed(uint8_t key) NONBANKED {
    return (joyp ^ old_joyp) & joyp & key;
}

int8_t key_debug(void) NONBANKED {
    return DEBUG_SEQUENCE_COUNT - debug_cnt;
}
