#include "game.h"

#include "defines.h"
#include "sched.h"
#include "screen.h"
#include "state.h"
#include "mmu.h"

namespace {

void aux_init_tablero() {
  for (uint32_t x = 0; x < TABLERO_N_COLS; ++x) {
    for (uint32_t y = 0; y < TABLERO_N_FILS; ++y) {
      crystal_map[x][y] = 0;
    }
  }

  for (uint32_t x = 15; x < TABLERO_N_COLS - 15; ++x) {
    for (uint32_t y = 10; y < TABLERO_N_FILS - 10; ++y) {
      crystal_map[x][y] = 1;
    }
  }

  for (uint32_t x = 25; x < TABLERO_N_COLS - 25; ++x) {
    for (uint32_t y = 15; y < TABLERO_N_FILS - 15; ++y) {
      crystal_map[x][y] = 2;
    }
  }

  for (uint32_t x = 30; x < TABLERO_N_COLS - 30; ++x) {
    for (uint32_t y = 18; y < TABLERO_N_FILS - 18; ++y) {
      crystal_map[x][y] = 3;
    }
  }
}

// Verifica que X esté dentro del tablero
#define AUX_INRANGE_X(x) (0 <= (x) && (x) <= SIZE_M)

#define GET_BASE(jugador) ((jugador == PLAYER_A) ? 0 : 77)

#define GAME_WON() ((game_puntaje[PLAYER_A] >= 300 || game_puntaje[PLAYER_B] >= 300) ? 1 : 0)

#define GET_WINNER() ((game_puntaje[PLAYER_A] > game_puntaje[PLAYER_B]) ? PLAYER_A : PLAYER_B)

void aux_check_game_over() {
  if (!GAME_WON()) return;
  for (uint8_t i = 0; i < 20; ++i) {
    sched_miners_state[i].alive = 0;
  }
  screen_game_won(GET_WINNER());
}

// Módulo real (para negativos)
int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}

} // fin del namespace

void game_init() {
  aux_init_tablero();
  screen_draw_crystals();
}

uint32_t game_get_id() {
  return 1 + (static_cast<uint32_t>(sched_this_miner) % 10u);
}

uint32_t game_get_player() {
  return GET_PLAYER_ID(sched_this_miner);
}

void game_move(e_direction_t d) {
  sched_miner_state_t* player = &sched_miners_state[sched_this_miner];

  int32_t mov_x = 0;
  int32_t mov_y = 0;

  if (d == Forward) mov_x = 1;
  if (d == Back) mov_x = -1;
  if (d == Left) mov_y = 1;
  if (d == Right) mov_y = -1;

  // Espejar movimientos para el jugador del lado derecho.
  if (game_get_player() == PLAYER_B) {
    mov_x *= -1;
    mov_y *= -1;
  }

  int32_t nuevo_x = player->pos_x + mov_x;
  int32_t nuevo_y = mod(player->pos_y + mov_y, TABLERO_N_FILS);

  if (!AUX_INRANGE_X(nuevo_x)) {
    sched_kill();
    return;
  }

  // El código de la tarea vive físicamente en la celda del tablero, moverse
  // copia su imagen a la celda nueva y remapea TASK_CODE
  mmu_task_move(static_cast<uint32_t>(sched_this_miner),
                static_cast<uint32_t>(player->pos_x),
                static_cast<uint32_t>(player->pos_y),
                static_cast<uint32_t>(nuevo_x),
                static_cast<uint32_t>(nuevo_y));

  // Redibujar mapa
  screen_draw_miner(GET_PLAYER_ID(sched_this_miner), static_cast<uint32_t>(nuevo_x), static_cast<uint32_t>(nuevo_y));
  screen_redraw_crystal(static_cast<uint32_t>(player->pos_x), static_cast<uint32_t>(player->pos_y));
  screen_draw_players();

  // Actualizar coordenadas en el estado del juego.
  player->pos_x = nuevo_x;
  player->pos_y = nuevo_y;

  // Depositar puntaje en la base.
  uint32_t player_id = game_get_player();
  if (static_cast<uint32_t>(nuevo_x) == GET_BASE(player_id)) {
    game_puntaje[player_id] += player->n_cristales;
    player->n_cristales = 0;

    aux_check_game_over();

    screen_draw_miner_info(GET_PLAYER_ID(sched_this_miner), GET_PLAYER_MINER_ID(sched_this_miner));
    screen_draw_score();
  }
}

int32_t game_take() {
  sched_miner_state_t* player = &sched_miners_state[sched_this_miner];
  int32_t x = player->pos_x;
  int32_t y = player->pos_y;

  uint32_t crystal_size = crystal_map[x][y];

  if (crystal_size == 0) return -1;

  uint32_t suma = player->n_cristales + crystal_size;
  if (suma <= GAME_MAX_CRYSTALS) {
    crystal_map[x][y] = 0;
    player->n_cristales = suma;
    screen_draw_miner_info(GET_PLAYER_ID(sched_this_miner), GET_PLAYER_MINER_ID(sched_this_miner));
    return static_cast<int32_t>(crystal_size);
  }

  return 0;
}

void game_move_player(uint32_t player_id, e_direction_t direction) {
  int32_t row = static_cast<int32_t>(sched_player_row[player_id]);

  if (direction == Up) row--;
  if (direction == Down) row++;

  if (row == -1) row = SIZE_N - 1;
  if (row == SIZE_N) row = 0;

  sched_player_row[player_id] = static_cast<uint32_t>(row);

  screen_draw_players();
}
