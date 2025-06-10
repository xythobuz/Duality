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

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <rand.h>

#include "sprites.h"
#include "game.h"
#include "sample.h"
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
#define DAMAGE_INC 4

#define HEALTH_RANGE (12 << POS_SCALE_OBJS)
#define HEALTH_INC HEALTH_MAX

#define PICKUP_SMALL_RANGE (12 << POS_SCALE_OBJS)
#define SHOT_RANGE (10 << POS_SCALE_OBJS)

#define SCORE_SMALL 5
#define SCORE_LARGE 10

//#define DESPAWN_RANGE (0x7F << POS_SCALE_OBJS)

#define INITIAL_DISTANCE 30 // from center
#define RESPAWN_DISTANCE 100 // from center
#define PLACEMENT_DISTANCE 42 // relative to each other

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

static const uint8_t obj_max[SPRITE_COUNT] = {
    1, // SPR_SHIP
    MAX_LIGHT, // SPR_LIGHT
    MAX_DARK, // SPR_DARK
    MAX_SHOT, // SPR_SHOT
    MAX_SHOT_LIGHT, // SPR_SHOT_LIGHT
    MAX_SHOT_DARK, // SPR_SHOT_DARK
    4, // SPR_HEALTH
    4, // SPR_POWER
    1, // SPR_EXPL
    1, // SPR_PAUSE
    1, // SPR_DEBUG
    1, // SPR_DEBUG_LARGE
};

void obj_init(void) BANKED {
    memset(objs, 0, sizeof(objs));
    memset(obj_cnt, 0, sizeof(obj_cnt));
}

static uint8_t is_too_close(int8_t x, int8_t y, int8_t center_dist) {
    if ((abs(x) < center_dist) && (abs(y) < center_dist)) {
        return 1;
    }

    for (uint8_t i = 0; i < MAX_OBJ; i++) {
        if (!objs[i].active) {
            continue;
        }

        int dst_x = abs((objs[i].off_x >> POS_SCALE_OBJS) - x);
        int dst_y = abs((objs[i].off_y >> POS_SCALE_OBJS) - y);

        if ((dst_x < PLACEMENT_DISTANCE) && (dst_y < PLACEMENT_DISTANCE)) {
            return 1;
        }
    }

    return 0;
}

static void generate_coords(int8_t *x_c, int8_t *y_c, int8_t center_dist) {
    int8_t x = 0;
    int8_t y = 0;

    do {
        x = arand();
        y = arand();
    } while (is_too_close(x, y, center_dist));

    *x_c = x;
    *y_c = y;
}

static void obj_respawn(int8_t center_dist) {
    for (uint8_t spr = SPR_LIGHT; spr <= SPR_SHOT_DARK; spr++) {
        if (spr == SPR_SHOT) {
            continue;
        }

        while (obj_cnt[spr] < obj_max[spr]) {
            int8_t x, y;
            generate_coords(&x, &y, center_dist);
            obj_add(spr, x, y, 0, 0);
        }
    }
}

void obj_spawn(void) BANKED {
    obj_respawn(INITIAL_DISTANCE);
}

enum OBJ_STATE obj_add(enum SPRITES sprite, int16_t off_x, int16_t off_y, int16_t spd_x, int16_t spd_y) BANKED {
    uint8_t next = 0xFF;
    for (uint8_t i = 0; i < MAX_OBJ; i++) {
        if (!objs[i].active) {
            next = i;
            break;
        }
    }
    if (next >= MAX_OBJ) {
        return OBJ_LIST_FULL;
    }

    if (obj_cnt[sprite] >= obj_max[sprite]) {
        return OBJ_TYPE_FULL;
    }

    obj_cnt[sprite]++;

    objs[next].active = 1;
    objs[next].sprite = sprite;
    objs[next].off_x = off_x << POS_SCALE_OBJS;
    objs[next].off_y = off_y << POS_SCALE_OBJS;
    objs[next].spd_x = spd_x;
    objs[next].spd_y = spd_y;
    objs[next].travel = 0;

    return OBJ_ADDED;
}

int16_t obj_do(int16_t *spd_off_x, int16_t *spd_off_y, int32_t *score, uint8_t *hiwater, uint8_t is_splash) BANKED {
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
            obj_cnt[objs[i].sprite]--;
            continue;
        }

        int abs_off_x = abs(objs[i].off_x);
        int abs_off_y = abs(objs[i].off_y);

        // handle collision
        switch (objs[i].sprite) {
            case SPR_DARK:
#ifdef DESPAWN_RANGE
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    objs[i].active = 0;
                    obj_cnt[objs[i].sprite]--;
                    obj_respawn(RESPAWN_DISTANCE);
                }
#endif // DESPAWN_RANGE

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
#ifdef DESPAWN_RANGE
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    objs[i].active = 0;
                    obj_cnt[objs[i].sprite]--;
                    obj_respawn(RESPAWN_DISTANCE);
                }
#endif // DESPAWN_RANGE

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
#ifdef DESPAWN_RANGE
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    objs[i].active = 0;
                    obj_cnt[objs[i].sprite]--;
                    obj_respawn(RESPAWN_DISTANCE);
                }
#endif // DESPAWN_RANGE

                if ((abs_off_x <= PICKUP_SMALL_RANGE) && (abs_off_y <= PICKUP_SMALL_RANGE)) {
                    (*score) -= SCORE_SMALL;
                    objs[i].active = 0;
                    obj_cnt[objs[i].sprite]--;
                    obj_respawn(RESPAWN_DISTANCE);
                }
                break;

            case SPR_SHOT_LIGHT:
#ifdef DESPAWN_RANGE
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    objs[i].active = 0;
                    obj_cnt[objs[i].sprite]--;
                    obj_respawn(RESPAWN_DISTANCE);
                }
#endif // DESPAWN_RANGE

                if ((abs_off_x <= PICKUP_SMALL_RANGE) && (abs_off_y <= PICKUP_SMALL_RANGE)) {
                    (*score) += SCORE_SMALL;
                    objs[i].active = 0;
                    obj_cnt[objs[i].sprite]--;
                    obj_respawn(RESPAWN_DISTANCE);
                }
                break;

            case SPR_SHOT:
                for (uint8_t j = 0; j < MAX_OBJ; j++) {
                    if ((!objs[j].active) || ((objs[j].sprite != SPR_LIGHT) && (objs[j].sprite != SPR_DARK))) {
                        continue;
                    }

                    if ((abs(objs[i].off_x - objs[j].off_x) <= SHOT_RANGE)
                            && (abs(objs[i].off_y - objs[j].off_y) <= SHOT_RANGE)) {
                        sample_play_explosion_orbs();

                        objs[i].active = 0;
                        objs[j].active = 0;

                        obj_cnt[objs[i].sprite]--;
                        obj_cnt[objs[j].sprite]--;

                        if (!is_splash) {
                            obj_respawn(RESPAWN_DISTANCE);
                        }

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

        if (!objs[i].active) {
            continue;
        }

        spr_draw(objs[i].sprite, FLIP_NONE, objs[i].off_x >> POS_SCALE_OBJS, objs[i].off_y >> POS_SCALE_OBJS, 0, hiwater);
    }

    return damage;
}
