/*
 * main.c
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * Based on examples from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/rand/rand.c
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
#include <gbdk/metasprites.h>
#include <rand.h>
#include <stdint.h>

#include "maps.h"
#include "obj.h"
#include "sprites.h"
#include "sound.h"
#include "input.h"
#include "game.h"

static void splash(void) {
    disable_interrupts();
    DISPLAY_OFF;
    map_title();
    SHOW_BKG;
    SHOW_SPRITES;
    SPRITES_8x8;
    DISPLAY_ON;
    enable_interrupts();

    obj_init();
    obj_add(SPR_LIGHT, 42, -42, 0, 0);
    obj_add(SPR_DARK, -42, -42, 0, 0);

    while(1) {
        key_read();
        if (key_down(0xFF)) {
            break;
        }

        uint8_t hiwater = SPR_NUM_START;
        obj_draw(0, 0, &hiwater);
        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        vsync();
    }
}

void main(void) {
    spr_init();
    snd_init();

#ifndef DEBUG
    splash();
#endif // DEBUG

    uint16_t seed = DIV_REG;
    waitpadup();
    seed |= ((uint16_t)DIV_REG) << 8;
    initarand(seed);

    game();
}
