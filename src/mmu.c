/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de funciones del manejador de memoria
*/

#include "mmu.h"

// Posicion base de la copia original del codigo de las tareas. Con un offset
// podemos obtener el codigo de la tarea de cada jugador. Cada tarea ocupa 8Kb.
#define MMU_BASE_ADDR_TAREAS          0x10000
// Posicion base de las areas libres de tareas. Con un offset podemos obtener
// el area libre de la tarea de cada jugador. Cada area ocupa 8Kb.
#define MMU_BASE_ADDR_AREA_COMPARTIDA 0x15000

// -----------------------------------------------------------------------------
// Macros para desmenuzar una direccion virtual
// -----------------------------------------------------------------------------


/** Toma una direccion virtual y devuelve su indice en el directorio de tablas
 *  de pagina.
 *
 * Documentacion:
 *   Ver `Figure 4-2` del manual de Intel.
 */
#define MMU_GET_DIRECTORY_INDEX(virtual_addr) (virtual_addr >> 22)

/** Toma una direccion virtual y devuelve su indice en su tabla de paginas.
 *
 * Para obtener en cual tabla de paginas se encuentra depende de su indice en
 * el directorio de tablas de pagina. (Ver `MMU_GET_DIRECTORY_INDEX`)
 *
 * Documentacion:
 *   Ver `Figure 4-2` del manual de Intel.
 */
#define MMU_GET_TABLE_INDEX(virtual_addr) ((virtual_addr << 10) >> 22)

/** Toma una direccion virtual y devuelve su offset en la pagina de la memoria
 *  fisica donde se encuentre. Dicha pagina se obtiene buscando en el CR3.
 *
 * Documentacion:
 *   Ver `Figure 4-2` del manual de Intel.
 */
#define MMU_GET_OFFSET(virtual_addr) ((virtual_addr << 20) >> 20)


// -----------------------------------------------------------------------------
// Macros para trabajar con addrs de 4K (en cr3, directorio y tablas)
// -----------------------------------------------------------------------------


/** Devuelve la direccion fisica del Page Directory.
 *
 * Limpia los 12 bits menos significativos para que este alineado a una pagina
 * de memoria. Esto lo dice el manual de intel (Cap 2.5, bullet para cr3) que
 * los 12 bits menos significativos se ignoran.
 */
#define MMU_GET_PAGE_DIR(cr3) \
    ((page_dir_entry_t*) (((uint32_t) cr3) & 0xFFFFF000))

/** Devuelve la direccion fisica de una tabla. Toma un Page Directory Entry.
 *
 * Argumentos:
 *   pde: Un page directory entry.
 *
 * Returns:
 *   Devuelve la direccion fisica de 32 bits de la pagina donde se encuentra la
 *   tabla.
 */
#define MMU_GET_TABLE_PHYS_ADDR(pde) \
    ((page_table_entry_t*) (pde.table_addr << 12))

/** Devuelve la direccion fisica de una pagina. Toma un Page Table Entry.
 *
 * Argumentos:
 *   pde: Un Page Table Entry.
 *
 * Returns:
 *   Devuelve la direccion fisica de 32 bits de la pagina de 4K.
 */
#define MMU_GET_PAGE_PHYS_ADDR(pte) (pte.page_addr << 12)


// -----------------------------------------------------------------------------
// Estado de la MMU
// -----------------------------------------------------------------------------

uint32_t next_free_kernel_page;
uint32_t next_free_task_page;

// -----------------------------------------------------------------------------
// Implementacion:
// -----------------------------------------------------------------------------

void mmu_init() {
    next_free_kernel_page = FREE_KERNEL_PAGES_START;
    next_free_task_page = FREE_TASKS_PAGES_START;
}

uint32_t mmu_nextFreeKernelPage() {
    uint32_t free_page = next_free_kernel_page;
    next_free_kernel_page += PAGE_SIZE;
    return free_page;
}

uint32_t mmu_nextFreeTaskPage() {
    uint32_t free_page = next_free_task_page;
    next_free_task_page += PAGE_SIZE;
    return free_page;
}

void mmu_mapPage(uint32_t virtual, page_dir_entry_t* cr3, uint32_t phy, uint32_t attrs) {

    // Obtiene indices para esta direccion virtual en el esquema de paginacion.
    uint32_t pd_index = MMU_GET_DIRECTORY_INDEX(virtual);
    uint32_t pt_index = MMU_GET_TABLE_INDEX(virtual);

    // Limpia la parte baja del CR3 para alinearlo a una pagina de memoria
    // fisica.
    page_dir_entry_t* page_directory = MMU_GET_PAGE_DIR(cr3);

    // Si la tabla no esta presente, construye una
    if ( ! page_directory[pd_index].p ) {

        // Pide una pag al kernel para la nueva tabla. Usa una pagina de kernel
        // porque el MMU funciona a nivel 0.
        uint32_t table_phys_addr = mmu_nextFreeKernelPage();

        page_directory[pd_index].p = 1;
        // Ponemos 1 para permitir mezcla de paginas de lectura/escritura y
        // solo lectura. Si la pagina en la tabla debe ser solo lectura, se
        // debe poner su bit `rw` en 0 en dicha entrada de la tabla.
        page_directory[pd_index].rw = 1;
        // Ponemos 1 para permitir mezcla de paginas de usuario y kernel en
        // dicha tabla.  Si la pagina es de kernel, se puede poner `.us=0` en
        // la Page Table Entry correspondiente.
        page_directory[pd_index].us = 1;
        page_directory[pd_index].pwt = 0;
        page_directory[pd_index].pcd = 0;
        page_directory[pd_index].a = 0;
        page_directory[pd_index].ign = 0;
        page_directory[pd_index].ps = 0; // Pagina de 4k
        page_directory[pd_index].g = 0;
        page_directory[pd_index].avl = 0;
        // Guarda la direccion fisica de la tabla apuntada como una direccion
        // de 4Kb.
        page_directory[pd_index].table_addr =
            (table_phys_addr >> 12);

        // Limpia el bit P de cada entrada de la nueva tabla.
        for (uint32_t i = 0; i < 1024; ++i) {
            ((page_table_entry_t*) table_phys_addr)[i].p = 0;
        }

    }

    // Mapea la pagina y escribe los atributos.
    uint32_t* page_table =
        (uint32_t*) MMU_GET_TABLE_PHYS_ADDR(page_directory[pd_index]);
    page_table[pt_index] = (phy & 0xFFFFF000) | PAG_P | attrs;

    // Luego de modificar el esquema de paginacion, se debe ejecutar `tlbflush`
    // para limpiar la cache de traducciones.
    tlbflush();
}

uint32_t mmu_unmapPage(uint32_t virtual, uint32_t cr3) {
    uint32_t pd_index = MMU_GET_DIRECTORY_INDEX(virtual);
    uint32_t pt_index = MMU_GET_TABLE_INDEX(virtual);

    page_dir_entry_t* dir = MMU_GET_PAGE_DIR(cr3);
    page_table_entry_t* table = MMU_GET_TABLE_PHYS_ADDR(dir[pd_index]);
    // Pone ausente la pagina
    table[pt_index].p = 0;

    // Luego de modificar el esquema de paginacion, se debe ejecutar `tlbflush`
    // para limpiar la cache de traducciones.
    tlbflush();

    return 0;
}

uint32_t mmu_initKernelDir() {
    uint32_t* kernel_page_dir = (uint32_t*) KERNEL_PAGE_DIR;
    // Pone ausente todo el directorio
    for (uint32_t i = 0; i < 1024; ++i) { kernel_page_dir[i] = 0; }

    kernel_page_dir[0] = KERNEL_PAGE_TABLE_0 | PAG_RW | PAG_P;
    uint32_t* kernel_page_table = (uint32_t*) KERNEL_PAGE_TABLE_0;
    // identity mapping all 1024 entries
    for (uint16_t i = 0; i < 1024; ++i) {
        kernel_page_table[i] = (i << 12) | PAG_RW | PAG_P;
    }

    // identity-mapea la memoria del tablero de juego para que el kernel pueda
    // mover paginas a gusto.
    uint32_t attrs_tablero = PAG_P | PAG_RW | PAG_S;
    for (uint32_t i = 0; i < SIZE_N * SIZE_M * 2; ++i) {
        uint32_t addr = MAPA_BASE + i * PAGE_SIZE;
        mmu_mapPage(addr,
                    (page_dir_entry_t*) kernel_page_dir,
                    addr,
                    attrs_tablero);
    }

    return (uint32_t) kernel_page_dir;
}

/** Inicializa directorio de paginas para una tarea.
 *
 * phys_addr_task -- Ubicacion src tarea en el kernel.
 *
 * Returns:
 *   Puntero al Page Directory.
 */
page_dir_entry_t*
mmu_initTaskDir(uint32_t jugador, uint32_t fila) {

    // Busca en el mapa del kernel el addr base del codigo de la tarea.
    uint32_t addr_codigo_task_original =
        MMU_BASE_ADDR_TAREAS + jugador * 2 * PAGE_SIZE;

    // Busca en el mapa del kernel el addr base del area compartida del jugador.
    uint32_t addr_area_compartida_jugador =
        MMU_BASE_ADDR_AREA_COMPARTIDA + jugador * 2 * PAGE_SIZE;

    // Calcula la posicion (fisica) en el tablero donde se debe copiar el
    // codigo de la tarea.
    uint32_t columna = (jugador == PLAYER_A) ? 0 : 77;
    uint32_t phys_dst_codigo_tarea = GET_ADDR_TABLERO(columna, fila);

    // CREA UN DIRECTORIO PARA LA TAREA.
    // Esta funcion pide inicializar un directorio para que la nueva tarea
    // pueda funcionar. El primer paso es pedir una pagina del kernel.
    // Pido pag al kernel para poner directorio para esta tarea.
    page_dir_entry_t* dir = (page_dir_entry_t*) mmu_nextFreeKernelPage();

    // inicializo directorio vacio
    for (uint32_t i = 0; i < 1024; i++) {
        dir[i].p = 0; // Ausente
    }

    // MAPEO KERNEL:
    // Se identity-mapea el codigo de kernel y area libre del kernel. Esto va
    // del addr 0 al address 0x3FFFFF. Esto son 4Mb = 1024 * 4kb, por lo que
    // podemos resolverlo mapeando las primeras 1024 paginas del kernel.
    for (uint32_t i = 0; i < 1024; ++i) {

        // Damos permiso de escritura a las paginas de codigo del kernel porque
        // se supone el kernel no va a escribirlas. (esto fue consultado con
        // los profes). Puede parecer desprolijo, pero bajo el supuesto que el
        // kernel no se manda cagadas esta bien.
        uint32_t attrs = PAG_P | PAG_RW | PAG_S;

        mmu_mapPage(i * PAGE_SIZE, dir, i * PAGE_SIZE, attrs);
    }
    // Tambien pide poner el Page Directory entry (indice 0) como supervisor
    // (u/s = 0).
    dir[0].us = 0;

    // identity-mapea la memoria del tablero de juego para que el kernel pueda
    // mover paginas a gusto.
    uint32_t attrs_tablero = PAG_P | PAG_RW | PAG_S;
    for (uint32_t i = 0; i < SIZE_N * SIZE_M * 2; ++i) {
        uint32_t addr = MAPA_BASE + i * PAGE_SIZE;
        mmu_mapPage(addr, dir, addr, attrs_tablero);
    }

    // COPIAR CODIGO TAREA
    // La tarea tiene su codigo fisicamente en la posicion del tablero en la
    // que se encuentra.

    // Primero hacemos un identity mapping para poder escribir en esa pagina.
    mmu_mapPage(phys_dst_codigo_tarea,
                (page_dir_entry_t*) rcr3(),
                phys_dst_codigo_tarea,
                PAG_P | PAG_RW);
    mmu_mapPage(phys_dst_codigo_tarea + PAGE_SIZE,
                (page_dir_entry_t*) rcr3(),
                phys_dst_codigo_tarea + PAGE_SIZE,
                PAG_P | PAG_RW);
    // Copiamos byte a byte
    uint8_t* src = (uint8_t*) addr_codigo_task_original;
    uint8_t* dst = (uint8_t*) phys_dst_codigo_tarea;
    for (uint32_t i = 0; i < 2 * PAGE_SIZE; ++i) {
        dst[i] = src[i];
    }
    // Desmapeamos
    mmu_unmapPage(phys_dst_codigo_tarea, rcr3());
    mmu_unmapPage(phys_dst_codigo_tarea + PAGE_SIZE, rcr3());

    // MAPEAMOS EN EL DIR DE TAREA EL CODIGO DE LA TAREA
    // Tenemos que mapear la direccion virtual 0x08000000 (donde la tarea ve su
    // codigo) a la direccion fisica de la posicion en el tablero de la tarea.
    uint32_t task_code_attrs = PAG_P | PAG_US | PAG_RW;
    mmu_mapPage(TASK_CODE,
                dir,
                phys_dst_codigo_tarea,
                task_code_attrs);
    mmu_mapPage(TASK_CODE + PAGE_SIZE,
                dir,
                phys_dst_codigo_tarea + PAGE_SIZE,
                task_code_attrs);

    // MAPEAMOS AREA LIBRE TAREA EN DIR TAREA
    // Tenemos que mapear a partir de la  direccion TASK_COMPARTIDA el area
    // libre de esta tarea.
    mmu_mapPage(TASK_COMPARTIDA,
                dir,
                addr_area_compartida_jugador,
                PAG_P | PAG_RW | PAG_US);
    mmu_mapPage(TASK_COMPARTIDA + PAGE_SIZE,
                dir,
                addr_area_compartida_jugador + PAGE_SIZE,
                PAG_P | PAG_RW | PAG_US);

    return dir;
}

void copy_page(uint32_t src, uint32_t dst) {
    uint8_t* src_p = (uint8_t*) src;
    uint8_t* dst_p = (uint8_t*) dst;
    for (uint32_t i = 0; i < PAGE_SIZE; ++i) {
        dst_p[i] = src_p[i];
    }
};
