/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de funciones del scheduler
*/

#ifndef __SCHED_H__
#define __SCHED_H__

#include "stdint.h"
#include "screen.h"
#include "tss.h"

// -----------------------------------------------------------------------------
// Funciones publicas
// -----------------------------------------------------------------------------

void sched_init();

/** Devuelve la proxima tarea a despachar. Alterna entre ambos jugadores. Si
 *  ningun jugador tiene tareas vivas, devuelve un selector para la tarea IDLE.
 *
 * Devuelve un selector de tarea.
 */
int16_t sched_nextTask();

/** Marca la tarea de jugador actual como muerta. Previene que el scheduler
 *  vuelva a saltar a ella.
 *
 *  No hace nada por detener la ejecucion de esta tarea en este quantum. Es
 *  trabajo del ISR que llamo a matarla de saltar a la proxima tarea o la tarea
 *  idle.
 */
void sched_kill();

/** Crea una nueva tarea para un jugador. No hace nada si el jugador tiene 10
 * tareas activas o si no le quedan spawns restantes.
 */
void sched_spawn(uint32_t player_id);

#endif /* !__SCHED_H__ */
