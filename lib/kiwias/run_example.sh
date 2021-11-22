#! /bin/bash

# Usage example
# $ ./run_example.sh examples/global_var/ -a

option=$2

if [[ "$option" == "--exec" || "$option" == "-a" ]]; then
    echo ---------------- value ----------------------
    riscv64-unknown-linux-gnu-gcc -static -o tmp $1main.s
    qemu-riscv64 tmp
    echo $?
    echo ---------------------------------------------
fi

if [[ "$option" == "--obj" || "$option" == "-a" || "$option" == "" ]]; then
    echo ---------------- objdump ----------------------
    riscv64-unknown-linux-gnu-as -o tmp.o $1main.s
    riscv64-unknown-linux-gnu-objdump -dr -t -Mno-aliases tmp.o
    echo ---------------------------------------------
fi

if [[ "$option" == "--elf" || "$option" == "-a" || "$option" == "" ]]; then
    echo ---------------- readelf ----------------------
    riscv64-unknown-linux-gnu-as -o tmp.o $1main.s
    riscv64-unknown-linux-gnu-readelf -a tmp.o
    echo ---------------------------------------------
fi

if [[ "$option" == "--hex" || "$option" == "-a" || "$option" == "" ]]; then
    echo ---------------- xxd ----------------------
    riscv64-unknown-linux-gnu-as -o tmp.o $1main.s
    xxd tmp.o
fi

