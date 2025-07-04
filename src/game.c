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

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <rand.h>
#include <stdint.h>

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
#include "game.h"

#define BAR_OFFSET_X (4 - 80)
#define HEALTH_OFFSET_Y -16
#define POWER_OFFSET_Y 16
#define PAUSE_BLINK_FRAMES 32

#define SPEED_INC 1
#define SPEED_DEC 1

#define SPEED_MAX_ACC 23
#define SPEED_MAX_ACC_DIAG 16
#define SPEED_MAX_ACC_D_LO 9
#define SPEED_MAX_ACC_D_HI 21
#define SPEED_MAX_IDLE 16
#define SPEED_MAX_DBG 256

#define POWER_MAX 0x1FF
#define POWER_SHIFT 1

#define POWER_INC 2
#define POWER_DEC 4

BANKREF(game)

enum ACCELERATION {
    ACC_X = 1,
    ACC_Y = 2,
    ACC_R = 4,
};

static int16_t spd_x = 0, spd_y = 0;
static enum SPRITE_ROT rot = 0;
static enum ACCELERATION acc = 0;
static uint16_t health = HEALTH_MAX;
static uint16_t power = POWER_MAX;
static int32_t score = 0;

static uint8_t pause_screen(void) NONBANKED {
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

        vsync();
    }

    return 0;
}

static void status(uint8_t health, uint8_t power, uint8_t *hiwater) NONBANKED {
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

static void show_explosion(uint16_t power) NONBANKED {
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

void game_get_mp_state(void) NONBANKED {
    static struct mp_player_state state;

    // TODO pass own pos to mp

    // TODO scale?
    state.spd_x = spd_x;
    state.spd_y = spd_y;

    state.rot = rot;
    mp_new_state(&state);
}

void game_set_mp_player2(struct mp_player_state *state) NONBANKED {
    // TODO update p2 pos
}

void game_set_mp_shot(struct mp_shot_state *state) NONBANKED {
    // TODO add shot
}

void handle_acceleration(void) BANKED {
    switch (rot) {
        case ROT_0:
            spd_y -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y < -SPEED_MAX_ACC) spd_y = -SPEED_MAX_ACC;
            } else {
                if (spd_y < -SPEED_MAX_DBG) spd_y = -SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            break;

        case ROT_22_5:
            spd_y -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y < -SPEED_MAX_ACC_D_HI) spd_y = -SPEED_MAX_ACC_D_HI;
            } else {
                if (spd_y < -SPEED_MAX_DBG) spd_y = -SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            spd_x += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x > SPEED_MAX_ACC_D_LO) spd_x = SPEED_MAX_ACC_D_LO;
            } else {
                if (spd_x > SPEED_MAX_DBG) spd_x = SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            break;

        case ROT_45:
            spd_y -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y < -SPEED_MAX_ACC_DIAG) spd_y = -SPEED_MAX_ACC_DIAG;
            } else {
                if (spd_y < -SPEED_MAX_DBG) spd_y = -SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            spd_x += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x > SPEED_MAX_ACC_DIAG) spd_x = SPEED_MAX_ACC_DIAG;
            } else {
                if (spd_x > SPEED_MAX_DBG) spd_x = SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            break;

        case ROT_67_5:
            spd_y -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y < -SPEED_MAX_ACC_D_LO) spd_y = -SPEED_MAX_ACC_D_LO;
            } else {
                if (spd_y < -SPEED_MAX_DBG) spd_y = -SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            spd_x += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x > SPEED_MAX_ACC_D_HI) spd_x = SPEED_MAX_ACC_D_HI;
            } else {
                if (spd_x > SPEED_MAX_DBG) spd_x = SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            break;

        case ROT_90:
            spd_x += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x > SPEED_MAX_ACC) spd_x = SPEED_MAX_ACC;
            } else {
                if (spd_x > SPEED_MAX_DBG) spd_x = SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            break;

        case ROT_112_5:
            spd_x += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x > SPEED_MAX_ACC_D_HI) spd_x = SPEED_MAX_ACC_D_HI;
            } else {
                if (spd_x > SPEED_MAX_DBG) spd_x = SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            spd_y += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y > SPEED_MAX_ACC_D_LO) spd_y = SPEED_MAX_ACC_D_LO;
            } else {
                if (spd_y > SPEED_MAX_DBG) spd_y = SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            break;

        case ROT_135:
            spd_x += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x > SPEED_MAX_ACC_DIAG) spd_x = SPEED_MAX_ACC_DIAG;
            } else {
                if (spd_x > SPEED_MAX_DBG) spd_x = SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            spd_y += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y > SPEED_MAX_ACC_DIAG) spd_y = SPEED_MAX_ACC_DIAG;
            } else {
                if (spd_y > SPEED_MAX_DBG) spd_y = SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            break;

        case ROT_157_5:
            spd_x += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x > SPEED_MAX_ACC_D_LO) spd_x = SPEED_MAX_ACC_D_LO;
            } else {
                if (spd_x > SPEED_MAX_DBG) spd_x = SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            spd_y += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y > SPEED_MAX_ACC_D_HI) spd_y = SPEED_MAX_ACC_D_HI;
            } else {
                if (spd_y > SPEED_MAX_DBG) spd_y = SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            break;

        case ROT_180:
            spd_y += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y > SPEED_MAX_ACC) spd_y = SPEED_MAX_ACC;
            } else {
                if (spd_y > SPEED_MAX_DBG) spd_y = SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            break;

        case ROT_202_5:
            spd_y += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y > SPEED_MAX_ACC_D_HI) spd_y = SPEED_MAX_ACC_D_HI;
            } else {
                if (spd_y > SPEED_MAX_DBG) spd_y = SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            spd_x -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x < -SPEED_MAX_ACC_D_LO) spd_x = -SPEED_MAX_ACC_D_LO;
            } else {
                if (spd_x < -SPEED_MAX_DBG) spd_x = -SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            break;

        case ROT_225:
            spd_y += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y > SPEED_MAX_ACC_DIAG) spd_y = SPEED_MAX_ACC_DIAG;
            } else {
                if (spd_y > SPEED_MAX_DBG) spd_y = SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            spd_x -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x < -SPEED_MAX_ACC_DIAG) spd_x = -SPEED_MAX_ACC_DIAG;
            } else {
                if (spd_x < -SPEED_MAX_DBG) spd_x = -SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            break;

        case ROT_247_5:
            spd_y += SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y > SPEED_MAX_ACC_D_LO) spd_y = SPEED_MAX_ACC_D_LO;
            } else {
                if (spd_y > SPEED_MAX_DBG) spd_y = SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            spd_x -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x < -SPEED_MAX_ACC_D_HI) spd_x = -SPEED_MAX_ACC_D_HI;
            } else {
                if (spd_x < -SPEED_MAX_DBG) spd_x = -SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            break;

        case ROT_270:
            spd_x -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x < -SPEED_MAX_ACC) spd_x = -SPEED_MAX_ACC;
            } else {
                if (spd_x < -SPEED_MAX_DBG) spd_x = -SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            break;

        case ROT_292_5:
            spd_x -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x < -SPEED_MAX_ACC_D_HI) spd_x = -SPEED_MAX_ACC_D_HI;
            } else {
                if (spd_x < -SPEED_MAX_DBG) spd_x = -SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            spd_y -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y < -SPEED_MAX_ACC_D_LO) spd_y = -SPEED_MAX_ACC_D_LO;
            } else {
                if (spd_y < -SPEED_MAX_DBG) spd_y = -SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            break;

        case ROT_315:
            spd_x -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x < -SPEED_MAX_ACC_DIAG) spd_x = -SPEED_MAX_ACC_DIAG;
            } else {
                if (spd_x < -SPEED_MAX_DBG) spd_x = -SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            spd_y -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y < -SPEED_MAX_ACC_DIAG) spd_y = -SPEED_MAX_ACC_DIAG;
            } else {
                if (spd_y < -SPEED_MAX_DBG) spd_y = -SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            break;

        case ROT_337_5:
            spd_x -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_x < -SPEED_MAX_ACC_D_LO) spd_x = -SPEED_MAX_ACC_D_LO;
            } else {
                if (spd_x < -SPEED_MAX_DBG) spd_x = -SPEED_MAX_DBG;
            }
            acc |= ACC_X;
            spd_y -= SPEED_INC;
            if (!(conf_get()->debug_flags & DBG_FAST)) {
                if (spd_y < -SPEED_MAX_ACC_D_HI) spd_y = -SPEED_MAX_ACC_D_HI;
            } else {
                if (spd_y < -SPEED_MAX_DBG) spd_y = -SPEED_MAX_DBG;
            }
            acc |= ACC_Y;
            break;
    }
}

int32_t game(enum GAME_MODE mode) NONBANKED {
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

    spd_x = 0;
    spd_y = 0;
    rot = 0;
    health = HEALTH_MAX;
    power = POWER_MAX;
    score = 0;

    obj_init();

    if (mode == GM_SINGLE) {
        if (!(conf_get()->debug_flags & DBG_NO_OBJ)) {
            obj_spawn();
        }
    }

    uint8_t x_off = win_game_draw(score);
    move_win(MINWNDPOSX + DEVICE_SCREEN_PX_WIDTH - x_off, MINWNDPOSY + DEVICE_SCREEN_PX_HEIGHT - 16);

    SHOW_WIN;
    DISPLAY_ON;
    enable_interrupts();

    snd_music(SND_GAME);

    while(1) {
        key_read();

        if (mode != GM_SINGLE) {
            mp_handle();
        }

        acc = 0;
        int32_t prev_score = score;

        if (key_pressed(J_LEFT)) {
            rot = (rot - 1) & (ROT_INVALID - 1);
            acc |= ACC_R;
        } else if (key_pressed(J_RIGHT)) {
            rot = (rot + 1) & (ROT_INVALID - 1);
            acc |= ACC_R;
        }

        if (key_down(J_A) && (power > 0)) {
            handle_acceleration();

            if (!(conf_get()->debug_flags & DBG_NO_FUEL)) {
                if (power >= POWER_DEC) {
                    power -= POWER_DEC;
                } else {
                    power = 0;
                }
            }
        } else if (!key_down(J_A) && (power < POWER_MAX)) {
            if (power <= (POWER_MAX - POWER_INC)) {
                power += POWER_INC;
            } else {
                power = POWER_MAX;
            }
        }

        // adjust speed down when not moving
        if (!(acc & ACC_X)) {
            if (spd_x != 0) {
                if (!(conf_get()->debug_flags & DBG_FAST)) {
                    if (spd_x > SPEED_MAX_IDLE) spd_x -= SPEED_DEC;
                    else if (spd_x < -SPEED_MAX_IDLE) spd_x += SPEED_DEC;
                } else {
                    spd_x = 0;
                }
            }
        }
        if (!(acc & ACC_Y)) {
            if (spd_y != 0) {
                if (!(conf_get()->debug_flags & DBG_FAST)) {
                    if (spd_y > SPEED_MAX_IDLE) spd_y -= SPEED_DEC;
                    else if (spd_y < -SPEED_MAX_IDLE) spd_y += SPEED_DEC;
                } else {
                    spd_y = 0;
                }
            }
        }

        if (key_pressed(J_B)) {
            int16_t shot_pos_x = 0, shot_pos_y = 0;
            int16_t shot_spd_x = 0, shot_spd_y = 0;

            switch (rot) {
                case ROT_0:
                    shot_pos_x = 0;
                    shot_pos_y = -SHIP_OFF;
                    shot_spd_x = spd_x;
                    shot_spd_y = spd_y - SHOT_SPEED;
                    break;

                case ROT_22_5:
                    shot_pos_x = SHIP_OFF / 2 - 1;
                    shot_pos_y = -SHIP_OFF / 2 - 4;
                    shot_spd_x = spd_x + SHOT_SPEED_D_LO;
                    shot_spd_y = spd_y - SHOT_SPEED_D_HI;
                    break;

                case ROT_45:
                    shot_pos_x = SHIP_OFF / 2 + 3;
                    shot_pos_y = -SHIP_OFF / 2 - 2;
                    shot_spd_x = spd_x + SHOT_SPEED_DIAG;
                    shot_spd_y = spd_y - SHOT_SPEED_DIAG;
                    break;

                case ROT_67_5:
                    shot_pos_x = SHIP_OFF / 2 + 5;
                    shot_pos_y = -SHIP_OFF / 2 + 2;
                    shot_spd_x = spd_x + SHOT_SPEED_D_HI;
                    shot_spd_y = spd_y - SHOT_SPEED_D_LO;
                    break;

                case ROT_90:
                    shot_pos_x = SHIP_OFF;
                    shot_pos_y = 0;
                    shot_spd_x = spd_x + SHOT_SPEED;
                    shot_spd_y = spd_y;
                    break;

                case ROT_112_5:
                    shot_pos_x = SHIP_OFF / 2 + 5;
                    shot_pos_y = SHIP_OFF / 2 + 0;
                    shot_spd_x = spd_x + SHOT_SPEED_D_HI;
                    shot_spd_y = spd_y + SHOT_SPEED_D_LO;
                    break;

                case ROT_135:
                    shot_pos_x = SHIP_OFF / 2 + 3;
                    shot_pos_y = SHIP_OFF / 2 + 2;
                    shot_spd_x = spd_x + SHOT_SPEED_DIAG;
                    shot_spd_y = spd_y + SHOT_SPEED_DIAG;
                    break;

                case ROT_157_5:
                    shot_pos_x = SHIP_OFF / 2 + 1;
                    shot_pos_y = SHIP_OFF / 2 + 4;
                    shot_spd_x = spd_x + SHOT_SPEED_D_LO;
                    shot_spd_y = spd_y + SHOT_SPEED_D_HI;
                    break;

                case ROT_180:
                    shot_pos_x = 0;
                    shot_pos_y = SHIP_OFF;
                    shot_spd_x = spd_x;
                    shot_spd_y = spd_y + SHOT_SPEED;
                    break;

                case ROT_202_5:
                    shot_pos_x = -SHIP_OFF / 2 + 2;
                    shot_pos_y = SHIP_OFF / 2 + 3;
                    shot_spd_x = spd_x - SHOT_SPEED_D_LO;
                    shot_spd_y = spd_y + SHOT_SPEED_D_HI;
                    break;

                case ROT_225:
                    shot_pos_x = -SHIP_OFF / 2 - 3;
                    shot_pos_y = SHIP_OFF / 2 + 2;
                    shot_spd_x = spd_x - SHOT_SPEED_DIAG;
                    shot_spd_y = spd_y + SHOT_SPEED_DIAG;
                    break;

                case ROT_247_5:
                    shot_pos_x = -SHIP_OFF / 2 - 5;
                    shot_pos_y = SHIP_OFF / 2 - 1;
                    shot_spd_x = spd_x - SHOT_SPEED_D_HI;
                    shot_spd_y = spd_y + SHOT_SPEED_D_LO;
                    break;

                case ROT_270:
                    shot_pos_x = -SHIP_OFF;
                    shot_pos_y = 0;
                    shot_spd_x = spd_x - SHOT_SPEED;
                    shot_spd_y = spd_y;
                    break;

                case ROT_292_5:
                    shot_pos_x = -SHIP_OFF / 2 - 2;
                    shot_pos_y = -SHIP_OFF / 2 + 2;
                    shot_spd_x = spd_x - SHOT_SPEED_D_HI;
                    shot_spd_y = spd_y - SHOT_SPEED_D_LO;
                    break;

                case ROT_315:
                    shot_pos_x = -SHIP_OFF / 2 - 3;
                    shot_pos_y = -SHIP_OFF / 2 - 2;
                    shot_spd_x = spd_x - SHOT_SPEED_DIAG;
                    shot_spd_y = spd_y - SHOT_SPEED_DIAG;
                    break;

                case ROT_337_5:
                    shot_pos_x = -SHIP_OFF / 2 + 1;
                    shot_pos_y = -SHIP_OFF / 2 - 4;
                    shot_spd_x = spd_x - SHOT_SPEED_D_LO;
                    shot_spd_y = spd_y - SHOT_SPEED_D_HI;
                    break;
            }

            int8_t ret = obj_add(SPR_SHOT,
                                 shot_pos_x, shot_pos_y,
                                 shot_spd_x, shot_spd_y);

            if (ret == OBJ_ADDED) {
                sample_play(SFX_SHOT);

                if (mode == GM_SINGLE) {
                    if (score > 0) {
                        score--;
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
                break;
            }

            // restart bg music
            snd_music(SND_GAME);
        }

        if (key_pressed(J_SELECT) && conf_get()->debug_flags) {
            map_dbg_reset();
        }

        map_move(spd_x, spd_y);

        uint8_t hiwater = SPR_NUM_START;
        status(health >> HEALTH_SHIFT, power >> POWER_SHIFT, &hiwater);

        if (conf_get()->debug_flags & DBG_MARKER) {
            spr_draw(SPR_DEBUG, FLIP_NONE, 0, 0, 0, &hiwater);
            spr_draw(SPR_DEBUG_LARGE, FLIP_NONE, 0, 0, 0, &hiwater);
        }

        spr_ship(rot, acc & (ACC_X | ACC_Y), &hiwater);

        int16_t damage = obj_do(&spd_x, &spd_y, &score, &hiwater,
                                (conf_get()->debug_flags & DBG_NO_OBJ) ? 1 : 0);
        if (damage > 0) {
            if (conf_get()->debug_flags & DBG_GOD_MODE) {
                damage = 0;
            }

            if (health > damage) {
                health -= damage;
            } else if (health <= damage) {
                health = 0;
                show_explosion(power);
                break;
            }
        } else if (damage < 0) {
            health += -damage;
            if (health > HEALTH_MAX) {
                health = HEALTH_MAX;
            }
        }

        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        if (score != prev_score) {
            uint8_t x_off = win_game_draw(score);
            move_win(MINWNDPOSX + DEVICE_SCREEN_PX_WIDTH - x_off, MINWNDPOSY + DEVICE_SCREEN_PX_HEIGHT - 16);
        }

        vsync();
    }

    return score;
}
