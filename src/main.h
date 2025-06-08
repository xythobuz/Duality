/*
 * main.h
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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <gbdk/platform.h>
#include <stdint.h>

enum debug_flag {
    DBG_MENU = (1 << 0),
    DBG_MARKER = (1 << 1),
    DBG_GOD_MODE = (1 << 2),
    DBG_CLEAR_SCORE = (1 << 3),

    DBG_FLAG_COUNT = 2
};

#define DEBUG_ENTRY_NAME_LEN 8

struct debug_entry {
    char name[DEBUG_ENTRY_NAME_LEN + 1];
    enum debug_flag flag;
};

BANKREF_EXTERN(main)

extern enum debug_flag debug_flags;
extern uint8_t debug_menu_index;

#define DEBUG_ENTRY_COUNT 4

extern const struct debug_entry debug_entries[DEBUG_ENTRY_COUNT];

#endif // __MAIN_H__
