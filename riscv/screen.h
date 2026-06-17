#pragma once

#include <stdint.h>

// Mantiene la API original de src/screen.c, pero renderiza usando UART + ANSI.

void screen_set_batch(uint32_t enabled);
void screen_flush();

void print(const char* text, uint32_t x, uint32_t y, uint16_t attr);
void print_dec(uint32_t numero, uint32_t size, uint32_t x, uint32_t y, uint16_t attr);
void print_hex(uint32_t numero, int32_t size, uint32_t x, uint32_t y, uint16_t attr);

void screen_drawBox(uint32_t fInit,
                    uint32_t cInit,
                    uint32_t fSize,
                    uint32_t cSize,
                    uint8_t character,
                    uint8_t attr);

void screen_draw();
void screen_game_won(uint8_t jugador);

// Funciones auxiliares de dibujo específicas del juego
void screen_draw_players();
void screen_draw_miner(uint32_t player_id, uint32_t x, uint32_t y);
void screen_redraw_crystal(uint32_t x, uint32_t y);
void screen_draw_crystals();
void screen_draw_miners_left();
void screen_draw_miner_info(uint16_t player_id, uint32_t miner_id);
void screen_draw_score();

