#ifndef MAX_CMD_LEN
#define MAX_CMD_LEN 200
#endif
#define MAX_ARGS 10

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "stack.h"
#include "builtins.h"
