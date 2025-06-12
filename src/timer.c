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

static uint16_t count = 0;

static void timer_isr(void) NONBANKED {
    if ((count & 0x03) == 0) {
        sample_isr();
    }
    snd_play();
    count++;
}

void timer_init(void) BANKED {
    CRITICAL {
        count = 0;
        add_TIM(timer_isr);
        if (_cpu == CGB_TYPE) {
            TMA_REG = 0x100 - 131; // 131.072kHz / 131 = ~1000Hz
        } else {
            TMA_REG = 0x100 - 65; // 65.536kHz / 65 = ~1008Hz
        }
        TAC_REG = TACF_65KHZ | TACF_START;

        set_interrupts(TIM_IFLAG | VBL_IFLAG);
    }
}

uint16_t timer_get(void) BANKED {
    uint16_t r;
    CRITICAL {
        r = count;
    }
    return r;
}
