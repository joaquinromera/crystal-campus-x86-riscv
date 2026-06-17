#pragma once

#include <stdint.h>

/* Bools */
#define TRUE  0x00000001u
#define FALSE 0x00000000u

/* Pantalla */
#define VIDEO_FILS 50
#define VIDEO_COLS 80

/* Tablero (igual al TP original) */
#define SIZE_N 40 /* filas */
#define SIZE_M 78 /* columnas */

#define TABLERO_N_COLS 78
#define TABLERO_N_FILS 40

/* Jugadores */
#define PLAYER_A 0x0
#define PLAYER_B 0x1

/* Lógica del juego */
#define GAME_MAX_CRYSTALS 30

/* Paginación / "tablero como memoria" (como TP3) */
#define PAGE_SIZE 4096u

/* Direcciones virtuales usadas por las tareas de usuario (como TP3) */
#define TASK_CODE 0x08000000ul
#define TASK_STACK (TASK_CODE + 2ul * PAGE_SIZE)
#define TASK_COMPARTIDA (TASK_CODE + 2ul * PAGE_SIZE)
