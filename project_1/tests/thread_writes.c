#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "fs/operations.h"
#include <assert.h>
#include <stdio.h>

#define N (10)
char const target_path[] = "/f1";
char *str1 = "AAA";
char *str2 = "BBB";
char *str3 = "CCC";

typedef struct {
    char const *target_path;
} alice_args_t;

typedef struct {
    char const *target_path;
} bob_args_t;

typedef struct {
    char const *target_path;
} fred_args_t;

void *alice_thread_fn() {
    for (size_t i = 0; i < N; i++) {
        int fhandle = tfs_open(target_path, TFS_O_APPEND);
        if (fhandle != -1) {
            tfs_write(fhandle, str1, strlen(str1));
            tfs_close(fhandle);
        }
    }
    return NULL;
}

void *bob_thread_fn() {
    for (size_t i = 0; i < N; i++) {
        int fhandle = tfs_open(target_path, TFS_O_APPEND);
        if (fhandle != -1) {
            tfs_write(fhandle, str2, strlen(str2));
            tfs_close(fhandle);
        }
    }
    return NULL;
}

void *fred_thread_fn() {
    for (size_t i = 0; i < N; i++) {
        int fhandle = tfs_open(target_path, TFS_O_APPEND);
        if (fhandle != -1) {
            tfs_write(fhandle, str3, strlen(str3));
            tfs_close(fhandle);
        }
    }
    return NULL;
}


int main() {
    pthread_t tid[3];
    tfs_init(NULL);
    
    int fhandle = tfs_open(target_path, TFS_O_CREAT);
    tfs_close(fhandle);

    if (pthread_create(&tid[0], NULL, alice_thread_fn, NULL) != 0) {
        return -1;
    }

    if (pthread_create(&tid[1], NULL, bob_thread_fn, NULL) != 0) {
        return -1;
    }

    if (pthread_create(&tid[2], NULL, fred_thread_fn, NULL) != 0) {
        return -1;
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);

    fhandle = tfs_open(target_path, 0);

    ssize_t r;
    char buffer[200];
    r = tfs_read(fhandle, buffer, 200);
    printf("%zd\n", r);
    assert(r == (10*(strlen(str1)+strlen(str2)+strlen(str3))));
    
    printf("Successful test.\n");
    return 0;
}
