#include "shell.h"
#include "stdio.h"
#include "syscall.h"

int shell(void* arg) {
    char buffer[100];

    printf("Lancement de super-shell!\n");
    (void) arg;

    while(1) {
        printf("super-shell $ ");
        read_line(buffer, 100);
        printf("%s\n", buffer);
    }

    return 0;
}

/**
 * Reads a line of input from the console into the provided buffer.
 * The function reads characters until a newline or carriage return is encountered,
 * or until the maximum length is reached.
 *
 * char* buffer : the buffer to store the input line.
 * int max_length : the maximum number of characters to read
 * return : the number of characters read
 */
int read_line(char* buffer, int max_length) {
    char c;
    int i = 0;

    while(i < max_length - 1) {
        if (!cons_read(1, &c)) { continue; }
        if (c == '\n' || c == '\r') {
            buffer[i] = '\0';
            break;
        }
        buffer[i++] = c;
    }

    return i;
}
