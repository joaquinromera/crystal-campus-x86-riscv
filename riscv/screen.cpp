#include "screen.h"

#include "colors.h"
#include "defines.h"
#include "state.h"
#include "uart.h"

namespace {

struct ca {
  uint8_t c;
  uint8_t a;
};

static ca g_video[VIDEO_FILS][VIDEO_COLS];
static uint32_t g_batch = 0;
static uint8_t g_last_attr = 0xFF;
static uint32_t g_tty_inited = 0;

uint8_t ansi_fg_code(uint8_t fg) {
  static constexpr uint8_t kMap[16] = {
      30, // negro
      34, // azul
      32, // verde
      36, // cyan
      31, // rojo
      35, // magenta
      33, // marrón/amarillo
      37, // gris claro
      90, // gris oscuro
      94, // azul claro
      92, // verde claro
      96, // cyan claro
      91, // rojo claro
      95, // magenta claro
      93, // marrón/amarillo claro
      97, // blanco
  };
  return kMap[fg & 0xF];
}

uint8_t ansi_bg_code(uint8_t bg) {
  static constexpr uint8_t kMap[8] = {
      40, // negro
      44, // azul
      42, // verde
      46, // cyan
      41, // rojo
      45, // magenta
      43, // marrón/amarillo
      47, // gris claro
  };
  return kMap[bg & 0x7];
}

void ansi_set_attr(uint8_t attr) {
  attr &= 0x7F; // ignorar bit de parpadeo
  if (attr == g_last_attr) return;

  uint8_t fg = attr & 0x0F;
  uint8_t bg = (attr >> 4) & 0x07;

  uart::puts("\x1b[");
  uart::write_u32_dec(ansi_fg_code(fg));
  uart::putc(';');
  uart::write_u32_dec(ansi_bg_code(bg));
  uart::putc('m');

  g_last_attr = attr;
}

void ansi_move_cursor(uint32_t x, uint32_t y) {
  uart::puts("\x1b[");
  uart::write_u32_dec(y + 1);
  uart::putc(';');
  uart::write_u32_dec(x + 1);
  uart::putc('H');
}

void screen_render_cell(uint32_t x, uint32_t y) {
  if (x >= VIDEO_COLS || y >= VIDEO_FILS) return;
  ansi_move_cursor(x, y);
  ansi_set_attr(g_video[y][x].a);
  uart::putc(static_cast<char>(g_video[y][x].c ? g_video[y][x].c : ' '));
}

void screen_put_cell(uint32_t x, uint32_t y, uint8_t c, uint8_t attr) {
  if (x >= VIDEO_COLS || y >= VIDEO_FILS) return;
  g_video[y][x].c = c;
  g_video[y][x].a = attr;
  if (!g_batch) screen_render_cell(x, y);
}

void tty_init_once() {
  if (g_tty_inited) return;
  g_tty_inited = 1;

  // Pantalla alternativa + ocultar cursor + limpiar
  uart::puts("\x1b[?1049h");
  uart::puts("\x1b[?25l");
  uart::puts("\x1b[2J");
  uart::puts("\x1b[H");
  g_last_attr = 0xFF;
}

} // fin del namespace

void screen_set_batch(uint32_t enabled) {
  g_batch = enabled ? 1u : 0u;
}

void screen_flush() {
  tty_init_once();

  uart::puts("\x1b[H");
  g_last_attr = 0xFF;

  for (uint32_t y = 0; y < VIDEO_FILS; y++) {
    for (uint32_t x = 0; x < VIDEO_COLS; x++) {
      ansi_set_attr(g_video[y][x].a);
      uart::putc(static_cast<char>(g_video[y][x].c ? g_video[y][x].c : ' '));
    }
    uart::putc('\n');
  }
}

void print(const char* text, uint32_t x, uint32_t y, uint16_t attr) {
  for (int32_t i = 0; text && text[i] != 0; i++) {
    screen_put_cell(x, y, static_cast<uint8_t>(text[i]), static_cast<uint8_t>(attr));
    x++;
    if (x == VIDEO_COLS) {
      x = 0;
      y++;
    }
  }
}

void print_dec(uint32_t numero, uint32_t size, uint32_t x, uint32_t y, uint16_t attr) {
  static constexpr char digits[] = "0123456789";

  for (uint32_t i = 0; i < size; i++) {
    uint32_t resto = numero % 10;
    numero = numero / 10;
    screen_put_cell(x + size - i - 1, y, digits[resto], static_cast<uint8_t>(attr));
  }
}

void print_hex(uint32_t numero, int32_t size, uint32_t x, uint32_t y, uint16_t attr) {
  uint8_t hexa[8];
  static constexpr char letras[] = "0123456789ABCDEF";
  hexa[0] = letras[(numero & 0x0000000F) >> 0];
  hexa[1] = letras[(numero & 0x000000F0) >> 4];
  hexa[2] = letras[(numero & 0x00000F00) >> 8];
  hexa[3] = letras[(numero & 0x0000F000) >> 12];
  hexa[4] = letras[(numero & 0x000F0000) >> 16];
  hexa[5] = letras[(numero & 0x00F00000) >> 20];
  hexa[6] = letras[(numero & 0x0F000000) >> 24];
  hexa[7] = letras[(numero & 0xF0000000) >> 28];
  for (int32_t i = 0; i < size; i++) {
    screen_put_cell(x + size - i - 1, y, hexa[i], static_cast<uint8_t>(attr));
  }
}

void screen_drawBox(uint32_t fInit,
                    uint32_t cInit,
                    uint32_t fSize,
                    uint32_t cSize,
                    uint8_t character,
                    uint8_t attr) {
  for (uint32_t f = fInit; f < fInit + fSize; f++) {
    for (uint32_t c = cInit; c < cInit + cSize; c++) {
      screen_put_cell(c, f, character, attr);
    }
  }
}

void screen_draw() {
  // Dibujar fondo del mapa
  screen_drawBox(1, 1, 40, 78, 0, C_BG_LIGHT_GREY);

  // Cajas de puntaje de los jugadores
  screen_drawBox(44, 31, 3, 9, 0, C_BG_RED);
  screen_drawBox(44, 40, 3, 9, 0, C_BG_BLUE);

  // Índices de vidas
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

  screen_draw_crystals();
}

void screen_game_won(uint8_t jugador) {
  uint16_t color = ((jugador == PLAYER_A) ? C_BG_RED : C_BG_BLUE) | C_FG_WHITE;

  screen_drawBox(1, 20, 40, 40, ' ', static_cast<uint8_t>(color));

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

  // Limpiar barras laterales
  screen_drawBox(1, col_player_a, SIZE_N, 1, ' ', C_BG_BLACK);
  screen_drawBox(1, col_player_b, SIZE_N, 1, ' ', C_BG_BLACK);

  // Dibujar jugadores
  print(" ", col_player_a, 1 + sched_player_row[PLAYER_A], C_PLAYER_A);
  print(" ", col_player_b, 1 + sched_player_row[PLAYER_B], C_PLAYER_B);
}

void screen_draw_miner(uint32_t player_id, uint32_t x, uint32_t y) {
  uint32_t color = (player_id == PLAYER_A) ? C_PLAYER_A : C_PLAYER_B;
  print(" ", x + 1, y + 1, color);
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
  print_dec(n, 1, x + 1, y + 1, C_CRYSTAL(n));
}

void screen_draw_miners_left() {
  print_dec(sched_miners_left[PLAYER_A], 2, 31, 48, C_PLAYER_A2);
  print_dec(sched_miners_left[PLAYER_B], 2, 47, 48, C_PLAYER_B2);
}

// Desplazamientos horizontales para los bloques de información de mineros
#define SCREEN_MINER_INFO_OFFSET_A 1
#define SCREEN_MINER_INFO_OFFSET_B 50
#define SCREEN_MINER_INFO_SIZE 3

void screen_draw_miner_info(uint16_t player_id, uint32_t miner_id) {
  uint32_t x_offset = (player_id == PLAYER_A) ? SCREEN_MINER_INFO_OFFSET_A : SCREEN_MINER_INFO_OFFSET_B;
  uint32_t x = x_offset + SCREEN_MINER_INFO_SIZE * miner_id;

  uint32_t color = (player_id == PLAYER_A) ? C_PLAYER_A2 : C_PLAYER_B2;

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
