# ifndef COMMUNICATIONHANDLER_H
# define COMMUNICATIONHANDLER_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/shm.h>
# include <sys/msg.h>
# include <openssl/sha.h>

# include "Utils.h"

int createSharedMemory ();
int closeSharedMemory (int);
int getSharedMemoryID ();
int* attachToSharedMemory(int);
int detachFromSharedMemory (int*);

int createMessageQueue (int);
int sendMessage (int, MessageHeader*, size_t);
ssize_t receiveMessage (int, MessageHeader*, size_t, long);
size_t getMessageSize (MessageHeader*);
int getMessageQueueID (int);
int closeMessageQueue (int);

# endif