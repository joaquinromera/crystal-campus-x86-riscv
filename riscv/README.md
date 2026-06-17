## Port RISC-V (QEMU virt)

Este es un port a RISC-V (rv64) del juego del TP3 original.

Corre en la máquina `virt` de QEMU y renderiza la pantalla en “modo texto” de 80x50 a través de la UART 16550 usando secuencias de escape ANSI de cursor/color.

### Compilar

```sh
make -C riscv
```

### Correr (QEMU)

```sh
make -C riscv run
```

O directamente:

```sh
qemu-system-riscv64 -machine virt -nographic -monitor none -serial stdio -bios none -kernel riscv/build/kernel.elf
```

### Controles

- Jugador ROJO: `W` (arriba), `S` (abajo), `E` o `Q` (spawnear minero)
- Jugador AZUL: `I` (arriba), `K` (abajo), `P` o `O` (spawnear minero)

### Notas

- El kernel corre en **S-mode** (bootstrap de M-mode en `riscv/machine.S`) y los mineros corren en **U-mode**.
- Las syscalls se implementan con `ecall` (trap U→S) y se manejan en `trap_handle`.Los IDs de syscall coinciden con el TP original (`move=177788`, `take=310311`, `getId=760279`).
- El scheduling está manejado por un tick enviado como **SSIP** (interrupción de software de supervisor), generado desde el handler de interrupción del timer de máquina en `riscv/machine.S`.
- MMU simil TP3: el paginado Sv39 está habilitado y cada minero tiene su propio `satp`
  (espacio de direcciones). El código del minero vive en una “imagen de celda” de 8KB y `move`
  copia esa imagen a la celda destino y remapea `TASK_CODE`
  Los binarios de usuario viven en `riscv/user/` y se embeben en el kernel en tiempo de linkeo.
