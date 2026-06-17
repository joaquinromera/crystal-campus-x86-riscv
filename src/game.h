/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
*/

#ifndef __GAME_H__
#define __GAME_H__

#include "stdint.h"
#include "defines.h"
#include "screen.h"
#include "mmu.h"
#include "sched.h"

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
uint32_t game_take();
uint32_t game_get_id();

void game_move_player(uint32_t player_id, e_direction_t direction);

#endif  /* !__GAME_H__ */
