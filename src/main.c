/*
 * main.c
 * Duality
 */

#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#include "bg_map.h"
#include "rockshp.h"
#include "thrustG.h"
#include "light.h"
#include "dark.h"
#include "shoot.h"

#define TILE_WIDTH          8
#define TILE_HEIGHT         8
#define NUM_BYTES_PER_TILE  16

#define ACC_X 1
#define ACC_Y 2

// The metasprite will be built starting with hardware sprite zero (the first)
#define SPR_NUM_START 0

// Metasprite tiles are loaded into VRAM starting at tile number 0
#define TILE_NUM_START 0

int16_t PosX, PosY;
int16_t SpdX, SpdY;
uint8_t PosF;
uint8_t idx, rot;

size_t num_tiles;

uint8_t joyp = 0, old_joyp = 0;

#define KEY_INPUT (old_joyp = joyp, joyp = joypad())
#define KEY_DOWN(KEY) (joyp & (KEY))
#define KEY_PRESSED(KEY) ((joyp ^ old_joyp) & joyp & (KEY))

struct sprites {
    const metasprite_t * const * ms;
    const uint8_t * ti;
    const palette_color_t * pa;
    uint8_t cnt;
    uint8_t off;
};

struct sprites metasprites[] = {
    {
        .ms = rockshp_metasprites,
        .ti = rockshp_tiles,
        .pa = rockshp_palettes,
        .cnt = rockshp_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = thrustG_metasprites,
        .ti = thrustG_tiles,
        .pa = thrustG_palettes,
        .cnt = thrustG_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = light_metasprites,
        .ti = light_tiles,
        .pa = light_palettes,
        .cnt = light_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = dark_metasprites,
        .ti = dark_tiles,
        .pa = dark_palettes,
        .cnt = dark_TILE_COUNT,
        .off = TILE_NUM_START
    }, {
        .ms = shoot_metasprites,
        .ti = shoot_tiles,
        .pa = shoot_palettes,
        .cnt = shoot_TILE_COUNT,
        .off = TILE_NUM_START
    }
};

void main(void) {
    disable_interrupts();
    DISPLAY_OFF;

    set_default_palette();

    // bg_map as background map
    set_bkg_palette(OAMF_CGB_PAL0, bg_map_PALETTE_COUNT, bg_map_palettes);
    set_bkg_data(0, bg_map_TILE_COUNT, bg_map_tiles);
    set_bkg_attributes(0, 0, bg_map_MAP_ATTRIBUTES_WIDTH, bg_map_MAP_ATTRIBUTES_HEIGHT, bg_map_MAP_ATTRIBUTES);
    set_bkg_tiles(0, 0, bg_map_WIDTH / bg_map_TILE_W, bg_map_HEIGHT / bg_map_TILE_H, bg_map_map);

    // metasprites
    uint8_t off = TILE_NUM_START;
    for (int i = 0; i < (sizeof(metasprites) / sizeof(metasprites[0])); i++) {
        metasprites[i].off = off;
        off += metasprites[i].cnt;

        set_sprite_palette(OAMF_CGB_PAL0 + i, 1, metasprites[i].pa);
        set_sprite_data(metasprites[i].off, metasprites[i].cnt, metasprites[i].ti);
    }

    SHOW_BKG;
    SHOW_SPRITES;
    SPRITES_8x8;
    DISPLAY_ON;
    enable_interrupts();

    // Set initial position to the center of the screen, zero out speed
    PosX = (DEVICE_SCREEN_PX_WIDTH / 2) << 4;
    PosY = (DEVICE_SCREEN_PX_HEIGHT / 2) << 4;
    SpdX = SpdY = 0;

    idx = 0; rot = 0;

    while(1) {
        KEY_INPUT;

        PosF = 0;
        // Game object
        if (KEY_DOWN(J_UP)) {
            SpdY -= 2;
            if (SpdY < -32) SpdY = -32;
            PosF |= ACC_Y;
        } else if (KEY_DOWN(J_DOWN)) {
            SpdY += 2;
            if (SpdY > 32) SpdY = 32;
            PosF |= ACC_Y;
        }

        if (KEY_DOWN(J_LEFT)) {
            SpdX -= 2;
            if (SpdX < -32) SpdX = -32;
            PosF |= ACC_X;
        } else if (KEY_DOWN(J_RIGHT)) {
            SpdX += 2;
            if (SpdX > 32) SpdX = 32;
            PosF |= ACC_X;
        }

        // Press B button to cycle through metasprite animations
        if (KEY_PRESSED(J_B)) {
            idx++;
            if (idx >= (sizeof(metasprites) / sizeof(metasprites[0]))) idx = 0;
        }

        // Press A button to cycle metasprite through Normal/Flip-Y/Flip-XY/Flip-X and sub-pals
        if (KEY_PRESSED(J_A)) {
            rot++; rot &= 0x3;
        }

        PosX += SpdX, PosY += SpdY;

        uint8_t hiwater = SPR_NUM_START;

        // NOTE: In a real game it would be better to only call the move_metasprite..()
        //       functions if something changed (such as movement or rotation). That
        //       reduces CPU usage on frames that don't need updates.
        //
        // In this example they are called every frame to simplify the example code

        // If not hidden the move and apply rotation to the metasprite
        switch (rot & 0x3) {
            case 1:
                hiwater += move_metasprite_flipy( metasprites[idx].ms[0],
                                                  metasprites[idx].off,
                                                  idx,
                                                  hiwater,
                                                  DEVICE_SPRITE_PX_OFFSET_X + (PosX >> 4),
                                                  DEVICE_SPRITE_PX_OFFSET_Y + (PosY >> 4));
                break;
            case 2:
                hiwater += move_metasprite_flipxy(metasprites[idx].ms[0],
                                                  metasprites[idx].off,
                                                  idx,
                                                  hiwater,
                                                  DEVICE_SPRITE_PX_OFFSET_X + (PosX >> 4),
                                                  DEVICE_SPRITE_PX_OFFSET_Y + (PosY >> 4));
                break;
            case 3:
                hiwater += move_metasprite_flipx( metasprites[idx].ms[0],
                                                  metasprites[idx].off,
                                                  idx,
                                                  hiwater,
                                                  DEVICE_SPRITE_PX_OFFSET_X + (PosX >> 4),
                                                  DEVICE_SPRITE_PX_OFFSET_Y + (PosY >> 4));
                break;
            default:
                hiwater += move_metasprite_ex(    metasprites[idx].ms[0],
                                                  metasprites[idx].off,
                                                  idx,
                                                  hiwater,
                                                  DEVICE_SPRITE_PX_OFFSET_X + (PosX >> 4),
                                                  DEVICE_SPRITE_PX_OFFSET_Y + (PosY >> 4));
                break;
        }

        // Hide rest of the hardware sprites, because amount of sprites differ between animation frames.
        hide_sprites_range(hiwater, MAX_HARDWARE_SPRITES);

        // Y Axis: update velocity (reduce speed) if no U/D button pressed
        if (!(PosF & ACC_Y)) {
            if (SpdY != 0) {
                if (SpdY > 0) SpdY--;
                else SpdY ++;
            }
        }

        // X Axis: update velocity (reduce speed) if no L/R button pressed
        if (!(PosF & ACC_X)) {
            if (SpdX != 0) {
                if (SpdX > 0) SpdX--;
                else SpdX ++;
            }
        }

        vsync();
    }
}
