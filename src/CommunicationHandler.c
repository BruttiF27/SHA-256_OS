# include "CommunicationHandler.h"

/**
 * Initializes the shared memory segment required for inter-process communication
 * @return Shared memory ID on success, -1 on failure
 */
int createSharedMemory () {
    key_t shmKey = ftok("src", 27);
    return shmget(shmKey, MEMORY_SIZE, IPC_CREAT | IPC_EXCL | 0666);
}

/**
 * Marks shared memory segment for deletion. It will be removed once every actor has disconnected from it.
 * @param shmid Shared memory ID
 * @return 0 if success, -1 if failure
 */
int closeSharedMemory (int shmid) { return shmctl(shmid, IPC_RMID, NULL); }

/**
 * Getter method to fetch currently existing shared memory
 * @return Shared memory ID on success, -1 on failure
 */
int getSharedMemoryID () {
    key_t shmKey = ftok("src", 27);
    return shmget(shmKey, MEMORY_SIZE, 0666);
}

/**
 * Attaches client to an existing shared memory
 * @param shmid Shared memory ID
 * @return Shared memory address on success, -1 on failure
 */
int* attachToSharedMemory (int shmid) { return (int*) shmat(shmid, NULL, 0); }

/**
 * Detaches client from an existing shared memory
 * @param shmaddr Shared memory address
 * @return 0 on success, -1 on failure
 */
int detachFromSharedMemory (int* shmaddr) { return shmdt(shmaddr); }