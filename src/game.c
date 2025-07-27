/*
 * game.c
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

#include <gbdk/metasprites.h>
#include <string.h>
#include <rand.h>
#include <stdint.h>

#include "banks.h"
#include "config.h"
#include "maps.h"
#include "obj.h"
#include "sprites.h"
#include "sound.h"
#include "input.h"
#include "main.h"
#include "sample.h"
#include "window.h"
#include "multiplayer.h"
#include "table_speed_shot.h"
#include "table_speed_move.h"
#include "timer.h"
#include "game.h"

#define BAR_OFFSET_X (4 - 80)
#define HEALTH_OFFSET_Y -16
#define POWER_OFFSET_Y 16
#define PAUSE_BLINK_FRAMES 32

#define SPEED_INC 1
#define SPEED_DEC 1

#define SPEED_MAX_IDLE 16
#define SPEED_MAX_DBG 256

#define POWER_MAX 0x1FF
#define POWER_SHIFT 1

#define POWER_INC 2
#define POWER_DEC 4

BANKREF(game)

const int8_t table_shot_offsets[ROT_INVALID * 2] = {
    0,                 -SHIP_OFF,         // 0.0
    SHIP_OFF / 2 - 1,  -SHIP_OFF / 2 - 4, // 22.5
    SHIP_OFF / 2 + 3,  -SHIP_OFF / 2 - 2, // 45.0
    SHIP_OFF / 2 + 5,  -SHIP_OFF / 2 + 2, // 67.5
    SHIP_OFF,          0,                 // 90.0
    SHIP_OFF / 2 + 5,  SHIP_OFF / 2 + 0,  // 112.5
    SHIP_OFF / 2 + 3,  SHIP_OFF / 2 + 2,  // 135.0
    SHIP_OFF / 2 + 1,  SHIP_OFF / 2 + 4,  // 157.5
    0,                 SHIP_OFF,          // 180.0
    -SHIP_OFF / 2 + 2, SHIP_OFF / 2 + 3,  // 202.5
    -SHIP_OFF / 2 - 3, SHIP_OFF / 2 + 2,  // 225.0
    -SHIP_OFF / 2 - 5, SHIP_OFF / 2 - 1,  // 247.5
    -SHIP_OFF,         0,                 // 270.0
    -SHIP_OFF / 2 - 2, -SHIP_OFF / 2 + 2, // 292.5
    -SHIP_OFF / 2 - 3, -SHIP_OFF / 2 - 2, // 315.0
    -SHIP_OFF / 2 + 1, -SHIP_OFF / 2 - 4, // 337.5
};

enum ACCELERATION {
    ACC_X = (1U << 1),
    ACC_Y = (1U << 2),
    ACC_R = (1U << 3),
};

static uint8_t fps_count = 0;
static uint16_t prev_fps_start = 0;

struct game_state game_state;
uint16_t frame_count = 0;
uint8_t game_fps = 0;

static void calc_fps(void) {
    frame_count++;
    fps_count++;
    uint16_t diff = timer_get() - prev_fps_start;
    if (diff >= TIMER_HZ) {
        prev_fps_start = timer_get();
        game_fps = fps_count;
        fps_count = 0;
    }
}

static uint8_t pause_screen(void) {
    snd_music_off();
    snd_note_off();

    uint8_t n = 0;

    while (1) {
        key_read();

        if (key_pressed(J_START)) {
            break;
        } else if (key_pressed(J_SELECT)) {
            return 1;
        }

        n = (n + 1) & (PAUSE_BLINK_FRAMES - 1);

        uint8_t hiwater = SPR_NUM_START;
        spr_draw(SPR_PAUSE, FLIP_NONE, 0, 0, (n < (PAUSE_BLINK_FRAMES / 2)) ? 0 : 1, &hiwater);
        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        if (conf_get()->debug_flags & DBG_OUT_ON) {
            uint8_t x_off = win_game_draw(game_state.score, 0);
            move_win(MINWNDPOSX + DEVICE_SCREEN_PX_WIDTH - x_off,
                     MINWNDPOSY + DEVICE_SCREEN_PX_HEIGHT - 16);
        }

        calc_fps();
        vsync();
    }

    return 0;
}

static void status(uint8_t health, uint8_t power, uint8_t *hiwater) {
    if (health > 0) {
        switch (health >> 6) {
            case 3:
                spr_draw(SPR_HEALTH, FLIP_X, BAR_OFFSET_X, HEALTH_OFFSET_Y - 24,
                         ((health >> 6) == 3) ? 7 - ((health >> 3) & 7) : 0, hiwater);
            case 2:
                spr_draw(SPR_HEALTH, FLIP_X, BAR_OFFSET_X, HEALTH_OFFSET_Y - 16,
                         ((health >> 6) == 2) ? 7 - ((health >> 3) & 7) : 0, hiwater);
            case 1:
                spr_draw(SPR_HEALTH, FLIP_X, BAR_OFFSET_X, HEALTH_OFFSET_Y - 8,
                         ((health >> 6) == 1) ? 7 - ((health >> 3) & 7) : 0, hiwater);
            case 0:
                spr_draw(SPR_HEALTH, FLIP_X, BAR_OFFSET_X, HEALTH_OFFSET_Y - 0,
                         ((health >> 6) == 0) ? 7 - ((health >> 3) & 7) : 0, hiwater);
        }
    }

    if (power > 0) {
        switch (power >> 6) {
            case 3:
                spr_draw(SPR_POWER, FLIP_X, BAR_OFFSET_X, POWER_OFFSET_Y + 0,
                         ((power >> 6) == 3) ? 7 - ((power >> 3) & 7) : 0, hiwater);
            case 2:
                spr_draw(SPR_POWER, FLIP_X, BAR_OFFSET_X, POWER_OFFSET_Y + 8,
                         ((power >> 6) == 2) ? 7 - ((power >> 3) & 7) : 0, hiwater);
            case 1:
                spr_draw(SPR_POWER, FLIP_X, BAR_OFFSET_X, POWER_OFFSET_Y + 16,
                         ((power >> 6) == 1) ? 7 - ((power >> 3) & 7) : 0, hiwater);
            case 0:
                spr_draw(SPR_POWER, FLIP_X, BAR_OFFSET_X, POWER_OFFSET_Y + 24,
                         ((power >> 6) == 0) ? 7 - ((power >> 3) & 7) : 0, hiwater);
        }
    }
}

static void show_explosion(uint16_t power) {
    snd_music_off();
    snd_note_off();
    sample_play(SFX_EXPL_SHIP);

    for (uint8_t n = 0; n < (4 * 4 * 4); n++) {
        uint8_t hiwater = SPR_NUM_START;
        status(0, power, &hiwater);
        if (n < (4 * 4)) {
            spr_draw(SPR_EXPL, FLIP_NONE, 0, 0, n >> 2, &hiwater);
        }
        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);
        vsync();
    }
}

void game_get_mp_state(void) BANKED {
    static struct mp_player_state mps;

    // TODO pass own pos to mp

    // TODO scale?
    mps.spd_x = game_state.spd_x;
    mps.spd_y = game_state.spd_y;

    mps.rot = game_state.rot;
    mp_new_state(&mps);
}

void game_set_mp_player2(struct mp_player_state *state) BANKED {
    // TODO update p2 pos
}

void game_set_mp_shot(struct mp_shot_state *state) BANKED {
    // TODO add shot
}

static void get_max_spd(int16_t *max_spd_x, int16_t *max_spd_y) NONBANKED {
    START_ROM_BANK(BANK(table_speed_move)) {
        *max_spd_x = table_speed_move[(game_state.rot * table_speed_move_WIDTH) + 0];
        *max_spd_y = -table_speed_move[(game_state.rot * table_speed_move_WIDTH) + 1];
    } END_ROM_BANK;
}

static void get_shot_spd(int16_t *shot_spd_x, int16_t *shot_spd_y) NONBANKED {
    START_ROM_BANK(BANK(table_speed_shot)) {
        *shot_spd_x = table_speed_shot[(game_state.rot * table_speed_move_WIDTH) + 0];
        *shot_spd_y = -table_speed_shot[(game_state.rot * table_speed_move_WIDTH) + 1];
    } END_ROM_BANK;
}

void game_init(void) BANKED {
    game_state.spd_x = 0;
    game_state.spd_y = 0;
    game_state.rot = 0;
    game_state.health = HEALTH_MAX;
    game_state.power = POWER_MAX;
    game_state.score = 0;
    memset(&obj_state, 0, sizeof(struct obj_state));
}

uint8_t game(enum GAME_MODE mode) BANKED {
    snd_music_off();
    snd_note_off();

    disable_interrupts();
    DISPLAY_OFF;
    map_load(0);
    map_fill(MAP_GAME_1 + conf_get()->game_bg, 1);
    SHOW_BKG;
    spr_init_pal();
    SHOW_SPRITES;
    SPRITES_8x8;

    frame_count = 0;
    fps_count = 0;
    prev_fps_start = 0;

    if (mode == GM_SINGLE) {
        if (!(conf_get()->debug_flags & DBG_NO_OBJ)) {
            obj_spawn();
        }
    }

    uint8_t x_off = win_game_draw(game_state.score, 1);
    move_win(MINWNDPOSX + DEVICE_SCREEN_PX_WIDTH - x_off,
             MINWNDPOSY + DEVICE_SCREEN_PX_HEIGHT - 16);

    SHOW_WIN;
    DISPLAY_ON;
    enable_interrupts();

    snd_music(SND_GAME);

    uint8_t return_value = 0xFF;
    while(1) {
        key_read();

        if (mode != GM_SINGLE) {
            mp_handle();
        }

        enum ACCELERATION acc = 0;
        int32_t prev_score = game_state.score;

        if (key_pressed(J_LEFT)) {
            game_state.rot = (game_state.rot - 1) & (ROT_INVALID - 1);
            acc |= ACC_R;
        } else if (key_pressed(J_RIGHT)) {
            game_state.rot = (game_state.rot + 1) & (ROT_INVALID - 1);
            acc |= ACC_R;
        }

        if (key_down(J_A) && (game_state.power > 0)) {
            int16_t max_spd_x;
            int16_t max_spd_y;
            get_max_spd(&max_spd_x, &max_spd_y);

            if (conf_get()->debug_flags & DBG_FAST) {
                if (max_spd_x > 0) {
                    max_spd_x = SPEED_MAX_DBG;
                } else if (max_spd_x < 0) {
                    max_spd_x = -SPEED_MAX_DBG;
                }

                if (max_spd_y > 0) {
                    max_spd_y = SPEED_MAX_DBG;
                } else if (max_spd_y < 0) {
                    max_spd_y = -SPEED_MAX_DBG;
                }
            }

            if (max_spd_x != 0) {
                if (max_spd_x > 0) {
                    game_state.spd_x += SPEED_INC;
                    if (game_state.spd_x > max_spd_x) {
                        game_state.spd_x = max_spd_x;
                    }
                } else {
                    game_state.spd_x -= SPEED_INC;
                    if (game_state.spd_x < max_spd_x) {
                        game_state.spd_x = max_spd_x;
                    }
                }

                acc |= ACC_X;
            }

            if (max_spd_y != 0) {
                if (max_spd_y > 0) {
                    game_state.spd_y += SPEED_INC;
                    if (game_state.spd_y > max_spd_y) {
                        game_state.spd_y = max_spd_y;
                    }
                } else {
                    game_state.spd_y -= SPEED_INC;
                    if (game_state.spd_y < max_spd_y) {
                        game_state.spd_y = max_spd_y;
                    }
                }

                acc |= ACC_Y;
            }

            if (!(conf_get()->debug_flags & DBG_NO_FUEL)) {
                if (game_state.power >= POWER_DEC) {
                    game_state.power -= POWER_DEC;
                } else {
                    game_state.power = 0;
                }
            }
        } else if (!key_down(J_A) && (game_state.power < POWER_MAX)) {
            if (game_state.power <= (POWER_MAX - POWER_INC)) {
                game_state.power += POWER_INC;
            } else {
                game_state.power = POWER_MAX;
            }
        }

        // adjust speed down when not moving
        if (!(acc & ACC_X)) {
            if (game_state.spd_x != 0) {
                if (!(conf_get()->debug_flags & DBG_FAST)) {
                    if (game_state.spd_x > SPEED_MAX_IDLE) game_state.spd_x -= SPEED_DEC;
                    else if (game_state.spd_x < -SPEED_MAX_IDLE) game_state.spd_x += SPEED_DEC;
                } else {
                    game_state.spd_x = 0;
                }
            }
        }
        if (!(acc & ACC_Y)) {
            if (game_state.spd_y != 0) {
                if (!(conf_get()->debug_flags & DBG_FAST)) {
                    if (game_state.spd_y > SPEED_MAX_IDLE) game_state.spd_y -= SPEED_DEC;
                    else if (game_state.spd_y < -SPEED_MAX_IDLE) game_state.spd_y += SPEED_DEC;
                } else {
                    game_state.spd_y = 0;
                }
            }
        }

        if (key_pressed(J_B)) {
            int16_t shot_spd_x;
            int16_t shot_spd_y;
            get_shot_spd(&shot_spd_x, &shot_spd_y);
            shot_spd_x += game_state.spd_x;
            shot_spd_y += game_state.spd_y;

            int16_t shot_pos_x = table_shot_offsets[(game_state.rot * 2) + 0];
            int16_t shot_pos_y = table_shot_offsets[(game_state.rot * 2) + 1];

            int8_t ret = obj_add(SPR_SHOT,
                                 shot_pos_x, shot_pos_y,
                                 shot_spd_x, shot_spd_y);

            if (ret == OBJ_ADDED) {
                sample_play(SFX_SHOT);

                if (mode == GM_SINGLE) {
                    if (game_state.score > 0) {
                        game_state.score--;
                    }
                } else {
                    static struct mp_shot_state state;

                    // TODO send absolute coordinate
                    state.pos_x = shot_pos_x;
                    state.pos_y = shot_pos_y;

                    // TODO scale?
                    state.spd_x = shot_spd_x;
                    state.spd_y = shot_spd_y;

                    mp_add_shot(&state);
                }
            }
        }

        if (key_pressed(J_START)) {
            if (pause_screen()) {
                return_value = 1;
                break;
            }

            // restart bg music
            snd_music(SND_GAME);
        }

        if (key_pressed(J_SELECT) && conf_get()->debug_flags) {
            map_dbg_reset();
        }

        map_move(game_state.spd_x, game_state.spd_y);

        uint8_t hiwater = SPR_NUM_START;
        status(game_state.health >> HEALTH_SHIFT, game_state.power >> POWER_SHIFT, &hiwater);

        if (conf_get()->debug_flags & DBG_MARKER) {
            spr_draw(SPR_DEBUG, FLIP_NONE, 0, 0, 0, &hiwater);
            spr_draw(SPR_DEBUG_LARGE, FLIP_NONE, 0, 0, 0, &hiwater);
        }

        spr_ship(game_state.rot, acc & (ACC_X | ACC_Y), &hiwater);

        int16_t damage = obj_do(&game_state.spd_x, &game_state.spd_y, &game_state.score, &hiwater,
                                (conf_get()->debug_flags & DBG_NO_OBJ) ? 1 : 0);
        if (damage > 0) {
            if (conf_get()->debug_flags & DBG_GOD_MODE) {
                damage = 0;
            }

            if (game_state.health > damage) {
                game_state.health -= damage;
                if ((!sample_running()) && (sample_last() != SFX_DAMAGE)) {
                    sample_play(SFX_DAMAGE);
                }
            } else if (game_state.health <= damage) {
                game_state.health = 0;
                show_explosion(game_state.power);
                return_value = 0;
                break;
            }
        } else if ((damage < 0) && (game_state.health < HEALTH_MAX)) {
            if ((!sample_running()) && (sample_last() != SFX_HEAL)) {
                sample_play(SFX_HEAL);
            }

            game_state.health += -damage;
            if (game_state.health > HEALTH_MAX) {
                game_state.health = HEALTH_MAX;
            }
        } else if (damage == 0) {
            sample_last_reset();
        }

        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        if ((game_state.score != prev_score)
                || (conf_get()->debug_flags & DBG_OUT_ON)) {
            uint8_t x_off = win_game_draw(game_state.score, 0);
            move_win(MINWNDPOSX + DEVICE_SCREEN_PX_WIDTH - x_off,
                     MINWNDPOSY + DEVICE_SCREEN_PX_HEIGHT - 16);
        }

        calc_fps();
        vsync();
    }

    return return_value;
}
