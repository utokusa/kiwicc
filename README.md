# kiwicc
kiwicc is a hobby C compiler for RISC-V.

It can compile itself include preprocessor.

kiwicc is inspired by [chibicc](https://github.com/rui314/chibicc) which is a C compiler for x86-64 and [compilerbook](https://www.sigbus.info/compilerbook).

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

# test kiwicc compiled with gcc (stage 1)
$ make test

# test stage 2 kiwicc
$ make kiwicc-stage2

# test stage 3 kiwicc
$ make kiwicc-stage3

# test kiwicc from stage 1 to stage 3
# https://stackoverflow.com/questions/60567540/why-does-gcc-compile-itself-3-times
$ make test-all
```

## Install kiwicc

```bash
$ make install
```

## Compile a C program for RISC-V and run it on QEMU user-mode emulation

```bash
# in development environment

# compile with kiwicc
$ qemu-riscv64 kiwicc foo.c -o tmp.s
$ riscv64-unknown-linux-gnu-gcc tmp.s -o a.out

# compile with kiwicc installed by `sudo -E make install`
$ kiwicc foo.c -o tmp.o # binfmt_misc calls qemu-riscv64 implicitly
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
(gdb) shell qemu-riscv64 -g 1234 ./kiwicc tests/tests.c -I./tests/test_include -I./tests -fno-pic -o tmp.s &
(gdb) target remote :1234
```



## Example

### Compile and run [2048](https://github.com/mevdschee/2048.c)

```bash
# in development environment

$ sudo -E make install
$ cd /home/user
$ git clone https://github.com/mevdschee/2048.c
$ cd 2048.c
$ qemu-riscv64 /opt/riscv/bin/kiwicc -o 2048.s 2048.c # `kiwicc -o 2048.s 2048.c` is also OK.
$ riscv64-unknown-linux-gnu-gcc -o 2048 2048.s
$ qemu-riscv64 2048
```

### Compile and run [Tree](http://mama.indstate.edu/users/ice/tree/) command

```bash
# in development environment

$ sudo -E make install
$ cd /home/user
$ mkdir -p ./tree
$ curl -sL --retry 3 --insecure "http://mama.indstate.edu/users/ice/tree/src/tree-1.8.0.tgz" | tar xz --no-same-owner --strip-components=1 -C ./tree
$ cd tree

# configure, build and run
$ sed -i -e 's/^CC=gcc/CC=kiwicc/' -e 's/^CFLAGS/#CFLAGS/' -e 's/$(CC) $(LDFLAGS)/riscv64-unknown-linux-gnu-gcc $(LDFLAGS)/' Makefile
$ make
$ qemu-riscv64 tree
```

## Reference

https://www.sigbus.info/compilerbook (Japanese)

https://github.com/rui314/chibicc

http://port70.net/~nsz/c/c11/n1570.html

https://riscv.org/technical/specifications/

https://github.com/riscv/riscv-asm-manual/blob/master/riscv-asm.md

https://github.com/riscv/riscv-elf-psabi-doc/blob/master/riscv-elf.md

https://msyksphinz-self.github.io/riscv-isadoc/html/index.html#