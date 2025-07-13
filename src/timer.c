/*
 * timer.c
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

#include "sample.h"
#include "sound.h"
#include "timer.h"

#define DMG_TMA_VAL (0x100UL -  64UL) // 16.384kHz /  64 = 256Hz
#define CGB_TMA_VAL (0x100UL - 128UL) // 32.768kHz / 128 = 256Hz

static uint16_t count = 0;

static void timer_isr(void) NONBANKED {
    sample_isr();
    snd_play();
    count++;
}

void timer_init(void) BANKED {
    CRITICAL {
        count = 0;
        add_TIM(timer_isr);
        TMA_REG = (_cpu == CGB_TYPE) ? CGB_TMA_VAL : DMG_TMA_VAL;
        TAC_REG = TACF_16KHZ | TACF_START;

        set_interrupts(TIM_IFLAG | VBL_IFLAG);
    }
}

uint16_t timer_get(void) NONBANKED {
    uint16_t r;
    CRITICAL {
        r = count;
    }
    return r << 2;
}
