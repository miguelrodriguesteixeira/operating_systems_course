#include "logging.h"
#include "string.h"
#include "registerclient.h"
#include <unistd.h>

int main(int argc, char **argv) {
    // Right amount of arguments
    if (!((argc == 4) || (argc == 5)) ) { 
        return -1; 
    }
    
    char copy[1+256+32];
    memset(copy,0, sizeof(copy));
    memcpy(copy+1, argv[2], strlen(argv[2]));

    if (strcmp("create", argv[3]) == 0 || strcmp("remove", argv[3]) == 0) {
        memcpy(copy+1+256, argv[4], strlen(argv[4]));
        if (strcmp("create", argv[3]) == 0) { // box creation
            copy[0] = 3; // code = 3
        }
        if (strcmp("remove", argv[3]) == 0) { // box elimination
            copy[0] = 5; // code = 5
        }
    }
    
    if(strcmp("list", argv[3]) == 0) { // list boxes
        copy[0] = 7; // code = 7
    }

    if (copy[0] == 0){
        return -1;
    }
    registerclient(argc, argv, copy);

    // Open pipe for reading
    int rx = open(argv[2], O_RDONLY);
    if (rx == -1) { // error occurred
        close(rx); // close pipe
        return -1; 
    }
    

    char initial_message[1026];
    if (read(rx,initial_message, 1026) == -1) { // error occurred
        close(rx); // close pipe
        return -1; 
    }

    char message[1025];
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wstringop-truncation"
    strncpy(message, initial_message+2, 1026-1);
    #pragma GCC diagnostic pop

    if (initial_message[0] != 8){
        if (initial_message[1] == -1) { // error occurred
            fprintf(stdout, "ERROR %s\n", message);
        }
        else { // succeeded
            fprintf(stdout, "OK\n");
        }
    }

    else {
        if (initial_message[2] == 0) { // no boxes found
            fprintf(stdout, "NO BOXES FOUND\n");
        }
        else {
            char box_name[33];
            size_t box_size, n_publishers, n_subscribers;
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wstringop-truncation"
            strncpy(box_name, message+1, 33-1);
            #pragma GCC diagnostic pop
            box_size = (size_t)message[32];
            n_publishers = (size_t)message[40];
            n_subscribers = (size_t)message[48]; // box list
            fprintf(stdout, "%s %zu %zu %zu\n", box_name, box_size, n_publishers, n_subscribers);
        }
    }
    
    close(rx); // close pipe
    return 0;
}
