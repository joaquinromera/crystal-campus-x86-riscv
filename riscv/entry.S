.section .text._start, "ax"
.global _start

/* Punto de entrada RISC-V (QEMU virt).
   Este build apunta a un QEMU bios none: entramos en M-mode y
   corremos un bootstrap mínimo en machine mode (ver riscv/machine.S) que:
     - habilita PMP para que S-mode pueda acceder a memoria,dispositivos
     - configura el tick del timer de máquina
     - baja a S-mode en kernel_main */
_start:
    /* La psABI de RISC-V requiere gp para accesos a small data. Evitar que linker lo convierta en `addi gp,gp,0`. */
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    /* Deshabilitar interrupciones */
    csrw mie, zero
    csrci mstatus, 0x8        /* MIE */
    csrw sie, zero
    csrci sstatus, 0x2        /* SIE */

    /* Configurar el stack */
    la sp, kernel_stack_top

    /* Limpiar .bss */
    la t0, __bss_start
    la t1, __bss_end
1:
    bgeu t0, t1, 2f
    sb zero, 0(t0)
    addi t0, t0, 1
    j 1b
2:

    /* Entrar a S-mode y saltar a kernel_main (no vuelve). */
    call machine_init

    /* Si kernel_main alguna vez retorna, detenerse acá. */
3:
    j 3b

.section .bss
.align 16
kernel_stack:
    .space 8192
kernel_stack_top:
