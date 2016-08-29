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
    char *address;
    char buf[MAX_LEN];
    int num_bytes;
} Server;

void parseargs(int argc, char **argv);
void connect_server();
void handle_input();

Server server = {
    .port = DEFAULT_PORT,
    .address = NULL,
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
            server.address = argv[1];
        case 1:
            break;
        default:
            fprintf(stderr, "usage: %s [SERVER_ADDRESS [PORT]]", argv[0]);
    }

    printf("Server address is %s\n", server.address);
    printf("Server post is %d\n", server.port);
}

void connect_server() {
};

void handle_input() {
    exit(0);
}
