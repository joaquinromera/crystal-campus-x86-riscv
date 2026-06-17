#include "sched.h"

#include "defines.h"
#include "screen.h"
#include "state.h"

namespace {

uint32_t aux_exists_free_slot(uint32_t player_id) {
  uint32_t base = (player_id == PLAYER_A) ? 0u : 10u;
  for (uint32_t i = 0; i < 10; ++i) {
    if (sched_miners_state[base + i].alive == FALSE) return TRUE;
  }
  return FALSE;
}

uint32_t aux_get_next_free_slot(uint32_t player_id) {
  uint32_t base = (player_id == PLAYER_A) ? 0u : 10u;
  for (uint32_t i = 0; i < 10; ++i) {
    uint32_t slot_ix = base + i;
    if (sched_miners_state[slot_ix].alive == FALSE) return slot_ix;
  }
  return 0xFF;
}

uint32_t aux_get_next_alive_miner(uint32_t player_id, uint16_t* ix) {
  uint32_t base = (player_id == PLAYER_A) ? 0u : 10u;

  for (uint32_t i = 0; i < 10; ++i) {
    uint32_t offset = (sched_last_miner[player_id] + i + 1) % 10;
    if (sched_miners_state[base + offset].alive == TRUE) {
      *ix = static_cast<uint16_t>(base + offset);
      return TRUE;
    }
  }
  return FALSE;
}

} // fin del namespace

void sched_init() {
  for (uint32_t i = 0; i < 20; ++i) {
    sched_miners_state[i].alive = FALSE;
    sched_miners_state[i].n_cristales = 0;
    sched_miners_state[i].pos_x = 0;
    sched_miners_state[i].pos_y = 0;
  }

  sched_player_row[PLAYER_A] = 20;
  sched_player_row[PLAYER_B] = 20;
  sched_miners_left[PLAYER_A] = 20;
  sched_miners_left[PLAYER_B] = 20;
  sched_last_miner[PLAYER_A] = 0;
  sched_last_miner[PLAYER_B] = 0;
  sched_this_miner = 0;
}

int32_t sched_nextTask() {
  uint32_t jugador_actual = (sched_this_miner < 10) ? PLAYER_A : PLAYER_B;
  uint32_t jugador_opuesto = (jugador_actual == PLAYER_A) ? PLAYER_B : PLAYER_A;

  uint16_t ix = 0;

  if (aux_get_next_alive_miner(jugador_opuesto, &ix)) {
    sched_this_miner = static_cast<uint8_t>(ix);
    sched_last_miner[jugador_opuesto] = static_cast<uint8_t>(ix % 10);
    return static_cast<int32_t>(ix);
  }

  if (aux_get_next_alive_miner(jugador_actual, &ix)) {
    sched_this_miner = static_cast<uint8_t>(ix);
    sched_last_miner[jugador_actual] = static_cast<uint8_t>(ix % 10);
    return static_cast<int32_t>(ix);
  }

  return -1;
}

void sched_kill() {
  sched_miners_state[sched_this_miner].alive = FALSE;

  uint32_t x = static_cast<uint32_t>(sched_miners_state[sched_this_miner].pos_x);
  uint32_t y = static_cast<uint32_t>(sched_miners_state[sched_this_miner].pos_y);
  screen_redraw_crystal(x, y);

  screen_draw_miner_info(GET_PLAYER_ID(sched_this_miner), GET_PLAYER_MINER_ID(sched_this_miner));
  screen_draw_players();
}

int32_t sched_spawn(uint32_t player_id) {
  if (sched_miners_left[player_id] == 0) return -1;
  if (!aux_exists_free_slot(player_id)) return -1;

  sched_miners_left[player_id]--;
  screen_draw_miners_left();

  uint32_t slot = aux_get_next_free_slot(player_id);

  uint32_t x = (player_id == PLAYER_A) ? 0u : 77u;
  uint32_t y = sched_player_row[player_id];

  sched_miners_state[slot].pos_x = static_cast<int32_t>(x);
  sched_miners_state[slot].pos_y = static_cast<int32_t>(y);
  sched_miners_state[slot].n_cristales = 0;
  sched_miners_state[slot].alive = TRUE;

  screen_draw_miner(player_id, x, y);
  screen_draw_miner_info(static_cast<uint16_t>(player_id), slot % 10u);

  return static_cast<int32_t>(slot);
}

