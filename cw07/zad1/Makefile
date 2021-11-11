CC=gcc
CCFLAGS=-Wall

pizzaHouse.run: pizzaHouse.c defines.h
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f *.run || true