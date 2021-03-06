#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <arpa/inet.h>

#include "macros.h"
#include "util.h"

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

char buf[MAX_LEN];
int num_bytes;

void parseargs(int argc, char **argv);
void connect_server();
void print_handle_prompt();
void handle_input();
void read_from(int fd, char *buf, int buf_size, int *num_bytes,
        void (*process)(int msglen));
void process_user(int msglen);
void process_server(int msglen);
char *extract_message(char *buf, int *num_bytes_pointer, int msglen);

Server server = {
    .port = DEFAULT_PORT,
    .hostname = DEFAULT_HOST,
    .num_bytes = 0
};

User *userlist = NULL;

int main(int argc, char **argv) {
    parseargs(argc, argv);
    connect_server();
    print_handle_prompt();
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

void print_handle_prompt() {
    printf("Enter your handle: ");
    fflush(stdout);
}

void handle_input() {
    fd_set fdlist;
    int maxfd = server.fd;

    FD_ZERO(&fdlist);
    FD_SET(server.fd, &fdlist);
    FD_SET(0, &fdlist);

    if (select(maxfd+1, &fdlist, NULL, NULL, NULL) < 0) {
        perror("select");
        exit(1);
    }

    if (FD_ISSET(server.fd, &fdlist))
        read_from(server.fd, server.buf, sizeof server.buf, &server.num_bytes,
                process_server);

    if (FD_ISSET(0, &fdlist))
        read_from(0, buf, sizeof buf, &num_bytes, process_user);
}

void process_server(int msglen) {
    char *message = extract_message(server.buf, &server.num_bytes, msglen);
}

void process_user(int msglen) {
    char *message = extract_message(buf, &num_bytes, msglen);
    send_string(server.fd, message);
    printf("Enter a message: ");
    fflush(stdout);
}

void read_from(int fd, char *buf, int buf_size, int *num_bytes,
        void (*process)(int msglen)) {
    socklen_t len = read(fd, buf + *num_bytes, buf_size - *num_bytes);

    switch (len) {
        case -1:
            perror("read");
            exit(1);
        case 0:
            fprintf(stderr, "Message too long or connection closed\n");
            exit(1);
        default:
            *num_bytes += len;
            char *q;

            while ((q = memnewline(buf, *num_bytes)))
                process(q - buf);
    }
}

char *extract_message(char *buf, int *num_bytes, int msglen) {
    char *msg = malloc(msglen + 1);
    memcpy(msg, buf, msglen);
    msg[msglen] = '\0';

    *num_bytes -= msglen;
    char *cursor;
    for (cursor = buf + msglen; *num_bytes > 0 &&
            memnewline(cursor, *num_bytes) == cursor; cursor++)
        (*num_bytes)--;

    memcpy(buf, cursor, *num_bytes);

    return msg;
}
