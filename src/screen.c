/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de funciones del scheduler
*/

#include "screen.h"

void print(const char* text, uint32_t x, uint32_t y, uint16_t attr) {
    ca (*p)[VIDEO_COLS] = (ca (*)[VIDEO_COLS]) VIDEO; // magia
    int32_t i;
    for (i = 0; text[i] != 0; i++) {
        p[y][x].c = (uint8_t) text[i];
        p[y][x].a = (uint8_t) attr;
        x++;
        if (x == VIDEO_COLS) {
            x = 0;
            y++;
        }
    }
}

void print_dec(uint32_t numero, uint32_t size, uint32_t x, uint32_t y, uint16_t attr) {
    ca (*p)[VIDEO_COLS] = (ca (*)[VIDEO_COLS]) VIDEO; // magia
    uint32_t i;
    uint8_t letras[16] = "0123456789";

    for(i = 0; i < size; i++) {
        uint32_t resto  = numero % 10;
        numero = numero / 10;
        p[y][x + size - i - 1].c = letras[resto];
        p[y][x + size - i - 1].a = attr;
    }
}

void print_hex(uint32_t numero, int32_t size, uint32_t x, uint32_t y, uint16_t attr) {
    ca (*p)[VIDEO_COLS] = (ca (*)[VIDEO_COLS]) VIDEO; // magia
    int32_t i;
    uint8_t hexa[8];
    uint8_t letras[16] = "0123456789ABCDEF";
    hexa[0] = letras[ ( numero & 0x0000000F ) >> 0  ];
    hexa[1] = letras[ ( numero & 0x000000F0 ) >> 4  ];
    hexa[2] = letras[ ( numero & 0x00000F00 ) >> 8  ];
    hexa[3] = letras[ ( numero & 0x0000F000 ) >> 12 ];
    hexa[4] = letras[ ( numero & 0x000F0000 ) >> 16 ];
    hexa[5] = letras[ ( numero & 0x00F00000 ) >> 20 ];
    hexa[6] = letras[ ( numero & 0x0F000000 ) >> 24 ];
    hexa[7] = letras[ ( numero & 0xF0000000 ) >> 28 ];
    for(i = 0; i < size; i++) {
        p[y][x + size - i - 1].c = hexa[i];
        p[y][x + size - i - 1].a = attr;
    }
}

void screen_drawBox(uint32_t fInit,
                    uint32_t cInit,
                    uint32_t fSize,
                    uint32_t cSize,
                    uint8_t character,
                    uint8_t attr ) {
    ca (*p)[VIDEO_COLS] = (ca (*)[VIDEO_COLS]) VIDEO;
    uint32_t f;
    uint32_t c;
    for (f = fInit; f < fInit+fSize; f++) {
    for (c = cInit; c < cInit+cSize; c++) {
          p[f][c].c = character;
          p[f][c].a = attr;
    }}
}

void screen_draw() {
    // Regla (borrar)
    // for (uint8_t c = 0; c < VIDEO_COLS; ++c) {
    //     print_dec(c, 1, c, VIDEO_FILS-1, C_FG_LIGHT_MAGENTA);
    // }

    // Dibujar mapa
    screen_drawBox(1,1,40,78,0,C_BG_LIGHT_GREY);

    // Puntaje jugador rojo
    screen_drawBox(44,31,3,9,0,C_BG_RED);

    // Puntaje jugador azul
    screen_drawBox(44,40,3,9,0,C_BG_BLUE);


    // Nums de vidas
    uint8_t col = 1;
    for (uint8_t vida = 0; vida < 10; ++vida) {
        print_dec(vida, 1, col, 44, C_FG_LIGHT_RED);
        print_dec(vida, 1, col + 49, 44, C_FG_LIGHT_BLUE);
        col += 3;
    }

    screen_draw_score();
    screen_draw_players();
    screen_draw_miners_left();

    for (uint32_t i = 0; i < 10; ++i) {
        screen_draw_miner_info(PLAYER_A, i);
        screen_draw_miner_info(PLAYER_B, i);
    }

    // Dibuja todos los cristales en pantalla
    screen_draw_crystals();

}

void screen_game_won(uint8_t jugador) {

    // Texto Blanco. Fondo depende del jugador
    uint16_t color = 
        ((jugador == PLAYER_A) ? C_BG_RED : C_BG_BLUE) | C_FG_WHITE;

    // Dibuja Fondo
    screen_drawBox(1, 20, 40, 40, ' ', color);

    // Texto
    print("GAME OVER", 35, 15, color);
    if (jugador == PLAYER_A) {
        print("PLAYER RED WON", 33, 20, color);
    } else {
        print("PLAYER BLUE WON", 32, 20, color);
    }
}

void screen_draw_players() {

    uint32_t col_player_a = 0;
    uint32_t col_player_b = VIDEO_COLS - 1;

    // Pone en negro las barras de los costados
    screen_drawBox(1, col_player_a, SIZE_N, 1, ' ', C_BG_BLACK);
    screen_drawBox(1, col_player_b, SIZE_N, 1, ' ', C_BG_BLACK);

    // Dibuja jugadores
    print(" ", col_player_a, 1 + sched_player_row[PLAYER_A], C_PLAYER_A);
    print(" ", col_player_b, 1 + sched_player_row[PLAYER_B], C_PLAYER_B);

}


void screen_draw_miner(uint32_t player_id, uint32_t x, uint32_t y) {

    uint32_t color = (player_id == PLAYER_A) ? C_PLAYER_A : C_PLAYER_B;

    // Dibuja posicion nueva
    print(" ", x+1, y+1, color);

}

void screen_draw_crystals() {
    for (uint32_t x = 0; x < TABLERO_N_COLS; ++x) {
        for (uint32_t y = 0; y < TABLERO_N_FILS; ++y) {
            screen_redraw_crystal(x, y);
        }
    }
}


void screen_redraw_crystal(uint32_t x, uint32_t y) {
    uint32_t n = crystal_map[x][y];
    print_dec(n, 1, x+1, y+1, C_CRYSTAL(n));
}

void screen_draw_miners_left() {
    print_dec(sched_miners_left[PLAYER_A], 2, 31, 48, C_PLAYER_A2);
    print_dec(sched_miners_left[PLAYER_B], 2, 47, 48, C_PLAYER_B2);
}

// Offset horizontal para poner los datos de los mineros A
#define SCREEN_MINER_INFO_OFFSET_A 1
// Offset horizontal para poner los datos de los mineros B
#define SCREEN_MINER_INFO_OFFSET_B 50
// Cantidad de pixels horizontal que toma la info de un minero
#define SCREEN_MINER_INFO_SIZE     3
void screen_draw_miner_info(uint16_t player_id, uint32_t miner_id) {

    uint32_t x_offset = (player_id == PLAYER_A)
        ? SCREEN_MINER_INFO_OFFSET_A
        : SCREEN_MINER_INFO_OFFSET_B;
    uint32_t x = x_offset + SCREEN_MINER_INFO_SIZE * miner_id;

    uint32_t color = (player_id == PLAYER_A)
        ? C_PLAYER_A2
        : C_PLAYER_B2;

    uint32_t miner_ix = GET_MINER_IX(player_id, miner_id);
    if (sched_miners_state[miner_ix].alive) {
        uint32_t n = sched_miners_state[miner_ix].n_cristales;
        print_dec(n, 2, x, 46, color);
    } else {
        print("X ", x, 46, color);
    }
}

void screen_draw_score() {
    print_dec(game_puntaje[PLAYER_A], 3, 32, 45, C_PLAYER_A);
    print_dec(game_puntaje[PLAYER_B], 3, 41, 45, C_PLAYER_B);
}
