#ifndef MAX_CMD_LEN
#define MAX_CMD_LEN 200
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

extern char **environ;

int cd(char*);
int set(const char*, const char*);
int unset(const char*);
int pwd(void);
int pushd(char*, Stack*);
int popd(Stack*);
int history(void);
int env(void);
void clearstdinBuffer(void);
