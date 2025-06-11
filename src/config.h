/*
 * config.h
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <gbdk/platform.h>
#include <gbdk/emu_debug.h>
#include <stdint.h>

#include "score.h"

enum debug_flag {
    DBG_NONE = 0,

    DBG_MENU = (1 << 0),
    DBG_MARKER = (1 << 1),
    DBG_GOD_MODE = (1 << 2),
    DBG_CLEAR_SCORE = (1 << 3),
    DBG_ZERO_SCORE = (1 << 4),
};

struct config {
    enum debug_flag debug_flags;
    uint8_t sfx_vol;
    uint8_t music_vol;
};

void conf_init(void) BANKED;
void conf_write_crc(void) BANKED;
struct scores *conf_scores(void) BANKED;
struct config *conf_get(void) BANKED;

BANKREF_EXTERN(config)

#endif // __CONFIG_H__
