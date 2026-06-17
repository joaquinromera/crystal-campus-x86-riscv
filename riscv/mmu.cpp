#include "mmu.h"

#include <stddef.h>
#include <stdint.h>

#include "defines.h"

extern "C" void* memset(void* dst, int c, size_t n);
extern "C" void* memcpy(void* dst, const void* src, size_t n);

extern "C" char __bss_end;

namespace {

// ---------------------------------------------------------------------------
// Bits de PTE Sv39
// ---------------------------------------------------------------------------

static constexpr uint64_t PTE_V = 1ull << 0;
static constexpr uint64_t PTE_R = 1ull << 1;
static constexpr uint64_t PTE_W = 1ull << 2;
static constexpr uint64_t PTE_X = 1ull << 3;
static constexpr uint64_t PTE_U = 1ull << 4;
static constexpr uint64_t PTE_G = 1ull << 5;
static constexpr uint64_t PTE_A = 1ull << 6;
static constexpr uint64_t PTE_D = 1ull << 7;

static constexpr uint64_t SATP_MODE_SV39 = 8ull;

static inline uint64_t align_up_u64(uint64_t value, uint64_t align) {
  return (value + align - 1) & ~(align - 1);
}

static inline uint64_t vpn2(uint64_t va) {
  return (va >> 30) & 0x1FF;
}
static inline uint64_t vpn1(uint64_t va) {
  return (va >> 21) & 0x1FF;
}
static inline uint64_t vpn0(uint64_t va) {
  return (va >> 12) & 0x1FF;
}

static inline uint64_t pte_make_table(uint64_t table_pa) {
  const uint64_t ppn = table_pa >> 12;
  return (ppn << 10) | PTE_V;
}

static inline uint64_t pte_make_leaf(uint64_t pa, uint64_t flags) {
  const uint64_t ppn = pa >> 12;
  return (ppn << 10) | flags;
}

static inline uint64_t pte_pa(uint64_t pte) {
  const uint64_t ppn = (pte >> 10) & 0x0FFF'FFFF'FFFFull;
  return ppn << 12;
}

static inline void sfence_vma_all() {
  asm volatile("sfence.vma x0, x0" ::: "memory");
}

static inline void sfence_vma_addr(uint64_t va) {
  asm volatile("sfence.vma %0, x0" : : "r"(va) : "memory");
}

static inline void fence_i() {
  asm volatile("fence.i" ::: "memory");
}

static inline uint64_t make_satp(uint64_t root_pa, uint16_t asid) {
  const uint64_t ppn = root_pa >> 12;
  return (SATP_MODE_SV39 << 60) | (static_cast<uint64_t>(asid) << 44) | (ppn & 0x0FFF'FFFF'FFFFull);
}

static inline void write_satp(uint64_t satp) {
  asm volatile("csrw satp, %0" : : "r"(satp) : "memory");
}

// ---------------------------------------------------------------------------
// Allocator simple para tablas de páginas.
// ---------------------------------------------------------------------------

static uint64_t g_alloc_page = 0;

static void* alloc_page_table() {
  if (g_alloc_page == 0) {
    g_alloc_page = align_up_u64(reinterpret_cast<uint64_t>(&__bss_end), PAGE_SIZE);
  }
  void* page = reinterpret_cast<void*>(g_alloc_page);
  g_alloc_page += PAGE_SIZE;
  memset(page, 0, PAGE_SIZE);
  return page;
}

// ---------------------------------------------------------------------------
// Disposición de "tablero como memoria"
// ---------------------------------------------------------------------------

static constexpr uint32_t kCellPages = 2;
static constexpr uint32_t kCellBytes = kCellPages * PAGE_SIZE;
static constexpr uint64_t kUserBase = TASK_CODE;

// Almacenamiento backup tablero: una región de 8 KB por celda (78x40).
alignas(PAGE_SIZE) static uint8_t g_board[TABLERO_N_COLS * TABLERO_N_FILS * kCellBytes];

alignas(PAGE_SIZE) static uint8_t g_shared[2][2 * PAGE_SIZE];

static inline uint64_t board_cell_pa(uint32_t x, uint32_t y) {
  const uint64_t ix = static_cast<uint64_t>(y) * TABLERO_N_COLS + x;
  return reinterpret_cast<uint64_t>(&g_board[ix * kCellBytes]);
}

// ---------------------------------------------------------------------------
// Estado de MMU por tarea
// ---------------------------------------------------------------------------

struct TaskMmu {
  uint64_t* root = nullptr;    // Tabla de páginas L2
  uint64_t* l1_0 = nullptr;    // Tabla de páginas L1 para VPN2==0
  uint64_t* l0_user = nullptr; // Tabla de páginas L0 para la región TASK_CODE (VPN2==0, VPN1==64)
  uint64_t satp = 0;
  uint16_t asid = 0;
  uint32_t initialized = 0;
};

static TaskMmu g_tasks[20];
static TaskMmu g_idle;

static inline void map_kernel_gigapage(uint64_t* root) {
  // Mapear identidad 0x8000_0000..0xBFFF_FFFF como hoja de 1 GB (solo supervisor).
  const uint64_t va = 0x8000'0000ull;
  const uint64_t pa = 0x8000'0000ull;
  const uint64_t flags = PTE_V | PTE_R | PTE_W | PTE_X | PTE_A | PTE_D;
  root[vpn2(va)] = pte_make_leaf(pa, flags);
}

static inline void ensure_l1_0(TaskMmu& t) {
  if (t.l1_0) return;
  t.l1_0 = reinterpret_cast<uint64_t*>(alloc_page_table());
  t.root[0] = pte_make_table(reinterpret_cast<uint64_t>(t.l1_0));
}

static inline void map_uart(TaskMmu& t) {
  // Base de UART0 en QEMU virt: 0x1000_0000. Mapear como hoja de 2 MB en L1.
  ensure_l1_0(t);
  const uint64_t va = 0x1000'0000ull;
  const uint64_t pa = 0x1000'0000ull;
  const uint64_t flags = PTE_V | PTE_R | PTE_W | PTE_A | PTE_D;
  t.l1_0[vpn1(va)] = pte_make_leaf(pa, flags);
}

static inline void ensure_user_l0(TaskMmu& t) {
  ensure_l1_0(t);
  if (t.l0_user) return;
  t.l0_user = reinterpret_cast<uint64_t*>(alloc_page_table());
  const uint64_t va = kUserBase;
  t.l1_0[vpn1(va)] = pte_make_table(reinterpret_cast<uint64_t>(t.l0_user));
}

static inline void map_user_code_pages(TaskMmu& t, uint64_t cell_pa) {
  ensure_user_l0(t);

  // TASK_CODE y TASK_CODE+4 KB mapean a la celda de 8 KB.
  const uint64_t flags = PTE_V | PTE_R | PTE_W | PTE_X | PTE_U | PTE_A | PTE_D;
  t.l0_user[0] = pte_make_leaf(cell_pa + 0 * PAGE_SIZE, flags);
  t.l0_user[1] = pte_make_leaf(cell_pa + 1 * PAGE_SIZE, flags);
}

static inline void map_user_shared(TaskMmu& t, uint32_t player_id) {
  ensure_user_l0(t);
  const uint64_t flags = PTE_V | PTE_R | PTE_W | PTE_U | PTE_A | PTE_D;

  // TASK_COMPARTIDA empieza en TASK_CODE + 2 pages
  t.l0_user[2] = pte_make_leaf(reinterpret_cast<uint64_t>(&g_shared[player_id][0]), flags);
  t.l0_user[3] = pte_make_leaf(reinterpret_cast<uint64_t>(&g_shared[player_id][PAGE_SIZE]), flags);
}

static inline void task_build_pagetable(TaskMmu& t, uint16_t asid, uint32_t player_id, uint64_t cell_pa) {
  t.root = reinterpret_cast<uint64_t*>(alloc_page_table());
  map_kernel_gigapage(t.root);
  ensure_l1_0(t);
  map_uart(t);
  map_user_code_pages(t, cell_pa);
  map_user_shared(t, player_id);
  t.asid = asid;
  t.satp = make_satp(reinterpret_cast<uint64_t>(t.root), asid);
  t.initialized = 1;
}

static inline void mmu_switch(const TaskMmu& t) {
  if (!t.initialized) return;
  write_satp(t.satp);
  sfence_vma_all();
}

// ---------------------------------------------------------------------------
// User binaries (generados por Makefile mediante -b binary)
// ---------------------------------------------------------------------------

extern "C" const uint8_t _binary_build_user_miner_bin_start[];
extern "C" const uint8_t _binary_build_user_miner_bin_end[];

extern "C" const uint8_t _binary_build_user_idle_bin_start[];
extern "C" const uint8_t _binary_build_user_idle_bin_end[];

static inline const uint8_t* miner_bin_begin() {
  return _binary_build_user_miner_bin_start;
}
static inline size_t miner_bin_size() {
  return static_cast<size_t>(_binary_build_user_miner_bin_end - _binary_build_user_miner_bin_start);
}

static inline const uint8_t* idle_bin_begin() {
  return _binary_build_user_idle_bin_start;
}
static inline size_t idle_bin_size() {
  return static_cast<size_t>(_binary_build_user_idle_bin_end - _binary_build_user_idle_bin_start);
}

static inline void write_cell_image(uint64_t cell_pa, const uint8_t* src, size_t src_size) {
  uint8_t* dst = reinterpret_cast<uint8_t*>(cell_pa);
  memset(dst, 0, kCellBytes);
  if (src && src_size) {
    if (src_size > kCellBytes) src_size = kCellBytes;
    memcpy(dst, src, src_size);
  }
  fence_i();
}

} // fin del namespace

void mmu_init() {
  // Inicializar el espacio de direcciones de idle y habilitar paginación inmediatamente
  if (!g_idle.initialized) {
    // Reservar una imagen privada de 8 KB para idle (fuera del tablero)
    alignas(PAGE_SIZE) static uint8_t idle_cell[kCellBytes];
    const uint64_t idle_cell_pa = reinterpret_cast<uint64_t>(idle_cell);

    write_cell_image(idle_cell_pa, idle_bin_begin(), idle_bin_size());
    task_build_pagetable(g_idle, /*asid=*/0, /*player_id=*/0, idle_cell_pa);
  }

  mmu_switch_to_idle();
}

void mmu_switch_to_idle() {
  mmu_switch(g_idle);
}

void mmu_switch_to_task(uint32_t slot) {
  if (slot >= 20) return;
  mmu_switch(g_tasks[slot]);
}

void mmu_task_init(uint32_t slot, uint32_t player_id, uint32_t x, uint32_t y) {
  if (slot >= 20) return;
  TaskMmu& t = g_tasks[slot];
  const uint64_t cell_pa = board_cell_pa(x, y);
  write_cell_image(cell_pa, miner_bin_begin(), miner_bin_size());

  if (!t.initialized) {
    // ASIDs: 1..20 para mineros, 0 para idle
    task_build_pagetable(t, static_cast<uint16_t>(slot + 1), player_id, cell_pa);
    return;
  }

  // Recrear, solo remapear TASK_CODE a la celda nueva.
  map_user_code_pages(t, cell_pa);
  map_user_shared(t, player_id);
  sfence_vma_addr(kUserBase);
  sfence_vma_addr(kUserBase + PAGE_SIZE);
  sfence_vma_addr(kUserBase + 2 * PAGE_SIZE);
  sfence_vma_addr(kUserBase + 3 * PAGE_SIZE);
}

void mmu_task_move(uint32_t slot, uint32_t old_x, uint32_t old_y, uint32_t new_x, uint32_t new_y) {
  if (slot >= 20) return;
  TaskMmu& t = g_tasks[slot];
  if (!t.initialized) return;

  const uint64_t src_pa = board_cell_pa(old_x, old_y);
  const uint64_t dst_pa = board_cell_pa(new_x, new_y);

  memcpy(reinterpret_cast<void*>(dst_pa), reinterpret_cast<const void*>(src_pa), kCellBytes);
  fence_i();

  // Remapear TASK_CODE a la celda nueva (actualizar solo las dos páginas de código)
  map_user_code_pages(t, dst_pa);
  sfence_vma_addr(kUserBase);
  sfence_vma_addr(kUserBase + PAGE_SIZE);
}
