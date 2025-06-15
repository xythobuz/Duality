/*
 * multiplayer.h
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

#ifndef __MULTIPLAYER_H__
#define __MULTIPLAYER_H__

#include <gbdk/platform.h>
#include <stdint.h>

uint8_t mp_master_ready(void) BANKED;
void mp_master_start(void) BANKED;
uint8_t mp_slave_ready(void) BANKED;
void mp_slave_start(void) BANKED;

extern uint8_t mp_connection_status;

#endif // __MULTIPLAYER_H__
