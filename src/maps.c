/*
 * maps.c
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * Based on examples from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/cross-platform/large_map
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

#include "banks.h"
#include "config.h"
#include "util.h"
#include "map_data.h"
#include "maps.h"

#define POS_SCALE_BG 6

// current unscaled ship position
static uint16_t abs_x, abs_y;

// define this to disable mirrored map scaling support
#define WRAP_BG // TODO

#ifndef WRAP_BG
#define bg_map_mapWidth (bg_map_WIDTH / bg_map_TILE_W)
#define bg_map_mapHeight (bg_map_HEIGHT / bg_map_TILE_H)

#define camera_max_x ((bg_map_mapWidth - DEVICE_SCREEN_WIDTH) * 8)
#define camera_max_y ((bg_map_mapHeight - DEVICE_SCREEN_HEIGHT) * 8)

#define MAP_FLIP_NONE 0x00
#define MAP_FLIP_X (0x20 | 0x01)
#define MAP_FLIP_Y (0x40 | 0x02)
#define MAP_FLIP_XY (MAP_FLIP_X | MAP_FLIP_Y)

// current and old positions of the camera in pixels
static uint16_t old_camera_x, old_camera_y;

// current and old position of the map in tiles
static uint8_t old_map_pos_x, old_map_pos_y;
#endif // ! WRAP_BG

BANKREF(maps)

static void map_load_helper(uint8_t i) NONBANKED {
    START_ROM_BANK(maps[i].bank) {
        if (maps[i].tile_copy == BG_COPY_NONE) {
            set_bkg_data(maps[i].tile_offset, maps[i].tile_count, maps[i].tiles);
        }
    } END_ROM_BANK

    if (_cpu == CGB_TYPE) {
        uint8_t bank = maps[i].bank;
        if (maps[i].palettes == num_pal_inv) {
            bank = BANK(map_data);
        }

        START_ROM_BANK_2(bank) {
            if (maps[i].palettes != NULL) {
                set_bkg_palette(maps[i].palette_index, maps[i].palette_count, maps[i].palettes);
            }
        } END_ROM_BANK
    }
}

void map_load(uint8_t is_splash) BANKED {
    uint8_t off = BG_TILE_NUM_START;
    uint8_t off_gbc = BG_TILE_NUM_START;

    for (uint8_t i = 0; i < MAP_COUNT; i++) {
        if (!(maps[i].load & BG_LOAD_ALL)) {
            if (is_splash) {
                if (!(maps[i].load & BG_LOAD_SPLASH)) {
                    continue;
                }
            } else {
                if (maps[i].load & BG_LOAD_SPLASH) {
                    continue;
                }
            }
        }

        if (_cpu != CGB_TYPE) {
            if (maps[i].load & BG_LOAD_GBC_ONLY) {
                continue;
            }
        }

        if (maps[i].tile_copy == BG_COPY_NONE) {
            if (maps[i].load & BG_LOAD_GBC_ONLY) {
                VBK_REG = VBK_BANK_1;
                maps[i].tile_offset = off_gbc;
                off_gbc += maps[i].tile_count;
            } else {
                maps[i].tile_offset = off;
                off += maps[i].tile_count;
            }
        } else {
            maps[i].tile_offset = maps[maps[i].tile_copy].tile_offset;
        }

        map_load_helper(i);
    }

    if (is_splash || (conf_get()->dmg_bg_inv == 0)) {
        BGP_REG = 0b11100100;
    } else {
        // invert BGP for DMG in-game
        BGP_REG = 0b00011011;
    }
}

void map_fill(enum MAPS map, uint8_t bkg) NONBANKED {
    START_ROM_BANK(maps[map].bank) {
        if (_cpu == CGB_TYPE) {
            VBK_REG = VBK_ATTRIBUTES;
            bkg ? fill_bkg_rect(0, 0, maps[map].width, maps[map].height, maps[map].palette_index)
                : fill_win_rect(0, 0, maps[map].width, maps[map].height, maps[map].palette_index);
        }

        VBK_REG = VBK_TILES;
        bkg ? set_bkg_based_tiles(0, 0, maps[map].width, maps[map].height, maps[map].map, maps[map].tile_offset)
            : set_win_based_tiles(0, 0, maps[map].width, maps[map].height, maps[map].map, maps[map].tile_offset);
    } END_ROM_BANK

    bkg ? move_bkg(0, 0)
        : move_win(MINWNDPOSX, MINWNDPOSY);
}

#ifndef WRAP_BG
static inline void set_bkg_sub_attr(uint8_t x, uint8_t y,
                                    uint8_t w, uint8_t h,
                                    const uint8_t *attr,
                                    uint8_t attr_val,
                                    uint8_t map_w) {
    if (attr) {
        set_bkg_submap_attributes(x, y, w, h, attr, map_w);
    } else {
        VBK_REG = VBK_ATTRIBUTES;
        fill_bkg_rect(x, y, w, h, attr_val);
        VBK_REG = VBK_TILES;
    }
}

static inline void set_bkg_sub(uint8_t x, uint8_t y,
                               uint8_t w, uint8_t h,
                               const uint8_t *map, const uint8_t *attr,
                               uint8_t attr_val,
                               uint8_t map_w) {
    START_ROM_BANK(BANK(bg_map)) {
        set_bkg_submap(x, y, w, h, map, map_w);
        set_bkg_sub_attr(x, y, w, h, attr, attr_val, map_w);
    } END_ROM_BANK
}
#endif // ! WRAP_BG

// TODO
#ifndef WRAP_BG
void map_game(void) NONBANKED {
    START_ROM_BANK(BANK(bg_map)) {
        abs_x = 0;
        abs_y = 0;
        old_camera_x = 0;
        old_camera_y = 0;
        old_map_pos_x = 0;
        old_map_pos_y = 0;

        move_bkg(0, 0);

        // Draw the initial map view for the whole screen
        set_bkg_sub(0, 0,
                    MIN(DEVICE_SCREEN_WIDTH + 1u, bg_map_mapWidth),
                    MIN(DEVICE_SCREEN_HEIGHT + 1u, bg_map_mapHeight),
                    bg_map_map, bg_map_MAP_ATTRIBUTES, MAP_FLIP_NONE, bg_map_mapWidth);
    } END_ROM_BANK
}
#endif // ! WRAP_BG

#ifndef WRAP_BG
static inline void set(uint8_t dst_x, uint8_t dst_y,
                       uint8_t src_x, uint8_t src_y,
                       uint8_t attr) {
    START_ROM_BANK(BANK(bg_map)) {
        set_bkg_tile_xy(dst_x, dst_y, bg_map_map[src_x + (src_y * (bg_map_WIDTH / bg_map_TILE_W))]);
        set_bkg_attribute_xy(dst_x, dst_y, attr);
    } END_ROM_BANK
}
#endif // ! WRAP_BG

void map_dbg_reset(void) NONBANKED {
#ifndef WRAP_BG
    uint16_t camera_x = abs_x >> POS_SCALE_BG;
    uint16_t camera_y = abs_y >> POS_SCALE_BG;
    uint8_t map_pos_x = camera_x >> 3;
    uint8_t map_pos_y = camera_y >> 3;
    for (uint8_t x = 0; x < DEVICE_SCREEN_WIDTH; x++) {
        for (uint8_t y = 0; y < DEVICE_SCREEN_HEIGHT; y++) {
            uint8_t is_flipped_x = ((camera_x >> 3) + x) & 0x10;
            uint8_t is_flipped_y = ((camera_y >> 3) + y) & 0x10;
            uint8_t attr = is_flipped_y ? (is_flipped_x ? MAP_FLIP_XY : MAP_FLIP_Y)
                                        : (is_flipped_x ? MAP_FLIP_X : MAP_FLIP_NONE);
            set(x, y,
                is_flipped_x ? bg_map_mapWidth - map_pos_x : map_pos_x,
                is_flipped_y ? bg_map_mapHeight - map_pos_y : map_pos_y,
                attr);
        }
    }
#endif // ! WRAP_BG
}

void map_move(int16_t delta_x, int16_t delta_y) NONBANKED {
    abs_x += delta_x;
    abs_y += delta_y;

    uint16_t camera_x = abs_x >> POS_SCALE_BG;
    uint16_t camera_y = abs_y >> POS_SCALE_BG;

    move_bkg(camera_x, camera_y);

#ifndef WRAP_BG

    uint8_t map_pos_x = camera_x >> 3;
    uint8_t map_pos_y = camera_y >> 3;

    uint8_t is_flipped_x_left = (camera_x >> 3) & 0x10;
    uint8_t is_flipped_x_right = ((camera_x >> 3) + DEVICE_SCREEN_WIDTH) & 0x10;
    uint8_t is_flipped_y_top = (camera_y >> 3) & 0x10;
    uint8_t is_flipped_y_bottom = ((camera_y >> 3) + DEVICE_SCREEN_HEIGHT) & 0x10;

    if (map_pos_x != old_map_pos_x) {
        old_map_pos_x = map_pos_x;

        if (camera_x < old_camera_x) {
            // moving left
            set_bkg_sub(map_pos_x, map_pos_y,
                        1, MIN(DEVICE_SCREEN_HEIGHT + 1, bg_map_mapHeight - map_pos_y),
                        bg_map_map, bg_map_MAP_ATTRIBUTES, MAP_FLIP_X, bg_map_mapWidth);
        } else if ((bg_map_mapWidth - DEVICE_SCREEN_WIDTH) > map_pos_x) {
            // moving right
            /*
            set_bkg_sub(map_pos_x + DEVICE_SCREEN_WIDTH, map_pos_y,
                        1, MIN(DEVICE_SCREEN_HEIGHT + 1, bg_map_mapHeight - map_pos_y),
                        bg_map_map, bg_map_MAP_ATTRIBUTES, MAP_FLIP_NONE, bg_map_mapWidth);
            */
            for (uint8_t i = 0; i < DEVICE_SCREEN_HEIGHT; i++) {
                uint8_t is_flipped_y = (map_pos_y + i) & 0x04;
                uint8_t attr = is_flipped_y ? (is_flipped_x_right ? MAP_FLIP_XY : MAP_FLIP_Y)
                                            : (is_flipped_x_right ? MAP_FLIP_X : MAP_FLIP_NONE);
                set(map_pos_x + DEVICE_SCREEN_WIDTH, map_pos_y + i,
                    is_flipped_x_right ? bg_map_mapWidth - map_pos_x : map_pos_x,
                    is_flipped_y ? bg_map_mapHeight - map_pos_y : map_pos_y,
                    attr);
            }
        }
    }

    if (map_pos_y != old_map_pos_y) {
        old_map_pos_y = map_pos_y;

        if (camera_y < old_camera_y) {
            // moving up
            set_bkg_sub(map_pos_x, map_pos_y,
                        MIN(DEVICE_SCREEN_WIDTH + 1, bg_map_mapWidth - map_pos_x), 1,
                        bg_map_map, bg_map_MAP_ATTRIBUTES, MAP_FLIP_Y, bg_map_mapWidth);
        } else if ((bg_map_mapHeight - DEVICE_SCREEN_HEIGHT) > map_pos_y) {
            // moving down
            set_bkg_sub(map_pos_x, map_pos_y + DEVICE_SCREEN_HEIGHT,
                        MIN(DEVICE_SCREEN_WIDTH + 1, bg_map_mapWidth - map_pos_x), 1,
                        bg_map_map, bg_map_MAP_ATTRIBUTES, MAP_FLIP_NONE, bg_map_mapWidth);
        }
    }

    // set old camera position to current camera position
    old_camera_x = camera_x;
    old_camera_y = camera_y;

#endif // ! WRAP_BG
}
