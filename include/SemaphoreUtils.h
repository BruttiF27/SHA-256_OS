# ifndef SEMAPHOREUTILS_H
# define SEMAPHOREUTILS_H

# include <sys/sem.h>
# include <sys/types.h>
# include <sys/ipc.h>
# include <errno.h>

# include "SharedMemoryUtils.h"

# define KEY_SEMAPHORE_SET 30

# define SEM_TOTAL 33

// Semaphore array intervals
# define SEM_REQUEST_BASE 0
# define SEM_REQUEST_MAX 14
# define SEM_RETURN_BASE 15
# define SEM_RETURN_MAX 30

// Counting semaphores used to free blocks and slots
# define SEM_REQUEST_FREE 31
# define SEM_RETURN_FREE 32

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};

int createSemaphoreSet ();
int removeSemaphoreSet (int);
int initializeSemaphoreSet (int);
int getSemaphoreSetID ();

int semaphoreOperation (int, int, int, int);
int semaphoreWait (int, int);
int semaphoreTryWait (int, int);
int semaphorePost (int, int);

# endif