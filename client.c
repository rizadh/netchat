#include <stdio.h>
#include <stdlib.h>
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
    .hostname = NULL,
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

    printf("Server hostname is %s\n", server.hostname);
    printf("Server post is %d\n", server.port);
}

void connect_server() {
};

void handle_input() {
    exit(0);
}
