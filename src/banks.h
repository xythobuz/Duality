/*
 * banks.h
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

#ifndef __BANKS_H__
#define __BANKS_H__

#include <gbdk/platform.h>

#define START_ROM_BANK_2(x) __xyz_previous__bank = CURRENT_BANK; SWITCH_ROM(x); do
#define START_ROM_BANK(x) uint8_t START_ROM_BANK_2(x)
#define END_ROM_BANK while (0); SWITCH_ROM(__xyz_previous__bank);

#endif // __BANKS_H__
