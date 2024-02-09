#include "logging.h"
#include "string.h"
#include "registerclient.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int rx = 0;
int n_messages = 0;

void print_message() {
    close(rx); // close pipe
    printf("%i", n_messages);
    return;
}

int main(int argc, char **argv) {
    if (signal(SIGINT, print_message) == SIG_ERR) {
        exit(EXIT_FAILURE); // error occurred
    }

    // Right amount of arguments
    if (argc != 4) {
        return -1;
    }

    if (strcmp("./sub", argv[0]) != 0){
        return -1;
    }

    char copy[1+256+32];
    memset(copy, 0, sizeof(copy)); // fill with zeros
    copy[0] = 2; // code = 2 

    memcpy(copy+1, argv[2], strlen(argv[2]));
    memcpy(copy+1+256, argv[3], strlen(argv[3]));
    registerclient(argc, argv, copy);

    // Open pipe for reading
    rx = open(argv[2], O_RDONLY);
    if (rx == -1) { // error occurred while opening pipe
        close(rx);
        return -1;
    }

    while (true) {
        char messages[1026];
        if (read(rx, messages, 1026) == -1){
            return -1;
        }

        char message[1025];
        strncpy(message, messages+1, 1026-1);
        int inicial = 0;

        for (int i = 0; i < 1025; i++){
            if (message[i] == 0 && message[i] != 0) {
                char print_message[1024];
                int size = i-inicial+1;
                strncpy(print_message, message+inicial, (size_t) size);
                fprintf(stdout, "%s\n", print_message);
                n_messages++; // increase the number of messages
                inicial = i+1;
            }
        }
    }
    close(rx);
    return 0;
}

