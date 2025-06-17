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

#include "gb/gb.h"
#include "gb/hardware.h"
#include "input.h"
#include "gbprinter.h"

BANKREF(gbprinter)

/** Width of the printed image in tiles
*/
#define PRN_TILE_WIDTH          20

#define PRN_LOW(A) ((A) & 0xFF)
#define PRN_HIGH(A) ((A) >> 8)

/** 0x88,0x33 are mandatory first bytes to initialise a communication with printer
 *   Any command sequence begins by these
 */
#define PRN_MAGIC               0x3388
#define PRN_LE(A)               PRN_LOW(A),PRN_HIGH(A)

/** magic number that is sent in the reply packet by the printer before the status byte
 */
#define PRN_MAGIC_DETECT        0x81

/** INIT command is mandatory to initialize communication protocol with the printer
 *   Two consecutive linked commands must never be more than 150 ms apart except the INIT command which is valid at least 10 seconds
 */
#define PRN_CMD_INIT            0x01

/** PRINT command
 *   Contains the palette, margins, number of prints and printing intensity
 */
#define PRN_CMD_PRINT           0x02

/** DATA command
 *   Can be any length between 0 and 640 bytes.
 *   DATA command with lenght 0 triggers PRN_STATUS_FULL and is mandatory before print command
 */
#define PRN_CMD_DATA            0x04

/** BREAK command
 *   Not very usefull but exists (see Game Boy Programming Manual)
 */
#define PRN_CMD_BREAK           0x08

/** STATUS command
 *   Used to check status bits
 *   Maybe be used alone before an INIT command to check physical connection with printer
 *   Resets PRN_STATUS_UNTRAN
 */
#define PRN_CMD_STATUS          0x0F

/** Palette format: the bits, grouped two by two, give the printing color of the encoded pixel value
 *   for the default palette 0xE4 = 0b11100100 = [3 2 1 0]
 *   Any value is valid, which means that 1 to 4 color images are possible
 *   0x00 acts the same as 0xE4 for the printer
 */
#define PRN_PALETTE_NORMAL      0b11100100u
#define PRN_PALETTE_INV         0b00011011u

/** Don't use margins
 */
#define PRN_NO_MARGINS          0x00

/** Exposure: 0x40 is default value, values from 0x80 to 0xFF act as 0x40
 *   Determines the time used by the printer head to heat the thermal paper
 */
#define PRN_EXPOSURE_LIGHT      0x00
#define PRN_EXPOSURE_DEFAULT    0x40
#define PRN_EXPOSURE_DARK       0x7F

/** Battery too low
 */
#define PRN_STATUS_LOWBAT       0x80

/** Error not specified according to the Game Boy Programming manual
 */
#define PRN_STATUS_ER2          0x40

/** Paper jam  (abnormal motor operation)
 */
#define PRN_STATUS_ER1          0x20

/** Packet error (but not checksum error)
 */
#define PRN_STATUS_ER0          0x10

/** Unprocessed data present in printer memory
 *   Allows to verify that printer got some data in memory with correct checksum
 *   is resetted by STATUS command
 */
#define PRN_STATUS_UNTRAN       0x08

/** status data ready, mandatory to allow printing
 *   is triggered by DATA command with lenght 0
 */
#define PRN_STATUS_FULL         0x04

/** Message sent by the printer while physically printing
 */
#define PRN_STATUS_BUSY         0x02

/** The received packet has a ckecksum error
 */
#define PRN_STATUS_SUM          0x01

#define PRN_STATUS_MASK_ERRORS  0xF0
#define PRN_STATUS_MASK_ANY     0xFF

#define PRN_SECONDS(A)          ((A)*60)

#define PRN_MAX_PROGRESS        8

#define PRN_STATUS_CANCELLED    PRN_STATUS_ER2

#define REINIT_SEIKO

#define START_TRANSFER          0x81
#define PRN_BUSY_TIMEOUT        PRN_SECONDS(2)
#define PRN_COMPLETION_TIMEOUT  PRN_SECONDS(20)
#define PRN_SEIKO_RESET_TIMEOUT 10

#define PRN_FINAL_MARGIN        0x03

typedef struct start_print_pkt_s {
    uint16_t magic;
    uint16_t command;
    uint16_t length;
    uint8_t print;
    uint8_t margins;
    uint8_t palette;
    uint8_t exposure;
    uint16_t crc;
    uint16_t trail;
} start_print_pkt_t;

static const uint8_t PRN_PKT_INIT[] = {
    PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_INIT),   PRN_LE(0), PRN_LE(0x01), PRN_LE(0)
};

static const uint8_t PRN_PKT_STATUS[] = {
    PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_STATUS), PRN_LE(0), PRN_LE(0x0F), PRN_LE(0)
};

static const uint8_t PRN_PKT_EOF[] = {
    PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_DATA),   PRN_LE(0), PRN_LE(0x04), PRN_LE(0)
};

static const uint8_t PRN_PKT_CANCEL[] = {
    PRN_LE(PRN_MAGIC), PRN_LE(PRN_CMD_BREAK),  PRN_LE(0), PRN_LE(0x01), PRN_LE(0)
};

static start_print_pkt_t PRN_PKT_START = {
    .magic = PRN_MAGIC, .command = PRN_CMD_PRINT,
    .length = 4, .print = TRUE,
    .margins = 0, .palette = PRN_PALETTE_NORMAL, .exposure = PRN_EXPOSURE_DARK,
    .crc = 0, .trail = 0
};

static uint16_t printer_status;
static uint8_t printer_tile_num;

static inline void gbprinter_set_print_params(uint8_t margins, uint8_t palette, uint8_t exposure) {
    PRN_PKT_START.crc = ((PRN_CMD_PRINT + 0x04u + 0x01u)
            + (PRN_PKT_START.margins = margins)
            + (PRN_PKT_START.palette = palette)
            + (PRN_PKT_START.exposure = exposure));
}

static uint8_t printer_send_receive(uint8_t b) {
    SB_REG = b;
    SC_REG = START_TRANSFER;
    while (SC_REG & 0x80);
    return SB_REG;
}

static uint8_t printer_send_byte(uint8_t b) {
    return (uint8_t)(printer_status = ((printer_status << 8) | printer_send_receive(b)));
}

static uint8_t printer_send_command(const uint8_t *command, uint8_t length) {
    uint8_t index = 0;
    while (index++ < length) printer_send_byte(*command++);
    return ((uint8_t)(printer_status >> 8) == PRN_MAGIC_DETECT) ? (uint8_t)printer_status : PRN_STATUS_MASK_ERRORS;
}

#define PRINTER_SEND_COMMAND(CMD) printer_send_command((const uint8_t *)&(CMD), sizeof(CMD))

static uint8_t printer_print_tile(const uint8_t *tiledata) {
    static const uint8_t PRINT_TILE[] = { 0x88,0x33,0x04,0x00,0x80,0x02 };
    static uint16_t printer_CRC;
    if (printer_tile_num == 0) {
        const uint8_t * data = PRINT_TILE;
        for (uint8_t i = sizeof(PRINT_TILE); i != 0; i--) printer_send_receive(*data++);
        printer_CRC = 0x04 + 0x80 + 0x02;
    }
    for(uint8_t i = 0x10; i != 0; i--, tiledata++) {
        printer_CRC += *tiledata;
        printer_send_receive(*tiledata);
    }
    if (++printer_tile_num == 40) {
        printer_send_receive((uint8_t)printer_CRC);
        printer_send_receive((uint8_t)(printer_CRC >> 8));
        printer_send_receive(0x00);
        printer_send_receive(0x00);
        printer_CRC = printer_tile_num = 0;
        return TRUE;
    }
    return FALSE;
}

static inline void printer_init(void) {
    printer_tile_num = 0;
    PRINTER_SEND_COMMAND(PRN_PKT_INIT);
}

uint8_t printer_check_cancel(void) {
    key_read();
    return key_pressed(J_B);
}

static uint8_t printer_wait(uint16_t timeout, uint8_t mask, uint8_t value) {
    uint8_t error;
    while (((error = PRINTER_SEND_COMMAND(PRN_PKT_STATUS)) & mask) != value) {
        if (printer_check_cancel()) {
            PRINTER_SEND_COMMAND(PRN_PKT_CANCEL);
            return PRN_STATUS_CANCELLED;
        }
        if (timeout-- == 0) return PRN_STATUS_MASK_ERRORS;
        if (error & PRN_STATUS_MASK_ERRORS) break;
        vsync();
    }
    return error;
}

uint8_t gbprinter_detect(uint8_t delay) BANKED {
    printer_init();
    uint8_t r = printer_wait(delay, PRN_STATUS_MASK_ANY, PRN_STATUS_OK);
#ifdef DEBUG
    EMU_printf("%s: %hu\n",  __func__, (uint8_t)r);
#endif // DEBUG
    return r;
}

uint8_t gbprinter_print_image(const uint8_t *image_map, const uint8_t *image,
                              int8_t pos_x, uint8_t width, uint8_t height,
                              uint8_t done) BANKED {
    uint8_t tile_data[16];
    uint8_t rows = ((height + 1) >> 1) << 1;
    uint8_t pkt_count = 0;

    if ((rows >> 1) == 0) return PRN_STATUS_OK;

    printer_tile_num = 0;

    for (uint8_t y = 0; y < rows; y++) {
        for (int16_t x = 0; x < PRN_TILE_WIDTH; x++) {
#ifdef DEBUG
            EMU_printf("%s: %hu %i\n",  __func__, (uint8_t)y, (int16_t)x);
#endif // DEBUG

            // overlay the picture tile if in range
            if ((y < height) && (x >= pos_x) && (x < (pos_x + width))) {
                uint8_t tile = image_map[(y * width) + (x - pos_x)];
                memcpy(tile_data, image + ((uint16_t)tile << 4), sizeof(tile_data));
            } else {
                memset(tile_data, 0, sizeof(tile_data));
            }

            // print the resulting tile
            if (printer_print_tile(tile_data)) {
                pkt_count++;

                if (printer_check_cancel()) {
                    PRINTER_SEND_COMMAND(PRN_PKT_CANCEL);
                    return PRN_STATUS_CANCELLED;
                }
            }

            if (pkt_count == 9) {
                pkt_count = 0;
                PRINTER_SEND_COMMAND(PRN_PKT_EOF);

                // setup margin if last packet
                gbprinter_set_print_params((y == (rows - 1)) ? PRN_FINAL_MARGIN : PRN_NO_MARGINS,
                                           PRN_PALETTE_NORMAL, PRN_EXPOSURE_DARK);

                PRINTER_SEND_COMMAND(PRN_PKT_START);

                // query printer status
                uint8_t error = printer_wait(PRN_BUSY_TIMEOUT, PRN_STATUS_BUSY, PRN_STATUS_BUSY);
                if (error & PRN_STATUS_MASK_ERRORS) {
                    return error;
                }

                error = printer_wait(PRN_COMPLETION_TIMEOUT, PRN_STATUS_BUSY, 0);
                if (error & PRN_STATUS_MASK_ERRORS) {
                    return error;
                }

#ifdef REINIT_SEIKO
                // reinit printer (required by Seiko?)
                if (y < (rows - 1)) {
                    PRINTER_SEND_COMMAND(PRN_PKT_INIT);
                    error = printer_wait(PRN_SEIKO_RESET_TIMEOUT, PRN_STATUS_MASK_ANY, PRN_STATUS_OK);
                    if (error) {
                        return error;
                    }
                }
#endif
            }
        }
    }

    if (pkt_count && done) {
        PRINTER_SEND_COMMAND(PRN_PKT_EOF);

        // setup printing if required
        gbprinter_set_print_params(PRN_FINAL_MARGIN, PRN_PALETTE_NORMAL, PRN_EXPOSURE_DARK);
        PRINTER_SEND_COMMAND(PRN_PKT_START);

        // query printer status
        uint8_t error = printer_wait(PRN_BUSY_TIMEOUT, PRN_STATUS_BUSY, PRN_STATUS_BUSY);
        if (error & PRN_STATUS_MASK_ERRORS) {
            return error;
        }

        error = printer_wait(PRN_COMPLETION_TIMEOUT, PRN_STATUS_BUSY, 0);
        if (error & PRN_STATUS_MASK_ERRORS) {
            return error;
        }
    }

    return PRINTER_SEND_COMMAND(PRN_PKT_STATUS);
}

uint8_t gbprinter_screenshot(uint8_t win) BANKED {
    static uint8_t map_buff[2 * DEVICE_SCREEN_WIDTH];
    static uint8_t tile_buff[2 * DEVICE_SCREEN_WIDTH * 16];

    for (int y = 0; y < DEVICE_SCREEN_HEIGHT; y += 2) {
        for (int y2 = 0; y2 < 2; y2++) {
            for (int x = 0; x < DEVICE_SCREEN_WIDTH; x++) {
                uint8_t tile = win ? get_win_tile_xy(x, y + y2) : get_bkg_tile_xy(x, y + y2);
                map_buff[x + (y2 * DEVICE_SCREEN_WIDTH)] = (x + (y2 * DEVICE_SCREEN_WIDTH));
                win ? get_win_data(tile, 1, tile_buff + ((x + (y2 * DEVICE_SCREEN_WIDTH)) * 16))
                    : get_bkg_data(tile, 1, tile_buff + ((x + (y2 * DEVICE_SCREEN_WIDTH)) * 16));
            }

            // black out rows we have sent, to indicate transfer progress
            win ? fill_win_rect(0, y + y2, DEVICE_SCREEN_WIDTH, 1, 0)
                : fill_bkg_rect(0, y + y2, DEVICE_SCREEN_WIDTH, 1, 0);
        }

        gbprinter_print_image(map_buff, tile_buff, 0, DEVICE_SCREEN_WIDTH, 2,
                              (y == (DEVICE_SCREEN_HEIGHT - 2)) ? 1 : 0);
    }
}
