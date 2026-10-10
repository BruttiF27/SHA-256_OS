# ifndef MAINCLIENT_SHA256_H
# define MAINCLIENT_SHA256_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <sys/shm.h>
# include <sys/stat.h>

# include "Converter_sha256.h"
# include "SharedMemoryUtils.h"
# include "MessageQueueUtils.h"
# include "FileHandler.h"

int usageCheck (int);
int connectSharedMemory (SharedMemoryContext*);
int connectMessageQueues (MessageQueueContext*);
void disconnectSharedMemory (SharedMemoryContext*);
MessageHeader createMessage (unsigned int, size_t, unsigned int, unsigned int);

# endif