CC=gcc
CCFLAGS=-Wall
CCLIBS=-lpthread

.PHONY: clean run

run: factory.run
	./factory.run

clean:
	rm -f *.run || true

factory.run: factory.c
	$(CC) $(CCFLAGS) $< -o $@ $(CCLIBS)
