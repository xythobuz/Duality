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

#include "input.h"

static uint8_t joyp = 0;
static uint8_t old_joyp = 0;

void key_read(void) NONBANKED {
    old_joyp = joyp;
    joyp = joypad();
}

uint8_t key_down(uint8_t key) NONBANKED {
    return joyp & key;
}

uint8_t key_pressed(uint8_t key) NONBANKED {
    return (joyp ^ old_joyp) & joyp & key;
}
