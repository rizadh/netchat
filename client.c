#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "macros.h"

typedef struct user {
    int id;
    char handle[MAX_HANDLE];
    struct user *next;
} User;

typedef struct server {
    int fd;
    int port;
    char *hostname;
    char buf[MAX_LEN];
    int num_bytes;
} Server;

void parseargs(int argc, char **argv);
void connect_server();
void handle_input();

Server server = {
    .port = DEFAULT_PORT,
    .hostname = DEFAULT_HOST,
    .num_bytes = 0
};

User *userlist = NULL;

int main(int argc, char **argv) {
    parseargs(argc, argv);
    connect_server();
    while (1)
        handle_input();
}

void parseargs(int argc, char **argv) {
    switch (argc) {
        case 3:
            server.port = atoi(argv[2]);
        case 2:
            server.hostname = argv[1];
        case 1:
            break;
        default:
            fprintf(stderr, "usage: %s [HOSTNAME [PORT]]", argv[0]);
    }
}

void connect_server() {
    int err;
    struct addrinfo *res, hints;
    char *port_string;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    asprintf(&port_string, "%d", server.port);

    if ((err = getaddrinfo(server.hostname, port_string, &hints, &res))) {
        fprintf(stderr, "%s\n", gai_strerror(err));
        exit(1);
    }

    if ((server.fd = socket(res->ai_family, res->ai_socktype,
                    res->ai_protocol)) < 0) {
        perror("socket");
        exit(1);
    }

    if (connect(server.fd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        exit(1);
    }
}

void handle_input() {
}
