#include "mbroker.h"

int n_sessions = 0;

int main(int argc, char **argv) {
    
    // Right amount of arguments
    if (argc != 3) {
        return -1; 
    }

    // Remove pipe if it does not exist
    if (unlink(argv[1]) != 0 && errno != ENOENT) {
        return -1; // error occurred
    }

    // Create pipe
    if (mkfifo(argv[1], 0640) != 0) {
        return -1; // error occurred
    }

    // Open pipe for reading
    // This waits for someone to open it for writting
    int rx = open(argv[1], O_RDONLY);
    if (rx == -1) {
        return -1; // error occurred
    }
    
    // Initialize TFS
    if (tfs_init(NULL) == -1) {
        return -1; // error occurred
    }

    // Create a queue for requests
    char* max_sessions = argv[2];
    pc_queue_t *queue = NULL;
    pcq_create(queue, (size_t)max_sessions);

    while (true) { // Read requests
        char buffer[BUFFER_SIZE];
        ssize_t ret = read(rx, buffer, BUFFER_SIZE - 1);
        
        if (ret == 0) {  // ret == 0 indicates EOF
            close(rx); // pipe closed
            return -1; // error occurred

        } 
        else if (ret == -1) { // ret == -1 indicates error
            close(rx); // pipe closed
            return -1; // error occurred
        }

        pcq_enqueue(queue, buffer); // Enqueue the request
        n_sessions++;
        switchcase(buffer);
        pcq_dequeue(queue); // Dequeue the request
    }
    close(rx);
    return 0;
}
