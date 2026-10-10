# ifndef MAINSERVER_SHA256_H
# define MAINSERVER_SHA256_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <signal.h>
# include <sys/shm.h>
# include <openssl/sha.h>

# include "Converter_sha256.h"
# include "SharedMemoryUtils.h"
# include "MessageQueueUtils.h"
# include "FileHandler.h"

__sig_atomic_t termination_flag = 0;

int initializeSharedMemory (SharedMemoryContext*);
int initializeMessageQueues (MessageQueueContext*);
void terminateSharedMemory (SharedMemoryContext*);
void terminateMessageQueues (MessageQueueContext*);

ssize_t collectBytes (SharedMemory*, int, MessageHeader*, unsigned char*);
int sendReply (int, long, unsigned int, size_t);
MessageHeader createMessage (long, unsigned int, size_t);

void sigintHandler (int);

# endif