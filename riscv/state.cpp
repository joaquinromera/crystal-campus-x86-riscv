#include "state.h"

sched_miner_state_t sched_miners_state[20];

uint32_t sched_player_row[2] = {20, 20};
uint32_t sched_miners_left[2] = {20, 20};
uint8_t sched_last_miner[2] = {0, 0};
uint8_t sched_this_miner = 0;

uint32_t crystal_map[TABLERO_N_COLS][TABLERO_N_FILS];
uint32_t game_puntaje[2] = {0, 0};

