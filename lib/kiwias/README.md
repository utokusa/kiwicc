# kiwias
kiwias is a RISC-V assembler written by C.

## memo

tmp.s
```c
	.text
	.globl	main
main:
	li	a0, 42
	jr	ra
```

```bash
riscv64-unknown-linux-gnu-gcc -v tmp.s
qemu-riscv64 a.out
echo $?
```

## Use ld directly
```bash
riscv64-unknown-linux-gnu-as -o tmp.o tmp.s
riscv64-unknown-linux-gnu-ld -dynamic-linker /opt/riscv/sysroot/lib/ld-linux-riscv64-lp64d.so.1 /opt/riscv/sysroot/usr/lib/crt1.o tmp.o -lc /opt/riscv/sysroot/usr/lib/crtn.o
qemu-riscv64 a.out
echo $?
```
## tools
- readelf
- objdump
- hexdump (can be installed by apt install bsdmainutils)
- cmp
```bash
riscv64-unknown-linux-gnu-objdump -s tmp.o
riscv64-unknown-linux-gnu-objdump -s -j .data tmp.o
riscv64-unknown-linux-gnu-objdump -dr -t -Mno-aliases tmp.o
hexdump -v -C tmp.o
xxd tmp.o
xxd -c 4 tmp.o # with column length 4 bypte
cmp -l out.o tmp.o
diff <(xxd out.o) <(xxd tmp.o)
```

## reference
- https://zenn.dev/dqneo/articles/012faee0b220fa

