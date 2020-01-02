#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <errno.h>

#define OK_TAG   "[\x1b[32m OK \x1b[0m] "
#define SEM_KEY 14839
#define MEM_KEY 94029
#define SEG_SIZE 4
