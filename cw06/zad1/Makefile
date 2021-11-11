CC=gcc
CCFLAGS=-Wall

.PHONY: clean

server.run: server.c defines.h server-defines.h
	$(CC) $(CCFLAGS) $< -o $@

client.run: client.c defines.h
	$(CC) $(CCFLAGS) $< -o $@

clean:
	rm -f *.run || true