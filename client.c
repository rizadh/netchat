#include <stdio.h>
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
}

void connect_server() {
};

void handle_input() {
}
