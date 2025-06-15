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

#define ENTRY_NAME_LEN 8

struct conf_entry {
    char name[ENTRY_NAME_LEN + 1];
    uint8_t *var;
    uint8_t max;
};

struct debug_entry {
    char name[ENTRY_NAME_LEN + 1];
    enum debug_flag flag;
    uint8_t max;
};

BANKREF_EXTERN(main)

#define CONF_ENTRY_COUNT 1
extern const struct conf_entry conf_entries[CONF_ENTRY_COUNT];

extern uint8_t debug_menu_index;
extern uint8_t debug_special_value;

#define DEBUG_ENTRY_COUNT 9
extern const struct debug_entry debug_entries[DEBUG_ENTRY_COUNT];

#endif // __MAIN_H__
