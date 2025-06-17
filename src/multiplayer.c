/*
 * multiplayer.c
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

#include "game.h"
#include "timer.h"
#include "multiplayer.h"

#define MASTER_HELLO 0x42
#define SLAVE_HELLO 0x23

#define RETRANSMIT_TIME 200

enum mp_state {
    MP_M_SEND = 0,
    MP_M_WAIT = 1,
    MP_M_REPLY = 2,

    MP_S_START = 0,
    MP_S_WAIT = 1,
    MP_S_REPLY = 2,
};

static enum mp_state state = 0;
static uint16_t next_t = 0;
static uint8_t our_turn = 0;

uint8_t mp_connection_status = 0;

uint8_t mp_master_ready(void) BANKED {
    switch (state) {
        case MP_M_SEND:
            SB_REG = MASTER_HELLO;
            SC_REG = SIOF_XFER_START | SIOF_CLOCK_INT;
            next_t = timer_get() + RETRANSMIT_TIME;
            mp_connection_status++;
            state = MP_M_WAIT;
            break;

        case MP_M_WAIT:
            if (!(SC_REG & SIOF_XFER_START)) {
                if (SB_REG == SLAVE_HELLO) {
                    SB_REG = SLAVE_HELLO;
                    SC_REG = SIOF_XFER_START | SIOF_CLOCK_EXT;
                    next_t = timer_get() + RETRANSMIT_TIME;
                    mp_connection_status++;
                    state = MP_M_REPLY;
                }
            }
            if (timer_get() >= next_t) {
                state = MP_M_SEND;
            }
            break;

        case MP_M_REPLY:
            if (!(SC_REG & SIOF_XFER_START)) {
                if (SB_REG == MASTER_HELLO) {
                    return 1;
                } else {
                    state = MP_M_SEND;
                }
            }
            if (timer_get() >= next_t) {
                state = MP_M_SEND;
            }
            break;

        default:
            state = 0;
            break;
    }

    return 0;
}

void mp_master_start(void) BANKED {
    our_turn = 1;
    game(GM_MULTI);
    state = 0;
}

uint8_t mp_slave_ready(void) BANKED {
    switch (state) {
        case MP_S_START:
            SB_REG = SLAVE_HELLO;
            SC_REG = SIOF_XFER_START | SIOF_CLOCK_EXT;
            next_t = timer_get() + RETRANSMIT_TIME;
            mp_connection_status++;
            state = MP_S_WAIT;
            break;

        case MP_S_WAIT:
            if (!(SC_REG & SIOF_XFER_START)) {
                if (SB_REG == MASTER_HELLO) {
                    SB_REG = MASTER_HELLO;
                    SC_REG = SIOF_XFER_START | SIOF_CLOCK_INT;
                    next_t = timer_get() + RETRANSMIT_TIME;
                    mp_connection_status++;
                    state = MP_S_REPLY;
                } else {
                    state = MP_S_START;
                }
            }
            if (timer_get() >= next_t) {
                state = MP_S_START;
            }
            break;

        case MP_S_REPLY:
            if (!(SC_REG & SIOF_XFER_START)) {
                if (SB_REG == SLAVE_HELLO) {
                    return 1;
                }
            }
            if (timer_get() >= next_t) {
                state = MP_S_START;
            }
            break;

        default:
            state = 0;
            break;
    }

    return 0;
}

void mp_slave_start(void) BANKED {
    our_turn = 0;
    game(GM_MULTI);
    state = 0;
}

void mp_handle(void) BANKED {

}
