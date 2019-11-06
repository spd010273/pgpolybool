CC		= gcc
CFLAGS	= -I../src/lib/ -Isrc/ -I../src/ -DRBTREE_TEST -g -DDEBUG
SRCS	= $(wildcard *.c ) $(wildcard src/*.c)

all:
	$(CC) $(CFLAGS) -o test_harness $(SRCS)

clean:
	rm -f test_harness *.o src/*.o
