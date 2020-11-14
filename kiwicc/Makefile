CFLAGS=-std=c11 -g -O0 -no-pie -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS: .c=.0)

kiwicc: $(OBJS)
	$(CC) $(CFLAGS) -o kiwicc $(OBJS) $(LDFLAGS)

$(OBJS): kiwicc.h

kiwicc-stage2: kiwicc $(SRCS) kiwicc.h self.sh
	./self.sh tmp-stage2 ./kiwicc kiwicc-stage2

kiwicc-stage3: kiwicc-stage2
	./self.sh tmp-stage3 ./kiwicc-stage2 kiwicc-stage3

test: kiwicc
	./kiwicc tests/tests.c > tmp.s
	gcc -xc -c -o tmp2.o tests/extern.c
	gcc -static -fno-common -o tmp tmp.s tmp2.o
	./tmp

test-stage2: kiwicc-stage2
	./kiwicc-stage2 tests/tests.c > tmp.s
	gcc -xc -c -o tmp2.o tests/extern.c
	gcc -static -fno-common -o tmp tmp.s tmp2.o
	./tmp

test-stage3: kiwicc-stage3
	diff kiwicc-stage2 kiwicc-stage3

test-all: test test-stage2 test-stage3

test-gcc:
	gcc tests/tests.c > tmp.s
	gcc -xc -c -o tmp2.o tests/extern.c
	gcc -static -fno-common -o tmp tmp.s tmp2.o
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
	rm -rf kiwicc kiwicc-stage* *.o *~ tmp* tests/*~ tests/*.o tests/tmp*

.PHONY: test tmp-kiwicc tmp-as tmp-gcc clean