/*
 * strings.h
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

#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <gbdk/platform.h>

enum STRINGS {
    STR_TOP = 0,
    STR_SCORE,
    STR_BLACK,
    STR_WHITE,
    STR_ENTER,
    STR_NAME,
    STR_START_OK,
    STR_CONF_MENU,
    STR_DEBUG_MENU,
    STR_DUALITY,
    STR_XYTHOBUZ,
    STR_GIT,
    STR_BUILD_DATE,
    STR_DATE,
    STR_TIME,
    STR_MP_TX,
    STR_WAIT,
    STR_VISIT,
    STR_URL,
    STR_PRINTOUT,
    STR_SUCCESS,
    STR_ERROR,
    STR_GB_PRINTER,
    STR_SCORE_PRINTOUT,
    STR_RESULT,
    STR_PRINTF_ERROR,
    STR_PRINTF_FRAMES,
    STR_PRINTF_TIMER,
    STR_PRINTF_STACK,
    STR_PRINTF_FPS,
    STR_SPINNER,
    STR_GAME_IN,
    STR_PROGRESS,
    STR_A_CONTINUE,
    STR_B_NEW_GAME,

    COUNT_STRINGS
};

#define SPINNER_LENGTH 4

const char *get_string(enum STRINGS s) BANKED;

BANKREF_EXTERN(strings)

#endif // __STRINGS_H__
