/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de funciones del manejador de memoria
*/

#ifndef __MMU_H__
#define __MMU_H__

#include "stdint.h"
#include "defines.h"
#include "i386.h"
#include "tss.h"
#include "game.h"

typedef struct {
    uint8_t p:1;
    uint8_t rw:1;
    uint8_t us:1;
    uint8_t pwt:1;
    uint8_t pcd:1;
    uint8_t a:1;
    uint8_t d:1;
    uint8_t pat:1;
    uint8_t g:1;
    uint8_t avl:3;
    uint32_t page_addr:20;
} page_table_entry_t;

/** Estructura de una entrada en el directorio de paginas.
 *
 * Doc:
 *   - Ver `Table 4.5` en el manual de intel.
 *
 */
typedef struct {
    uint8_t   p:1;
    uint8_t   rw:1;
    uint8_t   us:1;
    uint8_t   pwt:1;
    uint8_t   pcd:1;
    uint8_t   a:1;
    uint8_t   ign:1;
    uint8_t   ps:1;
    uint8_t   g:1;
    uint8_t   avl:3;
    uint32_t  table_addr:20;
} page_dir_entry_t;

void mmu_init();

uint32_t mmu_nextFreeKernelPage();
uint32_t mmu_nextFreeTaskPage();

void mmu_mapPage(uint32_t virtual, page_dir_entry_t* cr3, uint32_t phy, uint32_t attrs);

uint32_t mmu_unmapPage(uint32_t virtual, uint32_t cr3);

uint32_t mmu_initKernelDir();

page_dir_entry_t* mmu_initTaskDir(uint32_t jugador, uint32_t fila);

/** Copia los datos de una pagina (memoria lineal) a otra. Asume que ambas
 *  paginas estan mapeadas y podemos escribir en la pagina destino. */
void copy_page(uint32_t src, uint32_t dst);

#endif	/* !__MMU_H__ */




