/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de la tabla de descriptores globales
*/

#include "gdt.h"

gdt_entry gdt[GDT_COUNT] = {
    /* Descriptor nulo*/
    /* Offset = 0x00 */
    [GDT_IDX_NULL_DESC] = (gdt_entry) {
        (uint16_t)    0x0000,         /* limit[0:15]  */
        (uint16_t)    0x0000,         /* base[0:15]   */
        (uint8_t)     0x00,           /* base[23:16]  */
        (uint8_t)     0x00,           /* type         */
        (uint8_t)     0x00,           /* s            */
        (uint8_t)     0x00,           /* dpl          */
        (uint8_t)     0x00,           /* p            */
        (uint8_t)     0x00,           /* limit[16:19] */
        (uint8_t)     0x00,           /* avl          */
        (uint8_t)     0x00,           /* l            */
        (uint8_t)     0x00,           /* db           */
        (uint8_t)     0x00,           /* g            */
        (uint8_t)     0x00,           /* base[31:24]  */
    },
    [GDT_DATOS_KERNEL] = (gdt_entry) {
        (uint16_t)    LIMIT_00_15(0xFFFFF), /* limit[0:15]  */
        (uint16_t)    0x0000,                   /* base[0:15]   */
        (uint8_t)     0x00,                     /* base[23:16]  */
        (uint8_t)     0b0010,                   /* type = R/W   */
        (uint8_t)     0x01,                     /* s            */
        (uint8_t)     0x00,                     /* dpl          */
        (uint8_t)     0x01,                     /* p            */
        (uint8_t)     LIMIT_16_19(0xFFFFF), /* limit[16:19] */
        (uint8_t)     0x00,                     /* avl          */
        (uint8_t)     0x00,                     /* l            */
        (uint8_t)     0x01,                     /* db           */
        (uint8_t)     0x01,                     /* g            */
        (uint8_t)     0x00,                     /* base[31:24]  */
    },
    [GDT_DATOS_USER] = (gdt_entry) {
        (uint16_t)    LIMIT_00_15(0xFFFFF), /* limit[0:15]  */
        (uint16_t)    0x0000,                   /* base[0:15]   */
        (uint8_t)     0x00,                     /* base[23:16]  */
        (uint8_t)     0b0010,                   /* type = R/W   */
        (uint8_t)     0x01,                     /* s            */
        (uint8_t)     0x03,                     /* dpl          */
        (uint8_t)     0x01,                     /* p            */
        (uint8_t)     LIMIT_16_19(0xFFFFF), /* limit[16:19] */
        (uint8_t)     0x00,                     /* avl          */
        (uint8_t)     0x00,                     /* l            */
        (uint8_t)     0x01,                     /* db           */
        (uint8_t)     0x01,                     /* g            */
        (uint8_t)     0x00,                     /* base[31:24]  */
    },
    [GDT_CODIGO_KERNEL] = (gdt_entry) {
        (uint16_t)    LIMIT_00_15(0xFFFFF), /* limit[0:15]  */
        (uint16_t)    0x0000,                   /* base[0:15]   */
        (uint8_t)     0x00,                     /* base[23:16]  */
        (uint8_t)     0b1000,                   /* type = Exec only */
        (uint8_t)     0x01,                     /* s            */
        (uint8_t)     0x00,                     /* dpl          */
        (uint8_t)     0x01,                     /* p            */
        (uint8_t)     LIMIT_16_19(0xFFFFF), /* limit[16:19] */
        (uint8_t)     0x00,                     /* avl          */
        (uint8_t)     0x00,                     /* l            */
        (uint8_t)     0x01,                     /* db           */
        (uint8_t)     0x01,                     /* g            */
        (uint8_t)     0x00,                     /* base[31:24]  */
    },
    [GDT_CODIGO_USER] = (gdt_entry) {
        (uint16_t)    LIMIT_00_15(0xFFFFF), /* limit[0:15]  */
        (uint16_t)    0x0000,                   /* base[0:15]   */
        (uint8_t)     0x00,                     /* base[23:16]  */
        (uint8_t)     0b1000,                   /* type = Exec  */
        (uint8_t)     0x01,                     /* s            */
        (uint8_t)     0x03,                     /* dpl          */
        (uint8_t)     0x01,                     /* p            */
        (uint8_t)     LIMIT_16_19(0xFFFFF), /* limit[16:19] */
        (uint8_t)     0x00,                     /* avl          */
        (uint8_t)     0x00,                     /* l            */
        (uint8_t)     0x01,                     /* db           */
        (uint8_t)     0x01,                     /* g            */
        (uint8_t)     0x00,                     /* base[31:24]  */
    },
    [GDT_SCREEN] = (gdt_entry) {
        (uint16_t)    LIMIT_00_15(VIDEO_LIMIT), /* limit[0:15]  */
        (uint16_t)    BASE_00_15(VIDEO),        /* base[0:15]   */
        (uint8_t)     BASE_16_23(VIDEO),        /* base[23:16]  */
        (uint8_t)     0b0010,                   /* type = R/W   */
        (uint8_t)     0x01,                     /* s            */
        (uint8_t)     0x00,                     /* dpl          */
        (uint8_t)     0x01,                     /* p            */
        (uint8_t)     LIMIT_16_19(VIDEO_LIMIT), /* limit[16:19] */
        (uint8_t)     0x00,                     /* avl          */
        (uint8_t)     0x00,                     /* l            */
        (uint8_t)     0x01,                     /* db           */
        (uint8_t)     0x00,                     /* g            */
        (uint8_t)     BASE_24_31(VIDEO)         /* base[31:24]  */
    }
};

gdt_descriptor GDT_DESC = {
    sizeof(gdt) - 1,
    (uint32_t) &gdt
};
