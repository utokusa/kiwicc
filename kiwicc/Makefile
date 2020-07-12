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

tmp-kiwicc: kiwicc
	./kiwicc tests/tmp_test.c > tmp.s
	gcc -static -o tmp tmp.s
	echo './tmp ; echo $$?' | sh -

tmp-as:
	gcc -static -o tmp tmp.s
	echo './tmp ; echo $$?' | sh -

tmp-gcc:
	gcc -static -o tmp tests/tmp_test.c
	echo './tmp ; echo $$?' | sh -

clean:
	rm -rf kiwicc *.o *~ tmp* tests/*~ tests/*.o tests/tmp*

.PHONY: test tmp-kiwicc tmp-as tmp-gcc clean