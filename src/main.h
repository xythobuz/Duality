/*
 * main.h
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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <gbdk/platform.h>
#include <stdint.h>

#define ENTRY_NAME_LEN 8
#define CONF_ENTRY_COUNT 3
#define DEBUG_ENTRY_COUNT 13

enum HW_TYPE {
    HW_DMG = 0,
    HW_SGB,
    HW_GBC,

    HW_UNKNOWN,
    HW_ALL = HW_UNKNOWN,
};

struct conf_entry {
    char name[ENTRY_NAME_LEN + 1];
    uint8_t *var;
    uint8_t max;
    enum HW_TYPE type;
};

struct debug_entry {
    char name[ENTRY_NAME_LEN + 1];
    enum debug_flag flag;
    uint8_t max;
};

enum HW_TYPE get_hw(void) BANKED;

BANKREF_EXTERN(main)

extern const struct conf_entry conf_entries[CONF_ENTRY_COUNT];
extern const struct debug_entry debug_entries[DEBUG_ENTRY_COUNT];

extern uint8_t debug_menu_index;
extern uint8_t debug_special_value;

#endif // __MAIN_H__
