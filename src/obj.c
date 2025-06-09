/*
 * obj.c
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
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <rand.h>

#include "sprites.h"
#include "game.h"
#include "obj.h"

/*
 * sprite budget:
 *
 * fixed:
 * status bars: 8
 * ship: 5
 * thruster: 1
 * --> 14 fixed
 *
 * hardware tiles: 40 - 14 = 26
 *
 * dynamic:
 * shot: 1
 * light: 4
 * dark: 4
 * --> 2x dark & 2x light = 16
 * --> 5x shot & 4x small = 9
 * --> 16 + 9 = 25
 */
#define MAX_DARK 2
#define MAX_LIGHT 2
#define MAX_SHOT 5
#define MAX_SHOT_DARK 2
#define MAX_SHOT_LIGHT 2
#define MAX_OBJ ((4 * MAX_DARK) + (4 * MAX_LIGHT) + MAX_SHOT + MAX_SHOT_DARK + MAX_SHOT_LIGHT)

#define MAX_TRAVEL 128

#define POS_SCALE_OBJS 5
#define POS_OBJS_MAX (INT16_MAX >> (8 - POS_SCALE_OBJS))
#define POS_OBJS_MIN (-(INT16_MAX >> (8 - POS_SCALE_OBJS)) - 1)

#define GRAVITY_RANGE (24 << POS_SCALE_OBJS)
#define GRAVITY_SHIFT (POS_SCALE_OBJS + 4)

#define DAMAGE_RANGE (14 << POS_SCALE_OBJS)
#define DAMAGE_INC 5

#define HEALTH_RANGE (12 << POS_SCALE_OBJS)
#define HEALTH_INC HEALTH_MAX

#define PICKUP_SMALL_RANGE (12 << POS_SCALE_OBJS)
#define SHOT_RANGE (10 << POS_SCALE_OBJS)

#define SCORE_SMALL 5
#define SCORE_LARGE 10

#define DESPAWN_RANGE (250 << POS_SCALE_OBJS)

#define PLACEMENT_DISTANCE 42

struct obj {
    uint8_t active;
    enum SPRITES sprite;
    int16_t off_x;
    int16_t off_y;
    int16_t spd_x;
    int16_t spd_y;
    uint8_t travel;
};

static struct obj objs[MAX_OBJ];
static uint8_t obj_cnt[SPRITE_COUNT];

void obj_init(void) NONBANKED {
    memset(objs, 0, sizeof(objs));
    memset(obj_cnt, 0, sizeof(obj_cnt));
}

static uint8_t is_too_close(int8_t x, int8_t y, uint8_t n, int8_t *x_c, int8_t *y_c) NONBANKED {
    for (uint8_t i = 0; i < n; i++) {
        int dst_x = abs(x_c[i] - x);
        int dst_y = abs(y_c[i] - y);
        if ((dst_x < PLACEMENT_DISTANCE) && (dst_y < PLACEMENT_DISTANCE)) {
            return 1;
        }
    }
    return 0;
}

static void generate_coords(uint8_t n, int8_t *x_c, int8_t *y_c) NONBANKED {
    int8_t x = 0;
    int8_t y = 0;

    do {
        x = arand();
        y = arand();
    } while (is_too_close(x, y, n, x_c, y_c));

    x_c[n] = x;
    y_c[n] = y;
}

void obj_spawn(void) NONBANKED {
    int8_t x_coords[MAX_DARK + MAX_LIGHT + MAX_SHOT_DARK + MAX_SHOT_LIGHT];
    int8_t y_coords[MAX_DARK + MAX_LIGHT + MAX_SHOT_DARK + MAX_SHOT_LIGHT];
    memset(x_coords, 0, sizeof(x_coords));
    memset(y_coords, 0, sizeof(y_coords));

    for (uint8_t i = 0; i < MAX_DARK; i++) {
        uint8_t n = i;
        generate_coords(n, x_coords, y_coords);
        obj_add(SPR_DARK, x_coords[n], y_coords[n], 0, 0);
    }
    for (uint8_t i = 0; i < MAX_LIGHT; i++) {
        uint8_t n = MAX_DARK + i;
        generate_coords(n, x_coords, y_coords);
        obj_add(SPR_LIGHT, x_coords[n], y_coords[n], 0, 0);
    }
    for (uint8_t i = 0; i < MAX_SHOT_DARK; i++) {
        uint8_t n = MAX_DARK + MAX_LIGHT + i;
        generate_coords(n, x_coords, y_coords);
        obj_add(SPR_SHOT_DARK, x_coords[n], y_coords[n], 0, 0);
    }
    for (uint8_t i = 0; i < MAX_SHOT_LIGHT; i++) {
        uint8_t n = MAX_DARK + MAX_LIGHT + MAX_SHOT_LIGHT + i;
        generate_coords(n, x_coords, y_coords);
        obj_add(SPR_SHOT_LIGHT, x_coords[n], y_coords[n], 0, 0);
    }
}

enum OBJ_STATE obj_add(enum SPRITES sprite, int16_t off_x, int16_t off_y, int16_t spd_x, int16_t spd_y) NONBANKED {
    uint8_t obj_cnt = 0xFF;
    for (uint8_t i = 0; i < MAX_OBJ; i++) {
        if (!objs[i].active) {
            obj_cnt = i;
            break;
        }
    }
    if (obj_cnt >= MAX_OBJ) {
        return OBJ_LIST_FULL;
    }

    objs[obj_cnt].active = 1;
    objs[obj_cnt].sprite = sprite;
    objs[obj_cnt].off_x = off_x << POS_SCALE_OBJS;
    objs[obj_cnt].off_y = off_y << POS_SCALE_OBJS;
    objs[obj_cnt].spd_x = spd_x;
    objs[obj_cnt].spd_y = spd_y;
    objs[obj_cnt].travel = 0;

    obj_cnt += 1;
    return OBJ_ADDED;
}

int16_t obj_do(int16_t *spd_off_x, int16_t *spd_off_y, int32_t *score, uint8_t *hiwater) NONBANKED {
    int16_t damage = 0;

    // initial speed
    int16_t spd_x = *spd_off_x;
    int16_t spd_y = *spd_off_y;

    for (uint8_t i = 0; i < MAX_OBJ; i++) {
        if (!objs[i].active) {
            continue;
        }

        // move objects by their speed and compensate for movement of the background / ship
        objs[i].off_x = objs[i].off_x + objs[i].spd_x - spd_x;
        objs[i].off_y = objs[i].off_y + objs[i].spd_y - spd_y;

        if (objs[i].off_x > POS_OBJS_MAX) {
            objs[i].off_x -= POS_OBJS_MAX - POS_OBJS_MIN + 1;
        } else if (objs[i].off_x < POS_OBJS_MIN) {
            objs[i].off_x += POS_OBJS_MAX - POS_OBJS_MIN + 1;
        }
        if (objs[i].off_y > POS_OBJS_MAX) {
            objs[i].off_y -= POS_OBJS_MAX - POS_OBJS_MIN + 1;
        } else if (objs[i].off_y < POS_OBJS_MIN) {
            objs[i].off_y += POS_OBJS_MAX - POS_OBJS_MIN + 1;
        }

        // only update travel time if we're actually moving
        if ((objs[i].spd_x != 0) || (objs[i].spd_y != 0)) {
            objs[i].travel += 1;
        }

        // remove objects that have traveled for too long
        if (objs[i].travel >= MAX_TRAVEL) {
            objs[i].active = 0;
            continue;
        }

        int abs_off_x = abs(objs[i].off_x);
        int abs_off_y = abs(objs[i].off_y);

        // handle collision
        switch (objs[i].sprite) {
            case SPR_DARK:
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    // TODO find new (random) position
                    //objs[i].active = 0;
                }

                if ((abs_off_x <= GRAVITY_RANGE) && (abs_off_y <= GRAVITY_RANGE)) {
                    if (objs[i].off_x > 0) {
                        *spd_off_x += (GRAVITY_RANGE - objs[i].off_x) >> GRAVITY_SHIFT;
                    } else if (objs[i].off_x < 0) {
                        *spd_off_x += (-GRAVITY_RANGE - objs[i].off_x) >> GRAVITY_SHIFT;
                    }
                    if (objs[i].off_y > 0) {
                        *spd_off_y += (GRAVITY_RANGE - objs[i].off_y) >> GRAVITY_SHIFT;
                    } else if (objs[i].off_y < 0) {
                        *spd_off_y += (-GRAVITY_RANGE - objs[i].off_y) >> GRAVITY_SHIFT;
                    }
                }

                if ((abs_off_x <= DAMAGE_RANGE) && (abs_off_y <= DAMAGE_RANGE)) {
                    damage += DAMAGE_INC;
                }
                break;

            case SPR_LIGHT:
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    // TODO find new (random) position
                    //objs[i].active = 0;
                }

                if ((abs_off_x <= GRAVITY_RANGE) && (abs_off_y <= GRAVITY_RANGE)) {
                    if (objs[i].off_x > 0) {
                        *spd_off_x -= (GRAVITY_RANGE - objs[i].off_x) >> GRAVITY_SHIFT;
                    } else if (objs[i].off_x < 0) {
                        *spd_off_x -= (-GRAVITY_RANGE - objs[i].off_x) >> GRAVITY_SHIFT;
                    }
                    if (objs[i].off_y > 0) {
                        *spd_off_y -= (GRAVITY_RANGE - objs[i].off_y) >> GRAVITY_SHIFT;
                    } else if (objs[i].off_y < 0) {
                        *spd_off_y -= (-GRAVITY_RANGE - objs[i].off_y) >> GRAVITY_SHIFT;
                    }
                }

                if ((abs_off_x <= HEALTH_RANGE) && (abs_off_y <= HEALTH_RANGE)) {
                    damage -= HEALTH_INC;
                }
                break;

            case SPR_SHOT_DARK:
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    // TODO find new (random) position
                    //objs[i].active = 0;
                }

                if ((abs_off_x <= PICKUP_SMALL_RANGE) && (abs_off_y <= PICKUP_SMALL_RANGE)) {
                    (*score) -= SCORE_SMALL;
                    objs[i].active = 0;
                }
                break;

            case SPR_SHOT_LIGHT:
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    // TODO find new (random) position
                    //objs[i].active = 0;
                }

                if ((abs_off_x <= PICKUP_SMALL_RANGE) && (abs_off_y <= PICKUP_SMALL_RANGE)) {
                    (*score) += SCORE_SMALL;
                    objs[i].active = 0;
                }
                break;

            case SPR_SHOT:
                for (uint8_t j = 0; j < MAX_OBJ; j++) {
                    if ((!objs[j].active) || ((objs[j].sprite != SPR_LIGHT) && (objs[j].sprite != SPR_DARK))) {
                        continue;
                    }

                    if ((abs(objs[i].off_x - objs[j].off_x) <= SHOT_RANGE)
                            && (abs(objs[i].off_y - objs[j].off_y) <= SHOT_RANGE)) {
                        objs[i].active = 0;
                        objs[j].active = 0;

                        if (objs[j].sprite == SPR_LIGHT) {
                            (*score) += SCORE_LARGE;
                        } else {
                            (*score) -= SCORE_LARGE;
                        }

                        break;
                    }
                }
                break;

            default:
                break;
        }

        spr_draw(objs[i].sprite, FLIP_NONE, objs[i].off_x >> POS_SCALE_OBJS, objs[i].off_y >> POS_SCALE_OBJS, 0, hiwater);
    }

    return damage;
}
