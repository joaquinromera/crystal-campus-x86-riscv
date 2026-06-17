/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
  definicion de las rutinas de atencion de interrupciones
*/

#include "defines.h"
#include "idt.h"
#include "isr.h"
#include "tss.h"
#include "screen.h"
#include "game.h"

idt_entry idt[256] = { };

idt_descriptor IDT_DESC = {
    sizeof(idt) - 1,
    (uint32_t) &idt
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

/*
    La siguiente es una macro de EJEMPLO para ayudar a armar entradas de
    interrupciones. Para usar, descomentar y completar CORRECTAMENTE los
    atributos y el registro de segmento. Invocarla desde idt_inicializar() de
    la siguiene manera:

    void idt_inicializar() {
        IDT_ENTRY(0);
        ...
        IDT_ENTRY(19);
        ...
    }
*/

#define IDT_ENTRY(numero)                                                                          \
    idt[numero].offset_0_15 = (uint16_t) ((uint32_t)(&_isr ## numero) & (uint32_t) 0xFFFF);        \
    idt[numero].segsel = (uint16_t) GDT_CODIGO_KERNEL << 3;                                        \
    idt[numero].attr = (uint16_t) 0x8E00;                                                          \
    idt[numero].offset_16_31 = (uint16_t) ((uint32_t)(&_isr ## numero) >> 16 & (uint32_t) 0xFFFF);

/** Genera datos de una IDT Entry con permisos de nivel 3. */
#define IDT_ENTRY_USER(numero)                                                                          \
    idt[numero].offset_0_15 = (uint16_t) ((uint32_t)(&_isr ## numero) & (uint32_t) 0xFFFF);        \
    idt[numero].segsel = (uint16_t) GDT_CODIGO_KERNEL << 3;                                        \
    idt[numero].attr = (uint16_t) 0xEE00;                                                          \
    idt[numero].offset_16_31 = (uint16_t) ((uint32_t)(&_isr ## numero) >> 16 & (uint32_t) 0xFFFF);

char* IDT_MSGS[20] = {
                "Divide Error                 ",
                "Reserved                     ",
                "NMI Interrupt                ",
                "Breakpoint                   ",
                "Overflow                     ",
                "Bound Range Exceeded         ",
                "Invalid Opcode               ",
                "Device Not Available         ",
                "Double Fault                 ",
                "Coprocessor Segment Overrun  ",
                "Invalid TSS                  ",
                "Segment Not Present          ",
                "Stack-Segment Fault          ",
                "General Protection           ",
                "Page Fault                   ",
                "Intel reserved. Do not use.  ",
                "x87 fpu Floating-Point Error ",
                "Alignment Check              ",
                "Machine Check                ",
                "Simd Floating-Point Exception"};

void idt_init() {
    IDT_ENTRY(0);
    IDT_ENTRY(1);
    IDT_ENTRY(2);
    IDT_ENTRY(3);
    IDT_ENTRY(4);
    IDT_ENTRY(5);
    IDT_ENTRY(6);
    IDT_ENTRY(7);
    IDT_ENTRY(8);
    IDT_ENTRY(9);
    IDT_ENTRY(10);
    IDT_ENTRY(11);
    IDT_ENTRY(12);
    IDT_ENTRY(13);
    IDT_ENTRY(14);
    IDT_ENTRY(15);
    IDT_ENTRY(16);
    IDT_ENTRY(17);
    IDT_ENTRY(18);
    IDT_ENTRY(19);

    IDT_ENTRY(32); /* Reloj */
    IDT_ENTRY(33); /* Teclado */

    IDT_ENTRY_USER(47); /* Interrupcion de software */

}

void idt_print_nro(uint32_t i) {
    if ( i < 20 ) {
        print(IDT_MSGS[i], 0, 0, C_FG_WHITE | C_BG_BLACK);
    } else if ( i == 32) {
        print("Int de reloj", 0, 0, C_FG_WHITE | C_BG_BLACK);
    } else if ( i == 33) {
        print("Int de teclado", 0, 0, C_FG_WHITE | C_BG_BLACK);
    } else if ( i == 47) {
        print("Int de software", 0, 0, C_FG_WHITE | C_BG_BLACK);
    } else {
        print_dec(i, 3, 0, 0, C_FG_WHITE | C_BG_BLACK);
    }
}

#define SCAN_CODE_1 0x02
#define SCAN_CODE_2 0x03
#define SCAN_CODE_3 0x04
#define SCAN_CODE_4 0x05
#define SCAN_CODE_5 0x06
#define SCAN_CODE_6 0x07
#define SCAN_CODE_7 0x08
#define SCAN_CODE_8 0x09
#define SCAN_CODE_9 0x0a
#define SCAN_CODE_0 0x0b
#define SCAN_CODE_Y 0x15

void idt_int_teclado(uint8_t scan_code) {

    // Decomentar esto cuando quieras saber un scancode.
    // print_hex(scan_code, 2, 78, 0, C_FG_WHITE | C_BG_BLACK);
    // return;

    int i = -1;
    if (scan_code == SCAN_CODE_0) {
        i = 0;
    }
    else if (scan_code == SCAN_CODE_1) {
        i = 1;
    }
    else if (scan_code == SCAN_CODE_2) {
        i = 2;
    }
    else if (scan_code == SCAN_CODE_3) {
        i = 3;
    }
    else if (scan_code == SCAN_CODE_4) {
        i = 4;
    }
    else if (scan_code == SCAN_CODE_5) {
        i = 5;
    }
    else if (scan_code == SCAN_CODE_6) {
        i = 6;
    }
    else if (scan_code == SCAN_CODE_7) {
        i = 7;
    }
    else if (scan_code == SCAN_CODE_8) {
        i = 8;
    }
    else if (scan_code == SCAN_CODE_9) {
        i = 9;
    }
    else if (scan_code == PLAYER_A_MOVE_UP) {
        game_move_player(PLAYER_A, Up);
    }
    else if (scan_code == PLAYER_A_MOVE_DOWN) {
        game_move_player(PLAYER_A, Down);
    }
    else if (scan_code == PLAYER_A_DISPATCH) {
        sched_spawn(PLAYER_A);
    }
    else if (scan_code == PLAYER_B_MOVE_UP) {
        game_move_player(PLAYER_B, Up);
    }
    else if (scan_code == PLAYER_B_MOVE_DOWN) {
        game_move_player(PLAYER_B, Down);
    }
    else if (scan_code == PLAYER_B_DISPATCH) {
        sched_spawn(PLAYER_B);
    }
    else if (scan_code == SCAN_CODE_Y && !idt_modo_mensaje) {

        // Si se presiona Y y no estamos en modo error, togglea el valor de
        // `idt_modo_debug`
        idt_modo_debug = ! idt_modo_debug;

        // Imprime arriba si estamos en modo debug
        uint8_t color_texto = idt_modo_debug ? C_FG_WHITE : C_FG_BLACK;
        print("MODO DEBUG", 25, 0, C_BG_BLACK | color_texto);

    }
    else if (scan_code == SCAN_CODE_Y && idt_modo_mensaje) {
        // Si se presiona Y en modo de mensaje de error, se reestablece la
        // pantalla y se sale del modo de error.

        idt_modo_mensaje = FALSE;

        // Reestablecer pantalla... FIXME
        screen_draw();

        // Sale del modo de error
        idt_modo_mensaje = 0;

    }

    if (i == -1) return;

    print_dec(i, 1, 80-1, 0, C_FG_WHITE | C_BG_BLACK);
}

/** Devuelve true si la interrupcion devuelve un codigo de error */
#define aux_hay_error_code(vector) \
    (vector == 8) \
    || (vector == 10) \
    || (vector == 11) \
    || (vector == 12) \
    || (vector == 13) \
    || (vector == 14) \
    || (vector == 17)

#define aux_print_dato(nombre, dato, len_dato, fila) \
    print(nombre, 22, fila, C_BG_BLUE | C_FG_WHITE); \
    print_hex(dato, len_dato, 29, fila, C_BG_BLUE | C_FG_WHITE)

void idt_print_mensaje_error(uint8_t codigo_interrupcion,
                             idt_exception_context_t* estado) {

    // Solo imprime este mensaje si esta en modo debug y no se estaba mostrando
    // un mensaje anteriormente.
    if ( (! idt_modo_debug) || idt_modo_mensaje ) return;

    idt_modo_mensaje = TRUE;

    // Lee con un puntero el resto del stack. Esta parte quedo afuera del
    // struct porque pudo haber o no un codigo de error dependiendo de la
    // excepcion.
    uint32_t* p = (void*)( (void*) estado + sizeof(idt_exception_context_t) );
    uint32_t error_code = 0;
    if ( aux_hay_error_code(codigo_interrupcion) ) {
        error_code = *p;
        p++;
    }
    uint32_t eip    = p[0];
    uint32_t cs     = p[1];
    uint32_t eflags = p[2];
    uint32_t esp    = p[3];
    uint32_t ss     = p[4];

    // Texto Blanco. Fondo azul
    uint16_t color = C_BG_BLUE | C_FG_WHITE;

    // Dibuja Fondo
    screen_drawBox(1, 20, 40, 40, ' ', color);

    // Escribe nombre excepcion
    print("0x", 22, 3, color);
    print_hex(codigo_interrupcion, 2, 24, 3, color);
    print(IDT_MSGS[codigo_interrupcion], 27, 3, color);

    // Escribe estado
    aux_print_dato("eax", estado->eax, 8, 5);
    aux_print_dato("ebx", estado->ebx, 8, 6);
    aux_print_dato("ecx", estado->ecx, 8, 7);
    aux_print_dato("edx", estado->edx, 8, 8);
    aux_print_dato("esi", estado->esi, 8, 9);
    aux_print_dato("edi", estado->edi, 8, 10);
    aux_print_dato("esp", esp,         8, 11);
    aux_print_dato("ebp", estado->ebp, 8, 12);
    aux_print_dato("eip", eip,         8, 13);

    aux_print_dato("cs", cs,         4, 15);
    aux_print_dato("ds", estado->ds, 4, 16);
    aux_print_dato("es", estado->es, 4, 17);
    aux_print_dato("fs", estado->fs, 4, 18);
    aux_print_dato("gs", estado->gs, 4, 19);
    aux_print_dato("ss", ss,         4, 20);

    aux_print_dato("eflags", eflags, 8, 22);

    aux_print_dato("cr0", rcr0(), 8, 24);
    aux_print_dato("cr2", rcr2(), 8, 25);
    aux_print_dato("cr3", rcr3(), 8, 26);
    aux_print_dato("cr4", rcr4(), 8, 27);

    aux_print_dato("TR", rtr(), 8, 29);
    // Imprime los componentes del selector para que sea mas facil de leer.
    print_dec(rtr() >> 3, 2, 38, 29, color); // Ix de la tarea en gdt[]
    print_dec(rtr() & 0b11, 1, 41, 29, color); // DPL tarea

    aux_print_dato("ERROR", error_code, 8, 31);

}
