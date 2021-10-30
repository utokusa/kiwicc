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

cat <<EOF | riscv64-unknown-linux-gnu-as -o tmp.o -
	.text
	.globl	main
main:
	li	a0, 42
	jr	ra
EOF

cmp out.o tmp.o


function cmp_with_as() {
    echo -----------------------------
    input=$(cat $@)
    echo "$input"
    echo "$input"
    echo "$input" | riscv64-unknown-linux-gnu-as -o tmp_expected.o -
    echo "$input" | qemu-riscv64 ./kiwias -
    if ! diff <(xxd tmp_expected.o) <(xxd out.o)
    then
        echo [Error] The contents of the object files are different
        exit 1
    else
        echo ----------[Pass]-------------
    fi
}

cmp_with_as <<EOF
	.text
	.globl	main
main:
	li	a0, 42
	jr	ra
EOF

cmp_with_as <<EOF
	.text
	.globl	main
main:
	li	a0, 43
	jr	ra
EOF
