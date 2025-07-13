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

#include <gbdk/metasprites.h>
#include <string.h>
#include <rand.h>

#include "banks.h"
#include "config.h"
#include "gb/gb.h"
#include "gb/hardware.h"
#include "maps.h"
#include "obj.h"
#include "sprites.h"
#include "sound.h"
#include "input.h"
#include "game.h"
#include "score.h"
#include "sgb_border.h"
#include "border_sgb.h"
#include "timer.h"
#include "sample.h"
#include "window.h"
#include "gbprinter.h"
#include "multiplayer.h"
#include "table_speed_shot.h"
#include "main.h"

uint8_t debug_menu_index = 0;
uint8_t debug_special_value = 0;

static uint8_t anim_frame = 0;
static uint8_t anim_state = 0;
static enum HW_TYPE hw_type = HW_UNKNOWN;

BANKREF(main)

const struct conf_entry conf_entries[CONF_ENTRY_COUNT] = {
  //{ .name = "sfx-vol",  .var = &mem.config.sfx_vol,    .max = 3,  .type = HW_ALL },
    { .name = "musi-vol", .var = &mem.config.music_vol,  .max = 15, .type = HW_ALL },
    { .name = "game-map", .var = &mem.config.game_bg,    .max = 1,  .type = HW_ALL },
    { .name = "invt-map", .var = &mem.config.dmg_bg_inv, .max = 1,  .type = HW_DMG },
};

const struct debug_entry debug_entries[DEBUG_ENTRY_COUNT] = {
    { .name = "marker",   .flag = DBG_MARKER,      .max = 1         }, // 0
    { .name = "invuln",   .flag = DBG_GOD_MODE,    .max = 1         }, // 1
    { .name = "no-spawn", .flag = DBG_NO_OBJ,      .max = 1         }, // 2
    { .name = "no-fuel",  .flag = DBG_NO_FUEL,     .max = 1         }, // 3
    { .name = "fastmove", .flag = DBG_FAST,        .max = 1         }, // 4
    { .name = "show-fps", .flag = DBG_SHOW_FPS,    .max = 1         }, // 5
    { .name = "s-frames", .flag = DBG_SHOW_FRAMES, .max = 1         }, // 6
    { .name = "sh-timer", .flag = DBG_SHOW_TIMER,  .max = 1         }, // 7
    { .name = "sh-stack", .flag = DBG_SHOW_STACK,  .max = 1         }, // 8

    // keep at end
    { .name = "music",    .flag = DBG_NONE,        .max = SND_COUNT }, // 9
    { .name = "sfx-test", .flag = DBG_NONE,        .max = SFX_COUNT }, // 10
    { .name = "cl score", .flag = DBG_NONE,        .max = 1         }, // 11
    { .name = "0 scores", .flag = DBG_NONE,        .max = 1         }, // 12
};

#define DEBUG_MENU_MUSIC_INDEX (DEBUG_ENTRY_COUNT - 4)
#define DEBUG_MENU_SFX_INDEX   (DEBUG_ENTRY_COUNT - 3)
#define DEBUG_MENU_CLEAR_INDEX (DEBUG_ENTRY_COUNT - 2)
#define DEBUG_MENU_ZERO_INDEX  (DEBUG_ENTRY_COUNT - 1)

enum HW_TYPE get_hw(void) BANKED {
    return hw_type;
}

static void list_scores(uint8_t is_black) {
    for (uint8_t i = 0; i < SCORE_NUM; i++) {
        struct scores score;
        is_black ? score_lowest(i, &score) : score_highest(i, &score);
        win_score_draw(score, i, is_black);
    }
}

static void highscore(uint8_t is_black) {
    HIDE_WIN;

    move_win(MINWNDPOSX, MINWNDPOSY);
    hide_sprites_range(SPR_NUM_START, MAX_HARDWARE_SPRITES);
    win_score_clear(is_black ? 1 : 0, 0);

    SHOW_WIN;

    list_scores(is_black);

    while (1) {
        key_read();

        if (key_pressed(J_A) || key_pressed(J_B)) {
            break;
        } else if (key_pressed(J_SELECT)) {
            enum PRN_STATUS status = gbprinter_detect();
            if ((status & PRN_STATUS_MASK_ANY) == PRN_STATUS_OK) {
                win_score_clear(is_black, 1);
                list_scores(is_black);
                status = gbprinter_screenshot(1, is_black ? PRN_PALETTE_SC_B : PRN_PALETTE_SC_W);
            }

            if (status != PRN_STATUS_OK) {
                win_score_clear(2, 0);
                win_score_print(status);
                while (1) {
                    key_read();
                    if (key_pressed(0xFF)) break;
                    vsync();
                }
            }

            break;
        }

        vsync();
    }
}

static void about_screen(void) {
    HIDE_WIN;

    move_win(MINWNDPOSX, MINWNDPOSY);
    hide_sprites_range(SPR_NUM_START, MAX_HARDWARE_SPRITES);
    win_about();

    SHOW_WIN;

    while (1) {
        key_read();

        if (mp_master_ready()) {
            mp_master_start();
            break;
        }
        win_about_mp();

        if (key_pressed(J_A) || key_pressed(J_B) || key_pressed(J_SELECT)) {
            break;
        }

        vsync();
    }
}

static void conf_screen(void) {
    HIDE_WIN;

    uint8_t changed = 0;
    debug_menu_index = 0;

    move_win(MINWNDPOSX, MINWNDPOSY);
    hide_sprites_range(SPR_NUM_START, MAX_HARDWARE_SPRITES);
    win_conf();

    SHOW_WIN;

    while (1) {
        key_read();

        if (key_pressed(J_SELECT)) {
            if (changed) {
                conf_write_crc();
                changed = 0;
            }
            about_screen();
            break;
        } else if (key_pressed(J_UP)) {
            do {
                if (debug_menu_index > 0) {
                    debug_menu_index--;
                } else {
                    debug_menu_index = CONF_ENTRY_COUNT - 1;
                }
            } while ((conf_entries[debug_menu_index].type != HW_ALL)
                    && (conf_entries[debug_menu_index].type != hw_type));
            win_conf();
        } else if (key_pressed(J_DOWN)) {
            do {
                if (debug_menu_index < (CONF_ENTRY_COUNT - 1)) {
                    debug_menu_index++;
                } else {
                    debug_menu_index = 0;
                }
            } while ((conf_entries[debug_menu_index].type != HW_ALL)
                    && (conf_entries[debug_menu_index].type != hw_type));
            win_conf();
        } else if (key_pressed(J_LEFT)) {
            if (*conf_entries[debug_menu_index].var > 0) {
                (*conf_entries[debug_menu_index].var)--;
            } else {
                *conf_entries[debug_menu_index].var = conf_entries[debug_menu_index].max;
            }
            win_conf();
            changed = 1;
        } else if (key_pressed(J_RIGHT)) {
            if (*conf_entries[debug_menu_index].var < conf_entries[debug_menu_index].max) {
                (*conf_entries[debug_menu_index].var)++;
            } else {
                *conf_entries[debug_menu_index].var = 0;
            }
            win_conf();
            changed = 1;
        } else if (key_pressed(J_A) || key_pressed(J_B) || key_pressed(J_START)) {
            break;
        }

        vsync();
    }

    if (changed) {
        conf_write_crc();
    }

    debug_menu_index = 0;
}

static void splash_win(void) {
    HIDE_WIN;

    if (conf_get()->debug_flags & DBG_MENU) {
        win_debug();
        move_win(MINWNDPOSX, MINWNDPOSY);
    } else {
        // initially show the top 1 scores
        struct scores score;

        score_lowest(0, &score);
        int32_t low = score.score;

        score_highest(0, &score);
        int32_t high = score.score;

        win_splash_draw(-low, high);

        move_win(MINWNDPOSX, MINWNDPOSY);
    }

    SHOW_WIN;
}

static void splash_anim(uint8_t *hiwater) NONBANKED {
    if (++anim_frame >= 60) {
        anim_frame = 0;
        if (++anim_state >= 12) {
            anim_state = 0;
        }
    }

    int16_t spd_off_x = 0;
    int16_t spd_off_y = 0;
    int32_t score = 0;
    obj_do(&spd_off_x, &spd_off_y, &score, hiwater, 1);

    /*
     * 0: right
     * 1: right shoot
     * 2: right
     * 3: top-right (add)
     * 4: top
     * 5: top-left
     * 6: left
     * 7: left shoot
     * 8: left
     * 9: top-left (add)
     * 10: top
     * 11: top-right
     */

    switch (anim_state) {
        case 1:
            if (anim_frame == 0) {
                START_ROM_BANK(BANK(table_speed_shot)) {
                    obj_add(SPR_SHOT, SHIP_OFF, -42,
                            table_speed_shot[(ROT_90 * table_speed_shot_WIDTH) + 0],
                            -table_speed_shot[(ROT_90 * table_speed_shot_WIDTH) + 1]);
                } END_ROM_BANK;
                sample_play(SFX_SHOT);
            }
        case 0:
        case 2:
            spr_draw(SPR_SHIP, FLIP_NONE, -4, -42 - 1, 4, hiwater);
            break;

        case 3:
            if (anim_frame == 30) {
                obj_add(SPR_LIGHT, 42, -42, 0, 0);
            }
        case 11:
            spr_draw(SPR_SHIP, FLIP_NONE, 1, -42 - 1, 2, hiwater);
            break;

        case 9:
            if (anim_frame == 30) {
                obj_add(SPR_DARK, -42, -42, 0, 0);
            }
        case 5:
            spr_draw(SPR_SHIP, FLIP_X, -1, -42 - 1, 2, hiwater);
            break;

        case 4:
        case 10:
            spr_draw(SPR_SHIP, FLIP_NONE, -1, -42 + 4, 0, hiwater);
            break;

        case 7:
            if (anim_frame == 0) {
                START_ROM_BANK(BANK(table_speed_shot)) {
                    obj_add(SPR_SHOT, -SHIP_OFF, -42,
                            table_speed_shot[(ROT_270 * table_speed_shot_WIDTH) + 0],
                            -table_speed_shot[(ROT_270 * table_speed_shot_WIDTH) + 1]);
                } END_ROM_BANK;
                sample_play(SFX_SHOT);
            }
        case 6:
        case 8:
            spr_draw(SPR_SHIP, FLIP_X, 4, -42, 4, hiwater);
            break;
    }
}

void splash(void) BANKED {
    snd_music_off();
    snd_note_off();

    disable_interrupts();
    DISPLAY_OFF;
    map_load(1);
    map_fill(MAP_TITLE, 1);
    SHOW_BKG;
    spr_init_pal();
    SHOW_SPRITES;
    SPRITES_8x8;

    anim_frame = 0;
    anim_state = 0;

    memset(&obj_state, 0, sizeof(struct obj_state));
    obj_add(SPR_LIGHT, 42, -42, 0, 0);
    obj_add(SPR_DARK, -42, -42, 0, 0);

    splash_win();

    DISPLAY_ON;
    enable_interrupts();

    if (!(conf_get()->debug_flags & DBG_MENU)) {
        snd_music(SND_MENU);
    }

    while (1) {
        key_read();

        if (mp_slave_ready()) {
            mp_slave_start();
            splash_win();
        }
        win_splash_mp();

        if (key_pressed(J_LEFT) && (!(conf_get()->debug_flags & DBG_MENU))) {
            highscore(1);
            splash_win();
        } else if (key_pressed(J_RIGHT) && (!(conf_get()->debug_flags & DBG_MENU))) {
            highscore(0);
            splash_win();
        } else if (key_pressed(J_SELECT)) {
            conf_screen();
            splash_win();
        } else if (key_pressed(J_START)) {
            if ((key_debug() == 0) && (!(conf_get()->debug_flags & DBG_MENU))) {
                conf_get()->debug_flags |= DBG_MENU;
                snd_music_off();
                snd_note_off();
                conf_write_crc();
                splash_win();
            } else {
                break;
            }
        } else {
            if (conf_get()->debug_flags & DBG_MENU) {
                uint8_t switch_special = 0;

                if (key_pressed(J_UP)) {
                    if (debug_menu_index > 0) {
                        debug_menu_index--;
                    } else {
                        debug_menu_index = DEBUG_ENTRY_COUNT - 1;
                    }
                    debug_special_value = 0;
                    snd_music_off();
                    snd_note_off();
                    splash_win();
                } else if (key_pressed(J_DOWN)) {
                    if (debug_menu_index < (DEBUG_ENTRY_COUNT - 1)) {
                        debug_menu_index++;
                    } else {
                        debug_menu_index = 0;
                    }
                    debug_special_value = 0;
                    snd_music_off();
                    snd_note_off();
                    splash_win();
                } else if (key_pressed(J_LEFT)) {
                    if (debug_entries[debug_menu_index].flag != DBG_NONE) {
                        conf_get()->debug_flags ^= debug_entries[debug_menu_index].flag;
                        conf_write_crc();
                    } else {
                        if (debug_special_value > 0) {
                            debug_special_value--;
                        } else {
                            debug_special_value = debug_entries[debug_menu_index].max;
                        }
                        switch_special = 1;
                    }
                    splash_win();
                } else if (key_pressed(J_RIGHT) || key_pressed(J_A)) {
                    if (debug_entries[debug_menu_index].flag != DBG_NONE) {
                        conf_get()->debug_flags ^= debug_entries[debug_menu_index].flag;
                        conf_write_crc();
                    } else {
                        if (debug_special_value < debug_entries[debug_menu_index].max) {
                            debug_special_value++;
                        } else {
                            debug_special_value = 0;
                        }
                        switch_special = 1;
                    }
                    splash_win();
                } else if (key_pressed(J_B)) {
                    conf_get()->debug_flags &= ~DBG_MENU;
                    debug_special_value = 0;
                    conf_write_crc();
                    splash_win();
                    snd_music(SND_MENU);
                }

                if (switch_special && (debug_menu_index == DEBUG_MENU_MUSIC_INDEX)) {
                    snd_music_off();
                    if (debug_special_value > 0) {
                        snd_music(debug_special_value - 1);
                    }
                    snd_note_off();
                } else if ((switch_special || (!sample_running()))
                        && (debug_menu_index == DEBUG_MENU_SFX_INDEX)) {
                    if (debug_special_value > 0) {
                        sample_play(debug_special_value - 1);
                    }
                } else if (switch_special && debug_special_value
                        && (debug_menu_index == DEBUG_MENU_CLEAR_INDEX)) {
                    debug_special_value = 0;
                    score_reset();
                    conf_write_crc();
                    splash_win();
                } else if (switch_special && debug_special_value
                        && (debug_menu_index == DEBUG_MENU_ZERO_INDEX)) {
                    debug_special_value = 0;
                    score_zero();
                    conf_write_crc();
                    splash_win();
                }
            }
        }

        uint8_t hiwater = SPR_NUM_START;

        if (!(conf_get()->debug_flags & DBG_MENU)) {
            if (conf_get()->debug_flags & DBG_MARKER) {
                spr_draw(SPR_DEBUG, FLIP_NONE, 0, -10, 0, &hiwater);
                spr_draw(SPR_SHOT_LIGHT, FLIP_NONE, 0, -10, 0, &hiwater);

                spr_draw(SPR_DEBUG, FLIP_NONE, 0, 0, 0, &hiwater);
                spr_draw(SPR_SHOT, FLIP_NONE, 0, 0, 0, &hiwater);

                spr_draw(SPR_DEBUG, FLIP_NONE, 0, 10, 0, &hiwater);
                spr_draw(SPR_SHOT_DARK, FLIP_NONE, 0, 10, 0, &hiwater);

                spr_draw(SPR_DEBUG, FLIP_NONE, 42, -42, 0, &hiwater);
                spr_draw(SPR_DEBUG, FLIP_NONE, 0, -42, 0, &hiwater);
                spr_draw(SPR_DEBUG, FLIP_NONE, -42, -42, 0, &hiwater);
            }

            splash_anim(&hiwater);
        }

        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        vsync();
    }
}

uint16_t ask_name(int32_t score) BANKED {
    snd_music_off();
    snd_note_off();

    disable_interrupts();
    DISPLAY_OFF;
    map_load(1);
    map_fill(MAP_TITLE, 0);
    SHOW_BKG;
    spr_init_pal();
    SHOW_SPRITES;
    SPRITES_8x8;

    hide_sprites_range(SPR_NUM_START, MAX_HARDWARE_SPRITES);

    win_name(score);

    move_win(MINWNDPOSX, MINWNDPOSY);
    SHOW_WIN;

    DISPLAY_ON;
    enable_interrupts();

    snd_music(SND_GAMEOVER);

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

uint8_t ask_continue(void) BANKED {
    HIDE_WIN;

    uint8_t r = 0;

    move_win(MINWNDPOSX, MINWNDPOSY);
    hide_sprites_range(SPR_NUM_START, MAX_HARDWARE_SPRITES);
    win_continue();

    SHOW_WIN;

    while (1) {
        key_read();

        if (key_pressed(J_A)) {
            r = 1;
            break;
        } else if (key_pressed(J_B)) {
            r = 0;
            break;
        }

        vsync();
    }

    return r;
}

static void sgb_init(void) NONBANKED {
    // Wait 4 frames
    // For SGB on PAL SNES this delay is required on startup, otherwise borders don't show up
    for (uint8_t i = 0; i < 4; i++) {
        vsync();
    }

    DISPLAY_ON;

    if (!sgb_check()) {
        hw_type = (_cpu == CGB_TYPE) ? HW_GBC : HW_DMG;
        DISPLAY_OFF;
        return;
    } else {
        hw_type = HW_SGB;
    }

    START_ROM_BANK(BANK(border_sgb)) {
        set_sgb_border((const uint8_t *)border_sgb_tiles, sizeof(border_sgb_tiles),
                       (const uint8_t *)border_sgb_map, sizeof(border_sgb_map),
                       (const uint8_t *)border_sgb_palettes, sizeof(border_sgb_palettes));
    } END_ROM_BANK

    DISPLAY_OFF;
}

void main(void) NONBANKED {
    // load sgb border
    sgb_init();

    // "cheat" and enable double-speed CPU mode on GBC
    if (_cpu == CGB_TYPE) {
        cpu_fast();
    }

    DISPLAY_OFF;
    map_load(1);
    map_fill(MAP_TITLE, 1);
    SHOW_BKG;
    DISPLAY_ON;

    conf_init();
    timer_init();
    spr_init();
    snd_init();

    splash();

    uint16_t seed = DIV_REG;
    waitpadup();
    seed |= ((uint16_t)DIV_REG) << 8;
    initarand(seed);

    while (1) {
        if (conf_state()->in_progress && ask_continue()) {
            game_state = conf_state()->state_game;
            obj_state = conf_state()->state_obj;
        } else {
            game_init();
        }

        if (game(GM_SINGLE)) {
            // game was exited via pause menu
            conf_state()->in_progress = 1;
            conf_state()->state_game = game_state;
            conf_state()->state_obj = obj_state;
            conf_write_crc();
        } else {
            // game ended with player death
            conf_state()->in_progress = 0;
            conf_write_crc();

            if ((!(conf_get()->debug_flags))
                    && (game_state.score != 0)
                    && score_ranking(game_state.score)) {
                uint16_t name = ask_name(game_state.score);
                struct scores s = { .name = name, .score = game_state.score };
                score_add(s);
            }
        }

        splash();
    }
}
