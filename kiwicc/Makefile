CFLAGS=-std=c11 -g -O0 -no-pie -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS: .c=.0)

kiwicc: $(OBJS)
	$(CC) $(CFLAGS) -o kiwicc $(OBJS) $(LDFLAGS)

$(OBJS): kiwicc.h

test: kiwicc
	./kiwicc tests/tests.c > tmp.s
	echo 'int ret_777() { return 777; } int static_fn() { return 5; }' | \
			gcc -xc -c -o tmp2.o -
	gcc -static -o tmp tmp.s tmp2.o
	./tmp

clean:
	rm -rf kiwicc *.o *~ tmp* test/*~ tests/*.o

.PHONY: test clean