#pragma once

#include <stdint.h>

#include "game.h"

// ABI de syscalls RISC-V (U-mode -> trap a S-mode vía ecall):
//   a7 = id de syscall
//   a0 = arg0 / valor de retorno

static inline uint32_t syscall_getId() {
  register uint64_t a7 asm("a7") = 760279;
  register uint64_t a0 asm("a0");
  asm volatile("ecall" : "=r"(a0) : "r"(a7) : "memory");
  return (uint32_t)a0;
}

static inline int32_t syscall_take() {
  register uint64_t a7 asm("a7") = 310311;
  register uint64_t a0 asm("a0");
  asm volatile("ecall" : "=r"(a0) : "r"(a7) : "memory");
  return (int32_t)a0;
}

static inline void syscall_move(e_direction_t d) {
  register uint64_t a7 asm("a7") = 177788;
  register uint64_t a0 asm("a0") = (uint64_t)d;
  asm volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
}
