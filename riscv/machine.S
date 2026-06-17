.section .text

# Firmware mínimo en machine-mode para correr sin OpenSBI (-bios none).
#
# - Configura PMP para que S-mode pueda acceder a memoria/dispositivos.
# - Delega la mayoría de los traps a S-mode.
# - Usa la interrupción del timer de máquina (MTIP)
#   En cada MTIP, programa el próximo comparador y levanta SSIP para que el handler
#   de traps de S-mode existente haga el trabajo de tick del scheduler
#
# Esto mantiene el kernel casi sin cambios: sigue corriendo en S-mode y usa
# stvec/sret
#
# Direcciones del temporizador de QEMU virt (CLINT/ACLINT compatible con MTIMER).
# - mtimecmp(hart) = 0x0200_4000 + 8*hartid
# - mtime          = 0x0200_BFF8
.equ MTIMECMP_BASE, 0x02004000

.global machine_init
.align 4
machine_init:
    # Usar un stack de machine-mode para traps
    la t0, machine_stack_top
    csrw mscratch, t0

    # Vector de traps
    la t0, machine_trap_entry
    csrw mtvec, t0

    # Delegar interrupciones a S-mode: SSIP, STIP, SEIP.
    li t0, ((1 << 1) | (1 << 5) | (1 << 9))
    csrw mideleg, t0

    # Delegar excepciones a S-mode (los bits 0..15 alcanzan para el tp)
    li t0, 0xFFFF
    csrw medeleg, t0

    # PMP: permitir a S-mode acceso completo a memoria física y MMIO.
    # Usar una única entrada TOR que cubra 0..max
    li t0, -1
    csrw pmpaddr0, t0
    # R|W|X + TOR
    li t0, 0x0F
    csrw pmpcfg0, t0

    # Habilitar interrupciones del timer
    # MTIE
    li t0, (1 << 7)
    csrs mie, t0

    # Programar el primer tick
    csrr t0, time
    la t1, g_timer_delta_ticks
    ld t1, 0(t1)
    add t0, t0, t1
    csrr t1, mhartid
    slli t1, t1, 3
    li t2, MTIMECMP_BASE
    add t2, t2, t1
    sd t0, 0(t2)

    # Entrar a S-mode en kernel_main con interrupciones habilitadas durante la ejecución
    # mret va a setear MIE <- MPIE, así que acá seteamos MPIE=1.
    csrr t0, mstatus
    # Limpiar MPP (bits 12:11).
    li t1, (3 << 11)
    not t1, t1
    and t0, t0, t1
    # MPP = S (01).
    li t1, (1 << 11)
    or t0, t0, t1
    # MPIE = 1.
    li t1, (1 << 7)
    or t0, t0, t1
    csrw mstatus, t0

    la t0, kernel_main
    csrw mepc, t0
    mret

.global machine_trap_entry
.align 4
machine_trap_entry:
    # Cambiar al machine stack
    # - sp pasa a ser el stack pointer (mscratch)
    # - mscratch guarda el sp interrumpido (stack de S/U-mode)
    csrrw sp, mscratch, sp

    addi sp, sp, -32
    sd t0, 0(sp)
    sd t1, 8(sp)
    sd t2, 16(sp)
    sd t3, 24(sp)

    csrr t0, mcause
    # Interrupción -> bit alto seteado
    srli t1, t0, 63
    beqz t1, 2f

    # cause_code = mcause sin el bit de interrupción
    slli t0, t0, 1
    srli t0, t0, 1

    # Interrupción del timer = 7
    li t1, 7
    bne t0, t1, 2f

    # Programar la próxima interrupción del timer.
    csrr t0, time
    la t1, g_timer_delta_ticks
    ld t1, 0(t1)
    add t0, t0, t1
    csrr t1, mhartid
    slli t1, t1, 3
    li t2, MTIMECMP_BASE
    add t2, t2, t1
    sd t0, 0(t2)

    # Levantar una interrupción de software de supervisor para que corra el pulso del kernel en S-mode
    # SSIP
    li t0, 2
    csrs sip, t0

    # Restaurar y volver al contexto que se había interrumpido
    ld t3, 24(sp)
    ld t2, 16(sp)
    ld t1, 8(sp)
    ld t0, 0(sp)
    addi sp, sp, 32
    csrrw sp, mscratch, sp
    mret

2:
    # Trap inesperado en machine-mode
1:
    wfi
    j 1b

.section .bss
.align 16
machine_stack:
    .space 4096
machine_stack_top:
