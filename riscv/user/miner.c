#include <stdint.h>

#include "defines.h"
#include "syscall.h"

void user_main() {
  uint32_t id = syscall_getId();

  if (FALSE) {
    for (int i = 0; i < 10; i++) syscall_move(Forward);
    for (int i = 0; i < 5; i++) syscall_move(Right);
    for (int i = 0; i < 5; i++) syscall_move(Forward);
    for (int i = 0; i < 5; i++) syscall_move(Left);
    for (int i = 0; i < 3; i++) syscall_move(Back);
    for (;;) {
    }
  }

  // Tarea 5: provocar una falla (el TP x86 usaba un page fault para validar).
  if (id == 5) {
    syscall_move(Forward);
    syscall_move(Forward);
    syscall_move(Forward);
    syscall_move(Forward);
    syscall_move(Forward);
    syscall_move(Right);
    syscall_move(Right);
    syscall_move(Right);

    *(volatile uint32_t*)0x0 = 0xDEADBEEF;
    for (;;) {
    }
  }

  // Comportamiento default: avanzar juntando cristales y luego volver a la base
  for (;;) {
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
