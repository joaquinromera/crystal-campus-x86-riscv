ENTRY(_start)

SECTIONS {
  . = 0x08000000;

  .text : {
    KEEP(*(.text._start))
    *(.text*)
  }

  .rodata : {
    *(.rodata*)
  }

  .sdata : {
    PROVIDE(__global_pointer$ = . + 0x800);
    *(.sdata*)
  }

  .data : {
    *(.data*)
  }

  .bss : {
    *(.sbss*)
    *(.bss*)
    *(COMMON)
  }
}

