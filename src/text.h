/*
 * text.h
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

#ifndef __TEXT_H__
#define __TEXT_H__

#include <gbdk/platform.h>

#define MAX_DIGITS 7
#define TEXT_LINE_WIDTH 10

// ----------------------------------------------------------------------------
// Characters 16x16 (for menus)
// ----------------------------------------------------------------------------

void str3(uint16_t name, uint8_t x_off, uint8_t y_off,
          uint8_t is_black_a, uint8_t is_black_b, uint8_t is_black_c) BANKED;
void str_l(const char *s, uint8_t len, uint8_t x_off, uint8_t y_off, uint8_t is_black) BANKED;
void str(const char *s, uint8_t x_off, uint8_t y_off, uint8_t is_black) BANKED;
void str_center(const char *s, uint8_t y_off, uint8_t is_black) BANKED;
void str_lines(const char *s, uint8_t y_off, uint8_t is_black) BANKED;

// ----------------------------------------------------------------------------
// Numbers 16x16 (for scores)
// ----------------------------------------------------------------------------

uint8_t number(int32_t score, uint8_t x_off, uint8_t y_off, uint8_t is_black) BANKED;

// ----------------------------------------------------------------------------
// GBC-only ASCII 8x8 font (for detailed / debug output)
// ----------------------------------------------------------------------------

void str_ascii_l(const char *s, uint8_t len, uint8_t x_off, uint8_t y_off, uint8_t light) BANKED;
void str_ascii(const char *s, uint8_t x_off, uint8_t y_off, uint8_t light) BANKED;
void str_ascii_lines(const char *s, uint8_t y_off, uint8_t light) BANKED;

BANKREF_EXTERN(text)

#endif // __TEXT_H__
