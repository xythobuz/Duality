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

#include "maps.h"
#include "obj.h"
#include "sprites.h"
#include "sound.h"
#include "input.h"
#include "game.h"
#include "score.h"

static void highscore(uint8_t is_black) {
    hide_sprites_range(SPR_NUM_START, MAX_HARDWARE_SPRITES);
    win_score_clear();

    for (uint8_t i = 0; i < SCORE_NUM; i++) {
        int32_t score = is_black ? -score_lowest(i) : score_highest(i);
        win_score_draw(score, i, is_black);
    }

    move_win(MINWNDPOSX + 0, MINWNDPOSY);

    while (1) {
        key_read();

        if (key_pressed(J_A) || key_pressed(J_B)) {
            break;
        }

        vsync();
    }
}

static void splash_win(void) {
    // initially show the top 1 scores
    int32_t low = score_lowest(0);
    int32_t high = score_highest(0);

    // only show on splash if they fit
    if ((low >= -99999) && (high <= 99999)) {
        win_splash_draw(-low, high);
    }

    move_win(MINWNDPOSX + 0, MINWNDPOSY + DEVICE_SCREEN_PX_HEIGHT - 16);
    SHOW_WIN;
}

static void splash(void) {
    disable_interrupts();
    DISPLAY_OFF;
    map_title();
    SHOW_BKG;
    SHOW_SPRITES;
    SPRITES_8x8;
    DISPLAY_ON;
    enable_interrupts();

    obj_init();
    obj_add(SPR_LIGHT, 42, -42, 0, 0);
    obj_add(SPR_DARK, -42, -42, 0, 0);

    splash_win();

    while (1) {
        key_read();

        if (key_pressed(J_LEFT)) {
            highscore(1);
            splash_win();
        } else if (key_pressed(J_RIGHT)) {
            highscore(0);
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

void main(void) {
    spr_init();
    snd_init();
    win_init();

    splash();

    uint16_t seed = DIV_REG;
    waitpadup();
    seed |= ((uint16_t)DIV_REG) << 8;
    initarand(seed);

    while (1) {
        int32_t score = game();
        score_add(score);
        splash();
    }
}
