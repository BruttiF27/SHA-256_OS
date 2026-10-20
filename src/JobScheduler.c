# include "JobScheduler.h"

/**
 * Adds a job to a the backlog array if there's a free cell.
 * @param mtype Message type
 * @param size Data size
 * @param data Actual data of the job
 * @return 0 on success, -1 on failure
 */
int addJob (int mtype, size_t size, char* data) {
    if (backlogCount >= BACKLOG_SIZE) return -1;
    backlog[backlogCount].mtype = mtype;
    backlog[backlogCount].size  = size;
    backlog[backlogCount].data  = data;
    backlogCount++;
    return 0;
}

/**
 * Returns the index of the job with the smallest size
 * @return shortest job index on success, -1 on empty array
 */
int findShortestJob () {
    int best = -1;
    for (int i = 0; i < backlogCount; i++) {
        if (best == -1 || backlog[i].size < backlog[best].size) best = i;
    }
    return best;
}

/**
 * Removes a job from the backlog
 * @param bestIndex index of the job that was chosen from the backlog
 */
void removeJob (int bestIndex) {
    backlog[bestIndex] = backlog[backlogCount-1];
    backlogCount--;
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
    initializeSignalSet(&block, &old);

    fflush(stdout);
    pid_t pid = fork();

    if (pid > 0) activeChildren++;
    sigprocmask(SIG_SETMASK, &old, NULL);
    switch (pid) {
        case -1: {
            signalError(msqctx, job, "Could not create child process"); return;
        } case 0: {
            unsigned char digest[DIGEST_SLOT];
            if (digestContent(digest, job->data, (size_t) job->size)) { signalError(msqctx, job, "Could not digest data"); _exit(1); }

            int slot_id = writeReturnSharedMemory(sharedMemory, digest, 65, semID);
            if (slot_id == -1) { signalError(msqctx, job, "Could not write digest\n"); _exit(1); }

            sleep(10); // Uncomment to test concurrency and semaphores

            MessageHeader reply = createReply(job->mtype, slot_id, 65);
            if (sendMessage(msqctx->msqid_ret, &reply) == -1) { printf("Message could not be sent\n"); _exit(1); }
            else printf("Message sent by the server. Mtype: %ld\n", job->mtype);
            _exit(0);
        } default: break;
    }
}

/**
 * Defines signal set whose elements are to be blocked
 * @param block New signal set to replace the other
 * @param old Old signal set
 */
void initializeSignalSet (sigset_t* block, sigset_t* old) {
    sigemptyset(block);
    sigaddset(block, SIGCHLD);
    sigprocmask(SIG_BLOCK, block, old);
}

/**
 * Signals an error in stdout and sends default error message to the requester
 * @param msqctx Message queue context
 * @param job Current job taken
 * @param errorMessage String to print in stdout
 */
void signalError (MessageQueueContext* msqctx, Job* job, char* errorMessage) {
    printf("%s\n", errorMessage);
    MessageHeader reply = createReply(job->mtype, 0, 0);
    sendMessage(msqctx->msqid_ret, &reply);
}