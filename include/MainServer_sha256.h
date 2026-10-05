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
# include "CommunicationHandler.h"
# include "FileHandler.h"
# include "Utils.h"

__sig_atomic_t termination_flag = 0;

int initializeSharedMemory (int*);
int initializeMessageQueues (int*, int*);
void terminateSharedMemory (int);
void terminateMessageQueues (int, int);
MessageHeader createMessage (long);
void sigintHandler (int);

# endif