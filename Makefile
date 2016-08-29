CFLAGS=-Wall -ggdb

all: server client

server: server.o util.o
client: client.o util.o

server.o: server.c
client.o: client.c
util.o: util.c

clean:
	rm -rf server client *.o *.dSYM
