# include "SharedMemoryUtils.h"

/**
 * Initializes the shared memory segment required for inter-process communication
 * @return Shared memory ID on success, -1 on failure
 */
int createSharedMemory () {
    key_t shmKey = ftok("src", KEY_SHARED_MEMORY);
    return shmget(shmKey, MEMORY_SIZE, IPC_CREAT | IPC_EXCL | 0666);
}

/**
 * Marks shared memory segment for deletion. It will be removed once every actor has disconnected from it.
 * @param shmid Shared memory ID
 * @return 0 if success, -1 if failure
 */
int closeSharedMemory (int shmid) { return shmctl(shmid, IPC_RMID, NULL); }

/**
 * Attaches client to an existing shared memory
 * @param shmid Shared memory ID
 * @return Shared memory address on success, (void*)-1 on failure
 */
SharedMemory* attachToSharedMemory (int shmid) { return shmat(shmid, NULL, 0); }

/**
 * Detaches client from an existing shared memory
 * @param shmaddr Shared memory address
 * @return 0 on success, -1 on failure
 */
int detachFromSharedMemory (SharedMemory* sharedMemory) { return shmdt(sharedMemory); }

/**
 * Getter method to fetch currently existing shared memory
 * @return Shared memory ID on success, -1 on failure
 */
int getSharedMemoryID () {
    key_t shmKey = ftok("src", KEY_SHARED_MEMORY);
    return shmget(shmKey, MEMORY_SIZE, 0666);
}

/**
 * TODO
 * Reads data from a specific block inside shared memory
 * @param sharedMemory Shared memory
 * @param block_id Block ID where data will be read
 * @param outData Actual content that was read
 * @param size Size of the data. Must not be greater than BLOCK_SIZE
 * @return Number of bytes that have been copied
 */
ssize_t readRequestSharedMemory (SharedMemory* sharedMemory, unsigned int block_id, unsigned char* outData, size_t size) {
    if (sharedMemory == NULL || outData == NULL || block_id >= REQUEST_BLOCKS || size > BLOCK_SIZE) return -1;
    
    if (sharedMemory->requestSection[block_id].inUse) {
        memcpy(outData, sharedMemory->requestSection[block_id].data, size);
        sharedMemory->requestSection[block_id].inUse = 0;
        return (ssize_t) size;
    }
    
    return -1;
}

/**
 * TODO Will eventually need semaphore for protection
 * Reads data from a specific block inside shared memory
 * @param sharedMemory Shared memory
 * @param slot_id Slot where data will be read
 * @param outData Actual content that was read.
 * @param size Size of the data. Must not be greater than DIGEST_SLOT
 * @return Number of bytes that have been copied
 */
ssize_t readReturnSharedMemory (SharedMemory* sharedMemory, unsigned int slot_id, unsigned char* outData, size_t size) {
    if (sharedMemory == NULL || outData == NULL || slot_id >= RESULT_SLOTS || size > DIGEST_SLOT) return -1;

    if (sharedMemory->resultSection[slot_id].inUse) {
        memcpy(outData, sharedMemory->resultSection[slot_id].digest, size);
        sharedMemory->resultSection[slot_id].inUse = 0;
        return (ssize_t) size;
    }

    return -1;
}

/**
 * TODO will eventually need semaphore for protection
 * Writes data into a usable request block inside shared memory
 * @param sharedMemory Shared memory address
 * @param data data to be written inside the block
 * @param size data size
 * @return Number of the request block used on success [0, REQUEST_BLOCKS-1], -1 on failure
 */
int writeRequestSharedMemory (SharedMemory* sharedMemory, const unsigned char* data, size_t size) {
    if (sharedMemory == NULL || data == NULL || size > BLOCK_SIZE) return -1;
    for (int i = 0; i < REQUEST_BLOCKS; i++) {
        if (!sharedMemory->requestSection[i].inUse) {
            memcpy(sharedMemory->requestSection[i].data, data, size);
            sharedMemory->requestSection[i].inUse = 1;
            return i;
        }
    }

    return -1;
}

/**
 * TODO will eventually need semaphore for protection
 * Writes data into a usable return slot inside shared memory
 * @param sharedMemory Shared memory address
 * @param data data to be written inside the block
 * @param size data size
 * @return Number of the return slot used on success [0, RESULT_SLOTS-1], -1 on failure
 */
int writeReturnSharedMemory (SharedMemory* sharedMemory, const unsigned char* data, size_t size) {
    if (sharedMemory == NULL || data == NULL || size > DIGEST_SLOT) return -1;
    for (int i = 0; i < RESULT_SLOTS; i++) {
        if (!sharedMemory->resultSection[i].inUse) {
            memcpy(sharedMemory->resultSection[i].digest, data, size);
            sharedMemory->resultSection[i].inUse = 1;
            return i;
        }
    }

    return -1;
}