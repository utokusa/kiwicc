#! /bin/bash

# Usage example
# $ ./run_example.sh examples/global_var/

riscv64-unknown-linux-gnu-gcc -static -o tmp $1main.s
qemu-riscv64 tmp
echo $?

