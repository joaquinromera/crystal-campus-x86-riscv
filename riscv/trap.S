.section .text

# Desplazamientos dentro de TrapFrame (coinciden con riscv/trap.h)
.equ TF_RA,      0
.equ TF_SP,      8
.equ TF_GP,      16
.equ TF_TP,      24
.equ TF_T0,      32
.equ TF_T1,      40
.equ TF_T2,      48
.equ TF_S0,      56
.equ TF_S1,      64
.equ TF_A0,      72
.equ TF_A1,      80
.equ TF_A2,      88
.equ TF_A3,      96
.equ TF_A4,      104
.equ TF_A5,      112
.equ TF_A6,      120
.equ TF_A7,      128
.equ TF_S2,      136
.equ TF_S3,      144
.equ TF_S4,      152
.equ TF_S5,      160
.equ TF_S6,      168
.equ TF_S7,      176
.equ TF_S8,      184
.equ TF_S9,      192
.equ TF_S10,     200
.equ TF_S11,     208
.equ TF_T3,      216
.equ TF_T4,      224
.equ TF_T5,      232
.equ TF_T6,      240
.equ TF_SEPC,    248
.equ TF_SSTATUS, 256
.equ TF_SCAUSE,  264
.equ TF_STVAL,   272

.equ TF_SIZE,    280

.global trap_entry
.align 4
trap_entry:
    # Cambia a stack dedicado de traps (funciona para traps desde U-mode y S-mode)
    # Guarda el sp interrumpido en t6 para poder volcarlo en el TrapFrame
    mv t6, sp
    la sp, trap_stack_top

    # Preservar t0/t1 de usuario antes de usarlos
    addi sp, sp, -16
    sd t0, 0(sp)
    sd t1, 8(sp)

    # t0 = TrapFrame* actual
    la t0, g_current_trapframe
    ld t0, 0(t0)

    # Guardar registros enteros (sp viene de sscratch después del swap)
    sd ra, TF_RA(t0)
    # Guardar sp interrumpido (usuario o kernel)
    sd t6, TF_SP(t0)
    sd gp, TF_GP(t0)
    sd tp, TF_TP(t0)

    # Cambiar gp al global pointer del kernel para que el código C pueda usar accesos
    # gp-relativos a smalldata con seguridad (el código de usuario setea su propio gp)
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    # Restaurar t0/t1 de usuario guardados y volcarlos al marco de trap
    ld t1, 0(sp)
    sd t1, TF_T0(t0)
    ld t1, 8(sp)
    sd t1, TF_T1(t0)
    addi sp, sp, 16

    sd t2, TF_T2(t0)
    sd s0, TF_S0(t0)
    sd s1, TF_S1(t0)
    sd a0, TF_A0(t0)
    sd a1, TF_A1(t0)
    sd a2, TF_A2(t0)
    sd a3, TF_A3(t0)
    sd a4, TF_A4(t0)
    sd a5, TF_A5(t0)
    sd a6, TF_A6(t0)
    sd a7, TF_A7(t0)
    sd s2, TF_S2(t0)
    sd s3, TF_S3(t0)
    sd s4, TF_S4(t0)
    sd s5, TF_S5(t0)
    sd s6, TF_S6(t0)
    sd s7, TF_S7(t0)
    sd s8, TF_S8(t0)
    sd s9, TF_S9(t0)
    sd s10, TF_S10(t0)
    sd s11, TF_S11(t0)
    sd t3, TF_T3(t0)
    sd t4, TF_T4(t0)
    sd t5, TF_T5(t0)
    sd t6, TF_T6(t0)

    # Guardar CSRs de trap
    csrr t1, sepc
    sd t1, TF_SEPC(t0)
    csrr t1, sstatus
    sd t1, TF_SSTATUS(t0)
    csrr t1, scause
    sd t1, TF_SCAUSE(t0)
    csrr t1, stval
    sd t1, TF_STVAL(t0)

    # Llamar al manejador C: TrapFrame* trap_handle(TrapFrame* tf)
    mv a0, t0
    call trap_handle

    # a0 = próximo TrapFrame*
    mv t0, a0

    # g_current_trapframe = siguiente
    la t1, g_current_trapframe
    sd t0, 0(t1)

    # Restaurar CSRs de retorno del trap
    ld t1, TF_SEPC(t0)
    csrw sepc, t1
    ld t1, TF_SSTATUS(t0)
    csrw sstatus, t1

    # Restaurar registros enteros (excepto sp q se restaura vía swap)
    ld ra, TF_RA(t0)
    ld gp, TF_GP(t0)
    ld tp, TF_TP(t0)
    ld t2, TF_T2(t0)
    ld s0, TF_S0(t0)
    ld s1, TF_S1(t0)
    ld a0, TF_A0(t0)
    ld a1, TF_A1(t0)
    ld a2, TF_A2(t0)
    ld a3, TF_A3(t0)
    ld a4, TF_A4(t0)
    ld a5, TF_A5(t0)
    ld a6, TF_A6(t0)
    ld a7, TF_A7(t0)
    ld s2, TF_S2(t0)
    ld s3, TF_S3(t0)
    ld s4, TF_S4(t0)
    ld s5, TF_S5(t0)
    ld s6, TF_S6(t0)
    ld s7, TF_S7(t0)
    ld s8, TF_S8(t0)
    ld s9, TF_S9(t0)
    ld s10, TF_S10(t0)
    ld s11, TF_S11(t0)
    ld t3, TF_T3(t0)
    ld t4, TF_T4(t0)
    ld t5, TF_T5(t0)
    ld t6, TF_T6(t0)

    # Restaurar el sp interrumpido (usuario o kernel)
    ld sp, TF_SP(t0)

    # Restaurar t1/t0 al final (t0 acá es el puntero al marco)
    ld t1, TF_T1(t0)
    ld t0, TF_T0(t0)
    sret

.global enter_user
.align 4
enter_user:
    # a0 = TrapFrame*
    la t1, g_current_trapframe
    sd a0, 0(t1)

    # Cargar el sp de usuario directamente
    ld sp, TF_SP(a0)

    # Programar CSRs de retorno
    ld t1, TF_SEPC(a0)
    csrw sepc, t1
    ld t1, TF_SSTATUS(a0)
    csrw sstatus, t1

    # Restaurar registros (sp ya está seteado) Restaurar a0 al final
    ld ra, TF_RA(a0)
    ld gp, TF_GP(a0)
    ld tp, TF_TP(a0)
    ld t0, TF_T0(a0)
    ld t1, TF_T1(a0)
    ld t2, TF_T2(a0)
    ld s0, TF_S0(a0)
    ld s1, TF_S1(a0)
    ld a1, TF_A1(a0)
    ld a2, TF_A2(a0)
    ld a3, TF_A3(a0)
    ld a4, TF_A4(a0)
    ld a5, TF_A5(a0)
    ld a6, TF_A6(a0)
    ld a7, TF_A7(a0)
    ld s2, TF_S2(a0)
    ld s3, TF_S3(a0)
    ld s4, TF_S4(a0)
    ld s5, TF_S5(a0)
    ld s6, TF_S6(a0)
    ld s7, TF_S7(a0)
    ld s8, TF_S8(a0)
    ld s9, TF_S9(a0)
    ld s10, TF_S10(a0)
    ld s11, TF_S11(a0)
    ld t3, TF_T3(a0)
    ld t4, TF_T4(a0)
    ld t5, TF_T5(a0)
    ld t6, TF_T6(a0)
    ld a0, TF_A0(a0)

    sret

.section .bss
.align 16
trap_stack:
    .space 4096
trap_stack_top:
