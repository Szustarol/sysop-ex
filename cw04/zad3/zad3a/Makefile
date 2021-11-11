CC = gcc
CCFLAGS = -Wall

.PHONY: all clean

all: sender.run catcher.run
	@:

clean:
	rm -f *.run || true

sender.run: sender.c commonDefinitions.h
	$(CC) -o $@ $< $(CCFLAGS)

catcher.run: catcher.c commonDefinitions.h
	$(CC) -o $@ $< $(CCFLAGS)