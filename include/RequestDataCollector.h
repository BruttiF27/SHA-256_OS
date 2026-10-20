# ifndef REQUESTDATACOLLECTOR_H
# define REQUESTDATACOLLECTOR_H

# include <stdlib.h>
# include <unistd.h>
# include <signal.h>

# include "SharedMemoryUtils.h"
# include "MessageQueueUtils.h"

ssize_t collectBytes (SharedMemory*, int, MessageHeader*, unsigned char*, int, volatile __sig_atomic_t*);
int validateRequest (MessageHeader*);
int receiveChunk (int, MessageHeader*, long, volatile __sig_atomic_t*);
ssize_t storeChunk (SharedMemory*, MessageHeader*, unsigned int, unsigned char*, int);

# endif