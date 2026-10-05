# ifndef MAINCLIENT_SHA256_H
# define MAINCLIENT_SHA256_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <sys/shm.h>
# include <openssl/sha.h>

# include "Converter_sha256.h"
# include "CommunicationHandler.h"
# include "FileHandler.h"
# include "Utils.h"

int usageCheck (int);
int connectToSharedMemory (int*, int**);
int findMessageQueues (int*, int*);
int disconnectFromSharedMemory (int*);
MessageHeader createMessage ();

# endif