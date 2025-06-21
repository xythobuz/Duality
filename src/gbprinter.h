/*
 * gbprinter.h
 * Duality
 *
 * Based on the gbprinter example from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/tree/develop/gbdk-lib/examples/gb/gbprinter
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

#ifndef __GBPRINTER_H_INCLUDE__
#define __GBPRINTER_H_INCLUDE__

#include <gbdk/platform.h>
#include <stdint.h>

enum PRN_STATUS {
    PRN_STATUS_LOWBAT      = 0x80, // battery too low
    PRN_STATUS_ER2         = 0x40, // unspecified error
    PRN_STATUS_ER1         = 0x20, // paper jam
    PRN_STATUS_ER0         = 0x10, // packet error
    PRN_STATUS_UNTRAN      = 0x08, // unprinted data in buffer
    PRN_STATUS_FULL        = 0x04, // ready, triggered by DATA with len 0
    PRN_STATUS_BUSY        = 0x02, // printer is printing
    PRN_STATUS_CHECKSUM    = 0x01, // Checksum error
    PRN_STATUS_OK          = 0x00,

    PRN_STATUS_CANCELLED   = 0x100,
    PRN_STATUS_TIMEOUT     = 0x200,
    PRN_STATUS_NO_MAGIC    = 0x400,

    PRN_STATUS_MASK_ERRORS = 0x7F0,
    PRN_STATUS_MASK_ANY    = 0x7FF,
};

#define PRN_PALETTE_NORMAL 0b11100100u
#define PRN_PALETTE_INV    0b00011011u
#define PRN_PALETTE_SC_W   0b00110100u
#define PRN_PALETTE_SC_B   0b00011100u

enum PRN_STATUS gbprinter_detect(void) BANKED;
enum PRN_STATUS gbprinter_screenshot(uint8_t win, uint8_t palette) BANKED;

uint8_t gbprinter_error(enum PRN_STATUS status, char *buff);

BANKREF_EXTERN(gbprinter)
BANKREF_EXTERN(gbprinter_error)

#endif // __GBPRINTER_H_INCLUDE__
