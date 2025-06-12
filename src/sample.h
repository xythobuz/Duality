/*
 * sample.h
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

#ifndef __SAMPLE_H__
#define __SAMPLE_H__

#include <gbdk/platform.h>

enum SFXS {
    SFX_SHOT = 0,
    SFX_EXPL_ORB,
    SFX_EXPL_SHIP,

    SFX_COUNT
};

void sample_play(enum SFXS sfx) BANKED;

void sample_isr(void);

BANKREF_EXTERN(sample)

#endif // __SAMPLE_H__
