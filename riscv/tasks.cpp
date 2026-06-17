#include "syscall.h"

#include "defines.h"

extern "C" void miner_task() {
  uint32_t id = syscall_getId();

  if (FALSE) {
    // se mantiene por paridad con el TP original
    for (int i = 0; i < 10; i++) syscall_move(Forward);
    for (int i = 0; i < 5; i++) syscall_move(Right);
    for (int i = 0; i < 5; i++) syscall_move(Forward);
    for (int i = 0; i < 5; i++) syscall_move(Left);
    for (int i = 0; i < 3; i++) syscall_move(Back);
    while (1) {
    }
  } else if (id == 5) {
    // Tarea fallida en el original (page fault en x86). Acá simplemente termina.
    syscall_move(Forward);
    syscall_move(Forward);
    syscall_move(Forward);
    syscall_move(Forward);
    syscall_move(Forward);
    syscall_move(Right);
    syscall_move(Right);
    syscall_move(Right);
    return;
  } else {
    // Comportamiento principal: avanzar juntando cristales y luego volver a la base
    while (1) {
      int32_t avanzar = 1;
      int32_t x = 0;
      int32_t crystals = 0;
      while (avanzar) {
        syscall_move(Forward);
        x++;
        int32_t a = syscall_take();
        if (a != -1) crystals = crystals + a;
        if (crystals >= 15) avanzar = 0;
        if (x == 50) avanzar = 0;
      }
      for (int i = 0; i < x; i++) {
        syscall_move(Back);
      }
      syscall_move(Right);
    }
  }
}

