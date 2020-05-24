CFLAGS=-std=c11 -g -O0 -no-pie -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS: .c=.0)

9cc: $(OBJS)
	$(CC) $(CFLAGS) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean