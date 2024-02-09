#include "logging.h"
#include "string.h"
#include "registerclient.h"
#include <unistd.h>

void send_msg(int tx, char const *str) {
    size_t len = sizeof(str);
    size_t written = 0;

    while (written < len) {
        ssize_t ret = write(tx, str + written, len - written);
        if (ret < 0) {
            fprintf(stderr, "[ERR]: write failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE); // error occurred
        }
        written += (size_t)ret;
    }
}

int main(int argc, char **argv) {
    // Right amount of arguments
    if (argc != 4) {
        return -1;
    }

    if (strcmp("./pub", argv[0]) != 0){
        return -1;
    }

    char copy[1+256+32];
    memset(copy,0,sizeof(copy));
    copy[0] = 1; // code = 1 

    memcpy(copy+1, argv[2], strlen(argv[2]));
    memcpy(copy+1+256, argv[3], strlen(argv[3]));

    registerclient(argc, argv, copy);

    // Create pipe
    if (mkfifo(argv[2], 0640) != 0) {
        return -1; // error occurred
    }
    
    // Open pipe
    int rx = open(argv[2], O_WRONLY);
    if (rx == -1) { // error occurred
        close(rx); // close pipe
        return -1; 
    }

    while (true) {
        char inicial_message[1024];
        int j = scanf("%1023s", inicial_message);
        j++;

        char message[1025];
        memset(message, 0, sizeof(message));
        message[0] = 9;
        memcpy(message+1, inicial_message, strlen(inicial_message));
        
        send_msg(rx,message);
    }

    close(rx); // close pipe
    return 0;
}
