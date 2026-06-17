QEMU: un emulador/virtualizador de máquinas. Para RISC-V normalmente se usa qemu-system-riscv64, que emula una CPU RISC-V junto con una “placa” (generalmente -machine virt) y dispositivos (UART, timer, controlador de interrupciones, etc.) para poder ejecutar sin hardware real.

OpenSBI: firmware que corre en modo máquina (M-mode) sobre RISC-V e implementa el SBI (Supervisor Binary Interface). Es una capa que usa el OS/kernel corriendo en modo supervisor (S-mode) mediante ecall para cosas como timer/IPI, reinicio/apagado, terminal. También realiza la transferencia desde M-mode (bootloader/kernel).

“Bare QEMU”: correr QEMU sin ningún firmware, sin OpenSBI/BIOS (frecuentemente con -bios none). El binario arranca desde reset y tiene que hacer el bring-up de M-mode por su cuenta (configuración de traps, plumbing de timers/interrupciones, cambio de modo a S-mode si querés, etc.), y no se puede depender de llamadas SBI porque no hay ninguna implementación de SBI.

Especificaciones (similares al “Intel SDM”)

RISC-V ISA Manual Volumen I (Unprivileged) + Volumen II (Privileged): instrucciones, CSRs, traps/interrupciones y memoria virtual/tablas de páginas (Sv39/Sv48/etc.). (riscv.github.io)
RISC-V SBI spec (lo que implementa OpenSBI) si el kernel corre en S-mode y usa servicios de firmware (timer/IPI/apagado, etc.). (riscv.atlassian.net)
RISC-V ELF psABI (convenciones de llamada, reglas de stack/registros, formato ELF/relocaciones). (riscv-non-isa.github.io)
Para QEMU virt: qemu.eu

MIT xv6-riscv + su libro (github.com)

OSDev “RISC-V Bare Bones / Meaty Skeleton” (kernel de inicio rápido sobre QEMU virt). (wiki.osdev.org)

https://github.com/riscv/riscv-isa-manual

https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/

https://riscv.github.io/riscv-isa-manual/snapshot/privileged

https://riscv.atlassian.net/wiki/spaces/HOME/pages/16154769/RISC%25E2%2580%2591V%2BTechnical%2BSpecifications

https://riscv.atlassian.net/wiki/spaces/HOME/pages/810057837/RISC-V%2BSupervisor%2BBinary%2BInterface

https://riscv-non-isa.github.io/riscv-elf-psabi-doc/

https://qemu.eu/doc/8.0/system/riscv/virt.html

https://github.com/riscv/riscv-plic-spec

https://github.com/riscv/riscv-aclint

https://github.com/mit-pdos/xv6-riscv

https://github.com/mit-pdos/xv6-riscv-book

https://wiki.osdev.org/RISC-V_Bare_Bones

https://wiki.osdev.org/RISC-V_Meaty_Skeleton_with_QEMU_virt_board
