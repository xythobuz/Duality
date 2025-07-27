/*
 * window.c
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
#include <stdio.h>
#include <assert.h>

#include "banks.h"
#include "config.h"
#include "gb/hardware.h"
#include "score.h"
#include "text.h"
#include "git.h"
#include "main.h"
#include "maps.h"
#include "map_data.h"
#include "gbprinter.h"
#include "multiplayer.h"
#include "strings.h"
#include "timer.h"
#include "game.h"
#include "window.h"

BANKREF(window)

static char str_buff[128];
static uint16_t stack_pointer = 0;

static void get_sp(void) {
    __asm
    ld (_stack_pointer), sp
    __endasm;
}

void fill_win(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t tile, uint8_t attr) BANKED {
    if (_cpu == CGB_TYPE) {
        VBK_REG = VBK_ATTRIBUTES;
        fill_win_rect(x, y, w, h, attr);
    }

    VBK_REG = VBK_TILES;
    fill_win_rect(x, y, w, h, tile);
}

void win_splash_draw(int32_t lowest, int32_t highest) BANKED {
    map_fill(MAP_TITLE, 0);

    // only show on splash if they fit
    if ((lowest <= 99999) && (highest <= 99999)) {
        number(lowest, 0, DEVICE_SCREEN_HEIGHT - 4, 1);
        number(highest, 0xFE, DEVICE_SCREEN_HEIGHT - 4, 0);

        str(get_string(STR_TOP), 0, DEVICE_SCREEN_HEIGHT - 2, 1);
        str(get_string(STR_SCORE), 10, DEVICE_SCREEN_HEIGHT - 2, 0);
    }
}

void win_splash_mp(void) BANKED {
    static uint8_t prev = 0;
    if ((_cpu == CGB_TYPE) && (mp_connection_status != prev)) {
        prev = mp_connection_status;
        char c = mp_connection_status % SPINNER_LENGTH;
        str_ascii_l(&get_string(STR_SPINNER)[c], 1, 19, 0, 0);
    }
}

void win_score_clear(uint8_t is_black, uint8_t no_bg) BANKED {
    if (no_bg) {
        fill_win(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT, 1, BKGF_CGB_PAL0);
    } else {
        map_fill(MAP_TITLE, 0);
    }

    if (is_black < 2) {
        str_center(is_black ? get_string(STR_BLACK) : get_string(STR_WHITE), 1, is_black);
    }
}

void win_score_draw(struct scores score, uint8_t off, uint8_t is_black) BANKED {
    str3(score.name, 0, 4 + off * 3, is_black, is_black, is_black);
    number(is_black ? -score.score : score.score, 7, 4 + off * 3, is_black);
}

void win_score_print(enum PRN_STATUS status) BANKED {
    if (_cpu == CGB_TYPE) {
        str_ascii(get_string(STR_GB_PRINTER), 0, 0, 0);
        str_ascii(get_string(STR_SCORE_PRINTOUT), 0, 1, 0);
        str_ascii(get_string(STR_RESULT), 0, 3, 0);

        if (status == PRN_STATUS_OK) {
            str_ascii(get_string(STR_SUCCESS), 0, 8, 0);
        } else {
            sprintf(str_buff, get_string(STR_PRINTF_ERROR), (uint16_t)status);
            str_ascii(str_buff, 0, 5, 0);

            gbprinter_error(status, str_buff);
            str_ascii_lines(str_buff, 6, 0);
        }
    } else {
        str(get_string(STR_PRINTOUT), 0, 4, 0);
        if (status == PRN_STATUS_OK) {
            str(get_string(STR_SUCCESS), 0, 8, 0);
        } else {
            str(get_string(STR_ERROR), 0, 8, 1);
            number(status, 11, 8, 1);
        }
    }
}

static void get_git(char *line_buff) NONBANKED {
    START_ROM_BANK(BANK(git)) {
        strncpy(line_buff, git_version, 2 * TEXT_LINE_WIDTH);
    } END_ROM_BANK
}

void win_about(void) BANKED {
    map_fill(MAP_TITLE, 0);

    str_center(get_string(STR_DUALITY), 0, 1);
    str_center(get_string(STR_XYTHOBUZ), 2, 1);

    char line_buff[2 * TEXT_LINE_WIDTH + 1] = {0};
    get_git(line_buff);

    if (_cpu == CGB_TYPE) {
        str_ascii(get_string(STR_GIT), 0, 6, 0);
        str_ascii(line_buff, 0, 7, 0);

        str_ascii(get_string(STR_BUILD_DATE), 0, 10, 0);
        str_ascii(get_string(STR_DATE), 0, 11, 0);
        str_ascii(get_string(STR_TIME), 0, 12, 0);

        str_ascii(get_string(STR_MP_TX), 14, 11, 1);
        str_ascii(get_string(STR_WAIT), 14, 12, 1);

        str_ascii(get_string(STR_VISIT), 0, 15, 0);
        str_ascii(get_string(STR_URL), 0, 16, 0);
    } else {
        str_lines(line_buff, 7, 0);

        const char *date = get_string(STR_DATE);
        str_l(&date[7], 4,           0, 14, 1); // year (4)
        str_l(&date[0], 3, (4 * 2) + 1, 14, 1); // month (3)
        str_l(&date[4], 2, (7 * 2) + 2, 14, 1); // day (2)

        str(get_string(STR_TIME), 4, 16, 0);
    }
}

void win_about_mp(void) BANKED {
    static uint8_t prev = 0;
    if ((_cpu == CGB_TYPE) && (mp_connection_status != prev)) {
        prev = mp_connection_status;
        uint8_t c = mp_connection_status % SPINNER_LENGTH;
        str_ascii_l(&get_string(STR_SPINNER)[c], 1, 19, 12, 1);
    }
}

static uint8_t get_debug(char *name_buff, uint8_t i) NONBANKED {
    uint8_t n_len;
    START_ROM_BANK(BANK(main)) {
        strncpy(name_buff, debug_entries[i].name, ENTRY_NAME_LEN + 1);

        n_len = strlen(name_buff);
        name_buff[n_len] = ' ';
        if (debug_entries[i].flag == DBG_NONE) {
            if (debug_menu_index == i) {
                name_buff[n_len + 1] = debug_special_value + '0';
            } else {
                name_buff[n_len + 1] = '0';
            }
        } else {
            name_buff[n_len + 1] = (conf_get()->debug_flags & debug_entries[i].flag) ? '1' : '0';
        }
        name_buff[n_len + 2] = '\0';
        n_len += 2;
    } END_ROM_BANK
    return n_len;
}

void win_debug(void) BANKED {
    map_fill(MAP_TITLE, 0);

    str_center(get_string(STR_DEBUG_MENU), 0, 0);

    for (uint8_t i = debug_menu_index; i < (8 + debug_menu_index); i++) {
        char name_buff[ENTRY_NAME_LEN + 2 + 1] = {0};
        uint8_t n_len = get_debug(name_buff, i % DEBUG_ENTRY_COUNT);
        str(name_buff, (TEXT_LINE_WIDTH - n_len) * 2, ((i - debug_menu_index) * 2) + 3, (debug_menu_index == i) ? 1 : 0);
    }
}

static uint8_t is_conf_hw(uint8_t i) NONBANKED {
    uint8_t r;
    START_ROM_BANK(BANK(main)) {
        r = (conf_entries[i].type == HW_ALL) || (conf_entries[i].type == get_hw());
    } END_ROM_BANK
    return r;
}

static uint8_t get_conf(char *name_buff, uint8_t i) NONBANKED {
    uint8_t n_len;
    START_ROM_BANK(BANK(main)) {
        strncpy(name_buff, conf_entries[i].name, ENTRY_NAME_LEN + 1);

        n_len = strlen(name_buff);
        name_buff[n_len] = ' ';
        if (*conf_entries[i].var < 10) {
            name_buff[n_len + 1] = *conf_entries[i].var + '0';
        } else {
            name_buff[n_len + 1] = *conf_entries[i].var - 10 + 'A';
        }
        name_buff[n_len + 2] = '\0';
        n_len += 2;
    } END_ROM_BANK
    return n_len;
}

void win_conf(void) BANKED {
    map_fill(MAP_TITLE, 0);

    // TODO paging when more options added?!
    static_assert(CONF_ENTRY_COUNT <= 7, "too many conf menu entries");
    uint8_t off = ((10 - CONF_ENTRY_COUNT) / 2) + 3;

    str_center(get_string(STR_CONF_MENU), 0, 0);

    for (uint8_t i = 0; (i < CONF_ENTRY_COUNT) && (i < 7); i++) {
        if (!is_conf_hw(i)) {
            continue;
        }

        char name_buff[ENTRY_NAME_LEN + 2 + 1] = {0};
        uint8_t n_len = get_conf(name_buff, i);
        str(name_buff, (TEXT_LINE_WIDTH - n_len) * 2, off, (debug_menu_index == i) ? 1 : 0);

        off += 2;
    }
}

void win_name(int32_t score) BANKED {
    map_fill(MAP_TITLE, 0);

    str_center(get_string(STR_SCORE), 1, score < 0);
    number(score < 0 ? -score : score, 0xFF, 3, score < 0);

    str_center(get_string(STR_ENTER), 6, score < 0);
    str_center(get_string(STR_NAME), 8, score < 0);

    str_center(get_string(STR_START_OK), 16, score < 0);
}

void win_name_draw(uint16_t name, uint8_t is_black, uint8_t pos) BANKED {
    str3(name, TEXT_LINE_WIDTH - 3, 12,
         (pos == 0) ? !is_black : is_black,
         (pos == 1) ? !is_black : is_black,
         (pos == 2) ? !is_black : is_black);
}

void win_continue(void) BANKED {
    map_fill(MAP_TITLE, 0);

    str_center(get_string(STR_GAME_IN), 1, 0);
    str_center(get_string(STR_PROGRESS), 3, 0);

    str_center(get_string(STR_A_CONTINUE), 13, 1);
    str_center(get_string(STR_B_NEW_GAME), 15, 1);
}

uint8_t win_game_draw(int32_t score, uint8_t initial) BANKED {
    uint8_t is_black = 0;
    if (score < 0) {
        score = -score;
        is_black = 1;
    }

    if (conf_get()->debug_flags & DBG_OUT_ON) {
        uint8_t redraw = 0;

        static int32_t prev_score = 0;
        if (initial || (score != prev_score)) {
            prev_score = score;
            redraw = 1;

            // TODO hard-coded black bg tile
            fill_win(0, 0, 20, 2, 0x80, BKGF_CGB_PAL3);
        }

        uint8_t x_off = number(score, 0, 0, is_black) >> 3;
        uint8_t y_off = 0;

        uint8_t y_max = (_cpu == CGB_TYPE) ? 2 : 1;

        if ((conf_get()->debug_flags & DBG_SHOW_FPS) && (y_off < 2)) {
            static uint8_t prev_fps = 0;
            if ((game_fps != prev_fps) || redraw) {
                prev_fps = game_fps;
                if (_cpu == CGB_TYPE) {
                    sprintf(str_buff, get_string(STR_PRINTF_FPS), (uint8_t)game_fps);
                    str_ascii(str_buff, x_off, y_off, 1);
                } else {
                    number(game_fps, x_off + 1, y_off, 1);
                }
            }
            y_off++;
        }

        if ((conf_get()->debug_flags & DBG_SHOW_FRAMES) && (y_off < 2)) {
            static uint16_t prev_framecount = 0;
            if ((frame_count != prev_framecount) || redraw) {
                prev_framecount = frame_count;
                if (_cpu == CGB_TYPE) {
                    sprintf(str_buff, get_string(STR_PRINTF_FRAMES), (uint16_t)frame_count);
                    str_ascii(str_buff, x_off, y_off, 1);
                } else {
                    number(frame_count, x_off + 1, y_off, 1);
                }
            }
            y_off++;
        }

        if ((conf_get()->debug_flags & DBG_SHOW_TIMER) && (y_off < 2)) {
            static uint16_t prev_timer = 0;
            uint16_t timer = timer_get();
            if ((timer != prev_timer) || redraw) {
                if (_cpu == CGB_TYPE) {
                    sprintf(str_buff, get_string(STR_PRINTF_TIMER), (uint16_t)timer);
                    str_ascii(str_buff, x_off, y_off, 1);
                } else {
                    number(timer, x_off + 1, y_off, 1);
                }
            }
            y_off++;
        }

        if ((conf_get()->debug_flags & DBG_SHOW_STACK) && (y_off < 2)) {
            static uint16_t prev_stack_pointer = 0;
            get_sp();
            if ((stack_pointer != prev_stack_pointer) || redraw) {
                prev_stack_pointer = stack_pointer;
                if (_cpu == CGB_TYPE) {
                    sprintf(str_buff, get_string(STR_PRINTF_STACK), (uint16_t)stack_pointer);
                    str_ascii(str_buff, x_off, y_off, 1);
                } else {
                    number(stack_pointer, x_off + 1, y_off, 1);
                }
            }
            y_off++;
        }

        return DEVICE_SCREEN_PX_WIDTH;
    } else {
        // TODO hard-coded black bg tile
        fill_win(0, 0, 10, 2, 0x80, BKGF_CGB_PAL3);

        return number(score, 0, 0, is_black);
    }
}
