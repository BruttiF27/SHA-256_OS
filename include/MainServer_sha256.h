# ifndef MAINSERVER_SHA256_H
# define MAINSERVER_SHA256_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <signal.h>
# include <errno.h>
# include <sys/wait.h>
# include <sys/shm.h>
# include <openssl/sha.h>

# include "Converter_sha256.h"
# include "SharedMemoryUtils.h"
# include "MessageQueueUtils.h"
# include "SemaphoreUtils.h"
# include "FileHandler.h"

__sig_atomic_t termination_flag = 0;

int initializeSharedMemory (SharedMemoryContext*);
int initializeMessageQueues (MessageQueueContext*);
int initializeSemaphores (int*);
void terminateSharedMemory (SharedMemoryContext*);
void terminateMessageQueues (MessageQueueContext*);
void terminateSemaphores (int*);

ssize_t collectBytes (SharedMemory*, int, MessageHeader*, unsigned char*, int);
int sendReply (int, long, unsigned int, size_t);

void sigintHandler (int);
void sigchldHandler (int);

void terminateChildren ();

# endif