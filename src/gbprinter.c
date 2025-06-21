/*
 * gbprinter.c
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
#include <gbdk/emu_debug.h>
#include <stdio.h>
#include <string.h>

#include "input.h"
#include "window.h"
#include "gbprinter.h"

BANKREF(gbprinter)

#define PRN_TILE_WIDTH     20 // Width of the printed image in tiles
#define PRN_MAGIC          0x3388
#define PRN_MAGIC_DETECT   0x81 // magic reply from printer
#define PRN_DETECT_TIMEOUT 10 // 1/6th second
#define PRN_BUSY_TIMEOUT   (2 * 60) // 2s
#define PRN_PRINT_TIMEOUT  (20 * 60) // 20s

#define PRN_PALETTE_NORMAL 0b11100100u
#define PRN_PALETTE_INV    0b00011011u

#define PRN_NO_MARGINS     0x00
#define PRN_FINAL_MARGIN   0x03

#define PRN_EXPO_LIGHT     0x00
#define PRN_EXPO_DEFAULT   0x40
#define PRN_EXPO_DARK      0x7F

enum PRN_CMDS {
    PRN_CMD_INIT   = 0x01, // initialize printer
    PRN_CMD_PRINT  = 0x02, // print command
    PRN_CMD_DATA   = 0x04, // data command (up to 640 bytes)
    PRN_CMD_BREAK  = 0x08,
    PRN_CMD_STATUS = 0x0F, // query status
};

struct prn_header {
    uint16_t magic;
    uint8_t command;
    uint8_t compression;
    uint16_t length;
};

struct prn_config {
    uint8_t sheets;
    uint8_t margins;
    uint8_t palette;
    uint8_t exposure;
};

struct prn_footer {
    uint16_t crc;
    uint8_t alive;
    uint8_t status;
};

static uint8_t prn_send_receive(uint8_t b) {
    SB_REG = b;
    SC_REG = SIOF_XFER_START | SIOF_CLOCK_INT;
    while (SC_REG & SIOF_XFER_START);
    return SB_REG;
}

static void prn_send_block(uint8_t *data, uint16_t length, uint16_t *crc) {
    while (length-- > 0) {
        uint8_t v = *data;
        *crc += v;
        *data = prn_send_receive(v);
        data++;
    }
}

static enum PRN_STATUS printer_send_command(enum PRN_CMDS cmd,
                                            uint8_t *data, uint16_t length) {
    static struct prn_header header;
    static struct prn_footer footer;
    uint16_t crc = 0;

    header.magic = PRN_MAGIC;
    header.command = cmd;
    header.compression = 0;
    header.length = data ? length : 0;

    footer.crc = 0;
    footer.alive = 0;
    footer.status = 0;

    prn_send_block((uint8_t *)&header, sizeof(struct prn_header), &crc);
    if (data && (length > 0)) {
        prn_send_block(data, length, &crc);
    }

    footer.crc = crc;
    prn_send_block((uint8_t *)&footer, sizeof(struct prn_footer), &crc);

    enum PRN_STATUS r = footer.status;
    if (footer.alive != PRN_MAGIC_DETECT) {
        r |= PRN_STATUS_NO_MAGIC;
    }
    return r;
}

static uint8_t printer_check_cancel(void) {
    key_read();
    return key_pressed(J_B);
}

static enum PRN_STATUS printer_wait(uint16_t timeout, uint8_t mask, uint8_t value) {
    enum PRN_STATUS error;

    while (1) {
        error = printer_send_command(PRN_CMD_STATUS, NULL, 0);
        if ((error & mask) == value) {
            break;
        }

        if (printer_check_cancel()) {
            printer_send_command(PRN_CMD_BREAK, NULL, 0);
            return PRN_STATUS_CANCELLED;
        }

        if (timeout-- == 0) {
            return PRN_STATUS_TIMEOUT;
        }

        if (error & PRN_STATUS_MASK_ERRORS) {
            break;
        }

        vsync();
    }

    return error;
}

enum PRN_STATUS gbprinter_detect(void) BANKED {
    printer_send_command(PRN_CMD_INIT, NULL, 0);
    uint8_t r = printer_wait(PRN_DETECT_TIMEOUT, PRN_STATUS_MASK_ANY, PRN_STATUS_OK);

#ifdef DEBUG
    EMU_printf("%s: %hu\n",  __func__, (uint8_t)r);
#endif // DEBUG
    return r;
}

static void win_str_helper(const char *s, uint8_t y_pos) {
    static char line_buff[11];
    strncpy(line_buff, s, 10);
    line_buff[10] = '\0';
    win_str_center(line_buff, y_pos, 0);
}

enum PRN_STATUS gbprinter_screenshot(uint8_t win) BANKED {
    static uint8_t tile_buff[2 * DEVICE_SCREEN_WIDTH * 16];
    static struct prn_config params;
    enum PRN_STATUS r = PRN_STATUS_OK;

    printer_send_command(PRN_CMD_INIT, NULL, 0);

    for (int y = 0; y < DEVICE_SCREEN_HEIGHT; y += 2) {
        for (int y2 = 0; y2 < 2; y2++) {
            for (int x = 0; x < DEVICE_SCREEN_WIDTH; x++) {
                uint8_t tile = win ? get_win_tile_xy(x, y + y2) : get_bkg_tile_xy(x, y + y2);
                win ? get_win_data(tile, 1, tile_buff + ((x + (y2 * DEVICE_SCREEN_WIDTH)) * 16))
                    : get_bkg_data(tile, 1, tile_buff + ((x + (y2 * DEVICE_SCREEN_WIDTH)) * 16));
            }

            // black out rows we have sent, to indicate transfer progress
            win ? fill_win_rect(0, y + y2, DEVICE_SCREEN_WIDTH, 1, 0)
                : fill_bkg_rect(0, y + y2, DEVICE_SCREEN_WIDTH, 1, 0);
        }

        if (win) {
            if (y == 0) {
                win_str_helper("gb printer", 0);
            } else if (y == 2) {
                win_str_helper("transmit", 2);
            } else if (y == 8) {
                win_str_helper("in", 8);
            } else if (y == 10) {
                win_str_helper("progress", 10);
            } else if (y == 16) {
                win_str_helper("printing", 16);
            }
        }

        r = printer_send_command(PRN_CMD_DATA, tile_buff, sizeof(tile_buff));
        if ((r & ~PRN_STATUS_UNTRAN) != PRN_STATUS_OK) {
            goto end;
        }

        if (printer_check_cancel()) {
            printer_send_command(PRN_CMD_BREAK, NULL, 0);
            r = PRN_STATUS_CANCELLED;
            goto end;
        }
    }

    printer_send_command(PRN_CMD_DATA, NULL, 0);

    params.sheets = 1;
    params.margins = PRN_FINAL_MARGIN;
    params.palette = PRN_PALETTE_NORMAL;
    params.exposure = PRN_EXPO_DARK;

    printer_send_command(PRN_CMD_PRINT, (uint8_t *)&params, sizeof(struct prn_config));

    r = printer_wait(PRN_BUSY_TIMEOUT, PRN_STATUS_BUSY, PRN_STATUS_BUSY);
    if (r & PRN_STATUS_MASK_ERRORS) {
        goto end;
    }

    r = printer_wait(PRN_PRINT_TIMEOUT, PRN_STATUS_BUSY, 0);
    if (r & PRN_STATUS_MASK_ERRORS) {
        goto end;
    }

end:
#ifdef DEBUG
    EMU_printf("%s: %hu\n",  __func__, (uint8_t)r);
#endif // DEBUG
    return r;
}
