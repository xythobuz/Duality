/*
 * maps.c
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

#include "banks.h"
#include "title_map.h"
#include "bg_map.h"
#include "maps.h"

void map_title(void) NONBANKED {
    START_ROM_BANK(BANK(title_map));
        set_bkg_palette(OAMF_CGB_PAL0, title_map_PALETTE_COUNT, title_map_palettes);
        set_bkg_data(0, title_map_TILE_COUNT, title_map_tiles);
        if (title_map_MAP_ATTRIBUTES != NULL) {
            set_bkg_attributes(0, 0,
                               title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H,
                               title_map_MAP_ATTRIBUTES);
        } else {
            VBK_REG = VBK_ATTRIBUTES;
            fill_bkg_rect(0, 0,
                          title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H,
                          0x00);
            VBK_REG = VBK_TILES;
        }
        set_bkg_tiles(0, 0, title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H, title_map_map);
    END_ROM_BANK();
}

void map_game(void) NONBANKED {
    START_ROM_BANK(BANK(bg_map));
        set_bkg_palette(OAMF_CGB_PAL0, bg_map_PALETTE_COUNT, bg_map_palettes);
        set_bkg_data(0, bg_map_TILE_COUNT, bg_map_tiles);
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
    END_ROM_BANK();
}
