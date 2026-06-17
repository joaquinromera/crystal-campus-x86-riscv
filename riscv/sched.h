#pragma once

#include <stdint.h>

void sched_init();

// Devuelve el índice del próximo minero [0..19], o -1 si no hay ninguno ejecutable
int32_t sched_nextTask();

void sched_kill();

// Devuelve la ranura del minero creado [0..19], o -1 si no se puede crear
int32_t sched_spawn(uint32_t player_id);

