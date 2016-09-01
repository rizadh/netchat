#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "macros.h"

char *memnewline(char *p, int size) {
    for (; size > 0; size--, p++)
        if (*p == '\r' || *p == '\n') return(p);
    return(NULL);
}

void send_string(int fd, char *str) {
    int len = strlen(str);
    if (len + 2 > MAX_LEN) {
        fprintf(stderr, "Tried to send a message that was too long");
        return;
    }

    if (write(fd, str, len) != len || write(fd, "\r\n", 2) != 2) {
        perror("write");
        exit(1);
    };
}

int valid_string(char *str) {
    int i;
    for (i = strlen(str); i > 0; i--)
        if (!valid_character(str[i-1]))
            return 0;

    return 1;
}

int valid_character(char c) {
    char *lower_case = "abcdefghijklmnopqrstuvwxyz";
    char *upper_case = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char *numeric = "0123456789";
    char *symbols = " -:()[]{}!@#$%^&*|\\<>,.?/~`";
    char *valid_chars[] = {lower_case, upper_case, numeric, symbols};
    int i, j;

    for (i = 0; i < sizeof valid_chars / sizeof valid_chars[0]; i++)
        for (j = 0; j < strlen(valid_chars[i]);j++)
            if (c == valid_chars[i][j])
                return 1;

    return 0;
}
