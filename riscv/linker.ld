ENTRY(_start)

SECTIONS {
  /* La RAM de QEMU virt arranca en 0x80000000.
     Con `-bios none`, el reset vector de QEMU salta a 0x80000000, así que linkear el
     kernel en la base de la RAM. */
  . = 0x80000000;

  .text : {
    KEEP(*(.text._start))   /* Asegurar que _start quede primero */
    *(.text*)
  }

  .rodata : {
    *(.rodata*)
  }

  .sdata : {
    /* La ABI de RISC-V espera que gp se inicialice con __global_pointer$. */
    PROVIDE(__global_pointer$ = . + 0x800);
    *(.sdata*)
  }

  .data : {
    *(.data*)
  }

  .bss : {
    __bss_start = .;
    *(.sbss*)
    *(.bss*)
    *(COMMON)
    __bss_end = .;
  }
}
