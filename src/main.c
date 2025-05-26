#include <gbdk/platform.h>
#include <gbdk/metasprites.h>

#include "rockshp.h"
#include "bg_map.h"

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

const palette_color_t gray_pal[4] = {   RGB8(255,255,255),
                                        RGB8(170,170,170),
                                        RGB8(85,85,85),
                                        RGB8(0,0,0) };
const palette_color_t pink_pal[4] = {   RGB8(255,255,255),
                                        RGB8(255,0,255),
                                        RGB8(170,0,170),
                                        RGB8(85,0,85) };
const palette_color_t cyan_pal[4] = {   RGB8(255,255,255),
                                        RGB8(85,255,255),
                                        RGB8(0,170,170),
                                        RGB8(0,85,85) };
const palette_color_t green_pal[4] = {  RGB8(255,255,255),
                                        RGB8(170,255,170),
                                        RGB8(0,170,0),
                                        RGB8(0,85,0) };

const uint8_t pattern[] = {0x80,0x80,0x40,0x40,0x20,0x20,0x10,0x10,0x08,0x08,0x04,0x04,0x02,0x02,0x01,0x01};

void main(void) {
    DISPLAY_OFF;

    set_default_palette();

    // bg_map as background map
    set_bkg_palette(OAMF_CGB_PAL0, bg_map_PALETTE_COUNT, bg_map_palettes);
    set_bkg_data(0, bg_map_TILE_COUNT, bg_map_tiles);
    set_bkg_attributes(0, 0, bg_map_MAP_ATTRIBUTES_WIDTH, bg_map_MAP_ATTRIBUTES_HEIGHT, bg_map_MAP_ATTRIBUTES);
    set_bkg_tiles(0, 0, bg_map_WIDTH / bg_map_TILE_W, bg_map_HEIGHT / bg_map_TILE_H, bg_map_map);


    set_sprite_palette(OAMF_CGB_PAL0, 1, rockshp_palettes); //gray_pal);
    set_sprite_palette(OAMF_CGB_PAL1, 1, pink_pal);
    set_sprite_palette(OAMF_CGB_PAL2, 1, cyan_pal);
    set_sprite_palette(OAMF_CGB_PAL3, 1, green_pal);

    size_t i;
    num_tiles = sizeof(rockshp_tiles) >> 4;
    for(i = 0; i < num_tiles; i++)
    {
        //set_tile(i + get_tile_offset(0, 0), rockshp_tiles + (i << 4));
        set_sprite_data(i, 1, rockshp_tiles + (i << 4));
    }

    SHOW_BKG;
    SHOW_SPRITES;
    SPRITES_8x8;
    DISPLAY_ON;

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
            idx++; if (idx >= (sizeof(rockshp_metasprites) >> 1)) idx = 0;
        }

        // Press A button to cycle metasprite through Normal/Flip-Y/Flip-XY/Flip-X and sub-pals
        if (KEY_PRESSED(J_A)) {
            rot++; rot &= 0xF;
        }

        PosX += SpdX, PosY += SpdY;

        uint8_t hiwater = SPR_NUM_START;

        // NOTE: In a real game it would be better to only call the move_metasprite..()
        //       functions if something changed (such as movement or rotation). That
        //       reduces CPU usage on frames that don't need updates.
        //
        // In this example they are called every frame to simplify the example code

        // If not hidden the move and apply rotation to the metasprite
        uint8_t subpal = rot >> 2;
        switch (rot & 0x3) {
            case 1:
                hiwater += move_metasprite_flipy( rockshp_metasprites[idx],
                                                  TILE_NUM_START,
                                                  subpal,
                                                  hiwater,
                                                  DEVICE_SPRITE_PX_OFFSET_X + (PosX >> 4),
                                                  DEVICE_SPRITE_PX_OFFSET_Y + (PosY >> 4));
                break;
            case 2:
                hiwater += move_metasprite_flipxy(rockshp_metasprites[idx],
                                                  TILE_NUM_START,
                                                  subpal,
                                                  hiwater,
                                                  DEVICE_SPRITE_PX_OFFSET_X + (PosX >> 4),
                                                  DEVICE_SPRITE_PX_OFFSET_Y + (PosY >> 4));
                break;
            case 3:
                hiwater += move_metasprite_flipx( rockshp_metasprites[idx],
                                                  TILE_NUM_START,
                                                  subpal,
                                                  hiwater,
                                                  DEVICE_SPRITE_PX_OFFSET_X + (PosX >> 4),
                                                  DEVICE_SPRITE_PX_OFFSET_Y + (PosY >> 4));
                break;
            default:
                hiwater += move_metasprite_ex(    rockshp_metasprites[idx],
                                                  TILE_NUM_START,
                                                  subpal,
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
