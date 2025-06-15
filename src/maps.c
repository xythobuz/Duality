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

#include <assert.h>

#include "banks.h"
#include "title_map.h"
#include "bg_map.h"
#include "util.h"
#include "maps.h"

// currently this assumption is hard-coded
static_assert(bg_map_WIDTH == 256, "bg_map needs to be 256x256");
static_assert(bg_map_HEIGHT == 256, "bg_map needs to be 256x256");

#define POS_SCALE_BG 6

// define this to disable mirrored map scaling support
#define WRAP_BG // TODO

#define bg_map_mapWidth (bg_map_WIDTH / bg_map_TILE_W)
#define bg_map_mapHeight (bg_map_HEIGHT / bg_map_TILE_H)

#define camera_max_x ((bg_map_mapWidth - DEVICE_SCREEN_WIDTH) * 8)
#define camera_max_y ((bg_map_mapHeight - DEVICE_SCREEN_HEIGHT) * 8)

#define set_bkg_sub_attr(x, y, w, h, attr, map_w) \
    if (attr)                                     \
        set_bkg_submap_attributes(x, y, w, h, attr, map_w)

#define set_bkg_sub(x, y, w, h, map, attr, map_w) \
    set_bkg_submap(x, y, w, h, map, map_w);       \
    set_bkg_sub_attr(x, y, w, h, attr, map_w)

// current unscaled ship position
static uint16_t abs_x, abs_y;

// current and old positions of the camera in pixels
static uint16_t camera_x, camera_y, old_camera_x, old_camera_y;

// current and old position of the map in tiles
static uint8_t map_pos_x, map_pos_y, old_map_pos_x, old_map_pos_y;

void map_title(void) NONBANKED {
    START_ROM_BANK(BANK(title_map)) {

        set_bkg_palette(OAMF_CGB_PAL0, title_map_PALETTE_COUNT, title_map_palettes);
        set_bkg_data(0, title_map_TILE_COUNT, title_map_tiles);

        if (title_map_MAP_ATTRIBUTES != NULL) {
            set_bkg_attributes(0, 0,
                               title_map_WIDTH / title_map_TILE_W,
                               title_map_HEIGHT / title_map_TILE_H,
                               title_map_MAP_ATTRIBUTES);
        } else {
            VBK_REG = VBK_ATTRIBUTES;
            fill_bkg_rect(0, 0,
                          title_map_WIDTH / title_map_TILE_W,
                          title_map_HEIGHT / title_map_TILE_H,
                          0x00);
            VBK_REG = VBK_TILES;
        }

        set_bkg_tiles(0, 0,
                      title_map_WIDTH / title_map_TILE_W,
                      title_map_HEIGHT / title_map_TILE_H,
                      title_map_map);

    } END_ROM_BANK

    move_bkg(0, 0);
}

void map_game(void) NONBANKED {
    START_ROM_BANK(BANK(bg_map)) {

        set_bkg_palette(OAMF_CGB_PAL0, bg_map_PALETTE_COUNT, bg_map_palettes);
        set_bkg_data(0, bg_map_TILE_COUNT, bg_map_tiles);

#ifdef WRAP_BG

        if (bg_map_MAP_ATTRIBUTES != NULL) {
            set_bkg_attributes(0, 0,
                               bg_map_WIDTH / bg_map_TILE_W, bg_map_HEIGHT / bg_map_TILE_H,
                               bg_map_MAP_ATTRIBUTES);
        } else {
            VBK_REG = VBK_ATTRIBUTES;
            fill_bkg_rect(0, 0,
                          bg_map_WIDTH / bg_map_TILE_W, bg_map_HEIGHT / bg_map_TILE_H,
                          0x00);
            VBK_REG = VBK_TILES;
        }
        set_bkg_tiles(0, 0, bg_map_WIDTH / bg_map_TILE_W, bg_map_HEIGHT / bg_map_TILE_H, bg_map_map);

#else // WRAP_BG

        abs_x = 0;
        abs_y = 0;

        // Initial camera position in pixels set here.
        camera_x = 0;
        camera_y = 0;

        // Enforce map limits on initial camera position
        if (camera_x > camera_max_x) camera_x = camera_max_x;
        if (camera_y > camera_max_y) camera_y = camera_max_y;
        old_camera_x = camera_x; old_camera_y = camera_y;

        map_pos_x = camera_x >> 3;
        map_pos_y = camera_y >> 3;
        old_map_pos_x = old_map_pos_y = 255;

        move_bkg(camera_x, camera_y);

        // Draw the initial map view for the whole screen
        set_bkg_sub(map_pos_x, map_pos_y,
                    MIN(DEVICE_SCREEN_WIDTH + 1u, bg_map_mapWidth - map_pos_x),
                    MIN(DEVICE_SCREEN_HEIGHT + 1u, bg_map_mapHeight - map_pos_y),
                    bg_map_map, bg_map_MAP_ATTRIBUTES, bg_map_mapWidth);

#endif // WRAP_BG

    } END_ROM_BANK
}

void map_move(int16_t delta_x, int16_t delta_y) NONBANKED {
    abs_x += delta_x;
    abs_y += delta_y;

    camera_x = abs_x >> POS_SCALE_BG;
    camera_y = abs_y >> POS_SCALE_BG;

    // update hardware scroll position
    move_bkg(camera_x, camera_y);

#ifndef WRAP_BG

    map_pos_y = (uint8_t)(camera_y >> 3u);
    map_pos_x = (uint8_t)(camera_x >> 3u);

    START_ROM_BANK(BANK(bg_map)) {

        // up or down
        if (map_pos_y != old_map_pos_y) {
            if (camera_y < old_camera_y) {
                set_bkg_sub(map_pos_x, map_pos_y,
                            MIN(DEVICE_SCREEN_WIDTH + 1, bg_map_mapWidth - map_pos_x), 1,
                            bg_map_map, bg_map_MAP_ATTRIBUTES, bg_map_mapWidth);
            } else if ((bg_map_mapHeight - DEVICE_SCREEN_HEIGHT) > map_pos_y) {
                set_bkg_sub(map_pos_x, map_pos_y + DEVICE_SCREEN_HEIGHT,
                            MIN(DEVICE_SCREEN_WIDTH + 1, bg_map_mapWidth - map_pos_x), 1,
                            bg_map_map, bg_map_MAP_ATTRIBUTES, bg_map_mapWidth);
            }
            old_map_pos_y = map_pos_y;
        }

        // left or right
        if (map_pos_x != old_map_pos_x) {
            if (camera_x < old_camera_x) {
                set_bkg_sub(map_pos_x, map_pos_y,
                            1, MIN(DEVICE_SCREEN_HEIGHT + 1, bg_map_mapHeight - map_pos_y),
                            bg_map_map, bg_map_MAP_ATTRIBUTES, bg_map_mapWidth);
            } else if ((bg_map_mapWidth - DEVICE_SCREEN_WIDTH) > map_pos_x) {
                set_bkg_sub(map_pos_x + DEVICE_SCREEN_WIDTH, map_pos_y,
                            1, MIN(DEVICE_SCREEN_HEIGHT + 1, bg_map_mapHeight - map_pos_y),
                            bg_map_map, bg_map_MAP_ATTRIBUTES, bg_map_mapWidth);
            }
            old_map_pos_x = map_pos_x;
        }

    } END_ROM_BANK

    // set old camera position to current camera position
    old_camera_x = camera_x;
    old_camera_y = camera_y;

#endif // ! WRAP_BG
}
