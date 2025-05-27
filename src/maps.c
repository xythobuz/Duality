/*
 * maps.c
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 */

#include <gbdk/platform.h>

#include "maps.h"

#include "title_map.h"
#include "bg_map.h"

void map_title(void) {
    set_bkg_palette(OAMF_CGB_PAL0, title_map_PALETTE_COUNT, title_map_palettes);
    set_bkg_data(0, title_map_TILE_COUNT, title_map_tiles);
    set_bkg_attributes(0, 0, title_map_MAP_ATTRIBUTES_WIDTH, title_map_MAP_ATTRIBUTES_HEIGHT, title_map_MAP_ATTRIBUTES);
    set_bkg_tiles(0, 0, title_map_WIDTH / title_map_TILE_W, title_map_HEIGHT / title_map_TILE_H, title_map_map);
}

void map_game(void) {
    set_bkg_palette(OAMF_CGB_PAL0, bg_map_PALETTE_COUNT, bg_map_palettes);
    set_bkg_data(0, bg_map_TILE_COUNT, bg_map_tiles);
    set_bkg_attributes(0, 0, bg_map_MAP_ATTRIBUTES_WIDTH, bg_map_MAP_ATTRIBUTES_HEIGHT, bg_map_MAP_ATTRIBUTES);
    set_bkg_tiles(0, 0, bg_map_WIDTH / bg_map_TILE_W, bg_map_HEIGHT / bg_map_TILE_H, bg_map_map);
}
