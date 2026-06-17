; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================

%include "print.mac"

%define GDT_OFF_DATOS_KERNEL        144
%define GDT_OFF_DATOS_USER          155
%define GDT_OFF_CODIGO_KERNEL       160
%define GDT_OFF_CODIGO_USER         171
%define GDT_OFF_SCREEN              176
%define KERNEL_PAGE_DIR             0x27000
%define CR3_ACTIVATE_PAGING         0x80000000

extern GDT_DESC
extern screen_draw
extern screen_draw_crystals
extern IDT_DESC
extern idt_init
extern pic_reset
extern pic_enable
extern pic_disable
extern mmu_init
extern mmu_initKernelDir
extern mmu_initTaskDir
extern tss_init
extern fill_tss_idle
extern sched_init
extern game_init

global start

;; Saltear seccion de datos
jmp start

;;
;; Seccion de datos.
;; -------------------------------------------------------------------------- ;;
start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg

;;
;; Seccion de código.
;; -------------------------------------------------------------------------- ;;

;; Punto de entrada del kernel.
BITS 16
start:

    ; Deshabilitar interrupciones
    cli

    ; Cambiar modo de video a 80 X 50
    mov ax, 0003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font

    ; Imprimir mensaje de bienvenida
    print_text_rm start_rm_msg, start_rm_len, 0x07, 0, 0

    ; Habilitar A20
    
    ; Cargar la GDT
    lgdt [GDT_DESC]

    ; Setear el bit PE del registro CR0
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Saltar a modo protegido
    ; 160 == offset codigo kernel
    jmp 0xA0:modo_protegido
modo_protegido:
BITS 32

    ; Establecer selectores de segmentos
    mov ax, GDT_OFF_DATOS_KERNEL
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov ss, ax
    mov ax, GDT_OFF_SCREEN
    mov fs, ax

    ; Establecer la base de el stack
    mov esp, 0x27000
    mov ebp, 0x27000

    ; Imprimir mensaje de bienvenida
    print_text_pm start_pm_msg, start_pm_len, 0x07, 0, 0

    ; Inicializar pantalla

    ; Limpia la pantalla
    call screen_limpiar
    call screen_draw

    ; Inicializar el manejador de memoria
    call mmu_init

    ; Inicializar el directorio de paginas
    call mmu_initKernelDir

    ; Cargar directorio de paginas
    mov eax, KERNEL_PAGE_DIR
    mov cr3, eax

    ; Habilitar paginacion
    mov eax, cr0
    or eax, CR3_ACTIVATE_PAGING
    mov cr0, eax

    ; ; DESCOMENTAR PARA PROBAR mmu_initTaskDir
    ; ; Pruebo esquema de paginacion de tarea (la parte del kernel) cargando un
    ; ; mapa de memoria de tarea.
    ; push 0
    ; push 0
    ; xchg bx, bx
    ; call mmu_initTaskDir
    ; mov ebx, cr3 ; Preservo el directorio de kernel.
    ; mov cr3, eax ; Pone CR3 de prueba
    ; xchg bx, bx
    ; call screen_limpiar ; Limpia pantalla
    ; mov cr3, ebx ; Esto restaura el mapa de memoria original
    ; xchg bx, bx


    ; Inicializar tss
    call tss_init
    mov ax, 0xC0 ; 0xC0 = indice tarea inicial >> 3
    ltr ax

    ; Inicializar tss de la tarea Idle
    call fill_tss_idle

    ; Inicializar el scheduler
    call sched_init

    ; Inicializa estado del juego
    call game_init

    ; Inicializar la IDT
    call idt_init

    ; Cargar IDT
    lidt [IDT_DESC]
 
    ; Probar IDT: Causar interrupcion

    ; Configurar controlador de interrupciones
    call pic_reset
    call pic_enable

    sti

    ; Habilitar interrupciones

    ; Saltar a la primera tarea (Idle)
    jmp 0xB8:0 ; 0xB8 = indice tarea idle >> 3

    ; Ciclar infinitamente (por si algo sale mal...)
    mov eax, 0xFFFF
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    jmp $

;; -------------------------------------------------------------------------- ;;

; Limpia la pantalla (negro). Esta en asm para probar el segmento de video en
; `fs`.
screen_limpiar:
    mov ecx, 8000
    xor eax, eax
    jmp .cmp
.loop:
    dec ecx
    mov [fs:ecx], al
.cmp:
    cmp ecx, 0
    jnz .loop
    ret

%include "a20.asm"
