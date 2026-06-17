#pragma once

#include <stdint.h>

// Funciones auxiliares de paginación Sv39 + tablero como memoria del TP3

void mmu_init();

// Cambiar el espacio de direcciones actual (escribe satp + sfence.vma)
void mmu_switch_to_idle();
void mmu_switch_to_task(uint32_t slot);

// Inicializar el espacio de direcciones de una tarea minera y ubicar su imagen inicial de código/stack en la celda del tablero en (x,y)
void mmu_task_init(uint32_t slot, uint32_t player_id, uint32_t x, uint32_t y);

// Implementa el comportamiento del TP3 copir la imagen de 8 KB del minero desde (old_x,old_y)
// a (new_x,new_y) y remapea TASK_CODE a la nueva celda física
void mmu_task_move(uint32_t slot, uint32_t old_x, uint32_t old_y, uint32_t new_x, uint32_t new_y);
