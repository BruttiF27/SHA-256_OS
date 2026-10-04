# ifndef COMMUNICATIONHANDLER_H
# define COMMUNICATIONHANDLER_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/shm.h>
# include <openssl/sha.h>

# include "Utils.h"

int createSharedMemory ();
int closeSharedMemory (int);
int getSharedMemoryID ();
int* attachToSharedMemory(int);
int detachFromSharedMemory (int*);

# endif