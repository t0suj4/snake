CC = gcc -m32
CFLAGS = -I. -O2 -Wall -Wextra -pthread -std=c11 -D_POSIX_C_SOURCE=200809L
LIBS = ncurses
LFLAGS= -O2 $(shell pkg-config --libs $(LIBS)) -pthread -std=c11
TDEPS = list.h
TESTS = listtest.o list.o
DEPS = list.h utils.h gameboard.h listtest.h game.h gameobject.h snake.h draw.h
OBJS = snake.o list.o utils.o gameboard.o game.o draw.o gamelogic.o point.o

%.o: %.c $(TDEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: $(OBJS)
	$(CC) -o snake $(OBJS) $(LFLAGS)

test: $(TESTS)
	$(CC) -o listtest $(TESTS) -g -O0 -fsanitize=address
	./listtest

.PHONY: clean
clean:
	-rm snake $(OBJS)
