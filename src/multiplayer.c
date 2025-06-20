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

#include <gbdk/emu_debug.h>

#include "game.h"
#include "timer.h"
#include "multiplayer.h"

#define MASTER_HELLO 0x42
#define SLAVE_HELLO 0x23

#define RETRANSMIT_TIME_HELLO 200
#define RETRANSMIT_TIME_GAME 10

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
    };
};

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

static void mp_game_init(void);

static inline void Tx(uint8_t x) {
    SB_REG = x;
    SC_REG = SIOF_XFER_START | SIOF_CLOCK_INT;
}

static inline void Rx(uint8_t x) {
    SB_REG = x;
    SC_REG = SIOF_XFER_START | SIOF_CLOCK_EXT;
}

static inline uint8_t transmitting(void) {
    return SC_REG & SIOF_XFER_START;
}

// ----------------------------------------------------------------------------
// Initial Handshake
// ----------------------------------------------------------------------------

uint8_t mp_master_ready(void) BANKED {
    switch (state) {
        case MP_M_SEND:
            Tx(MASTER_HELLO);
            next_t = timer_get() + RETRANSMIT_TIME_HELLO;
            mp_connection_status++;
            state = MP_M_WAIT;
            break;

        case MP_M_WAIT:
            if (!transmitting()) {
                if (SB_REG == SLAVE_HELLO) {
                    Rx(SLAVE_HELLO);
                    next_t = timer_get() + RETRANSMIT_TIME_HELLO;
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
    mp_game_init();
    game(GM_MULTI);
    state = 0;
}

uint8_t mp_slave_ready(void) BANKED {
    switch (state) {
        case MP_S_START:
            Rx(SLAVE_HELLO);
            next_t = timer_get() + RETRANSMIT_TIME_HELLO;
            mp_connection_status++;
            state = MP_S_WAIT;
            break;

        case MP_S_WAIT:
            if (!transmitting()) {
                if (SB_REG == MASTER_HELLO) {
                    Tx(MASTER_HELLO);
                    next_t = timer_get() + RETRANSMIT_TIME_HELLO;
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
    mp_game_init();
    game(GM_MULTI);
    state = 0;
}

// ----------------------------------------------------------------------------
// Game Runtime
// ----------------------------------------------------------------------------

#define QUEUE_LEN 5

static struct mp_packet queue[QUEUE_LEN];
static uint8_t q_head = 0;
static uint8_t q_tail = 0;
static uint8_t q_full = 0;
static uint8_t byte_pos = 0;

static uint8_t q_len(void) {
    if (q_head == q_tail) {
        if (q_full) {
            return QUEUE_LEN;
        } else {
            return 0;
        }
    } else if (q_head > q_tail) {
        return q_head - q_tail;
    } else {
        return QUEUE_LEN - q_tail + q_head;
    }
}

static inline void q_inc_head(void) {
    if (q_full && (++q_tail == QUEUE_LEN)) {
        q_tail = 0;
    }
    if (++q_head == QUEUE_LEN) {
        q_head = 0;
    }
    q_full = (q_head == q_tail);
}

static inline void q_inc_tail(void) {
    q_tail++;
    if (q_tail >= QUEUE_LEN) {
        q_tail = 0;
    }
}

static void mp_game_init(void) {
    q_head = 0;
    q_tail = 0;
    q_full = 0;
    byte_pos = sizeof(struct mp_packet); // immediately query packet from game
    next_t = timer_get() + RETRANSMIT_TIME_GAME;
}

static inline void handle_rx(struct mp_packet *pkt) {
    switch (pkt->header.type) {
        case PKT_TYPE_PLAYER:
            game_set_mp_player2(&pkt->player);
            break;

        case PKT_TYPE_SHOT:
            game_set_mp_shot(&pkt->shot);
            break;

        default:
#ifdef DEBUG
            EMU_printf("%s: unknown type %hx\n", __func__, (uint8_t)pkt->header.type);
#endif // DEBUG
            break;
    }
}

static inline void tx_rx(uint8_t x) {
    our_turn ? Tx(x) : Rx(x);
    next_t = timer_get() + RETRANSMIT_TIME_GAME;
}

void mp_handle(void) BANKED {
    // ensure we always have something in the queue
    if ((q_len() == 0) && (byte_pos >= sizeof(struct mp_packet))) {
        game_get_mp_state();
    }

    if (byte_pos < sizeof(struct mp_packet)) {
        if ((our_turn && (timer_get() >= next_t)) || ((!our_turn) && (!transmitting()))) {
            uint8_t to_send = ((uint8_t *)(&queue[q_tail]))[byte_pos];
            ((uint8_t *)(&queue[q_tail]))[byte_pos] = SB_REG;
            tx_rx(to_send);

            byte_pos++;
            if (byte_pos >= sizeof(struct mp_packet)) {
                handle_rx(&queue[q_tail]);
                q_inc_tail();
                if (q_len() > 0) {
                    byte_pos = 0; // keep going
                }
            }

            our_turn = our_turn ? 0 : 1;
        }
    }
}

void mp_new_state(struct mp_player_state *state) BANKED {
#ifdef DEBUG
    if (q_full) {
        EMU_printf("%s: queue overflow\n", __func__);
    }
#endif // DEBUG

    queue[q_head].header.type = PKT_TYPE_PLAYER;
    queue[q_head].player = *state;
    q_inc_head();

    if ((q_len() == 1) && (byte_pos >= sizeof(struct mp_packet))) {
        byte_pos = 0; // start transmitting
    }
}

void mp_add_shot(struct mp_shot_state *state) BANKED {
#ifdef DEBUG
    if (q_full) {
        EMU_printf("%s: queue overflow\n", __func__);
    }
#endif // DEBUG

    queue[q_head].header.type = PKT_TYPE_SHOT;
    queue[q_head].shot = *state;
    q_inc_head();

    if ((q_len() == 1) && (byte_pos >= sizeof(struct mp_packet))) {
        byte_pos = 0; // start transmitting
    }
}
