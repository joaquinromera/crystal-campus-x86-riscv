/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de las rutinas de atencion de interrupciones
*/

#ifndef __IDT_H__
#define __IDT_H__

#include "stdint.h"

/* Struct de descriptor de IDT */
typedef struct str_idt_descriptor {
    uint16_t idt_length;
    uint32_t idt_addr;
} __attribute__((__packed__)) idt_descriptor;

/* Struct de una entrada de la IDT */
typedef struct str_idt_entry_fld {
    uint16_t offset_0_15;
    uint16_t segsel;
    uint16_t attr;
    uint16_t offset_16_31;
} __attribute__((__packed__, aligned (8))) idt_entry;


/** Estructura para acceder a los datos en el stack que le llegan a
 *  idt_print_mensaje_error. */
typedef struct {

    // Pido 32 bits para todos porque es el tamano que ocupan en el stack. en
    // los casos de 16 bits voy a tener cuidado de imprimir solo los bits menos
    // significativos.

    // Selectores de segmento pusheados manualmente:
    uint16_t ss_lvl0; // Estos cambian al llegar al ISR
    uint16_t gs; // Deberia ser igual al de nivel 3
    uint16_t fs; // Deberia ser igual al de nivel 3
    uint16_t es; // Deberia ser igual al de nivel 3
    uint16_t ds; // Deberia ser igual al de nivel 3
    uint16_t cs_lvl0; // Estos cambian al llegar al ISR

    // Valores pusheados por `pushad`
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t basura_esp; // Este ESP no va porque es al momento de PUSHAD
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;
} idt_exception_context_t;

typedef struct {
    // Valores pusheados antes de iniciarse la ISR
    uint32_t eip;
    uint16_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;

} idt_exception_context2_t;


// -----------------------------------------------------------------------------
// Estado global
// -----------------------------------------------------------------------------

/** Indica si estamos en modo debug. */
uint8_t idt_modo_debug = FALSE;
/** Indica si estamos mostrando un mensaje de error. */
uint8_t idt_modo_mensaje = FALSE;

extern idt_entry idt[];
extern idt_descriptor IDT_DESC;
extern char* IDT_MSGS[20];

// -----------------------------------------------------------------------------
// Rutinas publicas
// -----------------------------------------------------------------------------

void idt_init();

/* Rutinas de atencion de interrupciones */
void idt_print_nro(uint32_t i);
void idt_int_teclado(uint8_t i);
/** Imprime el mensaje en pantalla con el estado del CPU al momento de generar
 *  la excepcion. */
void idt_print_mensaje_error(uint8_t codigo_interrupcion,
                             idt_exception_context_t* estado);


#endif  /* !__IDT_H__ */
