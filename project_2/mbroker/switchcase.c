#include "switchcase.h"

struct BOX_INFO box_info[SESSIONS];

int n_file_sessions = 0;

int switchcase(char buffer[289]){
    char code = buffer[0];
        switch (code) {
            case (1): // pub ////////////////////////////////////////////////////
                char producer_pipe_name[257] = ""; // char 256 +1
                strncpy(producer_pipe_name, (char*)buffer + 1, 256-1);
                
                // Open producer pipe
                int rxt = open(producer_pipe_name, O_RDONLY);
                if (rxt == -1) {
                    close(rxt); // close pipe if error
                    break; 
                }

                char* message = "";
                char array[1026]="";

                while (true) { // Read message
                    ssize_t rett = read(rxt, array, sizeof(message));
                    memcpy(message, array,strlen(array)* sizeof(char));
                    if (rett == 0) { // ret == 0 signals EOF
                        fprintf(stderr, "[INFO]: parent closed the pipe\n");
                        break;
                    } 
                    else if (rett == -1) { // ret == -1 signals error
                        fprintf(stderr, "[ERR]: read failed: %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                }

                char box_name_producer[33] = "";
                strncpy(box_name_producer, buffer + 1+256, 289-256-1);

                // Create a slash to correct box name
                char slash_box_name_producer[40] = "/"; 
                strcat(slash_box_name_producer,box_name_producer);

                int flag = -1; // flag to see if error occurrs
                for(int j = 0; j < n_file_sessions; j++){
                    if (strcmp(box_info[n_file_sessions].box_name,box_name_producer) == 0){
                        flag = 0;
                        break;
                    }
                }
                if (flag == -1) { // error occurred
                    close(rxt); // close the pipe
                    break;
                }
                
                // Open producer file 
                int file_info_producer = tfs_open(slash_box_name_producer, TFS_O_APPEND);
                if (file_info_producer == -1) { // error occurred
                    close(rxt); // close the pipe
                    tfs_close(file_info_producer); // close the file
                    break;
                }
                message++; // message number increases

                // Write message on file
                if (tfs_write(file_info_producer, message, sizeof(message)) == -1) {
                    // error occurred
                    close(rxt); // close the pipe
                    tfs_close(file_info_producer); // close the file
                    break;
                }

                for (int i = 0; i < n_file_sessions; i++){
                    if (strcmp(box_info[i].box_name, box_name_producer) == 0){
                        box_info[i].pub++;
                        break;
                    }
                }

                close(rxt); // close the pipe
                tfs_close(file_info_producer); // close the file
                break;

            case (2): // sub ////////////////////////////////////////////////////
                char sub_pipe_name[257] = ""; // char 256 +1
                strncpy(sub_pipe_name, (char*)buffer + 1, 256-1);

                char box_name_sub[33] = "";
                strncpy(box_name_sub, buffer + 1+256, 289-256-1);

                char slash_box_name_sub[40] = "/";
                strcat(slash_box_name_sub,box_name_sub);
                
                // Create pipe
                if (mkfifo(sub_pipe_name, 0640) != 0) {
                    return -1; // error occurred
                }

                // Open pipe for reading
                int rt = open(sub_pipe_name, O_RDONLY);
                if (rt == -1) { // error occurred
                    close(rt); // close pipe
                    break;
                }
                
                int flag2 = -1; // flag to see if error occurrs
                for(int j = 0; j < n_file_sessions; j++) {
                    if (strcmp(box_info[n_file_sessions].box_name,box_name_sub) == 0) {
                        flag2 = 0;
                        break;
                    }
                }

                if (flag2 == -1) { // error occurred
                    close(rt); // close pipe
                    break;
                }
                
                // Open sub info file 
                int file_info_sub = tfs_open(slash_box_name_sub, 0);
                if (file_info_sub == -1) { // error occured
                    close(rt); // close pipe
                    tfs_close(file_info_sub); // close file
                    break;
                }
                
                // Read messages
                char message3[1026] = "";
                if (tfs_read(file_info_sub, message3, 1026) == -1) { // error occurred
                    close(rt); // close pipe
                    tfs_close(file_info_sub); // close file
                    break;
                }
                
                char sub_message[1026] = "";
                memset(sub_message, 0, sizeof(sub_message));
                sub_message[0] = 10;
                
                memcpy(sub_message+1, message3, strlen(message3));

                if (write(rt, sub_message, strlen(sub_message)) == -1){ // error occurred
                    close(rt); // close pipe
                    tfs_close(file_info_sub); // close file
                    break;
                }
                close(rt); // close pipe
                tfs_close(file_info_sub); // close file
                break;
            
            case (3): // box create ////////////////////////////////////////////////////
                char final_message[1+1+1025] = "";
                memset(final_message, 0, sizeof(final_message));
                final_message[0] = 4;
                final_message[1] = -1; // flag to see if error occurrs
                char message_manager[1025];
                char manager_pipe_name[257];

                strncpy(manager_pipe_name,(char*) buffer + 1, 256-1);
                char box_name_manager[33];
                strncpy(box_name_manager, buffer + 1+256, 289-256-1);
                
                // Create pipe
                if (mkfifo(manager_pipe_name, 0640) != 0) {
                    return -1; // error occurred
                }
                
                // Open manager pipe
                int manager_open = open(manager_pipe_name, O_WRONLY);
                if (manager_open == -1) {
                    close(manager_open); // close pipe if error
                    break; 
                }

                char slash_box_name_manager[40] = "/";
                strcat(slash_box_name_manager, box_name_manager);
                int flagg = -1; // another flag to see if error occurrs

                for(int j = 0; j < n_file_sessions; j++) {
                    if (strcmp(box_info[n_file_sessions].box_name,box_name_manager) == 0) {
                        flagg = 0;
                        break;
                    }
                }

                if (flagg == 0) {
                    strcpy(message_manager, "File already exists");
                    memcpy(final_message+2, message_manager, sizeof(message_manager));
                    if (write(manager_open, final_message, strlen(final_message)) == -1){
                        close(manager_open); // close pipe
                        break;
                    }
                    close(manager_open); // close pipe
                    break;
                }
                
                // Open manager info file
                int file_info_manager = tfs_open(slash_box_name_manager, TFS_O_CREAT);
                if (file_info_manager == -1) { // error occurred while opening file
                    strcpy(message_manager,"File cant be opened");
                    memcpy(final_message+2, message_manager, sizeof(message_manager));
                    if (write(manager_open, final_message, strlen(final_message)) == -1) {
                        close(manager_open); // close pipe
                        tfs_close(file_info_manager); // close file
                        break;
                    }
                    close(manager_open); // close pipe
                    tfs_close(file_info_manager); // close file
                    break;
                }
                
                if (n_file_sessions != SESSIONS) { // if has not reached max number of sessions
                    strcpy(box_info[n_file_sessions].box_name,box_name_manager);
                    n_file_sessions++; // increase number of sessions
                }

                else { // has reached max number of sessions
                    strcpy(message_manager, "Too many sessions");
                    memcpy(final_message+2, message_manager, sizeof(message_manager));
                    if (write(manager_open, final_message, strlen(final_message)) == -1) {
                        close(manager_open); // close pipe
                        tfs_close(file_info_manager); // close file
                        break;
                    }
                }

                strcpy(message_manager, "OK");
                final_message[1] = 0; // no error occurred
                memcpy(final_message+2, message_manager, sizeof(message_manager));
                
                // Writes message and ends
                if (write(manager_open, final_message, strlen(final_message)) == -1){
                        close(manager_open); // close pipe
                        tfs_close(file_info_manager); // close file
                        break;
                }
                close(manager_open); // close pipe
                tfs_close(file_info_manager); // close file
                break;
            
            case (5): // box remove ////////////////////////////////////////////////////
                char final_message2[1+1+1025] = "";
                memset(final_message2, 0, sizeof(final_message2));
                final_message2[0] = 6;
                final_message2[1] = -1; // flag to see if error occurrs
                char message_manager2[1025] = "";
                char manager_pipe_name2[257] = "";

                strncpy(manager_pipe_name2,(char*) buffer + 1, 256-1);
                char box_name_manager2[33] = "";
                strncpy(box_name_manager2, buffer + 1+256, 289-256-1);

                // Create pipe
                if (mkfifo(manager_pipe_name2, 0640) != 0) {
                    return -1; // error occurred
                }

                // Open manager pipe
                int manager_open2 = open(manager_pipe_name2, O_WRONLY);
                if (manager_open2 == -1) {
                    close(manager_open2); // close pipe if error
                    break; 
                }

                char slash_box_name_manager2[40] = "/";
                strcat(slash_box_name_manager2, box_name_manager2);
                int flagg2 = -1;
                int delete_zone;
                for (delete_zone = 0; delete_zone < n_file_sessions; delete_zone++) {
                    if (strcmp(box_info[n_file_sessions].box_name,box_name_manager2) == 0){
                        flagg2 = 0;
                        break;
                    }
                }

                if (flagg2 == -1) { // error occurred
                    strcpy(message_manager2, "File doesnt exist");
                    memcpy(final_message2+2, message_manager2, sizeof(message_manager2));
                    if (write(manager_open2, final_message2, strlen(final_message2)) == -1){
                        close(manager_open2); // close pipe
                        break;
                    }
                    close(manager_open2); // close pipe
                    break;
                }
                
                // Delete the file
                int file_info_manager2 = tfs_unlink(slash_box_name_manager2);
                if (file_info_manager2 == -1) { // error occurred
                    strcpy(message_manager2, "File cant be deleted/unlinked");
                    memcpy(final_message2+2, message_manager2, sizeof(message_manager2));
                    if (write(manager_open2, final_message2, strlen(final_message2)) == -1) {
                        close(manager_open2); // close pipe
                        tfs_close(file_info_manager2);  // close file
                        break;
                    }
                    close(manager_open2); // close pipe
                    tfs_close(file_info_manager2);  // close file
                    break;
                }

                strcpy(message_manager2, "OK");
                final_message2[1] = 0; // no error occurred
                memcpy(final_message2+2, message_manager2, sizeof(message_manager2));

                if (write(manager_open2, final_message2, strlen(final_message2)) == -1){
                    close(manager_open2); // close pipe
                    tfs_close(file_info_manager2);  // close file
                    break;
                }

                for (int x = delete_zone; x < n_file_sessions; x++){                 
                    box_info[x] = box_info[x + 1];
                }
                n_file_sessions--; // decrease number of sessions

                close(manager_open2); // close pipe
                tfs_close(file_info_manager2); // close file
                break;
            
            case (7): // box list ////////////////////////////////////////////////////
                char list_message[1+1+32+8+8+8] = "";
                memset(list_message, 0, sizeof(list_message));
                list_message[0] = 8;
                list_message[1] = 0; // flag that is not last
                char list_manager_pipe[257] = "";

                strncpy(list_manager_pipe,(char*) buffer + 1, 256-1);

                // Create pipe
                if (mkfifo(list_manager_pipe, 0640) != 0) {
                    return -1; // error occurred
                }

                // Open pipe
                int list_manager_open = open(list_manager_pipe, O_WRONLY);
                if (list_manager_open == -1) {
                    close(list_manager_open); // close pipe if error
                    break; 
                }

                if (n_file_sessions == 0) {
                    list_message[0] = 8;
                    list_message[1] = 1;
                    if (write(list_manager_open, list_message, strlen(list_message)) == -1){
                        close(list_manager_open); // close pipe
                        break;
                    }
                    close(list_manager_open); // close pipe
                    break;
                }

                for (int k = 0; k < n_file_sessions; k++) {
                    if (k == n_file_sessions-1) {
                        list_message[1] = 1;
                    }
                    memcpy(list_message+2, box_info[k].box_name, sizeof(box_info[k].box_name));

                    char slash_manager_list[40] = "/";
                    strcat(slash_manager_list, box_info[k].box_name);
                    
                    // Open file
                    int file_list = tfs_open(slash_manager_list, 0);
                    if (file_list == -1) { // error occurred
                        close(list_manager_open); // close pipe
                        tfs_close(file_list); // close file
                        break;
                    }

                    char messageFinal[1025] = "";
                    if (tfs_read(file_list, messageFinal, 1026) == -1) {
                        close(list_manager_open); // close pipe
                        tfs_close(file_list); // close file
                        break;
                    }
                    
                    int box_size = sizeof(messageFinal);
                    list_message[33] = (char)box_size;
                    list_message[41] = (char)box_info[k].sub;
                    list_message[49] = (char)box_info[k].pub;

                    if (write(list_manager_open, list_message, strlen(list_message)) == -1){
                        close(list_manager_open); // close pipe
                        tfs_close(file_list); // close file
                        break;
                    }
                    close(list_manager_open); // close pipe
                    tfs_close(file_list); // close file
                }
                break;
            
            default: // number not available
                return -1;
        }

return 0;
}
