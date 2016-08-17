#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

struct user {
    int listenfd;
    int user_id;
    char user_handle[20];
    char buf[80];
    int bytes_in_buf;
    struct client *next;
} *clientlist = NULL;

struct message {
    struct user *author;
    char text[80];
    struct message *next;
} *messagelist = NULL;

void parseargs(int argc, char**argv);
void create_socket();

int listen_fd;
int listen_port = DEFAULT_PORT;

int main(int argc, char **argv) {
    parseargs(argc, argv);
    create_socket();
    printf("All good with port %d\n", listen_port);
}

void parseargs(int argc, char **argv) {
    if (argc == 2) {
        listen_port = atoi(argv[1]);
    } else if (argc > 2) {
        fprintf(stderr, "usage: %s [-p LISTEN_PORT]\n", argv[0]);
    }
}

void create_socket() {
    struct sockaddr_in r;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&r, '\0', sizeof r);
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(listen_port);

    if (bind(listen_fd, (struct sockaddr *)&r, sizeof r) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(listen_fd, 10)) {
        perror("listen");
        exit(1);
    }
}
