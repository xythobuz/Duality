/*
 * gbprinter_error.c
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

#include <gbdk/platform.h>
#include <stdio.h>
#include <string.h>

#include "banks.h"
#include "gbprinter.h"

BANKREF(gbprinter_error)

#define ERROR_BITS 11

static const char str_lowbat[] = "battery too low";
static const char str_er2[] = "unknown error";
static const char str_er1[] = "paper jam";
static const char str_er0[] = "packet error";
static const char str_untran[] = "unprocessed";
static const char str_full[] = "data full";
static const char str_busy[] = "printer busy";
static const char str_sum[] = "checksum error";
static const char str_cancel[] = "cancelled";
static const char str_timeout[] = "timeout";
static const char str_magic[] = "wrong magic byte";

static const char * const error_strings[ERROR_BITS] = {
    str_sum, str_busy, str_full, str_untran,
    str_er0, str_er1, str_er2, str_lowbat,
    str_cancel, str_timeout, str_magic,
};

uint8_t gbprinter_error(enum PRN_STATUS status, char *buff) NONBANKED {
    if (status == PRN_STATUS_OK) {
        sprintf(buff, "ok");
        return 2;
    }

    uint8_t n = 0;
    START_ROM_BANK(BANK(gbprinter_error)) {
        for (uint8_t i = 0; i < ERROR_BITS; i++) {
            if (status & (1 << i)) {
                if (n != 0) {
                    buff[n++] = '\n';
                }
                strcpy(buff + n, error_strings[i]);
                n += strlen(error_strings[i]);
            }
        }
    } END_ROM_BANK
    return n;
}
