#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>

struct user {
    int fd;
    int id;
    char handle[20];
    char buf[80];
    int num_bytes;
    struct user *next;
} *userlist = NULL;

struct message {
    struct user *author;
    char text[80];
    struct message *next;
} *messagelist = NULL;

void parseargs(int argc, char**argv);
void create_socket();
void handle_users();
void add_user(int fd);
void read_user(struct user *p);
void process_user(struct user *p, int msglen);
void declare_user(struct user *p);
void disconnect_user(struct user *p);
char *memnewline(char *p, int size);

int socket_fd;
int listen_port = DEFAULT_PORT;

int main(int argc, char **argv) {
    parseargs(argc, argv);
    create_socket();
    while (1) {
        handle_users();
    }
}

void parseargs(int argc, char **argv) {
    if (argc == 2) {
        listen_port = atoi(argv[1]);
    } else if (argc > 2) {
        fprintf(stderr, "usage: %s [PORT]\n", argv[0]);
    }
}

void create_socket() {
    struct sockaddr_in r;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&r, '\0', sizeof r);
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(listen_port);

    if (bind(socket_fd, (struct sockaddr *)&r, sizeof r) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(socket_fd, 10)) {
        perror("listen");
        exit(1);
    }
}

void handle_users() {
    struct user *curr_user;
    fd_set fdlist;
    int maxfd = socket_fd;

    FD_ZERO(&fdlist);
    FD_SET(socket_fd, &fdlist);
    for (curr_user = userlist; curr_user; curr_user = curr_user->next) {
        if (curr_user->fd == -1) continue;
        FD_SET(curr_user->fd, &fdlist);
        if (curr_user->fd > maxfd) {
            maxfd = curr_user->fd;
        }
    }

    if (select(maxfd+1, &fdlist, NULL, NULL, NULL) < 0) {
        perror("select");
        exit(1);
    }

    if (FD_ISSET(socket_fd, &fdlist)) {
        int newfd;
        struct sockaddr client_addr;
        socklen_t len = sizeof client_addr;
        if ((newfd = accept(socket_fd, &client_addr, &len)) < 0) {
            perror("accept");
            exit(1);
        }
        add_user(newfd);
    } else for (curr_user = userlist; curr_user; curr_user = curr_user->next) {
        if (FD_ISSET(curr_user->fd, &fdlist)) {
            read_user(curr_user);
            return;
        }
    }
}

void add_user(int fd) {
    static int lastid = 0;
    struct user *newuser = malloc(sizeof (struct user));

    newuser->fd = fd;
    newuser->id = lastid++;
    newuser->handle[0] = '\0';
    newuser->num_bytes = 0;
    newuser->next = userlist;
    userlist = newuser;
}

void declare_user(struct user *p) {
    // TODO
}

void read_user(struct user *p) {
    socklen_t len =
        read(p->fd, p->buf + p->num_bytes, sizeof p->buf - p->num_bytes);

    if (len == -1) {
        perror("read");
        exit(1);
    } else if (len == 0) {
        if (p->num_bytes == sizeof p->buf) {
            process_user(p, p->num_bytes);
        } else {
            disconnect_user(p);
        }
    } else {
        char *q;
        p->num_bytes += len;

        if ((q = memnewline(p->buf, p->num_bytes))) {
            process_user(p, q - p->buf);
        }
    }
}

char *memnewline(char *p, int size) {
    for (; size > 0; size--, p++)
        if (*p == '\r' || *p == '\n') return(p);
    return(NULL);
}

void process_user(struct user *p, int msglen) {
    // Temporarily output to stdout
    write(1, p->buf, msglen);
    write(1, "\n", 1);
}

void disconnect_user(struct user *p) {
    close(p->fd);
    p->fd = -1;
}
