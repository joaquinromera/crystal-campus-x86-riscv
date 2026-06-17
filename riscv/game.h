#pragma once

#include <stdint.h>

typedef enum e_direction {
  Right = 1,
  Left = 2,
  Forward = 3,
  Back = 4,
  Up = 5,
  Down = 6,
} e_direction_t;

void game_init();
void game_move(e_direction_t d);
int32_t game_take();
uint32_t game_get_id();
uint32_t game_get_player();
void game_move_player(uint32_t player_id, e_direction_t direction);

