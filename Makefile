CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

ccc: $(OBJS)
	$(CC) -o ccc $(OBJS) $(LDFLAGS)

$(OBJS): ccc.h

test: ccc
	./test.sh

clean:
	rm -f ccc *.o *~
