    .text
    .globl main
main:
    la t1, i
    lw t1, (t1)
    addi a0, t1, 0
    jr ra
