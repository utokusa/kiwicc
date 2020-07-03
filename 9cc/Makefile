CFLAGS=-std=c11 -g -O0 -no-pie -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS: .c=.0)

9cc: $(OBJS)
	$(CC) $(CFLAGS) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./9cc tests/tests.c > tmp.s
	echo 'int ret_777() { return 777; } int static_fn() { return 5; }' | \
			gcc -xc -c -o tmp2.o -
	gcc -static -o tmp tmp.s tmp2.o
	./tmp

clean:
	rm -rf 9cc *.o *~ tmp* test/*~ tests/*.o

.PHONY: test clean