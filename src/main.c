/*
 * main.c
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * Based on examples from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/rand/rand.c
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
#include <gbdk/metasprites.h>
#include <rand.h>
#include <stdint.h>

#include "asm/types.h"
#include "gb/gb.h"
#include "maps.h"
#include "obj.h"
#include "sprites.h"
#include "sound.h"
#include "input.h"
#include "game.h"
#include "score.h"
#include "sgb_border.h"
#include "border_sgb.h"

static void highscore(uint8_t is_black) NONBANKED {
    HIDE_WIN;

    move_win(MINWNDPOSX, MINWNDPOSY);
    hide_sprites_range(SPR_NUM_START, MAX_HARDWARE_SPRITES);
    win_score_clear(is_black ? 1 : 0);

    SHOW_WIN;

    for (uint8_t i = 0; i < SCORE_NUM; i++) {
        struct scores score = is_black ? score_lowest(i) : score_highest(i);
        win_score_draw(score, i, is_black);
    }

    while (1) {
        key_read();

        if (key_pressed(J_A) || key_pressed(J_B)) {
            break;
        }

        vsync();
    }
}

static void about_screen(void) NONBANKED {
    HIDE_WIN;

    move_win(MINWNDPOSX, MINWNDPOSY);
    hide_sprites_range(SPR_NUM_START, MAX_HARDWARE_SPRITES);
    win_about();

    SHOW_WIN;

    while (1) {
        key_read();

        if (key_pressed(J_A) || key_pressed(J_B) || key_pressed(J_SELECT)) {
            break;
        }

        vsync();
    }
}

static void splash_win(void) NONBANKED {
    HIDE_WIN;

    // initially show the top 1 scores
    int32_t low = score_lowest(0).score;
    int32_t high = score_highest(0).score;

    // only show on splash if they fit
    if ((low >= -99999) && (high <= 99999)) {
        win_splash_draw(-low, high);
    }

    move_win(MINWNDPOSX, MINWNDPOSY + DEVICE_SCREEN_PX_HEIGHT - 16);
    SHOW_WIN;
}

static void splash(void) NONBANKED {
    disable_interrupts();
    DISPLAY_OFF;
    map_title();
    move_bkg(0, 0);
    SHOW_BKG;
    spr_init_pal();
    SHOW_SPRITES;
    SPRITES_8x8;

    obj_init();
    obj_add(SPR_LIGHT, 42, -42, 0, 0);
    obj_add(SPR_DARK, -42, -42, 0, 0);

    win_init(1);
    splash_win();

    DISPLAY_ON;
    enable_interrupts();

    while (1) {
        key_read();

        if (key_pressed(J_LEFT)) {
            highscore(1);
            splash_win();
        } else if (key_pressed(J_RIGHT)) {
            highscore(0);
            splash_win();
        } else if (key_pressed(J_SELECT)) {
            about_screen();
            splash_win();
        } else if (key_pressed(0xFF)) {
            break;
        }

        uint8_t hiwater = SPR_NUM_START;
        obj_draw(0, 0, &hiwater);
        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        vsync();
    }
}

uint16_t ask_name(int32_t score) NONBANKED {
    disable_interrupts();
    DISPLAY_OFF;
    map_title();
    move_bkg(0, 0);
    SHOW_BKG;
    spr_init_pal();
    SHOW_SPRITES;
    SPRITES_8x8;

    hide_sprites_range(SPR_NUM_START, MAX_HARDWARE_SPRITES);

    win_init(1);
    win_name(score);

    move_win(MINWNDPOSX, MINWNDPOSY);
    SHOW_WIN;

    DISPLAY_ON;
    enable_interrupts();

    char name[3] = { 'a', 'a', 'a' };
    uint8_t pos = 0;
    win_name_draw(convert_name(name[0], name[1], name[2]), score < 0, pos);

    while (1) {
        key_read();

        if (key_pressed(J_LEFT)) {
            if (pos > 0) {
                pos--;
                win_name_draw(convert_name(name[0], name[1], name[2]), score < 0, pos);
            }
        } else if (key_pressed(J_RIGHT)) {
            if (pos < 3) {
                pos++;
                win_name_draw(convert_name(name[0], name[1], name[2]), score < 0, pos);
            }
        } else if (key_pressed(J_UP)) {
            if (pos < 3) {
                name[pos]++;
                if (name[pos] > 'z') {
                    name[pos] -= 'z' - 'a' + 1;
                }
                win_name_draw(convert_name(name[0], name[1], name[2]), score < 0, pos);
            }
        } else if (key_pressed(J_DOWN)) {
            if (pos < 3) {
                name[pos]--;
                if (name[pos] < 'a') {
                    name[pos] += 'z' - 'a' + 1;
                }
                win_name_draw(convert_name(name[0], name[1], name[2]), score < 0, pos);
            }
        } else if (key_pressed(J_A)) {
            if (pos < 3) {
                pos++;
                win_name_draw(convert_name(name[0], name[1], name[2]), score < 0, pos);
            } else {
                break;
            }
        } else if (key_pressed(J_START)) {
            break;
        }

        vsync();
    }

    return convert_name(name[0], name[1], name[2]);
}

static void sgb_init(void) NONBANKED {
    // Wait 4 frames
    // For SGB on PAL SNES this delay is required on startup, otherwise borders don't show up
    for (uint8_t i = 0; i < 4; i++) {
        vsync();
    }

    DISPLAY_ON;

    SWITCH_ROM(BANK(border_sgb));

    set_sgb_border((const uint8_t *)border_sgb_tiles, sizeof(border_sgb_tiles),
                   (const uint8_t *)border_sgb_map, sizeof(border_sgb_map),
                   (const uint8_t *)border_sgb_palettes, sizeof(border_sgb_palettes));

    DISPLAY_OFF;

}

void main(void) NONBANKED {
    // load sgb border
    sgb_init();

    // "cheat" and enable double-speed CPU mode on GBC
    if (_cpu == CGB_TYPE) {
        cpu_fast();
    }

    spr_init();
    snd_init();

    splash();

    uint16_t seed = DIV_REG;
    waitpadup();
    seed |= ((uint16_t)DIV_REG) << 8;
    initarand(seed);

    while (1) {
        int32_t score = game();

        if ((score != 0) && score_ranking(score)) {
            uint16_t name = ask_name(score);
            struct scores s = { .name = name, .score = score };
            score_add(s);
        }

        splash();
    }
}
