CC=gcc
CCFLAGS=-Wall -g3
LIBS=-lrt -lpthread

pizzaHouse.run: pizzaHouse.c defines.h
	$(CC) $(CCFLAGS) $< -o $@ $(LIBS)

clean:
	rm -f *.run || true