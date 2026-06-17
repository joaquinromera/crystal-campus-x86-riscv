#pragma once

#include <stdint.h>

// Wrapper mínimo de SBI v0.2 (OpenSBI en QEMU virt)
// a0=error, a1=valor

struct SbiRet {
  long error;
  long value;
};

static inline SbiRet sbi_call(long ext, long fid,
                              long arg0, long arg1, long arg2,
                              long arg3, long arg4, long arg5) {
  register long a0 asm("a0") = arg0;
  register long a1 asm("a1") = arg1;
  register long a2 asm("a2") = arg2;
  register long a3 asm("a3") = arg3;
  register long a4 asm("a4") = arg4;
  register long a5 asm("a5") = arg5;
  register long a6 asm("a6") = fid;
  register long a7 asm("a7") = ext;
  asm volatile("ecall"
               : "+r"(a0), "+r"(a1)
               : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
               : "memory");
  return {a0, a1};
}

static constexpr long SBI_EXT_TIME = 0x54494D45; // "TIME"

static inline void sbi_set_timer(uint64_t stime_value) {
  (void)sbi_call(SBI_EXT_TIME, 0 /* set_timer */,
                 (long)stime_value, 0, 0, 0, 0, 0);
}

static inline uint64_t rdtime() {
  uint64_t value;
  asm volatile("rdtime %0" : "=r"(value));
  return value;
}

