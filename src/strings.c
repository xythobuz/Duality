/*
 * strings.c
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

#include <string.h>

#include "banks.h"
#include "strings.h"

BANKREF(strings)

static const char            string_top[] = "top";
static const char          string_score[] = "score";
static const char          string_black[] = "black";
static const char          string_white[] = "white";
static const char          string_enter[] = "enter";
static const char           string_name[] = "name";
static const char       string_start_ok[] = "start ok";
static const char      string_conf_menu[] = "Conf Menu";
static const char     string_debug_menu[] = "Debug Menu";
static const char        string_duality[] = "Duality";
static const char       string_xythobuz[] = "xythobuz";
static const char            string_git[] = "Git Commit Hash:";
static const char     string_build_date[] = "Build Date:";
static const char           string_date[] = __DATE__;
static const char           string_time[] = __TIME__;
static const char          string_mp_tx[] = "MP Tx:";
static const char           string_wait[] = "Wait ";
static const char          string_visit[] = "Visit:";
static const char            string_url[] = "https://xythobuz.de";
static const char       string_printout[] = "printout";
static const char        string_success[] = "success";
static const char          string_error[] = "error";
static const char     string_gb_printer[] = "GB Printer";
static const char string_score_printout[] = "Score Printout";
static const char         string_result[] = "Result:";
static const char   string_printf_error[] = "error: 0x%x";
static const char  string_printf_frames[] = "Frames: 0x%x";
static const char   string_printf_timer[] = " Timer: 0x%x";
static const char   string_printf_stack[] = " Stack: 0x%x";
static const char     string_printf_fps[] = "   FPS: %hd";

static const char * const strings[COUNT_STRINGS] = {
    string_top,            // STR_TOP
    string_score,          // STR_SCORE
    string_black,          // STR_BLACK
    string_white,          // STR_WHITE
    string_enter,          // STR_ENTER
    string_name,           // STR_NAME
    string_start_ok,       // STR_START_OK
    string_conf_menu,      // STR_CONF_MENU
    string_debug_menu,     // STR_DEBUG_MENU
    string_duality,        // STR_DUALITY
    string_xythobuz,       // STR_XYTHOBUZ
    string_git,            // STR_GIT
    string_build_date,     // STR_BUILD_DATE
    string_date,           // STR_DATE
    string_time,           // STR_TIME
    string_mp_tx,          // STR_MP_TX
    string_wait,           // STR_WAIT
    string_visit,          // STR_VISIT
    string_url,            // STR_URL
    string_printout,       // STR_PRINTOUT
    string_success,        // STR_SUCCESS
    string_error,          // STR_ERROR
    string_gb_printer,     // STR_GB_PRINTER
    string_score_printout, // STR_SCORE_PRINTOUT
    string_result,         // STR_RESULT
    string_printf_error,   // STR_PRINTF_ERROR
    string_printf_frames,  // STR_PRINTF_FRAMES
    string_printf_timer,   // STR_PRINTF_TIMER
    string_printf_stack,   // STR_PRINTF_STACK
    string_printf_fps,     // STR_PRINTF_FPS
};

#define MAX_STR_LEN 32
static char str_buff[MAX_STR_LEN + 1];

const char *get_string(enum STRINGS s) BANKED {
    strncpy(str_buff, strings[s], MAX_STR_LEN);
    return str_buff;
}
