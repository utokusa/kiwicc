#! /bin/bash

# # Link the binary generated by kiwias and see the return value
# expected="42"
# cat <<EOF | qemu-riscv64 ./kiwias -
#     .text
#     .globl main
# main:
#     addi a0, zero, 42
#     jr ra
# EOF
# riscv64-unknown-linux-gnu-ld -dynamic-linker /opt/riscv/sysroot/lib/ld-linux-riscv64-lp64d.so.1 /opt/riscv/sysroot/usr/lib/crt1.o out.o -lc /opt/riscv/sysroot/usr/lib/crtn.o
# qemu-riscv64 a.out
# actual="$?"
# 
# if [ "$actual" = "$expected" ]; then
#     echo "$expected => $actual"
# else
#     echo "$expected => $expected expected, but got $actual"
#     exit 1
# fi
# 
# cat <<EOF | riscv64-unknown-linux-gnu-as -o tmp.o -
#     .text
#     .globl main
# main:
#     addi a0, zero, 42
#     jr ra
# EOF
# 
# # Compare the object files using cmp command
# cmp out.o tmp.o


function cmp_with_as() {
    echo -----------------------------
    input=$(cat $@)
    echo "$input"
    echo "$input" | riscv64-unknown-linux-gnu-as -o tmp_expected.o -
    echo "$input" | qemu-riscv64 ./kiwias -
    if ! diff <(xxd -b tmp_expected.o) <(xxd -b out.o)
    then
        echo [Error] The contents of the object files are different
        echo ---------[Actual]------------
        xxd out.o
        echo ---------[Expected]----------
        xxd tmp_expected.o
        exit 1
    else
        echo ----------[Pass]-------------
    fi
}

cmp_with_as <<EOF
    .text
    .globl main
main:
    addi a0, zero, 42
    jr ra
EOF

cmp_with_as <<EOF
    .text
    .globl main
main:
    addi a0, zero, 291
    jr ra
EOF

# Test max signed 12-bit integer
cmp_with_as <<EOF
    .text
    .globl main
main:
    addi a0, zero, 2047
    jr ra
EOF

cmp_with_as <<EOF
    .text
    .globl main
main:
    addi a0, zero, -1
    jr ra
EOF

cmp_with_as <<EOF
    .text
    .globl main
main:
    addi a0, zero, -2
    jr ra
EOF

cmp_with_as <<EOF
    .text
    .globl main
main:
    addi a1, zero, 2
    addi a0, a1, 3
    jr ra
EOF

cmp_with_as <<EOF
    .text
    .globl main
main:
    addi t1, zero, 1
    addi a1, t1, 2
    addi a0, a1, 3
    jr ra
EOF

cmp_with_as <<EOF
    .text
    .globl main
main:
    addi x18, x0, 1
    addi x11, x18, 2
    addi x10, x11, 3
    jr ra
EOF
