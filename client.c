struct user {
    int id;
    char handle[MAX_HANDLE];
} *userlist;

void parseargs(int argc, char **argv);
void connect_server();
void handle_input();

int server_fd;
int server_port = DEFAULT_PORT;

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
