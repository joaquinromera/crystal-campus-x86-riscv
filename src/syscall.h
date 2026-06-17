/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de la tabla de descriptores globales
*/

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "stdint.h"
#include "game.h"
// #include "screen.h"

#define LS_INLINE static __inline __attribute__((always_inline))

/*
 * Syscalls
 */

LS_INLINE uint32_t syscall_getId() {
    int32_t func = 760279;
    int32_t ret;
  __asm__ volatile (
    "int $47"          /* make the request to the OS */
    : "=a" (ret)       /* return value in eax ("a") */
    : "a"  (func)      /* pass in eax ("a") arg "func"*/
    : "memory", "cc"); /* announce to the compiler that the memory and condition codes have been modified */
    return ret;
}

LS_INLINE int32_t syscall_take() {
    int32_t func = 310311;
    int32_t ret;
  __asm__ volatile (
    "int $47"        /* make the request to the OS */
    : "=a" (ret)     /* return result in eax ("a") */
    : "a"  (func)    /* pass in eax ("a") */
    : "memory", "cc"); /* announce to the compiler that the memory and condition codes have been modified */
    return ret;
}

LS_INLINE void syscall_move(e_direction_t d) {
    int32_t func = 177788;
  __asm__ volatile (
    "int $47"      /* make the request to the OS */
    : /*  */
    : "a"  (func), /* pass in eax ("a") */
      "b"  (d)     /* pass in ebx ("b") */
    : "memory", "cc"); /* announce to the compiler that the memory and condition codes have been modified */
}

#endif  /* !__SYSCALL_H__ */
