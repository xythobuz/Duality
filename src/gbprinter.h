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

#define PRN_STATUS_OK           0x00
#define PRINTER_DETECT_TIMEOUT  10

uint8_t gbprinter_detect(uint8_t delay) BANKED;
uint8_t gbprinter_print_image(const uint8_t *image_map, const uint8_t *image,
                              int8_t pos_x, uint8_t width, uint8_t height) BANKED;
uint8_t gbprinter_screenshot(void) BANKED;
uint8_t gbprinter_error(uint8_t status, char *buff);

BANKREF_EXTERN(gbprinter)
BANKREF_EXTERN(gbprinter_error)

#endif // __GBPRINTER_H_INCLUDE__
