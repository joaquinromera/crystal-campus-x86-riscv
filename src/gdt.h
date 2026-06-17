/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de la tabla de descriptores globales
*/

#ifndef __GDT_H__
#define __GDT_H__

#include "stdint.h"
#include "defines.h"

#define LIMIT_00_15(addr) (addr & 0xFFFF)
#define LIMIT_16_19(addr) ((addr >> 16) & 0xF)
#define BASE_00_15(addr)  (addr & 0xFFFF)
#define BASE_16_23(addr)  ((addr >> 16) & 0xFF)
#define BASE_24_31(addr)  ((addr >> 24) & 0xFF)

typedef struct str_gdt_descriptor {
    uint16_t  gdt_length;
    uint32_t    gdt_addr;
} __attribute__((__packed__)) gdt_descriptor;

typedef struct str_gdt_entry {
    uint16_t  limit_0_15;
    uint16_t  base_0_15;
    uint8_t   base_23_16;
    uint8_t   type:4;
    uint8_t   s:1;
    uint8_t   dpl:2;
    uint8_t   p:1;
    uint8_t   limit_16_19:4;
    uint8_t   avl:1;
    uint8_t   l:1;
    uint8_t   db:1;
    uint8_t   g:1;
    uint8_t   base_31_24;
} __attribute__((__packed__, aligned (8))) gdt_entry;

/* Tabla GDT */
extern gdt_entry gdt[];
extern gdt_descriptor GDT_DESC;

#endif  /* !__GDT_H__ */
