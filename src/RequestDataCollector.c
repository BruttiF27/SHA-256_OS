# include "RequestDataCollector.h"

/**
 * Wrapper to fetch all bytes, even if written in more than one block.
 * @param sharedMemory Shared memory
 * @param msqid_req Message queue ID of the request queue
 * @param first First message of the sequence
 * @param out Actual read data
 * @return Bytes read on success, -1 on failure
 */
ssize_t collectBytes (SharedMemory* shm, int msqid_req, MessageHeader* first, unsigned char* out, int sem_id, volatile __sig_atomic_t* stop) {
    if (validateRequest(first) == -1) return -1;
    
    size_t bytes = 0;
    MessageHeader current = *first;
    unsigned int block_total = first->block_total;

    for (int got = 0; got < block_total; got++) {
        if (got > 0 && receiveChunk(msqid_req, &current, first->mtype, stop) == -1) return -1;
        ssize_t n = storeChunk(shm, &current, block_total, out, sem_id);
        if (n == -1) return -1;
        bytes += n;
    }
    return (ssize_t) bytes;
}

/**
 * Validates a message request to read data from shared memory
 * @param first First message of the request
 * @return 0 on success, -1 on failure
 */
int validateRequest (MessageHeader* first) {
    if (first->block_total == 0 || first->block_total > REQUEST_BLOCKS) return -1;
    return 0;
}

/**
 * Fetches the message used to understand which chunk is to be read
 * @param msqid_req Request message queue id
 * @param out Struct where current message will be stored
 * @param mtype Message type
 * @param stop Termination flag variable, stops execution when needed
 * @return 0 on success, -1 on failure
 */
int receiveChunk (int msqid_req, MessageHeader* out, long mtype, volatile __sig_atomic_t* stop) {
    ssize_t received;
    do {
        received = receiveMessage(msqid_req, out, mtype, 0);
    } while (received == -1 && errno == EINTR && !*stop);
    return (received == -1) ? -1 : 0;
}

/**
 * Actually reads the requested chunk of memory
 * @param shm Shared memory
 * @param chunk Current message associated to chunk of memory
 * @param block_total Total blocks used by the request
 * @param out Actual read data
 * @param sem_id Semaphore ID
 * @return Size of the data that was read on success, -1 on failure.
 */
ssize_t storeChunk (SharedMemory* shm, MessageHeader* chunk, unsigned int block_total, unsigned char* out, int sem_id) {
    if (chunk->block_index >= block_total) return -1;

    unsigned char* dest = out + (size_t) chunk->block_index * BLOCK_SIZE;
    return readRequestSharedMemory(shm, chunk->block_id, dest, chunk->size, sem_id);
}