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

    SharedMemoryContext shmctx = {-1, NULL};
    MessageQueueContext msqctx = {-1, -1};
    int semid = -1;
    if (initializeSession(&shmctx, &msqctx, &semid) == -1) exit(1);

    MessageHeader message;
    unsigned char rawData[BLOCK_SIZE * REQUEST_BLOCKS];
    do {
        int flags = (backlogCount == 0 && activeChildren == 0) ? 0 : IPC_NOWAIT;
        ssize_t received = receiveMessage(msqctx.msqid_req, &message, 0, flags);
        
        // Failed reception
        if (received == -1) {
            int err = errno;
            if (termination_flag == 1) break;
            if (err == ENOMSG || err == EINTR) {
                /* Falling through is an intended behavior */
            } else printf("Message could not be received\n");
        
        // Admin message
        } else if (message.mtype == ADMIN_MTYPE) {
            if (message.size > 0 && message.size < 65) {
                maxChildren = (int) message.size;
                printf("Process limit updated to %d\n", maxChildren);
            }

        // Normal conversion request
        } else {
            MessageHeader reply;
            long mtype = message.mtype;
            ssize_t total = collectBytes(shmctx.sharedMemory, msqctx.msqid_req, &message, rawData, semid, &termination_flag);
            if (total == -1) {
                if (termination_flag == 1) break;
                reply = createReply(mtype, 0, 0);
                sendMessage(msqctx.msqid_ret, &reply);
            } else {
                unsigned char* requestData = malloc(total);
                if (requestData != NULL) memcpy(requestData, rawData, total);

                if (requestData == NULL || addJob(mtype, total, requestData) == -1) {
                    free(requestData);
                    reply = createReply(mtype, 0, 0);
                    sendMessage(msqctx.msqid_ret, &reply);
                }
            }
        }

        while (activeChildren < maxChildren && backlogCount > 0) {
            int shortestJob = findShortestJob();
            dispatchJob(&backlog[shortestJob], shmctx.sharedMemory, &msqctx, semid);
            free(backlog[shortestJob].data);
            removeJob(shortestJob);
        }
        
        if (backlogCount > 0 || activeChildren > 0) usleep(50000); 
    } while (termination_flag != 1);

    terminateSession(&shmctx, &msqctx, &semid);

    return 0;
}

/**
 * Initializes every component requires for IPC.
 * @param shmctx Pointer to shared memory context
 * @param msqctx Pointer to message queue context
 * @param semid Pointer to semaphore id
 * @return 0 on success, -1 on failure
 */
int initializeSession (SharedMemoryContext* shmctx, MessageQueueContext* msqctx, int* semid) {
    if (initializeSharedMemory(shmctx) == -1) { return -1; }
    if (initializeMessageQueues(msqctx) == -1) { terminateSharedMemory(shmctx); return -1; }
    if (initializeSemaphores(semid) == -1) { terminateMessageQueues(msqctx); terminateSharedMemory(shmctx); return -1; }
    return 0;
}

/**
 * Terminates every child left alive and all components used for IPC.
 * @param shmctx Pointer to shared memory context
 * @param msqctx Pointer to message queue context
 * @param semid Pointer to semaphore id
 */
void terminateSession (SharedMemoryContext* shmctx, MessageQueueContext* msqctx, int* semid) {
    terminateChildren();
    terminateSemaphores(semid);
    terminateMessageQueues(msqctx);
    terminateSharedMemory(shmctx);
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