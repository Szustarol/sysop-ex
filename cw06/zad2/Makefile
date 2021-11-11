CC=gcc
CCFLAGS=-Wall
LIBS=-lrt

.PHONY: clean both

both: server.run client.run
	@:

server.run: server.c defines.h server_defines.h
	$(CC) $(CCFLAGS) $< -o $@ $(LIBS)

client.run: client.c defines.h
	$(CC) $(CCFLAGS) $< -o $@ $(LIBS)

clean:
	rm -f *.run || true