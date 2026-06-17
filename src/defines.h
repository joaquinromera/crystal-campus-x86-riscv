/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

    Definiciones globales del sistema.
*/

#ifndef __DEFINES_H__
#define __DEFINES_H__

#include "screen.h"

/* Bool */
/* -------------------------------------------------------------------------- */
#define TRUE                    0x00000001
#define FALSE                   0x00000000
#define ERROR                   1

/* Atributos paginas */
/* -------------------------------------------------------------------------- */
#define PAG_P                   0x00000001
#define PAG_R                   0x00000000
#define PAG_RW                  0x00000002
#define PAG_S                   0x00000000
#define PAG_US                  0x00000004
#define PAGE_SIZE				0x00001000

/* Misc */
/* -------------------------------------------------------------------------- */
#define SIZE_N                  40 // X
#define SIZE_M                  78 // Y

/* Indices en la gdt */
/* -------------------------------------------------------------------------- */
#define GDT_COUNT 50

#define GDT_IDX_NULL_DESC           0
#define GDT_DATOS_KERNEL            18
#define GDT_DATOS_USER              19
#define GDT_CODIGO_KERNEL           20
#define GDT_CODIGO_USER             21
#define GDT_SCREEN                  22
#define GDT_TSS_IDLE                23
#define GDT_TSS_INIT                24

// Indice de la primer tarea en la GDT
#define GDT_PLAYER_TASK_BASE        25


/* Offsets en la gdt */
/* -------------------------------------------------------------------------- */
#define GDT_OFF_NULL_DESC           (GDT_IDX_NULL_DESC << 3)
#define GDT_OFF_DATOS_KERNEL        (GDT_DATOS_KERNEL << 3)
#define GDT_OFF_DATOS_USER          ((GDT_DATOS_USER << 3) | 0x3)
#define GDT_OFF_CODIGO_KERNEL       (GDT_CODIGO_KERNEL << 3)
#define GDT_OFF_CODIGO_USER         ((GDT_CODIGO_USER << 3) | 0x3)
#define GDT_OFF_SCREEN              (GDT_SCREEN << 3)


/* Selectores de segmentos */
/* -------------------------------------------------------------------------- */

/* Direcciones de memoria */
/* -------------------------------------------------------------------------- */
#define BOOTSECTOR               0x00001000 /* direccion fisica de comienzo del bootsector (copiado) */
#define KERNEL                   0x00001200 /* direccion fisica de comienzo del kernel */
#define VIDEO                    0x000B8000 /* direccion fisica del buffer de video */
#define VIDEO_LIMIT              (2 * VIDEO_FILS * VIDEO_COLS - 1) /* Se multiplica por 2 porque cada celda ocupa 2 bytes. */
#define VIDEO_FILS               50
#define VIDEO_COLS               80
#define FREE_KERNEL_PAGES_START  0x00100000 
#define FREE_TASKS_PAGES_START   0x01c60000
// Posicion base del mapa de juego donde se ubica codigo de las tareas.
#define MAPA_BASE                0x00400000

/* TSS */
/* ------------------------------------------------------------------------ */

#define TSS_SIZE                 0x67 // Tamano tss ver manual

/* Tablero */
/* ------------------------------------------------------------------------ */
#define TABLERO_N_COLS 78
#define TABLERO_N_FILS 40


/* Identificadores de los jugadores */
/* ------------------------------------------------------------------------ */
#define PLAYER_A                0x0
#define PLAYER_B                0x1

/* Identificadores de los controles de los jugadores */
/* ------------------------------------------------------------------------ */
#define PLAYER_A_MOVE_UP        0x11
#define PLAYER_A_MOVE_DOWN      0x1F
#define PLAYER_A_DISPATCH       0x2A
#define PLAYER_B_MOVE_UP        0x17
#define PLAYER_B_MOVE_DOWN      0x25
#define PLAYER_B_DISPATCH       0x36

/* Direcciones virtuales de código, stack y datos */
/* Estas se usan para completar TSS */
/* -------------------------------------------------------------------------- */
#define TASK_CODE               0x08000000 /* direccion virtual del codigo */
// Base del stack de una tarea
#define TASK_STACK              0x08000000 + 2 * PAGE_SIZE
// Direccion virtual (dir tarea) del su area compartida
#define TASK_COMPARTIDA         0x08000000 + 2 * PAGE_SIZE
// Direccion de stack del kernel
#define KERNEL_STACK            0x00026000

/* Direcciones fisicas de codigos */
/* -------------------------------------------------------------------------- */
/* En estas direcciones estan los códigos de todas las tareas. De aqui se
 * copiaran al destino indicado por TASK_<i>_CODE_ADDR.
 */

/* Direcciones fisicas de directorios y tablas de paginas del KERNEL */
/* -------------------------------------------------------------------------- */
#define KERNEL_PAGE_DIR          0x00027000
#define KERNEL_PAGE_TABLE_0      0x00028000
#define CRYSTALS_MAP             0x00029000

/* Logica del juego */
/* -------------------------------------------------------------------------- */
// Un minero puede cargar hasta 30 cristales.
#define GAME_MAX_CRYSTALS        30

#endif  /* !__DEFINES_H__ */
