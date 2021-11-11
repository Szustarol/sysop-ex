CC = gcc
CCFLAGS = -Wall
CCLIBS = -lpthread -lm

invert.run: invert.c
	$(CC) $(CCFLAGS) $< -o $@ $(CCLIBS)

clean:
	rm -f *.run || true