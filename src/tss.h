/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de estructuras para administrar tareas
*/

#ifndef __TSS_H__
#define __TSS_H__

#include "stdint.h"
#include "defines.h"
#include "i386.h"
#include "gdt.h"
#include "mmu.h"

typedef struct str_tss {
    uint16_t  ptl;
    uint16_t  unused0;
    uint32_t    esp0;
    uint16_t  ss0;
    uint16_t  unused1;
    uint32_t    esp1;
    uint16_t  ss1;
    uint16_t  unused2;
    uint32_t    esp2;
    uint16_t  ss2;
    uint16_t  unused3;
    uint32_t    cr3;
    uint32_t    eip;
    uint32_t    eflags;
    uint32_t    eax;
    uint32_t    ecx;
    uint32_t    edx;
    uint32_t    ebx;
    uint32_t    esp;
    uint32_t    ebp;
    uint32_t    esi;
    uint32_t    edi;
    uint16_t  es;
    uint16_t  unused4;
    uint16_t  cs;
    uint16_t  unused5;
    uint16_t  ss;
    uint16_t  unused6;
    uint16_t  ds;
    uint16_t  unused7;
    uint16_t  fs;
    uint16_t  unused8;
    uint16_t  gs;
    uint16_t  unused9;
    uint16_t  ldt;
    uint16_t  unused10;
    uint16_t  dtrap;
    uint16_t  iomap;
} __attribute__((__packed__, aligned (8))) tss;

extern tss tss_initial;
extern tss tss_idle;

/** Inicializa el descriptor en la GDT para la tarea inicial.
 *
 * El TSS de la tarea inicial es un placeholder para guardar el contexto al
 * realizar el primer salto de tarea.
 */
void tss_init();

/** Inicializa TSS para un jugador. Arma tambien su esquema de memoria.
 */
void fill_tss(tss* actual, uint32_t jugador, uint32_t fila);

/** Llena los valores de la TSS y el descriptor GDT de la tarea idle.
 * (Se pide en ej 6.b).
 */
void fill_tss_idle();

void tss_initGdtEntry(uint32_t ix, tss* base, uint32_t kernel);

#endif  /* !__TSS_H__ */
