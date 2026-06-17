/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
*/

#include "game.h"

void aux_init_tablero();

// -----------------------------------------------------------------------------
// Estado
// -----------------------------------------------------------------------------

uint32_t game_puntaje[2] = {0, 0};

// -----------------------------------------------------------------------------
// funciones
// -----------------------------------------------------------------------------

/** Verifica que la coordenada este en rango para el tablero */
#define AUX_INRANGE_X(x) (0 <= x && x <= SIZE_M)

#define GET_BASE(jugador) \
    ((jugador == PLAYER_A) ? 0 : 77)

#define GAME_WON() \
    (game_puntaje[PLAYER_A] >= 300 || game_puntaje[PLAYER_B] >= 300) ? 1 : 0

#define GET_WINNER() \
    (game_puntaje[PLAYER_A] > game_puntaje[PLAYER_B]) ? PLAYER_A : PLAYER_B

void aux_check_game_over()
{
    if (GAME_WON())
    {
        for (uint8_t i = 0; i < 20; ++i)
        {
            sched_miners_state[i].alive = 0;
        }
        screen_game_won(GET_WINNER());
    }
}

void game_init()
{
    // Inicializa y dibuja los cristales en pantalla
    aux_init_tablero();
    screen_draw_crystals();
}

uint32_t game_get_id()
{
    return 1 + (((uint16_t)rtr() >> 3) - GDT_PLAYER_TASK_BASE) % 10;
}

/** Devuelve el nro del jugador actual */
uint32_t game_get_player()
{
    // Indice en la gdt de esta tarea.
    uint16_t gdt_ix = rtr() >> 3;
    uint16_t task_id = gdt_ix - GDT_PLAYER_TASK_BASE;
    return (task_id < 10) ? 0 : 1;
}

// El operador `%` no es el modulo, es el resto. Esto me tira el modulo posta.
int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

void game_move(e_direction_t d)
{

    // Obtiene puntero al estado de esta tarea.
    sched_miner_state_t *player = &sched_miners_state[sched_this_miner];

    // ------------------------------------------------------------------------
    // Calcula defasaje
    // ------------------------------------------------------------------------

    int32_t mov_x = 0;
    int32_t mov_y = 0;

    // Arma los offsets
    if (d == Forward)
    {
        mov_x = 1;
    }
    else if (d == Back)
    {
        mov_x = -1;
    }
    if (d == Left)
    {
        mov_y = 1;
    }
    else if (d == Right)
    {
        mov_y = -1;
    };

    // Espeja los movimientos para el jugador de la derecha.
    if (game_get_player() == 1)
    {
        mov_x *= -1;
        mov_y *= -1;
    }

    // Sumo delta de x. No esta definido que pasa si me paso de uno de los
    // limites verticales.
    int32_t nuevo_x = player->pos_x + mov_x;
    // Sumo delta de Y. Usa modulo si se pasa.
    int32_t nuevo_y = mod(player->pos_y + mov_y, TABLERO_N_FILS);

    // Mata la tarea si se pasa de rango en x.
    if (!AUX_INRANGE_X(nuevo_x))
    {
        sched_kill();
        return;
    }

    // ------------------------------------------------------------------------
    // Copia el codigo a la nueva ubicacion
    // ------------------------------------------------------------------------

    uint32_t addr_src = GET_ADDR_TABLERO(player->pos_x, player->pos_y);
    uint32_t addr_dst = GET_ADDR_TABLERO(nuevo_x, nuevo_y);

    // copia los datos de las 2 pags de codigo de la tarea
    copy_page(addr_src, addr_dst);
    copy_page(addr_src + PAGE_SIZE, addr_dst + PAGE_SIZE);

    // Actualiza el esquema de memoria de esta tarea para que el codigo apunte
    // a la nueva posicion en el tablero. Tiene que tener permiso de escritura
    // porque esta todo junto codigo y stack.
    uint32_t task_code_attrs = PAG_RW | PAG_P | PAG_US;
    page_dir_entry_t *cr3 = (page_dir_entry_t *)rcr3();
    mmu_mapPage(TASK_CODE, cr3, addr_dst, task_code_attrs);
    mmu_mapPage(TASK_CODE + PAGE_SIZE,
                cr3,
                addr_dst + PAGE_SIZE,
                task_code_attrs);

    // Dibuja el mapa
    screen_draw_miner(GET_PLAYER_ID(sched_this_miner), nuevo_x, nuevo_y);
    screen_redraw_crystal(player->pos_x, player->pos_y);
    screen_draw_players();

    // Actualizamos coords en estado del juego.
    player->pos_x = nuevo_x;
    player->pos_y = nuevo_y;

    // Se fija si es necesario incrementar puntaje de los jugadores
    uint32_t player_id = game_get_player();
    if (nuevo_x == GET_BASE(player_id))
    {
        // Descarga los cristales del minero al puntaje del jugador.
        game_puntaje[player_id] += player->n_cristales;
        player->n_cristales = 0;

        aux_check_game_over();

        // Reescribe # cristales del minero
        screen_draw_miner_info(GET_PLAYER_ID(sched_this_miner),
                               GET_PLAYER_MINER_ID(sched_this_miner));
        // Reescribe # cristales del jugador
        screen_draw_score();
    }
}

uint32_t game_take()
{

    // Obtiene posicion del minero que llamo la syscall.
    sched_miner_state_t *player =
        &sched_miners_state[sched_this_miner];
    int32_t x = player->pos_x;
    int32_t y = player->pos_y;

    // cant cristales en esa posicion.
    uint32_t crystal_size = crystal_map[x][y];

    // Si no hay cristal devuelve -1;
    if (crystal_size == 0)
        return -1;

    // Se fija si esta dentro de la capacidad del minero tomar ese cristal.
    uint32_t suma = player->n_cristales + crystal_size;
    if (suma <= GAME_MAX_CRYSTALS)
    {
        // Decrementa cristales en el mapa
        crystal_map[x][y] = 0;
        // Incrementa cantidad de cirstales en el jugador.
        player->n_cristales = suma;

        // Imprime el estado del minero en pantalla.
        screen_draw_miner_info(GET_PLAYER_ID(sched_this_miner),
                               GET_PLAYER_MINER_ID(sched_this_miner));

        return crystal_size;
    }
    else
    {
        return 0; // No se mino nada.
    }
}

void game_move_player(uint32_t player_id, e_direction_t direction)
{

    int32_t row = sched_player_row[player_id];

    // Desplaza
    if (direction == Up)
    {
        row--;
    }
    if (direction == Down)
    {
        row++;
    }

    // Wrapea el desplazamiento entre [0:40)
    if (row == -1)
    {
        row = SIZE_N - 1;
    }
    if (row == SIZE_N)
    {
        row = 0;
    }

    // Asigna el desplazamiento
    sched_player_row[player_id] = row;

    // Redibuja los jugadores
    screen_draw_players();
}

void aux_init_tablero()
{

    for (uint32_t x = 0; x < TABLERO_N_COLS; ++x)
    {
        for (uint32_t y = 0; y < TABLERO_N_FILS; ++y)
        {
            crystal_map[x][y] = 0;
        }
    }

    for (uint32_t x = 15; x < TABLERO_N_COLS - 15; ++x)
    {
        for (uint32_t y = 10; y < TABLERO_N_FILS - 10; ++y)
        {
            crystal_map[x][y] = 1;
        }
    }

    for (uint32_t x = 25; x < TABLERO_N_COLS - 25; ++x)
    {
        for (uint32_t y = 15; y < TABLERO_N_FILS - 15; ++y)
        {
            crystal_map[x][y] = 2;
        }
    }

    for (uint32_t x = 30; x < TABLERO_N_COLS - 30; ++x)
    {
        for (uint32_t y = 18; y < TABLERO_N_FILS - 18; ++y)
        {
            crystal_map[x][y] = 3;
        }
    }
}
