#include "registerclient.h"
#include "logging.h"
#include <unistd.h>

int registerclient(int argc, char**argv, char*copy){
    // Right amount of arguments
    if ((argc > 6) || (argc < 1)) {
        return -1; 
    }

    // Open pipe
    int fd = open(argv[1], O_WRONLY);
    if (fd == -1) { // error occurred
        printf("%s", "not opened");
        close(fd); // close pipe
        return -1;
    }

    if (write(fd, copy, 289) == -1) {
        close(fd);  // close pipe
        return -1; // error occurred
        
    }

    close(fd); // close pipe
    return 0;
}
