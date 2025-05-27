/*
 * sound.c
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * Based on examples from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/cross-platform/metasprites/src/metasprites.c
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/galaxy/galaxy.c
 * https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/rand/rand.c
 */

#include <gbdk/platform.h>

#include "sound.h"

void snd_init(void) {
    NR52_REG = 0x80; // sound on
    NR51_REG = 0xFF; // all channels on left and right

#ifdef DEBUG
    NR50_REG = 0x33; // left and right on half volume
#else
    NR50_REG = 0x77; // left and right on full volume
#endif
}

void snd_noise(void) {
    NR41_REG = 0x2F; // length timer, higher value is shorter time (up to 0x3F)
    NR42_REG = 0xF0; // initially full volume, no volume changes over time
    NR43_REG = 0x11; // frequency distribution
    NR44_REG = 0xC0; // trigger and enable length
}
