/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de funciones del scheduler
*/

#ifndef __SCREEN_H__
#define __SCREEN_H__

/* Definicion de la pantalla */
#define VIDEO_FILS 50
#define VIDEO_COLS 80

#include "stdint.h"
#include "colors.h"
#include "defines.h"
#include "state.h"
#include "i386.h"

/* Estructura de para acceder a memoria de video */
typedef struct ca_s {
    uint8_t c;
    uint8_t a;
} ca;

void print(const char* text, uint32_t x, uint32_t y, uint16_t attr);
void print_dec(uint32_t numero, uint32_t size, uint32_t x, uint32_t y, uint16_t attr);
void print_hex(uint32_t numero, int32_t size, uint32_t x, uint32_t y, uint16_t attr);

void screen_drawBox(uint32_t fInit, uint32_t cInit,
                    uint32_t fSize, uint32_t cSize,
                    uint8_t character, uint8_t attr );


void screen_draw();

void screen_game_won(uint8_t jugador);

// -----------------------------------------------------------------------------
// Funciones para dibujar cosas del juego
// -----------------------------------------------------------------------------

/** Dibuja los jugadores en los bordes de la pantalla */
void screen_draw_players();

/** Dibuja los mineros en el campo de juego */
void screen_draw_miner(uint32_t player_id, uint32_t x, uint32_t y);

/** Dibuja el valor de un cristal especifico */
void screen_redraw_crystal(uint32_t x, uint32_t y);

/** Dibuja todos los cristales de `crystal_map` en el tablero. */
void screen_draw_crystals();

/** Dibuja cantidad de mineros restantes */
void screen_draw_miners_left();

/** Escribe la informacion del minero en pantalla. Debe llamarse cada vez que
 *  cambie el estado de n_cristales del minero. */
void screen_draw_miner_info(uint16_t player_id, uint32_t miner_id);

/** Escribe el puntaje de ambos jugadores en pantalla */
void screen_draw_score();

#endif  /* !__SCREEN_H__ */
