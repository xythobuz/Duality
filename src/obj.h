/*
 * obj.h
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * Based on examples from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/cross-platform/metasprites/src/metasprites.c
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/galaxy/galaxy.c
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/rand/rand.c
 */

#ifndef __OBJ_H__
#define __OBJ_H__

#include <stdint.h>
#include "sprites.h"

#define OBJ_ADDED 0
#define OBJ_LIST_FULL -1

void obj_init(void);
int8_t obj_add(enum SPRITES sprite, int16_t off_x, int16_t off_y, int16_t spd_x, int16_t spd_y);
void obj_draw(int16_t spd_x, int16_t spd_y, uint8_t *hiwater);

#endif // __OBJ_H__
