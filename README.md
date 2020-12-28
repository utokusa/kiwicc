# kiwicc
kiwicc is a hobby C compilor for RISC-V.

## Reference
https://www.sigbus.info/compilerbook

http://port70.net/~nsz/c/c11/n1570.html

https://riscv.org/technical/specifications/

https://github.com/riscv/riscv-asm-manual/blob/master/riscv-asm.md

https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md

https://msyksphinz-self.github.io/riscv-isadoc/html/index.html#

## Setup
```bash
$ docker build -t riscv_dev .
```

## Run development environment

```bash
# run docker container for development
$ sh start_kiwicc_dev.sh
```

## Build kiwicc

```bash
# in development environment
$ make
```

## Test kiwicc

```bash
# in development environment
$ make test
```

## Compile a C program for RISC-V and run it on QEMU user-mode emulation

```bash
# in development environment

# compile with kiwicc
$ qemu-riscv64 kiwicc foo.c -o tmp.s
$ riscv64-unknown-linux-gnu-gcc tmp.s -o a.out

# compile with GCC
$ riscv64-unknown-linux-gnu-gcc -g -O0 foo.c

# run on QEMU user-mode emulation
$ qemu-riscv64 a.out

# debugging with gdb
$ riscv64-unknown-linux-gnu-gdb a.out
(gdb) shell qemu-riscv64 -g 1234 a.out &
(gdb) target remote :1234

# debugging kiwicc with gdb
$ riscv64-unknown-linux-gnu-gdb kiwicc
(gdb) shell qemu-riscv64 -g 1234 ./kiwicc tests/tests.c -I./tests -fno-pic -o tmp.s &
(gdb) target remote :1234
```

