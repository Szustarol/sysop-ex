CC = gcc
CCFLAGS = -Wall -g3
CCLIBS = -lpthread
TERMINAL = gnome-terminal

.PHONY: runserver clean runclient test_setup

test_setup: server.run client.run
	$(TERMINAL) -- ./server.run 33333 ticTacToeServerSock
	sleep 1
	$(TERMINAL) -- ./client.run client1 WEB 127.0.0.1:33333
	sleep 1
	$(TERMINAL) -- ./client.run client2 WEB 127.0.0.1:33333

runserver: server.run
	./server.run 33333 ticTacToeServerSock

runclient: client.run
	./client.run client1 WEB 127.0.0.1:33333

client.run: client.c defines.h
	$(CC) $(CCFLAGS) $< -o $@ $(CCLIBS)

server.run: server.c serverClients.h defines.h
	$(CC) $(CCFLAGS) $< -o $@ $(CCLIBS)

clean:
	rm -f *.run || true
	rm -f ticTacToeServerSock || true