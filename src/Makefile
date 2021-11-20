CC=riscv64-unknown-linux-gnu-gcc
CFLAGS=-std=c11 -g -O0 -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS: .c=.0)

kiwicc: $(OBJS) kiwicc.h
	$(CC) $(CFLAGS) -o kiwicc $(OBJS) $(LDFLAGS)

$(OBJS): kiwicc.h

kiwicc-stage2: kiwicc $(SRCS) kiwicc.h self.sh
	./self.sh tmp-stage2 ./kiwicc kiwicc-stage2

kiwicc-stage3: kiwicc-stage2
	./self.sh tmp-stage3 ./kiwicc-stage2 kiwicc-stage3

# Install kiwicc by running `sudo -E make install`
install: kiwicc-stage2
	cp kiwicc-stage2 $(RISCV)/bin/kiwicc
	mkdir -p $(RISCV)/lib/kiwicc
	cp -r ./include $(RISCV)/lib/kiwicc

# Unstall kiwicc by running `sudo -E make uninstall`
uninstall:
	rm $(RISCV)/bin/kiwicc
	rm -r $(RISCV)/lib/kiwicc/include

test: kiwicc
	qemu-riscv64 -L $(RISCV)/sysroot ./kiwicc tests/tests.c -I./tests/test_include -I./tests -o tmp.o
	$(CC) -xc -c -o tmp2.o tests/extern.c
	$(CC) -o tmp tmp.o tmp2.o
	qemu-riscv64 -L $(RISCV)/sysroot ./tmp

test-nopic: kiwicc
	qemu-riscv64 -L $(RISCV)/sysroot ./kiwicc tests/tests.c -I./tests/test_include -I./tests -fno-pic -o tmp.o
	$(CC) -xc -c -o tmp2.o tests/extern.c
	$(CC) -o tmp tmp.o tmp2.o
	qemu-riscv64 -L $(RISCV)/sysroot ./tmp

test-stage2: kiwicc-stage2
	qemu-riscv64 -L $(RISCV)/sysroot ./kiwicc-stage2 tests/tests.c -I./tests/test_include -I./tests -o tmp.o
	$(CC) -xc -c -o tmp2.o tests/extern.c
	$(CC) -static -fno-common -o tmp tmp.o tmp2.o
	qemu-riscv64 -L $(RISCV)/sysroot ./tmp

test-stage3: kiwicc-stage3
	diff kiwicc-stage2 kiwicc-stage3

test-all: test test-nopic test-stage2 test-stage3

test-gcc:
	$(CC) tests/tests.c -o tmp.s
	$(CC) -xc -c -o tmp2.o tests/extern.c
	$(CC) -o tmp tmp.s tmp2.o
	qemu-riscv64 -L $(RISCV)/sysroot ./tmp

tmp-kiwicc: kiwicc
	qemu-riscv64 -L $(RISCV)/sysroot ./kiwicc tests/tmp_test.c -I./tests -o tmp.o
	$(CC) -o tmp tmp.o
	echo 'qemu-riscv64 -L $(RISCV)/sysroot ./tmp; echo $$?' | sh -

tmp-kiwicc-s: kiwicc
	qemu-riscv64 -L ${RISCV}/sysroot ./kiwicc tests/tmp_test.c -I./tests -S -o tmp.s

tmp-as:
	$(CC) -static -o tmp tmp.s tmp2.o
	echo 'qemu-riscv64 -L $(RISCV)/sysroot ./tmp; echo $$?' | sh -

tmp-gcc:
	$(CC) -S -g -O0 -fpic -o tmp.s tests/tmp_test.c
	$(CC) -o tmp tmp.s
	echo 'qemu-riscv64 -L $(RISCV)/sysroot ./tmp; echo $$?' | sh -

tmp-gcc-x64:
	gcc -S -O0 -g -o tmp.s tests/tmp_test.c
	gcc -o tmp tmp.s
	echo './tmp; echo $$?' | sh -

clean:
	rm -rf kiwicc kiwicc-stage* *.o *~ tmp* tests/*~ tests/*.o tests/tmp*

.PHONY: test tmp-kiwicc tmp-as tmp-gcc clean install uninstall
