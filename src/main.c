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

#include "maps.h"
#include "obj.h"
#include "sprites.h"
#include "sound.h"

#define SPR_NUM_START 0

static uint8_t joyp = 0;
static uint8_t old_joyp = 0;

#define KEY_INPUT (old_joyp = joyp, joyp = joypad())
#define KEY_DOWN(KEY) (joyp & (KEY))
#define KEY_PRESSED(KEY) ((joyp ^ old_joyp) & joyp & (KEY))

enum ACCELERATION {
    ACC_X = 1,
    ACC_Y = 2,
    ACC_R = 4,
};

#define SPEED_INC 1
#define SPEED_MAX 16
#define SHOT_SPEED 23

#define BAR_OFFSET_X (4 - 80)
#define HEALTH_OFFSET_Y -16
#define POWER_OFFSET_Y 16

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

    while(1) {
        KEY_INPUT;
        if (KEY_DOWN(0xFF)) {
            break;
        }

        uint8_t hiwater = SPR_NUM_START;
        obj_draw(0, 0, &hiwater);
        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        vsync();
    }
}

static void status(uint8_t health, uint8_t power, uint8_t *hiwater) {
    if (health > 0) {
        switch (health >> 6) {
            case 3:
                spr_draw(SPR_HEALTH, FLIP_X, BAR_OFFSET_X, HEALTH_OFFSET_Y - 24, hiwater);
            case 2:
                spr_draw(SPR_HEALTH, FLIP_X, BAR_OFFSET_X, HEALTH_OFFSET_Y - 16, hiwater);
            case 1:
                spr_draw(SPR_HEALTH, FLIP_X, BAR_OFFSET_X, HEALTH_OFFSET_Y - 8, hiwater);
            case 0:
                spr_draw(SPR_HEALTH, FLIP_X, BAR_OFFSET_X, HEALTH_OFFSET_Y - 0, hiwater);
        }
    }

    if (power > 0) {
        switch (power >> 6) {
            case 3:
                spr_draw(SPR_POWER, FLIP_X, BAR_OFFSET_X, POWER_OFFSET_Y + 0, hiwater);
            case 2:
                spr_draw(SPR_POWER, FLIP_X, BAR_OFFSET_X, POWER_OFFSET_Y + 8, hiwater);
            case 1:
                spr_draw(SPR_POWER, FLIP_X, BAR_OFFSET_X, POWER_OFFSET_Y + 16, hiwater);
            case 0:
                spr_draw(SPR_POWER, FLIP_X, BAR_OFFSET_X, POWER_OFFSET_Y + 24, hiwater);
        }
    }
}

static void game(void) {
    disable_interrupts();
    DISPLAY_OFF;
    map_game();
    SHOW_BKG;
    SHOW_SPRITES;
    SPRITES_8x8;
    DISPLAY_ON;
    enable_interrupts();

    int16_t PosX = 0;
    int16_t PosY = 0;
    int16_t SpdX = 0;
    int16_t SpdY = 0;
    enum SPRITE_ROT rot = 0;
    enum ACCELERATION prev_acc = 0xFF; // so we draw the ship on the first frame
    uint8_t ship_hiwater = 0;
    uint8_t health = 0xFF;
    uint8_t power = 0xFF;

    obj_init();

    // TODO remove
    obj_add(SPR_LIGHT, 64, 64, 0, 0);
    obj_add(SPR_DARK, -64, -64, 0, 0);
    obj_add(SPR_SHOT_LIGHT, 32, 32, 0, 0);
    obj_add(SPR_SHOT_DARK, -32, -32, 0, 0);

    while(1) {
        KEY_INPUT;

        enum ACCELERATION acc = 0;

        if (KEY_PRESSED(J_LEFT)) {
            rot = (rot - 1) & (ROT_INVALID - 1);
            acc |= ACC_R;
        } else if (KEY_PRESSED(J_RIGHT)) {
            rot = (rot + 1) & (ROT_INVALID - 1);
            acc |= ACC_R;
        }

        if (KEY_DOWN(J_A) && (power > 0)) {
            power--;
            switch (rot) {
                case ROT_0:
                    SpdY -= SPEED_INC;
                    if (SpdY < -SPEED_MAX) SpdY = -SPEED_MAX;
                    acc |= ACC_Y;
                    break;

                case ROT_90:
                    SpdX += SPEED_INC;
                    if (SpdX > SPEED_MAX) SpdX = SPEED_MAX;
                    acc |= ACC_X;
                    break;

                case ROT_180:
                    SpdY += SPEED_INC;
                    if (SpdY > SPEED_MAX) SpdY = SPEED_MAX;
                    acc |= ACC_Y;
                    break;

                case ROT_270:
                    SpdX -= SPEED_INC;
                    if (SpdX < -SPEED_MAX) SpdX = -SPEED_MAX;
                    acc |= ACC_X;
                    break;

                default:
                    break;
            }
        } else if (!KEY_DOWN(J_A) && (power < 0xFF)) {
            power++;
        }

        if (KEY_PRESSED(J_B)) {
            int8_t ret = -1;
            switch (rot) {
                case ROT_0:
                    ret = obj_add(SPR_SHOT, 0, -SHIP_OFF, SpdX, SpdY - SHOT_SPEED);
                break;

                case ROT_90:
                    ret = obj_add(SPR_SHOT, SHIP_OFF, 0, SpdX + SHOT_SPEED, SpdY);
                break;

                case ROT_180:
                    ret = obj_add(SPR_SHOT, 0, SHIP_OFF, SpdX, SpdY + SHOT_SPEED);
                break;

                case ROT_270:
                    ret = obj_add(SPR_SHOT, -SHIP_OFF, 0, SpdX - SHOT_SPEED, SpdY);
                break;

                default:
                    break;
            }

            if (ret == OBJ_ADDED) {
                snd_noise();
            }
        }

        PosX += SpdX;
        PosY += SpdY;
        move_bkg(PosX >> 4, PosY >> 4);

        uint8_t hiwater = SPR_NUM_START;

        // re-draw ship sprite when we've just rotated or are starting or stopping acceleration
        if ((acc & ACC_R) || ((prev_acc & (ACC_X | ACC_Y)) != (acc & (ACC_X | ACC_Y)))) {
            spr_ship(rot, acc & (ACC_X | ACC_Y), &hiwater);
            ship_hiwater = hiwater;
        } else {
            hiwater = ship_hiwater;
        }

        obj_draw(SpdX, SpdY, &hiwater);
        status(health, power, &hiwater);

        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        prev_acc = acc;

        vsync();
    }
}

void main(void) {
    spr_init();
    snd_init();

#ifndef DEBUG
    splash();
#endif // DEBUG

    uint16_t seed = DIV_REG;
    waitpadup();
    seed |= ((uint16_t)DIV_REG) << 8;
    initarand(seed);

    game();
}
