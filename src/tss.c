/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de estructuras para administrar tareas
*/

#include "tss.h"

tss tss_initial;
tss tss_idle;

void tss_init() {

    // No hace falta inicializar los campos de `tss_initial` porque es
    // solamente un placeholder en memoria para que el procesador guarde el
    // contexto al hacer el primer salto de tarea.
    tss_initGdtEntry(GDT_TSS_INIT, &tss_initial, 0);

}

/** Inicializa un GDT entry para un descriptor de TSS.
 *
 * Argumentos:
 * ix -- Indice en la GDT.
 * base -- Puntero a la TSS a registrar en la GDT
 * dpl -- Indica el DPL de la tarea.
 */
void tss_initGdtEntry(uint32_t ix, tss* base, uint32_t dpl) {
    gdt[ix].limit_0_15   = TSS_SIZE;               // Tamano de una TSS
    gdt[ix].base_0_15    = (uint32_t) base & 0xFFFF;        // base[15:0]
    gdt[ix].base_23_16   = ((uint32_t) base >> 16) & 0xFF;  // base[23:16]
    gdt[ix].type         = 0b1001;
    gdt[ix].s            = 0x0;                  // 0 = sistema
    gdt[ix].dpl          = dpl;
    gdt[ix].p            = 0x1;
    gdt[ix].limit_16_19  = 0x0;
    gdt[ix].avl          = 0x0;
    gdt[ix].l            = 0x0;
    gdt[ix].db           = 0x0;
    gdt[ix].g            = 0x0;
    gdt[ix].base_31_24   = ((uint32_t) base >> 24);
};

void fill_tss(tss* actual, uint32_t jugador, uint32_t fila){

    actual->eip = TASK_CODE;
    actual->cr3 = (uint32_t) mmu_initTaskDir(jugador, fila);
    actual->eflags  = 0x00000202; // despues va 202
    actual->iomap = 0xFFFF;
    actual->cs = GDT_OFF_CODIGO_USER;
    actual->gs = GDT_OFF_DATOS_USER;
    actual->fs = GDT_OFF_DATOS_USER;
    actual->ds = GDT_OFF_DATOS_USER;
    actual->es = GDT_OFF_DATOS_USER;
    actual->ss = GDT_OFF_DATOS_USER;
    actual->eax = 0x0;
    actual->ecx = 0x0;
    actual->edx = 0x0;
    actual->ebx = 0x0;
    actual->esi = 0x0;
    actual->edi = 0x0;
    actual->esp = TASK_STACK;
    actual->ebp = TASK_STACK;

    // Pide una pagina de kernel para Stack lvl 0. No es necesario hacer este
    // paso al momento de construir el esquema de paginacion (mmu_initTaskDir)
    // pues el kernel ya viene identity-mapeado.
    actual->esp0 = mmu_nextFreeKernelPage() + PAGE_SIZE;

    actual->ss0  = GDT_OFF_DATOS_KERNEL;
};

void fill_tss_idle() {

    tss_idle.eip = 0x00014000;
    tss_idle.eflags = 0x00000202;
    tss_idle.cr3 = KERNEL_PAGE_DIR; // Usa dir del kernel
    tss_idle.esp = KERNEL_STACK;
    tss_idle.ebp = KERNEL_STACK;
    tss_idle.cs = GDT_OFF_CODIGO_KERNEL;
    tss_idle.gs = GDT_OFF_DATOS_KERNEL;
    tss_idle.fs = GDT_OFF_DATOS_KERNEL;
    tss_idle.ds = GDT_OFF_DATOS_KERNEL;
    tss_idle.es = GDT_OFF_DATOS_KERNEL;
    tss_idle.ss = GDT_OFF_DATOS_KERNEL;
    tss_idle.eax = 0x0;
    tss_idle.ecx = 0x0;
    tss_idle.edx = 0x0;
    tss_idle.ebx = 0x0;
    tss_idle.esi = 0x0;
    tss_idle.edi = 0x0;
    tss_idle.ss0 = GDT_OFF_DATOS_KERNEL;
    tss_idle.esp0 = KERNEL_STACK;
    tss_idle.iomap = 0xFFFF;

    // Inicializa su descriptor de GDT
    tss_initGdtEntry(GDT_TSS_IDLE, &tss_idle, 0);
};
