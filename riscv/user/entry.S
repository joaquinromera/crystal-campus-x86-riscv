.section .text._start, "ax"
.global _start

_start:
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    call user_main

1:
    j 1b

