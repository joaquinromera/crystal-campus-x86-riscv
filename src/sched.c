/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de funciones del scheduler
*/

#include "sched.h"

// -----------------------------------------------------------------------------
// Estado
// -----------------------------------------------------------------------------

uint32_t sched_player_row[] = {20, 20};
uint32_t sched_miners_left[2] = {20, 20};
uint8_t sched_last_miner[2] = {0, 0};
uint8_t sched_this_miner = 0;

// -----------------------------------------------------------------------------
// Funciones AUX
// -----------------------------------------------------------------------------


/** Devuelve TRUE si existe un slot libre para este jugador. De no existir
 *  (porque tiene 20 mineros spawneados) devuvelve FALSE.
 */
uint32_t aux_exists_free_slot(uint32_t player_id) {

    // Para el jugador A, sus tareas son [0:9]. Para el jugador B, sus tareas
    // son [10:19]
    uint32_t base = (player_id == PLAYER_A) ? 0 : 10;
    // Se fija si el jugador tiene alguno de sus slots para tareas libre.
    for (uint32_t i = 0; i < 10; ++i) {
        if (sched_miners_state[base + i].alive == FALSE) { // Hay un slot libre
            return TRUE;
        }
    }

    // No hay slots libres.
    return FALSE;
}

/** Devuelve el proximo slot (de 0 a 19) libre para inicializar el nuevo
 * minero.  Se indefine si el resultado de `aux_exists_free_slot(player_id)` es
 * FALSE.
 */
uint32_t aux_get_next_free_slot(uint32_t player_id) {
    // Para el jugador A, sus tareas son [0:9]. Para el jugador B, sus tareas
    // son [10:19]
    uint32_t base = (player_id == PLAYER_A) ? 0 : 10;
    // Se fija si el jugador tiene alguno de sus slots para tareas libre.
    for (uint32_t i = 0; i < 10; ++i) {
        uint32_t slot_ix = base + i;
        if (sched_miners_state[base + i].alive == FALSE) { // Hay un slot libre
            return slot_ix;
        }
    }

    // No se llega a esta linea. Es para evitar el warning
    return 0xff;
}

/** Escribe en `ix` el proximo minero vivo (despues del ultimo minero
 *  ejecutado) para un jugador. Si no tiene mineros vivos, devuelve FALSE. */
uint32_t aux_get_next_alive_miner(uint32_t player_id, uint16_t *ix) {
    // Los mineros del jugador B empiezan a partir del 10.
    uint32_t base = (player_id == PLAYER_A) ? 0 : 10;

    // Busca entre los mineros de ese jugador el proximo minero vivo para
    // saltar a el.
    for (uint32_t i = 0; i < 10; ++i) {

        // Cicla entre los 10 mineros. Se suma 1 para que empiece a recorrer
        // desde el proximo.
        uint32_t offset = (sched_last_miner[player_id] + i + 1) % 10;
        // Si encuentra uno vivo salta a el.
        if (sched_miners_state[base + offset].alive == TRUE) {
            *ix = base + offset;
            return TRUE;
        }

    }

    // Si el jugador no tiene otro minero vivo devuelve true. Se indefine el
    // valor de *ix.
    return FALSE;

}

/** Devuelve indice GDT de un minero */
#define AUX_GET_GDT_IX(minero_ix) (GDT_PLAYER_TASK_BASE + minero_ix)

/** Devuelve el selector de GDT de una tarea. */
uint16_t aux_get_gdt_selector(uint8_t ix) {
    uint16_t gdt_ix = AUX_GET_GDT_IX(ix);
    // Le agrega el DPL para usuario.
    return (gdt_ix << 3) | 0x3;
}


// -----------------------------------------------------------------------------
// Funciones publicas
// -----------------------------------------------------------------------------


void sched_init() {

    // Inicializa todas las tareas como muertas.
    for (uint32_t i = 0; i < 20; ++i) {
        sched_miners_state[i].alive = FALSE;
    }

}

int16_t sched_nextTask() {

    // Obtiene cual fue el ultimo judador en correr una tarea.
    uint32_t jugador_actual = (sched_this_miner < 10) ? PLAYER_A : PLAYER_B;
    // Obtiene el ID del otro jugador
    uint32_t jugador_opuesto =
        (jugador_actual == PLAYER_A) ? PLAYER_B : PLAYER_A;

    // Selector de la proxima tarea a ejecutarse.
    uint16_t ix;

    // Busca una tarea del jugador opuesto para despachar
    uint32_t hay_tarea_jugador_opuesto =
        aux_get_next_alive_miner(jugador_opuesto, &ix);

    if (hay_tarea_jugador_opuesto) {
        sched_this_miner = ix;
        sched_last_miner[jugador_opuesto] = ix % 10;
        // Devuelve el selector de tarea para la proxima tarea a ejecutar.
        return aux_get_gdt_selector(ix);
    }

    // Si no encontro una tarea del jugador opuesta, 
	// busca una del jugador actual
    uint32_t hay_tarea_este_jugador =
        aux_get_next_alive_miner(jugador_actual, &ix);
    if (hay_tarea_este_jugador) {
        sched_this_miner = ix;
        sched_last_miner[jugador_actual] = ix % 10;
        return aux_get_gdt_selector(ix);
    }

    // No hay tareas vivas para ningun jugador. Salta a la tarea idle.
    return GDT_TSS_IDLE << 3;
}

void sched_kill() {
    // Marca la tarea como muerta. Previene que vuelva a saltar a esta tarea.
    sched_miners_state[sched_this_miner].alive = FALSE;

    // Pone en 0 el bit presente de un minero.
    gdt[GDT_PLAYER_TASK_BASE + AUX_GET_GDT_IX(sched_this_miner)].p = 0;

    // Limpia la posicion del tablero del minero.
    uint32_t x = sched_miners_state[sched_this_miner].pos_x;
    uint32_t y = sched_miners_state[sched_this_miner].pos_y;
    screen_redraw_crystal(x, y);

    // Actualiza el estado del minero en pantalla
    screen_draw_miner_info(GET_PLAYER_ID(sched_this_miner),
                           GET_PLAYER_MINER_ID(sched_this_miner));
    screen_draw_players();
}

void sched_spawn(uint32_t player_id) {

    // -------------------------------------------------------------------------
    // Guardas que impiden spawnear
    // -------------------------------------------------------------------------

    // No spawnea un nuevo minero si ya gasto sus 20 spawns.
    if (sched_miners_left[player_id] == 0) {
        return;
    }

    // No spawnea un nuevo minero si ya tiene 20 mineros vivos.
    if (! aux_exists_free_slot(player_id) ) {
        return;
    }

    // -------------------------------------------------------------------------
    // Spawnea una nueva tarea.
    // -------------------------------------------------------------------------

    // Decrementa cant mineros restantes
    sched_miners_left[player_id]--;
    screen_draw_miners_left();

    // Obtiene el indice en `sched_miners_state` para la nueva tarea.
    uint32_t slot = aux_get_next_free_slot(player_id);
    // Obtiene el indice en la GDT para la nueva tarea
    uint32_t gdt_ix = GDT_PLAYER_TASK_BASE + slot;

    // Completa una TSS para la tarea nueva.
    tss* new_tss = (tss*) mmu_nextFreeKernelPage();
    fill_tss(new_tss, player_id, sched_player_row[player_id]);
    // Completa el descriptor de la GDT para la nueva TSS.
    tss_initGdtEntry(gdt_ix, new_tss, 3);

    // Inicializa el estado del scheduler.
    sched_miners_state[slot].pos_x = (player_id == PLAYER_A) ? 0 : 77;
    sched_miners_state[slot].pos_y = sched_player_row[player_id];
    sched_miners_state[slot].n_cristales = 0;
    sched_miners_state[slot].alive = TRUE;

    // Dibuja el minero
    uint32_t x = (player_id == PLAYER_A) ? 0 : 77;
    uint32_t y = sched_player_row[player_id];
    screen_draw_miner(player_id, x, y);

    // Dibuja el estado del minero
    screen_draw_miner_info(player_id, slot % 10);

}
