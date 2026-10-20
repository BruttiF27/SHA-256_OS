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
# include "JobScheduler.h"
# include "SemaphoreUtils.h"
# include "FileHandler.h"
# include "RequestDataCollector.h"

volatile __sig_atomic_t termination_flag = 0;
int maxChildren = 16;

/* Basic init/remove operations */
int initializeSession (SharedMemoryContext*, MessageQueueContext*, int*);
void terminateSession (SharedMemoryContext*, MessageQueueContext*, int*);
void terminateChildren ();

/* Signal handler functions */
void sigintHandler (int);
void sigchldHandler (int);

# endif