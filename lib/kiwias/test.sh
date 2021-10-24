#! /bin/bash


expected="42"
qemu-riscv64 ./kiwias
riscv64-unknown-linux-gnu-ld -dynamic-linker /opt/riscv/sysroot/lib/ld-linux-riscv64-lp64d.so.1 /opt/riscv/sysroot/usr/lib/crt1.o out.o -lc /opt/riscv/sysroot/usr/lib/crtn.o
qemu-riscv64 a.out
actual="$?"

if [ "$actual" = "$expected" ]; then
    echo "$expected => $actual"
else
    echo "$expected => $expected expected, but got $actual"
    exit 1
fi

