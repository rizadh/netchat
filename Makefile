all: server client

server: server.c
	gcc -Wall -DDEFAULT_PORT=1024 -o server server.c
client: client.c
	gcc -Wall -o client client.c

clean:
	rm -f server client
