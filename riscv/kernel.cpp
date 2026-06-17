#include <stdint.h>

#include "colors.h"
#include "defines.h"
#include "game.h"
#include "mmu.h"
#include "sched.h"
#include "screen.h"
#include "state.h"
#include "trap.h"
#include "uart.h"

extern "C" {
TrapFrame* g_current_trapframe = nullptr;
}

// Período del timer. Controla la "velocidad del juego" porque las syscalls `move/take` ceden a la tarea idle y recién retoman en el próximo pulso.
extern "C" const uint64_t g_timer_delta_ticks = 250'000;

namespace {

struct Task {
  TrapFrame tf;
  uint32_t initialized;
};

static Task g_tasks[20];
static TrapFrame g_idle_tf;
static uint32_t g_idle_initialized = 0;

static int32_t g_current_slot = -1; // -1 = idle

static uint32_t g_spinner = 0;
static uint32_t g_spawn_cooldown_a = 0;
static uint32_t g_spawn_cooldown_b = 0;
static constexpr uint32_t kSpawnCooldownTicks = 6;

// Disposición de la línea de encabezado (fila 0).
static constexpr uint16_t kHeaderAttr = C_BG_BLACK | C_FG_WHITE;
static constexpr uint32_t kHeaderY = 0;
static constexpr uint32_t kHeaderRedX = 1;
static constexpr char kHeaderRed[] = "RED: W/S spawn E(Q)";
static constexpr char kHeaderBlue[] = "BLUE: I/K spawn P(O)";
static constexpr uint32_t kHeaderBlueX = VIDEO_COLS - 3 - (sizeof(kHeaderBlue) - 1);
static constexpr char kHeaderExcPrefix[] = "TASK EXC ";
static constexpr uint32_t kHeaderExcWidth = (sizeof(kHeaderExcPrefix) - 1) + 2; // + 2 dígitos hex
static constexpr uint32_t kHeaderExcX = kHeaderBlueX - 1 - kHeaderExcWidth;
static constexpr uint32_t kHeaderExcHexX = kHeaderExcX + (sizeof(kHeaderExcPrefix) - 1);

static void draw_header() {
  print(kHeaderRed, kHeaderRedX, kHeaderY, kHeaderAttr);
  print(kHeaderBlue, kHeaderBlueX, kHeaderY, kHeaderAttr);
}

static void init_user_trapframe(TrapFrame& tf, uint64_t entry_pc) {
  tf = {};
  tf.sepc = entry_pc;
  tf.sp = TASK_STACK;

  // Volver a U-mode con interrupciones habilitadas (SIE se restaura desde SPIE en sret).
  // sstatus.SPIE = 1, sstatus.SPP = 0
  tf.sstatus = (1ull << 5);
}

static void init_idle_once() {
  if (g_idle_initialized) return;
  init_user_trapframe(g_idle_tf, TASK_CODE);
  g_idle_initialized = 1;
}

static void init_task_if_needed(uint32_t slot) {
  Task& t = g_tasks[slot];
  if (t.initialized) return;
  init_user_trapframe(t.tf, TASK_CODE);
  t.initialized = 1;
}

static void spawn_miner(uint32_t player_id) {
  int32_t slot = sched_spawn(player_id);
  if (slot < 0) return;
  const uint32_t u_slot = static_cast<uint32_t>(slot);
  // Resetear el contexto de usuario para que los mineros recreados arranquen desde un estado limpio.
  g_tasks[u_slot].initialized = 0;
  init_task_if_needed(u_slot);

  // Inicializar tablas de páginas por tarea y ubicar la tarea inicial en la celda del tablero donde se crea el minero.
  const uint32_t x = static_cast<uint32_t>(sched_miners_state[u_slot].pos_x);
  const uint32_t y = static_cast<uint32_t>(sched_miners_state[u_slot].pos_y);
  mmu_task_init(u_slot, player_id, x, y);
}

static void handle_key(uint8_t ch) {
  // Mostrar la última tecla recibida en hex (ayuda a validar la entrada UART sin
  // imprimir accidentalmente caracteres de control como '\r' en la terminal).
  print_hex(ch, 2, VIDEO_COLS - 2, 0, C_BG_BLACK | C_FG_WHITE);

  switch (ch) {
    case 'w':
    case 'W':
      game_move_player(PLAYER_A, Up);
      break;
    case 's':
    case 'S':
      game_move_player(PLAYER_A, Down);
      break;
    case 'e':
    case 'E':
    case 'q':
    case 'Q':
      if (g_spawn_cooldown_a == 0) {
        spawn_miner(PLAYER_A);
        g_spawn_cooldown_a = kSpawnCooldownTicks;
      }
      break;

    case 'i':
    case 'I':
      game_move_player(PLAYER_B, Up);
      break;
    case 'k':
    case 'K':
      game_move_player(PLAYER_B, Down);
      break;
    case 'p':
    case 'P':
    case 'o':
    case 'O':
      if (g_spawn_cooldown_b == 0) {
        spawn_miner(PLAYER_B);
        g_spawn_cooldown_b = kSpawnCooldownTicks;
      }
      break;

    default:
      break;
  }
}

static void poll_uart_input() {
  for (;;) {
    int c = uart::getc_nonblock();
    if (c < 0) break;
    handle_key(static_cast<uint8_t>(c));
  }
}

static void update_spinner() {
  static constexpr char kSpin[] = "|/-\\";
  char s[2] = {kSpin[g_spinner++ & 3u], 0};
  print(s, VIDEO_COLS - 1, VIDEO_FILS - 1, C_BG_BLACK | C_FG_WHITE);
}

static TrapFrame* yield_to_idle() {
  init_idle_once();
  mmu_switch_to_idle();
  g_current_slot = -1;
  return &g_idle_tf;
}

static TrapFrame* schedule_next() {
  int32_t next = sched_nextTask();
  if (next < 0) {
    return yield_to_idle();
  }

  uint32_t slot = static_cast<uint32_t>(next);
  init_task_if_needed(slot);
  mmu_switch_to_task(slot);
  g_current_slot = next;
  sched_this_miner = static_cast<uint8_t>(next);
  return &g_tasks[slot].tf;
}

} // fin del namespace

extern "C" TrapFrame* trap_handle(TrapFrame* tf) {
  const uint64_t scause = tf->scause;
  const uint64_t is_interrupt = scause >> 63;
  const uint64_t cause_code = scause & 0x7FFF'FFFF'FFFF'FFFFull;

  if (is_interrupt) {
    // Interrupción de software de supervisor (SSIP) usada como pulso periódico:
    // el handler de interrupción del timer en machine-mode levanta SSIP
    if (cause_code == 1) {
      const uint64_t ssip_mask = (1ull << 1);
      asm volatile("csrc sip, %0" : : "r"(ssip_mask));
      if (g_spawn_cooldown_a) g_spawn_cooldown_a--;
      if (g_spawn_cooldown_b) g_spawn_cooldown_b--;
      poll_uart_input();
      update_spinner();
      return schedule_next();
    }
    return tf;
  }

  // Excepciones
  if (cause_code == 8) { // ecall desde U-mode
    tf->sepc += 4;       // saltear la instrucción ecall
    poll_uart_input();

    const uint64_t sys_id = tf->a7;
    if (g_current_slot >= 0) {
      sched_this_miner = static_cast<uint8_t>(g_current_slot);
    }

    if (sys_id == 177788) { // move(d=a0)
      game_move(static_cast<e_direction_t>(tf->a0));
      return yield_to_idle(); // yield hasta el próximo pulso del timer (x86 salta a la TSS de idle)
    }
    if (sys_id == 310311) { // take() -> a0
      tf->a0 = static_cast<int64_t>(game_take());
      return yield_to_idle(); // yield hasta el próximo pulso del timer
    if (sys_id == 760279) { // getId() -> a0
      tf->a0 = static_cast<uint64_t>(game_get_id());
      return tf; // sin ceder ejecución
    }

    if (g_current_slot >= 0) {
      sched_kill();
    }
    return yield_to_idle();
  }

  // Cualquier otra excepción: matar el minero actual (si hay uno) y seguir.
  if (g_current_slot >= 0) {
    print(kHeaderExcPrefix, kHeaderExcX, kHeaderY, C_BG_BLACK | C_FG_LIGHT_RED);
    print_hex(static_cast<uint32_t>(cause_code), 2, kHeaderExcHexX, kHeaderY, C_BG_BLACK | C_FG_LIGHT_RED);
    sched_kill();
  }
  return yield_to_idle();
}

extern "C" void kernel_main() {
  uart::puts("RISC-V kernel starting...\n");

  // Habilitar paginación Sv39 e instalar el espacio de direcciones de idle
  // A partir de este punto, el scheduler puede cambiar espacios de direcciones por tarea
  mmu_init();

  sched_init();

  // Dibujar la UI inicial por lote (sin saturar el UART celda por celda).
  screen_set_batch(1);
  game_init();
  screen_draw();
  screen_set_batch(0);
  screen_flush();

  // Arranca sin mineros, crearlos por teclado
  draw_header();

  init_idle_once();

  // Configuración de traps (S-mode).
  const uint64_t stvec_val = reinterpret_cast<uint64_t>(trap_entry);
  asm volatile("csrw stvec, %0" : : "r"(stvec_val));

  // Habilitar interrupciones
  const uint64_t sie_mask = (1ull << 1) | (1ull << 5); // SSIE (+ STIE por completitud)
  asm volatile("csrs sie, %0" : : "r"(sie_mask));

  // Elegir la primera tarea ejecutable y entrar a U-mode.
  TrapFrame* first = schedule_next();
  enter_user(first);
}
