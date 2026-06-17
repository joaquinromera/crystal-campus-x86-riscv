/**
 * Guarda el estado global del juego.
 */

#ifndef STATE_H
#define STATE_H

#include "stdint.h"
#include "defines.h"

/** Toma un indice del minero en `sched_miners_state` y devuelve el
 *  identificador del jugador al que le pertenece.
 */
#define GET_PLAYER_ID(miner_ix) ((miner_ix < 10) ? PLAYER_A : PLAYER_B)

/** Toma un indice del minero en `sched_miners_state` y devuelve el
 *  identificador del minero para un jugador dado. Es decir el numero del
 *  minero del 0 al 9.
 */
#define GET_PLAYER_MINER_ID(miner_ix) (miner_ix % 10)

/** Devuelve el indice del minero en sched_miners_state */
#define GET_MINER_IX(player_id, miner_id) (player_id * 10 + miner_id)

/** Obtiene el indice en la memoria fisica del tablero a partir de las
 *  coordenadas del minero */
#define GET_IX_TABLERO(x, y) (TABLERO_N_COLS * y + x)

/** Obtiene el address en la memoria fisica del tablero a partir de las
 *  coordenadas del minero. */
#define GET_ADDR_TABLERO(x, y) \
    (MAPA_BASE + GET_IX_TABLERO(x, y) * PAGE_SIZE * 2)

/** Estructura con el estado de una tarea minera */
typedef struct
{
    int32_t pos_x;
    int32_t pos_y;
    uint32_t n_cristales;
    uint32_t alive;
} sched_miner_state_t;

/** Estado de las tareas.
 * Los mineros del jugador B empiezan a partir del 10.
 */
sched_miner_state_t sched_miners_state[20];

// Indica la altura en la que se encuentra cada jugador.
// Lo tenemos en `sched.h` porque necesitamos para spawnear una nueva tarea.
extern uint32_t sched_player_row[2];

// Indica cuantos mineros restantes tiene cada jugador.
extern uint32_t sched_miners_left[2];

/** Indica el indice en `sched_miners_state` de la ultima tarea minera
 * despachada por el scheduler para cada jugador
 */
extern uint8_t sched_last_miner[2];

/** Indica la tarea actual */
extern uint8_t sched_this_miner;

/** Mapa de cristales */
uint32_t crystal_map[78][40];

/** Puntaje de cada jugador */
extern uint32_t game_puntaje[2];

#endif /* ifndef STATE_H */
