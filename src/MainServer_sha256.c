# include "MainServer_sha256.h"

/**
 * Must be run first in order for the software to work properly
 * 1. Creates shared memory and message queues, then waits for requests, sent through the latter ones
 * 2. Upon receiving a message, reads the requested block (or blocks) where the client wrote file data
 * 3. Data is then converted
 * 4. Writes converted data on a usable block in its half of the memory
 * 5. Sends message to client that the data can be fetched from shared memory
 */
int main (int argc, char *argv[]) {
    signal(SIGINT, sigintHandler);
    signal(SIGCHLD, sigchldHandler);

    // Initiate step 1
    SharedMemoryContext shmctx = {-1, NULL};
    if (initializeSharedMemory(&shmctx) == -1) exit(1);
    MessageQueueContext msqctx = {-1, -1};
    if (initializeMessageQueues(&msqctx) == -1) {
        terminateSharedMemory(&shmctx);
        exit(1);
    }
    int semID = -1;
    if (initializeSemaphores(&semID) == -1) {
        terminateMessageQueues(&msqctx);
        terminateSharedMemory(&shmctx);
        exit(1);
    }

    MessageHeader message;
    unsigned char rawData[BLOCK_SIZE * REQUEST_BLOCKS];
    do {
        int flags = (backlogCount == 0 && activeChildren == 0) ? 0 : IPC_NOWAIT;
        ssize_t received = receiveMessage(msqctx.msqid_req, &message, getMessageSize(&message), 0, flags);
        
        // Failed reception
        if (received == -1) {
            int err = errno;
            if (termination_flag == 1) break;
            if (err == ENOMSG || err == EINTR) {
                /* TODO The hell am I supposed to do here */
            } else printf("Message could not be received\n");
        
        // Admin message
        } else if (message.mtype == ADMIN_MTYPE) {
            if (message.size > 0 && message.size < 65) {
                maxChildren = (int) message.size;
                printf("Process limit updated to %d\n", maxChildren);
            }

        // Normal conversion request
        } else {
            long mtype = message.mtype;
            ssize_t total = collectBytes(shmctx.sharedMemory, msqctx.msqid_req, &message, rawData, semID);
            if (total == -1) {
                if (termination_flag == 1) break;
                sendReply(msqctx.msqid_ret, mtype, 0, 0);
            } else {
                unsigned char* requestData = malloc(total);
                if (requestData != NULL) memcpy(requestData, rawData, total);

                if (requestData == NULL || addJob(mtype, total, requestData) == -1) {
                    free(requestData);
                    sendReply(msqctx.msqid_ret, mtype, 0, 0);
                }
            }
        }

        while (activeChildren < maxChildren && backlogCount > 0) {
            int shortestJob = findShortestJob();
            dispatchJob(&backlog[shortestJob], shmctx.sharedMemory, &msqctx, semID);
            free(backlog[shortestJob].data);
            removeJob(shortestJob);
        }
        
        if (backlogCount > 0 || activeChildren > 0) usleep(50000); 
    } while (termination_flag != 1);

    terminateChildren();
    
    terminateSemaphores(&semID);
    terminateMessageQueues(&msqctx);
    terminateSharedMemory(&shmctx);

    return 0;
}

/**
 * Initializes the shared memory environment required for IPC and attaches the server to it
 * @param context Shared memory context
 * @return 0 on success, -1 on failure
 */
int initializeSharedMemory (SharedMemoryContext* context) {
    context->shmid = createSharedMemory();
    if (context->shmid == -1) {
        printf("Shared memory could not be created\n");
        return -1;
    }

    context->sharedMemory = attachToSharedMemory(context->shmid);
    if (context->sharedMemory == (void*)-1) {
        printf("Could not attach server to shared memory\n");
        closeSharedMemory(context->shmid);
        context->shmid = -1;
        context->sharedMemory = NULL;
        return -1;
    }

    return 0;
}

/**
 * Initializes the message queues environment needed for IPC
 * @param context Message queue context
 * @return 0 on success, -1 on failure
 */
int initializeMessageQueues (MessageQueueContext* context) {
    context->msqid_req = createMessageQueue(KEY_REQUEST_QUEUE);
    if (context->msqid_req == -1) {
        printf("Request message queue could not be created\n");
        return -1;
    }

    context->msqid_ret = createMessageQueue(KEY_RETURN_QUEUE);
    if (context->msqid_ret == -1) {
        printf("Return message queue could not be created\n");
        closeMessageQueue(context->msqid_req);
        context->msqid_req = -1;
        return -1;
    }

    return 0;
}

/**
 * Initializes the semaphores needed for shared memory protection
 * @param semID Semaphore set ID
 * @return 0 on success, -1 on failure
 */
int initializeSemaphores (int* semID) {
    *semID = createSemaphoreSet();
    if (*semID == -1) {
        printf("Semaphore set could not be created\n");
        return -1;
    }

    if (initializeSemaphoreSet((*semID)) == -1) {
        printf("Semaphores could not be initialized\n");
        removeSemaphoreSet((*semID));
        *semID = -1;
        return -1;
    }

    return 0;
}

/**
 * Terminates the IPC shared memory environment
 * @param context Shared memory context
 */
void terminateSharedMemory (SharedMemoryContext* context) {
    if (detachFromSharedMemory(context->sharedMemory) == -1) printf("Could not detach from shared memory\n");
    if (closeSharedMemory(context->shmid) == -1) printf("Shared memory could not be closed\n");
}

/**
 * Terminates the IPC message queues
 * @param context Message queue context
 */
void terminateMessageQueues (MessageQueueContext* context) {
    if (closeMessageQueue(context->msqid_req) == -1) printf("Could not close request message queue\n");
    if (closeMessageQueue(context->msqid_ret) == -1) printf("Could not close return message queue\n");
}

/**
 * Terminates the semaphores
 * @param semID Semaphore set ID
 */
void terminateSemaphores (int* semID) { if (removeSemaphoreSet((*semID)) == -1) printf("Semaphore set could not be removed\n"); }

/**
 * Wrapper to fetch all bytes, even if written in more than one block.
 * @param sharedMemory Shared memory
 * @param msqid_req Message queue ID of the request queue
 * @param first First message of the sequence
 * @param out Actual read data
 * @return Bytes read on success, -1 on failure
 */
ssize_t collectBytes (SharedMemory* sharedMemory, int msqid_req, MessageHeader* first, unsigned char* out, int semID) {
    long mtype = first->mtype;
    unsigned int total = first->block_total;

    if (total == 0 || total > REQUEST_BLOCKS) return -1;

    MessageHeader current = *first;
    size_t bytes = 0;

    for (unsigned int got = 0; got < total; got++) {
        // The first chunk is already in current; other chunks are filtered by mtype
        if (got > 0) {
            ssize_t received;
            do {
                received = receiveMessage(msqid_req, &current, getMessageSize(&current), mtype, 0);
            } while (received == -1 && errno == EINTR && !termination_flag);
            if (received == -1) return -1;
        }

        // block_index says where the chunk belongs: basically a sequence number
        if (current.block_index >= total) return -1;
        unsigned char* dest = out + (size_t) current.block_index * BLOCK_SIZE;

        // block_id says where the chunk is in shared memory
        ssize_t n = readRequestSharedMemory(sharedMemory, current.block_id, dest, current.size, semID);
        if (n == -1) return -1;

        bytes += (size_t) n;
    }

    return (ssize_t) bytes;
}

/**
 * Wrapper to send a reply to clients
 * @param msqid_ret Message queue ID for return queue
 * @param mtype Message type
 * @param slot_id Slot used inside return section
 * @param size Message size (success=65, failure=0)
 * @return 0 on success, -1 on failure
 */
int sendReply (int msqid_ret, long mtype, unsigned int slot_id, size_t size) {
    MessageHeader reply = createReply(mtype, slot_id, size);
    return sendMessage(msqid_ret, &reply, getMessageSize(&reply));
}

/**
 * Signal handler for SIGINT, marks process for termination.
 * @param sigNum Signal number
 */
void sigintHandler (int sigNum) { termination_flag = 1; }

/**
 * Signal handler for SIGCHLD, reaps every terminated child process without blocking
 * @param sigNum Signal number
 */
void sigchldHandler (int sigNum) {
    int saved = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0) activeChildren--;
    errno = saved;
}

/**
 * Called on shutdown, waits for every child process that are still running
 */
void terminateChildren () {
    while (1) {
        pid_t child = waitpid(-1, NULL, 0);
        if (child > 0 || (child == -1 && errno == EINTR)) continue;
        else break;
    }
}

/**
 * Creates the environment where children processes are to operate.
 * 1. Defines signal set to be used
 * 2. Forks children
 * 3. Digest data read from shared memory
 * 4. Write the digest in the result section and sends a message to the client.
 * 5. The child kills itself.
 */
void dispatchJob (Job* job, SharedMemory* sharedMemory, MessageQueueContext* msqctx, int semID) {
    sigset_t block, old;
    sigemptyset(&block);
    sigaddset(&block, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block, &old);

    fflush(stdout);
    pid_t pid = fork();

    if (pid > 0) activeChildren++;
    sigprocmask(SIG_SETMASK, &old, NULL);

    if (pid == -1) {
        printf("Could not create child process\n");
        sendReply(msqctx->msqid_ret, job->mtype, 0, 0);
        return;
    }

    if (pid == 0) {
        // Initiate step 3
        unsigned char digest[DIGEST_SLOT];
        if (digestContent(digest, job->data, (size_t) job->size)) {
            printf("Could not digest data\n");
            sendReply(msqctx->msqid_ret, job->mtype, 0, 0);
            _exit(1);
        }

        // 2. write digest to a result slot
        int slot_id = writeReturnSharedMemory(sharedMemory, digest, 65, semID);
        if (slot_id == -1) {
            printf("Could not write digest\n");
            sendReply(msqctx->msqid_ret, job->mtype, 0, 0);
            _exit(1);
        }

        // Uncomment to test concurrency and semaphores
        sleep(5);

        // Initiate step 5
        if (sendReply(msqctx->msqid_ret, job->mtype, slot_id, 65) == -1) printf("Message could not be sent\n");
        else printf("Message sent by the server. Mtype: %ld\n", job->mtype);

        _exit(0);
    }
}