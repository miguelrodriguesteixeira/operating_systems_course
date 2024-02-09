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
char const target_path1[] = "/f1";
char const link_path1[] = "/l1";
char const target_path2[] = "/f2";




typedef struct {
    char const *target_path1;
    char const *link_path1;
} alice_args_t;

typedef struct {
    char const *target_path2;
    char const *target_path1;
} bob_args_t;

typedef struct {
    char const *target_path1;
} fred_args_t;

void *alice_thread_fn(void *arg) {
    alice_args_t const *args = (alice_args_t const *)arg;

    for (size_t i = 0; i < N; i++) {
        tfs_link(args->target_path1,args->link_path1);
    }
    
    return NULL;
}

void *bob_thread_fn(void *arg) {
    bob_args_t const *args = (bob_args_t const *)arg;

    for (size_t i = 0; i < N; i++) {
        tfs_sym_link(args->target_path2,args->target_path1);
    }

    return NULL;
}

void *fred_thread_fn(void *arg) {
    fred_args_t const *args = (fred_args_t const *)arg;

    for (size_t i = 0; i < N; i++) {
        tfs_unlink(args->target_path1);
    }

    return NULL;
}

int main() {
    pthread_t tid[3];
    tfs_init(NULL);
    tfs_open(target_path1, TFS_O_CREAT);
    alice_args_t alice_args = {.target_path1= target_path1, .link_path1=link_path1};

    bob_args_t bob_args = {.target_path2 = target_path2, .target_path1= target_path1};
    
    fred_args_t fred_args ={.target_path1 = target_path1};

    if (pthread_create(&tid[0], NULL, alice_thread_fn, (void *)&alice_args) != 0) {
        return -1;
    }

    if (pthread_create(&tid[1], NULL, bob_thread_fn, (void *)&bob_args) != 0) {
        return -1;
    }

    if (pthread_create(&tid[2], NULL, fred_thread_fn, (void *)&fred_args) != 0) {
        return -1;
    }

    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);
    printf("Successful test.\n");
    return 0;
}