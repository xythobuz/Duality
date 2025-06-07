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

#include "gb/gb.h"
#include "maps.h"
#include "obj.h"
#include "sprites.h"
#include "sound.h"
#include "input.h"
#include "game.h"

enum ACCELERATION {
    ACC_X = 1,
    ACC_Y = 2,
    ACC_R = 4,
};

#define BAR_OFFSET_X (4 - 80)
#define HEALTH_OFFSET_Y -16
#define POWER_OFFSET_Y 16
#define PAUSE_BLINK_FRAMES 32

static uint8_t pause_screen(void) {
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
    snd_explode();

    for (uint8_t n = 0; n < (4 * 4); n++) {
        uint8_t hiwater = SPR_NUM_START;
        spr_draw(SPR_EXPL, FLIP_NONE, 0, 0, n >> 2, &hiwater);

        // can't draw objects, we used the pallettes for the explosion
        //obj_draw(0, 0, &hiwater);

        status(0, power >> POWER_SHIFT, &hiwater);
        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);
        vsync();
    }
}

int32_t game(void) NONBANKED {
    disable_interrupts();
    DISPLAY_OFF;
    map_game();
    SHOW_BKG;
    spr_init_pal();
    SHOW_SPRITES;
    SPRITES_8x8;

    int16_t pos_x = 0;
    int16_t pos_y = 0;
    int16_t spd_x = 0;
    int16_t spd_y = 0;
    enum SPRITE_ROT rot = 0;
    enum ACCELERATION prev_acc = 0xFF; // so we draw the ship on the first frame
    uint8_t ship_hiwater = 0;
    uint16_t health = HEALTH_MAX;
    uint16_t power = POWER_MAX;
    int32_t score = 0;

    obj_init();

    // TODO remove
    obj_add(SPR_LIGHT, 64, 64, 0, 0);
    obj_add(SPR_DARK, -64, -64, 0, 0);
    obj_add(SPR_SHOT_LIGHT, 32, 32, 0, 0);
    obj_add(SPR_SHOT_DARK, -32, -32, 0, 0);

    win_init(0);
    uint8_t x_off = win_game_draw(score);
    move_win(MINWNDPOSX + DEVICE_SCREEN_PX_WIDTH - x_off, MINWNDPOSY + DEVICE_SCREEN_PX_HEIGHT - 16);

    SHOW_WIN;
    DISPLAY_ON;
    enable_interrupts();

    while(1) {
        key_read();

        enum ACCELERATION acc = 0;

        if (key_pressed(J_LEFT)) {
            rot = (rot - 1) & (ROT_INVALID - 1);
            acc |= ACC_R;
        } else if (key_pressed(J_RIGHT)) {
            rot = (rot + 1) & (ROT_INVALID - 1);
            acc |= ACC_R;
        }

        if (key_down(J_A) && (power > 0)) {
            if (power >= POWER_DEC) {
                power -= POWER_DEC;
            } else {
                power = 0;
            }

            switch (rot) {
                case ROT_0:
                    spd_y -= SPEED_INC;
                    if (spd_y < -SPEED_MAX_ACC) spd_y = -SPEED_MAX_ACC;
                    acc |= ACC_Y;
                    break;

                case ROT_45:
                    spd_y -= SPEED_INC;
                    if (spd_y < -SPEED_MAX_ACC) spd_y = -SPEED_MAX_ACC;
                    acc |= ACC_Y;
                    spd_x += SPEED_INC;
                    if (spd_x > SPEED_MAX_ACC) spd_x = SPEED_MAX_ACC;
                    acc |= ACC_X;
                    break;

                case ROT_90:
                    spd_x += SPEED_INC;
                    if (spd_x > SPEED_MAX_ACC) spd_x = SPEED_MAX_ACC;
                    acc |= ACC_X;
                    break;

                case ROT_135:
                    spd_x += SPEED_INC;
                    if (spd_x > SPEED_MAX_ACC) spd_x = SPEED_MAX_ACC;
                    acc |= ACC_X;
                    spd_y += SPEED_INC;
                    if (spd_y > SPEED_MAX_ACC) spd_y = SPEED_MAX_ACC;
                    acc |= ACC_Y;
                    break;

                case ROT_180:
                    spd_y += SPEED_INC;
                    if (spd_y > SPEED_MAX_ACC) spd_y = SPEED_MAX_ACC;
                    acc |= ACC_Y;
                    break;

                case ROT_225:
                    spd_y += SPEED_INC;
                    if (spd_y > SPEED_MAX_ACC) spd_y = SPEED_MAX_ACC;
                    acc |= ACC_Y;
                    spd_x -= SPEED_INC;
                    if (spd_x < -SPEED_MAX_ACC) spd_x = -SPEED_MAX_ACC;
                    acc |= ACC_X;
                    break;

                case ROT_270:
                    spd_x -= SPEED_INC;
                    if (spd_x < -SPEED_MAX_ACC) spd_x = -SPEED_MAX_ACC;
                    acc |= ACC_X;
                    break;

                case ROT_315:
                    spd_x -= SPEED_INC;
                    if (spd_x < -SPEED_MAX_ACC) spd_x = -SPEED_MAX_ACC;
                    acc |= ACC_X;
                    spd_y -= SPEED_INC;
                    if (spd_y < -SPEED_MAX_ACC) spd_y = -SPEED_MAX_ACC;
                    acc |= ACC_Y;
                    break;

                default:
                    break;
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
                if (spd_x > SPEED_MAX_IDLE) spd_x -= SPEED_DEC;
                else if (spd_x < -SPEED_MAX_IDLE) spd_x += SPEED_DEC;
            }
        }
        if (!(acc & ACC_Y)) {
            if (spd_y != 0) {
                if (spd_y > SPEED_MAX_IDLE) spd_y -= SPEED_DEC;
                else if (spd_y < -SPEED_MAX_IDLE) spd_y += SPEED_DEC;
            }
        }

        if (key_pressed(J_B)) {
            int8_t ret = -1;
            switch (rot) {
                case ROT_0:
                    ret = obj_add(SPR_SHOT, 0, -SHIP_OFF, spd_x, spd_y - SHOT_SPEED);
                    break;

                case ROT_45:
                    ret = obj_add(SPR_SHOT, SHIP_OFF / 2 + 3, -SHIP_OFF / 2 - 2, spd_x + SHOT_SPEED, spd_y - SHOT_SPEED);
                    break;

                case ROT_90:
                    ret = obj_add(SPR_SHOT, SHIP_OFF, 0, spd_x + SHOT_SPEED, spd_y);
                    break;

                case ROT_135:
                    ret = obj_add(SPR_SHOT, SHIP_OFF / 2 + 3, SHIP_OFF / 2 + 2, spd_x + SHOT_SPEED, spd_y + SHOT_SPEED);
                    break;

                case ROT_180:
                    ret = obj_add(SPR_SHOT, 0, SHIP_OFF, spd_x, spd_y + SHOT_SPEED);
                    break;

                case ROT_225:
                    ret = obj_add(SPR_SHOT, -SHIP_OFF / 2 - 3, SHIP_OFF / 2 + 2, spd_x - SHOT_SPEED, spd_y + SHOT_SPEED);
                    break;

                case ROT_270:
                    ret = obj_add(SPR_SHOT, -SHIP_OFF, 0, spd_x - SHOT_SPEED, spd_y);
                    break;

                case ROT_315:
                    ret = obj_add(SPR_SHOT, -SHIP_OFF / 2 - 3, -SHIP_OFF / 2 - 2, spd_x - SHOT_SPEED, spd_y - SHOT_SPEED);
                    break;

                default:
                    break;
            }

            if (ret == OBJ_ADDED) {
                snd_shot();
            }
        }

        // re-draw ship sprite when we've just rotated or are starting or stopping acceleration
        uint8_t redraw = (acc & ACC_R) || ((prev_acc & (ACC_X | ACC_Y)) != (acc & (ACC_X | ACC_Y)));

        if (key_pressed(J_START)) {
            if (pause_screen()) {
                break;
            }

            // re-draw ship sprite
            redraw = 1;
        }

        pos_x = (pos_x + spd_x) & POS_MASK_BG;
        pos_y = (pos_y + spd_y) & POS_MASK_BG;
        move_bkg(pos_x >> POS_SCALE_BG, pos_y >> POS_SCALE_BG);

        uint8_t hiwater = SPR_NUM_START;

#ifdef DEBUG
        spr_draw(SPR_DEBUG, FLIP_NONE, 0, 0, 0, &hiwater);
#endif

        if (redraw) {
            spr_ship(rot, acc & (ACC_X | ACC_Y), &hiwater);
            ship_hiwater = hiwater;
        } else {
            hiwater = ship_hiwater;
        }

        int32_t prev_score = score;
        int16_t damage = obj_do(&spd_x, &spd_y, &score, &hiwater);
        if (damage > 0) {
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

        status(health >> HEALTH_SHIFT, power >> POWER_SHIFT, &hiwater);

        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        prev_acc = acc;

        if (score != prev_score) {
            uint8_t x_off = win_game_draw(score);
            move_win(MINWNDPOSX + DEVICE_SCREEN_PX_WIDTH - x_off, MINWNDPOSY + DEVICE_SCREEN_PX_HEIGHT - 16);
        }

        vsync();
    }

    return score;
}
