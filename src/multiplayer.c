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

#define PKT_TYPE_PLAYER 0x00
#define PKT_TYPE_SHOT 0x01

struct mp_header {
    uint8_t type;
};

struct mp_packet {
    struct mp_header header;
    union {
        struct mp_player_state player;
        struct mp_shot_state shot;
    } data;
};

#define PKT_SIZE_PLAYER (sizeof(struct mp_player_state) + sizeof(struct mp_header))
#define PKT_SIZE_SHOT (sizeof(struct mp_shot_state) + sizeof(struct mp_header))

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

static uint8_t *data = NULL;
static uint8_t remaining = 0;

static uint8_t rx_buff[sizeof(struct mp_packet)] = {0};
static uint8_t rx_len = 0;

uint8_t mp_connection_status = 0;

static inline void Tx(uint8_t x) {
    SB_REG = x;
    SC_REG = SIOF_XFER_START | SIOF_CLOCK_INT;
}

static inline void Rx(uint8_t x) {
    SB_REG = x;
    SC_REG = SIOF_XFER_START | SIOF_CLOCK_EXT;
}

static inline void tx_rx(uint8_t x) {
    our_turn ? Tx(x) : Rx(x);
}

static inline void transmit(struct mp_packet *pkt) {

}

static inline uint8_t transmitting(void) {
    return SC_REG & SIOF_XFER_START;
}

uint8_t mp_master_ready(void) BANKED {
    switch (state) {
        case MP_M_SEND:
            Tx(MASTER_HELLO);
            next_t = timer_get() + RETRANSMIT_TIME;
            mp_connection_status++;
            state = MP_M_WAIT;
            break;

        case MP_M_WAIT:
            if (!transmitting()) {
                if (SB_REG == SLAVE_HELLO) {
                    Rx(SLAVE_HELLO);
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
            if (!transmitting()) {
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
    remaining = 0;
    next_t = timer_get() + RETRANSMIT_TIME;

    game(GM_MULTI);

    state = 0;
}

uint8_t mp_slave_ready(void) BANKED {
    switch (state) {
        case MP_S_START:
            Rx(SLAVE_HELLO);
            next_t = timer_get() + RETRANSMIT_TIME;
            mp_connection_status++;
            state = MP_S_WAIT;
            break;

        case MP_S_WAIT:
            if (!transmitting()) {
                if (SB_REG == MASTER_HELLO) {
                    Tx(MASTER_HELLO);
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
            if (!transmitting()) {
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
    remaining = 0;
    next_t = timer_get() + RETRANSMIT_TIME;

    game(GM_MULTI);

    state = 0;
}

uint8_t mp_handle(void) BANKED {
    if ((our_turn) && (timer_get() >= next_t) && data && (remaining > 0)) {
        if (rx_len < sizeof(struct mp_packet)) {
            rx_buff[rx_len++] = SB_REG;
        }
        tx_rx(*(data++));
        remaining--;
    } else if ((!our_turn) && (!transmitting()) && data && (remaining > 0)) {
        if (rx_len < sizeof(struct mp_packet)) {
            rx_buff[rx_len++] = SB_REG;
        }
        tx_rx(*(data++));
        remaining--;
    }

    return our_turn && (remaining == 0);
}

void mp_new_state(struct mp_player_state *state) BANKED {
    // TODO
}

void mp_add_shot(struct mp_shot_state *state) BANKED {
    // TODO
}
