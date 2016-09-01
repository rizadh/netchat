#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/uio.h>
#include "util.h"
#include "macros.h"

struct user {
    int fd;
    int id;
    char handle[MAX_HANDLE];
    char buf[MAX_LEN];
    int num_bytes;
    int buffer_overflowed;
    struct user *next;
} *userlist = NULL;

struct message {
    struct user *author;
    char text[MAX_LEN];
    struct message *next;
} *messagelist = NULL;

void parseargs(int argc, char**argv);
void create_socket();
void handle_users();
void add_user(int fd);
void read_user(struct user *p);
void process_user(struct user *p, int msglen);
void declare_user(struct user *p);
void update_user(struct user *p);
void disconnect_user(struct user *p);
void handle_message(struct user *p, char *msg);
void add_message(struct user *p, char *msg);
void set_handle(struct user *p, char *msg);
void send_to_all(char *msg, struct user *exception);
char *get_user_declaration(struct user *p);
int USER_ISLIVE(struct user *p);

int socket_fd;
int listen_port = DEFAULT_PORT;
char protocol_string[MAX_LEN];

int main(int argc, char **argv) {
    parseargs(argc, argv);
    create_socket();
    sprintf(protocol_string, "netchat %d", PROTOCOL_VERSION);
    while (1) {
        handle_users();
    }
}

void parseargs(int argc, char **argv) {
    if (argc == 2)
        listen_port = atoi(argv[1]);
    else if (argc > 2) {
        fprintf(stderr, "usage: %s [PORT]\n", argv[0]);
        exit(1);
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
        if (curr_user->fd > 0)
            FD_SET(curr_user->fd, &fdlist);
        if (curr_user->fd > maxfd)
            maxfd = curr_user->fd;
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
        if (curr_user->fd > 0 && FD_ISSET(curr_user->fd, &fdlist)) {
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

    send_string(fd, protocol_string);
    printf("User %d connected at fd %d\n", newuser->id, newuser->fd);
}

void declare_user(struct user *p) {
    char *declaration = get_user_declaration(p);

    send_to_all(declaration, p);

    free(declaration);
}

void update_user(struct user *p) {
    struct user *curruser;
    struct message *currmsg;
    char *str;

    for (curruser = userlist; curruser; curruser = curruser->next) {
        if (curruser->handle[0] != '\0' && curruser != p) {
            asprintf(&str, "user %d %s", curruser->id, curruser->handle);
            send_string(p->fd, str);
        }
    }

    for (currmsg = messagelist; currmsg; currmsg = currmsg->next) {
        asprintf(&str, "said %d %s", currmsg->author->id, currmsg->text);
        send_string(p->fd, str);
    }
}

void send_to_all(char *msg, struct user *exception) {
    struct user *p;
    for (p = userlist; p; p = p->next)
        if (USER_ISLIVE(p) && exception != p)
            send_string(p->fd, msg);
}

char *get_user_declaration(struct user *p) {
    char *str;
    asprintf(&str, "user %d %s", p->id, p->handle);

    return str;
}

void read_user(struct user *p) {
    socklen_t len =
        read(p->fd, p->buf + p->num_bytes, sizeof p->buf - p->num_bytes);

    if (len == -1) {
        perror("read");
        exit(1);
    } else if (len == 0) {
        if (p->num_bytes == sizeof p->buf) {
            p->num_bytes = 0;
            p->buffer_overflowed = 1;
        } else {
            disconnect_user(p);
        }
    } else {
        char *q;
        p->num_bytes += len;

        while ((q = memnewline(p->buf, p->num_bytes)))
            process_user(p, q - p->buf);
    }
}

void process_user(struct user *p, int msglen) {
    char *rxmsg = malloc(msglen + 1);
    memcpy(rxmsg, p->buf, msglen);
    rxmsg[msglen] = '\0';

    p->num_bytes -= msglen;
    char *cursor;
    for (cursor = p->buf + msglen; p->num_bytes > 0 && (*cursor == '\r' ||
            *cursor == '\n'); cursor++)
        p->num_bytes--;

    memcpy(p->buf, cursor, p->num_bytes);

    if (p->buffer_overflowed) {
        p->buffer_overflowed = 0;
        fprintf(stderr, "User %d sent too long a message, was ignored\n",
                p->id);
    } else {
        handle_message(p, rxmsg);
    }
}

void handle_message(struct user *p, char *msg) {
    if (!valid_string(msg)) {
        fprintf(stderr, "User %d sent an illegal character\n", p->id);
        disconnect_user(p);
        return;
    }

    if (p->handle[0] == '\0')
        set_handle(p, msg);
    else
        add_message(p, msg);

    free(msg);
}

void set_handle(struct user *p, char *msg) {
    int len;
    if ((len = strlen(msg)) > MAX_HANDLE) {
        fprintf(stderr, "Handle too long: user %d\n", p->id);
        return;
    }

    memcpy(p->handle, msg, len);
    p->handle[len] = '\0';

    update_user(p);
    declare_user(p);

    printf("User %d handle was set to %s\n", p->id, p->handle);
}

int USER_ISLIVE(struct user *p) {
    return p->fd > 0 && p->handle[0] != '\0';
}

void add_message(struct user *p, char *msg) {
    char *str;

    struct message *newmsg = malloc(sizeof (struct message));
    newmsg->author = p;
    newmsg->next = NULL;
    memcpy(newmsg->text, msg, strlen(msg) + 1);
    struct message **pp;
    for (pp = &messagelist; *pp; pp = &(*pp)->next) ;

    *pp = newmsg;


    asprintf(&str, "said %d %s", p->id, msg);

    send_to_all(str, p);

    free(str);

    printf("User %d said %s\n", p->id, msg);
}

void disconnect_user(struct user *p) {
    if (p->fd != -1) {
        close(p->fd);
        printf("User %d was disconnected\n", p->id);
    } else 
        fprintf(stderr, "Tried to disconnect already disconnected user %d\n",
                p->id);

    p->fd = -1;
}
