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
#include <stdlib.h>
#include <rand.h>

#include "sprites.h"
#include "game.h"
#include "sample.h"
#include "obj.h"

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

#define CHECK_COL_AT_SHOTS

struct obj_state obj_state;

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

static uint8_t is_too_close(int8_t x, int8_t y, int8_t center_dist) {
    if ((abs(x) < center_dist) && (abs(y) < center_dist)) {
        return 1;
    }

    for (uint8_t i = 0; i < MAX_OBJ; i++) {
        if (!obj_state.objs[i].active) {
            continue;
        }

        int dst_x = abs((obj_state.objs[i].off_x >> POS_SCALE_OBJS) - x);
        int dst_y = abs((obj_state.objs[i].off_y >> POS_SCALE_OBJS) - y);

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

static void obj_respawn_type(enum SPRITES spr, int8_t center_dist) {
    while (obj_state.obj_cnt[spr] < obj_max[spr]) {
        int8_t x, y;
        generate_coords(&x, &y, center_dist);
        obj_add(spr, x, y, 0, 0);
    }
}

void obj_spawn(void) BANKED {
    obj_respawn_type(SPR_LIGHT, INITIAL_DISTANCE);
    obj_respawn_type(SPR_DARK, INITIAL_DISTANCE);
    obj_respawn_type(SPR_SHOT_LIGHT, INITIAL_DISTANCE);
    obj_respawn_type(SPR_SHOT_DARK, INITIAL_DISTANCE);
}

enum OBJ_STATE obj_add(enum SPRITES sprite, int16_t off_x, int16_t off_y, int16_t spd_x, int16_t spd_y) BANKED {
    uint8_t next = 0xFF;
    for (uint8_t i = 0; i < MAX_OBJ; i++) {
        if (!obj_state.objs[i].active) {
            next = i;
            break;
        }
    }
    if (next >= MAX_OBJ) {
        return OBJ_LIST_FULL;
    }

    if (obj_state.obj_cnt[sprite] >= obj_max[sprite]) {
        return OBJ_TYPE_FULL;
    }

    obj_state.obj_cnt[sprite]++;

    obj_state.objs[next].active = 1;
    obj_state.objs[next].sprite = sprite;
    obj_state.objs[next].off_x = off_x << POS_SCALE_OBJS;
    obj_state.objs[next].off_y = off_y << POS_SCALE_OBJS;
    obj_state.objs[next].spd_x = spd_x;
    obj_state.objs[next].spd_y = spd_y;
    obj_state.objs[next].travel = 0;
    obj_state.objs[next].frame = 0;
    obj_state.objs[next].frame_index = 0;
    obj_state.objs[next].frame_count = 1;
    obj_state.objs[next].frame_duration = 0;

    return OBJ_ADDED;
}

static uint8_t handle_shot_col(uint8_t shot, uint8_t orb, int32_t *score, uint8_t is_splash) {
    if ((abs(obj_state.objs[shot].off_x - obj_state.objs[orb].off_x) <= SHOT_RANGE)
            && (abs(obj_state.objs[shot].off_y - obj_state.objs[orb].off_y) <= SHOT_RANGE)) {
        sample_play(SFX_EXPL_ORB);

        obj_state.objs[orb].active = 0;

        obj_state.obj_cnt[obj_state.objs[shot].sprite]--;
        obj_state.obj_cnt[obj_state.objs[orb].sprite]--;

        obj_state.objs[shot].sprite = SPR_EXPL;
        obj_state.objs[shot].travel = 0;
        obj_state.objs[shot].frame = 0;
        obj_state.objs[shot].frame_index = 0;
        obj_state.objs[shot].frame_count = 4;
        obj_state.objs[shot].frame_duration = 4;
        obj_state.obj_cnt[SPR_EXPL]++;

        // move explosion to center of orb instead of shot
        obj_state.objs[shot].off_x = obj_state.objs[orb].off_x;
        obj_state.objs[shot].off_y = obj_state.objs[orb].off_y;

        // also would look kinda cool with shot speed still applied?
        obj_state.objs[shot].spd_x = 0;
        obj_state.objs[shot].spd_y = 0;

        if (!is_splash) {
            obj_respawn_type(obj_state.objs[orb].sprite, RESPAWN_DISTANCE);
        }

        if (obj_state.objs[orb].sprite == SPR_LIGHT) {
            (*score) += SCORE_LARGE;
        } else {
            (*score) -= SCORE_LARGE;
        }

        return 1;
    }

    return 0;
}

int16_t obj_do(int16_t *spd_off_x, int16_t *spd_off_y, int32_t *score, uint8_t *hiwater, uint8_t is_splash) BANKED {
    int16_t damage = 0;

    // initial speed
    int16_t spd_x = *spd_off_x;
    int16_t spd_y = *spd_off_y;

    for (uint8_t i = 0; i < MAX_OBJ; i++) {
        if (!obj_state.objs[i].active) {
            continue;
        }

        // move objects by their speed and compensate for movement of the background / ship
        obj_state.objs[i].off_x = obj_state.objs[i].off_x + obj_state.objs[i].spd_x - spd_x;
        obj_state.objs[i].off_y = obj_state.objs[i].off_y + obj_state.objs[i].spd_y - spd_y;

        if (obj_state.objs[i].off_x > POS_OBJS_MAX) {
            obj_state.objs[i].off_x -= POS_OBJS_MAX - POS_OBJS_MIN + 1;
        } else if (obj_state.objs[i].off_x < POS_OBJS_MIN) {
            obj_state.objs[i].off_x += POS_OBJS_MAX - POS_OBJS_MIN + 1;
        }
        if (obj_state.objs[i].off_y > POS_OBJS_MAX) {
            obj_state.objs[i].off_y -= POS_OBJS_MAX - POS_OBJS_MIN + 1;
        } else if (obj_state.objs[i].off_y < POS_OBJS_MIN) {
            obj_state.objs[i].off_y += POS_OBJS_MAX - POS_OBJS_MIN + 1;
        }

        // only update travel time if we're actually moving
        if ((obj_state.objs[i].spd_x != 0) || (obj_state.objs[i].spd_y != 0)) {
            obj_state.objs[i].travel += 1;
        }

        // remove objects that have traveled for too long
        if (obj_state.objs[i].travel >= MAX_TRAVEL) {
            obj_state.objs[i].active = 0;
            obj_state.obj_cnt[obj_state.objs[i].sprite]--;
            continue;
        }

        int abs_off_x = abs(obj_state.objs[i].off_x);
        int abs_off_y = abs(obj_state.objs[i].off_y);

        // handle collision
        switch (obj_state.objs[i].sprite) {
            case SPR_DARK:
#ifdef DESPAWN_RANGE
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    objs[i].active = 0;
                    obj_cnt[SPR_DARK]--;
                    obj_respawn_type(SPR_DARK, RESPAWN_DISTANCE);
                }
#endif // DESPAWN_RANGE

                if ((abs_off_x <= GRAVITY_RANGE) && (abs_off_y <= GRAVITY_RANGE)) {
                    if (obj_state.objs[i].off_x > 0) {
                        *spd_off_x += (GRAVITY_RANGE - obj_state.objs[i].off_x) >> GRAVITY_SHIFT;
                    } else if (obj_state.objs[i].off_x < 0) {
                        *spd_off_x += (-GRAVITY_RANGE - obj_state.objs[i].off_x) >> GRAVITY_SHIFT;
                    }
                    if (obj_state.objs[i].off_y > 0) {
                        *spd_off_y += (GRAVITY_RANGE - obj_state.objs[i].off_y) >> GRAVITY_SHIFT;
                    } else if (obj_state.objs[i].off_y < 0) {
                        *spd_off_y += (-GRAVITY_RANGE - obj_state.objs[i].off_y) >> GRAVITY_SHIFT;
                    }
                }

                if ((abs_off_x <= DAMAGE_RANGE) && (abs_off_y <= DAMAGE_RANGE)) {
                    damage += DAMAGE_INC;
                }

#ifndef CHECK_COL_AT_SHOTS
                for (uint8_t shot = 0; shot < MAX_OBJ; shot++) {
                    if ((!objs[shot].active) || (objs[shot].sprite != SPR_SHOT)) {
                        continue;
                    }

                    if (handle_shot_col(shot, i, score, is_splash)) {
                        break;
                    }
                }
#endif // ! CHECK_COL_AT_SHOTS
                break;

            case SPR_LIGHT:
#ifdef DESPAWN_RANGE
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    objs[i].active = 0;
                    obj_cnt[SPR_LIGHT]--;
                    obj_respawn_type(SPR_LIGHT, RESPAWN_DISTANCE);
                }
#endif // DESPAWN_RANGE

                if ((abs_off_x <= GRAVITY_RANGE) && (abs_off_y <= GRAVITY_RANGE)) {
                    if (obj_state.objs[i].off_x > 0) {
                        *spd_off_x -= (GRAVITY_RANGE - obj_state.objs[i].off_x) >> GRAVITY_SHIFT;
                    } else if (obj_state.objs[i].off_x < 0) {
                        *spd_off_x -= (-GRAVITY_RANGE - obj_state.objs[i].off_x) >> GRAVITY_SHIFT;
                    }
                    if (obj_state.objs[i].off_y > 0) {
                        *spd_off_y -= (GRAVITY_RANGE - obj_state.objs[i].off_y) >> GRAVITY_SHIFT;
                    } else if (obj_state.objs[i].off_y < 0) {
                        *spd_off_y -= (-GRAVITY_RANGE - obj_state.objs[i].off_y) >> GRAVITY_SHIFT;
                    }
                }

                if ((abs_off_x <= HEALTH_RANGE) && (abs_off_y <= HEALTH_RANGE)) {
                    damage -= HEALTH_INC;
                }

#ifndef CHECK_COL_AT_SHOTS
                for (uint8_t shot = 0; shot < MAX_OBJ; shot++) {
                    if ((!objs[shot].active) || (objs[shot].sprite != SPR_SHOT)) {
                        continue;
                    }

                    if (handle_shot_col(shot, i, score, is_splash)) {
                        break;
                    }
                }
#endif // ! CHECK_COL_AT_SHOTS
                break;

            case SPR_SHOT_DARK:
#ifdef DESPAWN_RANGE
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    objs[i].active = 0;
                    obj_cnt[SPR_SHOT_DARK]--;
                    obj_respawn_type(SPR_SHOT_DARK, RESPAWN_DISTANCE);
                }
#endif // DESPAWN_RANGE

                if ((abs_off_x <= PICKUP_SMALL_RANGE) && (abs_off_y <= PICKUP_SMALL_RANGE)) {
                    (*score) -= SCORE_SMALL;
                    obj_state.objs[i].active = 0;
                    obj_state.obj_cnt[SPR_SHOT_DARK]--;
                    obj_respawn_type(SPR_SHOT_DARK, RESPAWN_DISTANCE);
                }
                break;

            case SPR_SHOT_LIGHT:
#ifdef DESPAWN_RANGE
                if ((abs_off_x >= DESPAWN_RANGE) || (abs_off_y >= DESPAWN_RANGE)) {
                    objs[i].active = 0;
                    obj_cnt[SPR_SHOT_LIGHT]--;
                    obj_respawn_type(SPR_SHOT_LIGHT, RESPAWN_DISTANCE);
                }
#endif // DESPAWN_RANGE

                if ((abs_off_x <= PICKUP_SMALL_RANGE) && (abs_off_y <= PICKUP_SMALL_RANGE)) {
                    (*score) += SCORE_SMALL;
                    obj_state.objs[i].active = 0;
                    obj_state.obj_cnt[SPR_SHOT_LIGHT]--;
                    obj_respawn_type(SPR_SHOT_LIGHT, RESPAWN_DISTANCE);
                }
                break;

#ifdef CHECK_COL_AT_SHOTS
            case SPR_SHOT:
                for (uint8_t orb = 0; orb < MAX_OBJ; orb++) {
                    if ((!obj_state.objs[orb].active)
                            || ((obj_state.objs[orb].sprite != SPR_LIGHT)
                                && (obj_state.objs[orb].sprite != SPR_DARK))) {
                        continue;
                    }

                    if (handle_shot_col(i, orb, score, is_splash)) {
                        break;
                    }
                }
                break;
#endif // CHECK_COL_AT_SHOTS

            default:
                break;
        }

        if (!obj_state.objs[i].active) {
            continue;
        }

        spr_draw(obj_state.objs[i].sprite, FLIP_NONE,
                 obj_state.objs[i].off_x >> POS_SCALE_OBJS,
                 obj_state.objs[i].off_y >> POS_SCALE_OBJS,
                 obj_state.objs[i].frame_index, hiwater);

        obj_state.objs[i].frame++;
        if (obj_state.objs[i].frame >= obj_state.objs[i].frame_duration) {
            obj_state.objs[i].frame = 0;
            obj_state.objs[i].frame_index++;
            if (obj_state.objs[i].frame_index >= obj_state.objs[i].frame_count) {
                obj_state.objs[i].frame_index = 0;

                if (obj_state.objs[i].sprite == SPR_EXPL) {
                    obj_state.objs[i].active = 0;
                    obj_state.obj_cnt[SPR_EXPL]--;
                }
            }
        }
    }

    return damage;
}
