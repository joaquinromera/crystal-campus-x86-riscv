#pragma once

#include <stdint.h>

#include "defines.h"

/* Funciones auxiliares (misma semántica que el tp original) */
#define GET_PLAYER_ID(miner_ix) ((miner_ix < 10) ? PLAYER_A : PLAYER_B)
#define GET_PLAYER_MINER_ID(miner_ix) ((miner_ix) % 10)
#define GET_MINER_IX(player_id, miner_id) ((player_id) * 10 + (miner_id))

typedef struct {
  int32_t pos_x;
  int32_t pos_y;
  uint32_t n_cristales;
  uint32_t alive;
} sched_miner_state_t;

extern sched_miner_state_t sched_miners_state[20];
extern uint32_t sched_player_row[2];
extern uint32_t sched_miners_left[2];
extern uint8_t sched_last_miner[2];
extern uint8_t sched_this_miner;

extern uint32_t crystal_map[TABLERO_N_COLS][TABLERO_N_FILS];
extern uint32_t game_puntaje[2];

