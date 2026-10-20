# ifndef SHAREDMEMORYUTILS_H
# define SHAREDMEMORYUTILS_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/shm.h>
# include <openssl/sha.h>

# include "SemaphoreUtils.h"

# define BLOCK_SIZE 4096
# define REQUEST_BLOCKS 15
# define RESULT_SLOTS 16
# define DIGEST_SLOT 128
# define MEMORY_SIZE (sizeof(SharedMemory))

# define KEY_SHARED_MEMORY 27

typedef struct Block {
    unsigned char data[BLOCK_SIZE];         // Actual contents of the block
} Block;

typedef struct ResultSlot {
    unsigned char digest[DIGEST_SLOT];      // Digest produced by the server
} ResultSlot;

typedef struct SharedMemory {
    Block requestSection[REQUEST_BLOCKS];   // [0-14] Blocks are allocated for client requests
    ResultSlot resultSection[RESULT_SLOTS]; // 15th Block is allocated for server digests. 16*128 = 4096
} SharedMemory;

typedef struct SharedMemoryContext {
    int shmid;                              // Shared memory address
    SharedMemory* sharedMemory;             // Actual shared memory
} SharedMemoryContext;

/* Basic open/close procedures */
int initializeSharedMemory (SharedMemoryContext*);
void terminateSharedMemory (SharedMemoryContext*);
int createSharedMemory ();
int closeSharedMemory (int);

/* Secondary operations of connection and field fetching */
SharedMemory* attachToSharedMemory (int);
int detachFromSharedMemory (SharedMemory*);
int getSharedMemoryID ();

/* Active operation of R/W inside shared memory sections */
ssize_t readRequestSharedMemory (SharedMemory*, unsigned int, unsigned char*, size_t, int);
ssize_t readReturnSharedMemory (SharedMemory*, unsigned int, unsigned char*, size_t, int);
int writeRequestSharedMemory (SharedMemory*, const unsigned char*, size_t, int);
int writeReturnSharedMemory (SharedMemory*, const unsigned char*, size_t, int);

# endif