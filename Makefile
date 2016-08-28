CFLAGS=-Wall -DPROTOCOL_VERSION=1 -DMAX_HANDLE=20 -DMAX_LEN=80 -DDEFAULT_PORT=1024 -ggdb

all: server client

server: server.o util.o
client: client.o util.o

server.o: server.c
client.o: client.c
util.o: util.c

clean:
	rm -rf server client *.o *.dSYM
