/*
 * sample.c
 * Duality
 *
 * Copyright (C) 2025 Thomas Buck <thomas@xythobuz.de>
 *
 * Based on examples from gbdk-2020:
 * https://github.com/gbdk-2020/gbdk-2020/tree/develop/gbdk-lib/examples/gb/wav_sample
 *
 * And the docs for the DMG APU:
 * https://gbdev.io/pandocs/Audio_Registers.html
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

#include "sfx_shoot.h"
#include "sfx_expl_orb.h"
#include "sfx_expl_ship.h"
#include "sample.h"

BANKREF(sample)

static volatile uint8_t play_bank = 1;
static volatile const uint8_t *play_sample = 0;
static volatile uint16_t play_length = 0;

struct sfxs {
    uint8_t bank;
    uint8_t * const smp;
    uint16_t len;
};

static const struct sfxs sfxs[SFX_COUNT] = {
    { .bank = BANK(sfx_shoot),     .smp = sfx_shoot,     .len = sfx_shoot_SIZE >> 4 },     // SFX_SHOT
    { .bank = BANK(sfx_expl_orb),  .smp = sfx_expl_orb,  .len = sfx_expl_orb_SIZE >> 4 },  // SFX_EXPL_ORB
    { .bank = BANK(sfx_expl_ship), .smp = sfx_expl_ship, .len = sfx_expl_ship_SIZE >> 4 }, // SFX_EXPL_SHIP
};

void sample_play(enum SFXS sfx) BANKED {
    if (sfx >= SFX_COUNT) {
        return;
    }

    CRITICAL {
        play_bank = sfxs[sfx].bank;
        play_sample = sfxs[sfx].smp;
        play_length = sfxs[sfx].len;
    }
}

void sample_isr(void) NONBANKED NAKED {
    __asm
    ld hl, #_play_length    ; something left to play?
    ld a, (hl+)
    or (hl)
    ret z

    ld hl, #_play_sample
    ld a, (hl+)
    ld h, (hl)
    ld l, a                 ; HL = current position inside the sample

    ; load new waveform
    ld a, (#__current_bank) ; save bank and switch
    ld e, a
    ld a, (#_play_bank)
    ld (_rROMB0), a

    ldh a, (_NR51_REG)
    ld c, a
    and #0b10111011
    ldh (_NR51_REG), a

    xor a
    ldh (_NR30_REG), a

    .irp ofs,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    ld a, (hl+)
    ldh (__AUD3WAVERAM+ofs), a
    .endm

    ld a, #0x80
    ldh (_NR30_REG), a
    ld a, #0xFE             ; length of wave
    ldh (_NR31_REG), a
    ld a, #0x20             ; volume
    ldh (_NR32_REG), a
    xor a                   ; low freq bits are zero
    ldh (_NR33_REG), a
    ld a, #0xC7             ; start; no loop; high freq bits are 111
    ldh (_NR34_REG), a

    ld a, c
    ldh (_NR51_REG), a

    ld a, e                 ; restore bank
    ld (_rROMB0), a

    ld a, l                 ; save current position
    ld (#_play_sample), a
    ld a, h
    ld (#_play_sample+1), a

    ld hl, #_play_length    ; decrement length variable
    ld a, (hl)
    sub #1
    ld (hl+), a
    ld a, (hl)
    sbc #0
    ld (hl), a
    ret
    __endasm;
}
