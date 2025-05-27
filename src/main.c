/*
 * main.c
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * Based on examples from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/cross-platform/metasprites/src/metasprites.c
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/galaxy/galaxy.c
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/rand/rand.c
 */

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>
#include <rand.h>
#include <stdint.h>

#include "maps.h"
#include "obj.h"
#include "sprites.h"

// The metasprite will be built starting with hardware sprite zero (the first)
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
#define SHOT_SPEED 8

static void splash(void) {
    disable_interrupts();
    DISPLAY_OFF;

    set_default_palette();
    map_title();

    SHOW_BKG;
    DISPLAY_ON;
    enable_interrupts();

    while(1) {
        KEY_INPUT;
        if (KEY_DOWN(0xFF)) {
            break;
        }
        vsync();
    }
}

static void game(void) {
    disable_interrupts();
    DISPLAY_OFF;

    set_default_palette();
    map_game();

    SHOW_BKG;

    spr_init();
    obj_init();

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

    // TODO remove
    obj_add(SPR_LIGHT, 64, 64, 0, 0);
    obj_add(SPR_DARK, -64, -64, 0, 0);

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

        if (KEY_DOWN(J_A)) {
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
        }

        if (KEY_PRESSED(J_B)) {
            switch (rot) {
                case ROT_0:
                    obj_add(SPR_SHOT, 0, -SHIP_OFF, 0, -SHOT_SPEED);
                break;

                case ROT_90:
                    obj_add(SPR_SHOT, SHIP_OFF, 0, SHOT_SPEED, 0);
                break;

                case ROT_180:
                    obj_add(SPR_SHOT, 0, SHIP_OFF, 0, SHOT_SPEED);
                break;

                case ROT_270:
                    obj_add(SPR_SHOT, -SHIP_OFF, 0, -SHOT_SPEED, 0);
                break;

                default:
                    break;
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
        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        prev_acc = acc;

        vsync();
    }
}

void main(void) {
#ifndef DEBUG
    splash();
#endif // DEBUG

    uint16_t seed = DIV_REG;
    waitpadup();
    seed |= ((uint16_t)DIV_REG) << 8;
    initarand(seed);

    game();
}
