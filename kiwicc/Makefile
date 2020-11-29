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

test: kiwicc
	./kiwicc tests/tests.c -I./tests -o tmp.s
	gcc -xc -c -o tmp2.o tests/extern.c
	gcc -o tmp tmp.s tmp2.o
	./tmp

test-nopic: kiwicc
	./kiwicc tests/tests.c -I./tests  -fno-pic -o tmp.s
	gcc -xc -c -o tmp2.o tests/extern.c
	gcc -static -o tmp tmp.s tmp2.o
	./tmp

test-stage2: kiwicc-stage2
	./kiwicc-stage2 tests/tests.c -I./tests  -o tmp.s
	gcc -xc -c -o tmp2.o tests/extern.c
	gcc -static -fno-common -o tmp tmp.s tmp2.o
	./tmp

test-stage3: kiwicc-stage3
	diff kiwicc-stage2 kiwicc-stage3

test-all: test test-nopic test-stage2 test-stage3

test-gcc:
	gcc tests/tests.c -o tmp.s
	gcc -xc -c -o tmp2.o tests/extern.c
	gcc -o tmp tmp.s tmp2.o
	./tmp

tmp-kiwicc: kiwicc
	./kiwicc tests/tmp_test.c -o tmp.s
	gcc -o tmp tmp.s
	echo './tmp ; echo $$?' | sh -

tmp-as:
	gcc -static -o tmp tmp.s
	echo './tmp ; echo $$?' | sh -

tmp-gcc:
	gcc -o tmp tests/tmp_test.c
	echo './tmp ; echo $$?' | sh -

clean:
	rm -rf kiwicc kiwicc-stage* *.o *~ tmp* tests/*~ tests/*.o tests/tmp*

.PHONY: test tmp-kiwicc tmp-as tmp-gcc clean