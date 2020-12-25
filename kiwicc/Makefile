CC=riscv64-unknown-linux-gnu-gcc
CFLAGS=-std=c11 -g -O0 -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS: .c=.0)

kiwicc: $(OBJS) kiwicc.h
	$(CC) $(CFLAGS) -o kiwicc $(OBJS) $(LDFLAGS)

$(OBJS): kiwicc.h

# Not available
kiwicc-stage2: kiwicc $(SRCS) kiwicc.h self.sh
	./self.sh tmp-stage2 ./kiwicc kiwicc-stage2

# Not available
kiwicc-stage3: kiwicc-stage2
	./self.sh tmp-stage3 ./kiwicc-stage2 kiwicc-stage3

test: kiwicc
# TODO: remove  -fno-pic
	qemu-riscv64 -L $(RISCV)/sysroot ./kiwicc tests/tests.c -I./tests -fno-pic -o tmp.s
	$(CC) -xc -c -o tmp2.o tests/extern.c
	$(CC) -o tmp tmp.s tmp2.o
	qemu-riscv64 -L $(RISCV)/sysroot ./tmp

test-nopic: kiwicc
	qemu-riscv64 -L $(RISCV)/sysroot ./kiwicc tests/tests.c -I./tests -fno-pic -o tmp.s
	$(CC) -xc -c -o tmp2.o tests/extern.c
	$(CC) -o tmp tmp.s tmp2.o
	qemu-riscv64 -L $(RISCV)/sysroot ./tmp

# Not available
test-stage2: kiwicc-stage2
	./kiwicc-stage2 tests/tests.c -I./tests  -o tmp.s
	gcc -xc -c -o tmp2.o tests/extern.c
	gcc -static -fno-common -o tmp tmp.s tmp2.o
	./tmp

# Not available
test-stage3: kiwicc-stage3
	diff kiwicc-stage2 kiwicc-stage3

# Not available
test-all: test test-nopic test-stage2 test-stage3

test-gcc:
	$(CC) tests/tests.c -o tmp.s
	$(CC) -xc -c -o tmp2.o tests/extern.c
	$(CC) -o tmp tmp.s tmp2.o
	qemu-riscv64 -L $(RISCV)/sysroot ./tmp

tmp-kiwicc: kiwicc
# TODO: remove  -fno-pic
	qemu-riscv64 -L $(RISCV)/sysroot ./kiwicc tests/tmp_test.c -o -fno-pic -o  tmp.s
	$(CC) -o tmp tmp.s
	echo 'qemu-riscv64 -L $(RISCV)/sysroot ./tmp; echo $$?' | sh -

tmp-as:
	$(CC) -static -o tmp tmp.s tmp2.o
	echo 'qemu-riscv64 -L $(RISCV)/sysroot ./tmp; echo $$?' | sh -

tmp-gcc:
	$(CC) -S -O0 -o tmp.s tests/tmp_test.c
	$(CC) -o tmp tmp.s
	echo 'qemu-riscv64 -L $(RISCV)/sysroot ./tmp; echo $$?' | sh -

tmp-gcc-x64:
	gcc -S -O0 -o tmp.s tests/tmp_test.c
	gcc -o tmp tmp.s
	echo './tmp; echo $$?' | sh -

clean:
	rm -rf kiwicc kiwicc-stage* *.o *~ tmp* tests/*~ tests/*.o tests/tmp*

.PHONY: test tmp-kiwicc tmp-as tmp-gcc clean