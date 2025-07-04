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
    PRN_STATUS_OK          = 0x0000, // everything is fine

    // status flags from printer
    PRN_STATUS_CHECKSUM    = 0x0001, // checksum error
    PRN_STATUS_BUSY        = 0x0002, // printer is printing
    PRN_STATUS_FULL        = 0x0004, // ready, triggered by DATA with len 0
    PRN_STATUS_UNTRAN      = 0x0008, // unprinted data in buffer
    PRN_STATUS_ER0         = 0x0010, // packet error
    PRN_STATUS_ER1         = 0x0020, // paper jam
    PRN_STATUS_ER2         = 0x0040, // unspecified error
    PRN_STATUS_LOWBAT      = 0x0080, // battery too low

    // status flags from driver code
    PRN_STATUS_CANCELLED   = 0x0100, // user has aborted the print by pressing B
    PRN_STATUS_TIMEOUT     = 0x0200, // timeout waiting for printer response
    PRN_STATUS_NO_MAGIC    = 0x0400, // printer did not respond with proper 'alive'

    // status flags for user code
    PRN_STATUS_AT_DETECT   = 0x0800, // set in gbprinter_detect
    PRN_STATUS_AT_DATA     = 0x1000, // set at abort on tile data
    PRN_STATUS_AT_BUSY     = 0x2000, // set at abort on busy wait print
    PRN_STATUS_AT_FINAL    = 0x4000, // set at abort on final wait

    // masks to check for errors
    PRN_STATUS_MASK_ERRORS = 0x07F0,
    PRN_STATUS_MASK_ANY    = 0x07FF,
};

#define PRN_PALETTE_NORMAL 0b11100100u
#define PRN_PALETTE_INV    0b00011011u
#define PRN_PALETTE_SC_W   0b00110100u
#define PRN_PALETTE_SC_B   0b00011100u

enum PRN_STATUS gbprinter_detect(void) BANKED;
enum PRN_STATUS gbprinter_screenshot(uint8_t win, uint8_t palette) BANKED;

uint8_t gbprinter_error(enum PRN_STATUS status, char *buff) BANKED;

BANKREF_EXTERN(gbprinter)
BANKREF_EXTERN(gbprinter_error)

#endif // __GBPRINTER_H_INCLUDE__
