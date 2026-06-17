; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================
; definicion de rutinas de atencion de interrupciones

%include "print.mac"

BITS 32

sched_task_offset:     dw 0xFFFF
sched_task_selector:   dd 0xFFFFFFFF

extern idt_print_nro
extern idt_int_teclado
extern idt_modo_mensaje
extern idt_print_mensaje_error 

;; PIC
extern pic_finish1

;; Sched
extern sched_nextTask
extern sched_kill

;; Syscall implementations
extern game_move
extern game_take
extern game_get_id


;;
;; Definición de MACROS
;; -------------------------------------------------------------------------- ;;

; Rutina de atencion para las excepciones. Se encarga de matar a la tarea de
; jugador que la origina.
%macro ISR_EXCEPTION 1
global _isr%1

_isr%1:
    pushad

    ; Pasamos los selectores de segmento del contexto actual (ds, es, fs, es,
    ; gs no deberian cambiar) para levantarlos desde C (usando el struct)
    ; porque no encontramos como leer un selector de segmento en C.
    mov ax, cs ; Contexto lvl 0
    push ax
    mov ax, ds
    push ax
    mov ax, es
    push ax
    mov ax, fs
    push ax
    mov ax, gs
    push ax
    mov ax, ss ; Contexto lvl 0
    push ax

    mov eax, %1

    ; Intenta imprimir el mensaje de debug. Es esa funcion la que se encarga de
    ; no hacer nada si no estamos en modo debug.
    push esp ; 2do param = estado antes de la excepcion
    push eax ; 1er param = codigo interrupcion
    call idt_print_mensaje_error
    add esp, 8 ; Saco los params del stack

    add esp, 24 ; Saco los 6 selectores de segmento que pushe manualmente

    ; Imprime el nro de excepcion en pantalla
    ; push eax
    ; call idt_print_nro
    ; pop eax

    call sched_kill

    ; salta a la tarea idle
    jmp 0xB8:0 ; 0xB8 = ix tarea idle << 3

    ; Iret, pero nunca va a volver a ejecutar esta tarea entonces no se alcanza
    ; esta porcion de codigo.
    popad
    iret

%endmacro

%macro ISR 1
global _isr%1

_isr%1:
    mov eax, %1
    push eax
    call idt_print_nro
    pop eax
    iret

%endmacro

; Rutina atencion interrupcion Clock
global _isr32
_isr32:
    pushad
    call pic_finish1
    call nextClock

    ; Se fija si esta en modo mensaje de error. Eso inhibe el scheduling
    ; mientras este el mensaje en pantalla.
    cmp byte [idt_modo_mensaje], 0
    jne .fin ; No hace scheduling

    call sched_nextTask
    ; Se fija de no hacer un salto si `sched_nextTask` devuelve la misma tarea.
    str cx
    cmp ax, cx
    je .fin
    ; Salta a la prox tarea
    mov [selector], ax
    jmp far [offset]

.fin:
    popad
    iret

; Rutina atencion interrupcion teclado
global _isr33
_isr33:
    pushad
    call pic_finish1
    in al, 0x60
    push eax
    call idt_int_teclado
    pop eax
    popad
    iret

%define syscal_move_id  177788
%define syscal_take_id  310311
%define syscal_getid_id 760279
global _isr47
_isr47:
    pushad

    ; Se fija cual syscall se debe atender basado en el valor de EAX
    cmp eax, syscal_move_id
    je .move
    cmp eax, syscal_take_id
    je .take
    cmp eax, syscal_getid_id
    je .getid
    ; Caso en el que no se reconoce la syscall, se debe matar la tarea y saltar
    ; a la idle.
    call sched_kill ; Mata esta tarea
    jmp .jmp_to_idle      ; 0xB8 = ix tarea idle << 3

.move:
    push ebx
    call game_move
    pop ebx
    jmp .jmp_to_idle

.take:
    call game_take
    ; la funcion `game_take` me devolvio el peso del cristal obtenido en `EAX`.
    ; Para no pisarlo al llamar a `POPAD`, tenemos que escribirlo en el stack
    ; en la ubicacion del `EAX` preservado.
    mov [esp + 28], eax
    jmp .jmp_to_idle

.getid:
    call game_get_id
    ; la funcion `game_get_id` me devolvio el id en `EAX`. Para no pisarlo al
    ; llamar a `POPAD`, tenemos que escribirlo en el stack en la ubicacion del
    ; `EAX` preservado.
    mov [esp + 28], eax

    jmp .fin

.jmp_to_idle:
    jmp 0xb8:0

.fin:
    popad
    iret

;; Rutina de atención de las EXCEPCIONES
;; -------------------------------------------------------------------------- ;;

;; {{{
ISR_EXCEPTION 0
ISR_EXCEPTION 1
ISR_EXCEPTION 2
ISR_EXCEPTION 3
ISR_EXCEPTION 4
ISR_EXCEPTION 5
ISR_EXCEPTION 6
ISR_EXCEPTION 7
ISR_EXCEPTION 8
ISR_EXCEPTION 9
ISR_EXCEPTION 10
ISR_EXCEPTION 11
ISR_EXCEPTION 12
ISR_EXCEPTION 13
ISR_EXCEPTION 14
ISR_EXCEPTION 15
ISR_EXCEPTION 16
ISR_EXCEPTION 17
ISR_EXCEPTION 18
ISR_EXCEPTION 19
ISR_EXCEPTION 20

;; }}}

;; Rutina de atención del RELOJ
;; -------------------------------------------------------------------------- ;;
offset:     dd 0x00
selector:   dw 0x00

;; Rutina de atención del TECLADO
;; -------------------------------------------------------------------------- ;;

;; Rutinas de atención de las SYSCALLS
;; -------------------------------------------------------------------------- ;;

;; Funciones Auxiliares
;; -------------------------------------------------------------------------- ;;
isrNumber:           dd 0x00000000
isrClock:            db '|/-\'
nextClock:
    pushad
    inc DWORD [isrNumber]
    mov ebx, [isrNumber]
    cmp ebx, 0x4
    jl .ok
            mov DWORD [isrNumber], 0x0
            mov ebx, 0
    .ok:
            add ebx, isrClock
            print_text_pm ebx, 1, 0x0f, 49, 79
            popad
    ret
