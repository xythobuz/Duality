/*
 * config.ba0.c
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

#include <stddef.h>
#undef NULL

#include "banks.h"
#include "score.h"
#include "sample.h"
#include "sound.h"
#include "config.h"

struct config_mem mem;

BANKREF(config)

static uint32_t calc_crc(void) {
    const uint8_t *d = (const uint8_t *)mem;

    uint32_t c = 0xFFFFFFFF;
    for (size_t i = 0; i < offsetof(struct config_mem, crc); i++) {

        // adapted from "Hacker's Delight"
        c ^= d[i];
        for (size_t j = 0; j < 8; j++) {
            uint32_t mask = -(c & 1);
            c = (c >> 1) ^ (0xEDB88320 & mask);
        }
    }

    return ~c;
}

void conf_init(void) BANKED {
    ENABLE_RAM;
    SWITCH_RAM(0);

    if (calc_crc() != mem.crc) {
        mem.config.debug_flags = 0;
        //mem.config.sfx_vol = 0x03;
        mem.config.music_vol = 0x07;

        if (_cpu == CGB_TYPE) {
            mem.config.game_bg = 0;
        } else {
            mem.config.game_bg = 1;
        }

        mem.config.dmg_bg_inv = 1;

        score_reset();

        mem.state.in_progress = 0;
    }
}

void conf_write_crc(void) BANKED {
    mem.crc = calc_crc();
}
