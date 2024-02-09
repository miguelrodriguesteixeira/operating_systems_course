#include "logging.h"
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "operations.h"

#define BUFFER_SIZE (289)
#define SESSIONS (64)

struct BOX_INFO{
    int sub;
    int pub;
    char box_name[33];
};

int switchcase(char buffer[289]);
