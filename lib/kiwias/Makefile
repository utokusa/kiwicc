CC=riscv64-unknown-linux-gnu-gcc
CFLAGS=-std=c11 -g -O0 -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS: .c=.0)

kiwias: $(OBJS) kiwias.h
	$(CC) $(CFLAGS) -o kiwias $(OBJS) $(LDFLAGS)

$(OBJS): kiwias.h

test: kiwias
	./test.sh

clean:
	rm -rf kiwias *.o *~ tmp* tests/*~ tests/*.o tests/tmp*, a.out

.PHONY: test clean
