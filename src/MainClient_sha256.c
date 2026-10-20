# include "MainClient_sha256.h"

/**
 * Must be run after the server activates for the software to work properly
 * 1. Connects to an existing shared memory segment, defined in the server.
 * 2. Fetches the filename from argv[1] and writes its contents inside a usable block of client shared memory section
 * 3. Sends a message to the server to advise him to fetch the written contents and waits for the answer
 * 4. Once the reply message is received, the converted data can be read from a slot inside the server shared memory section
 * 5. Print digest on stdout and terminate environment
 */
int main (int argc, char *argv[]) {
    if (usageCheck(argc) == -1) exit(1);

    // Initiate step 1
    SharedMemoryContext shmctx = {-1, NULL};
    if (connectSharedMemory(&shmctx) == -1) exit(1);
    MessageQueueContext msqctx = {-1, -1};
    if (connectMessageQueues(&msqctx) == -1) terminateSession(&shmctx, -1, "Could not connect to message queue");
    int semID = -1;
    if (connectSemaphores(&semID) == -1) terminateSession(&shmctx, -1, "Could not connect to semaphore set");

    int fileDescriptor = open(argv[1], O_RDONLY, 0);
    struct stat stats;
    if (fileDescriptor < 0 || fstat(fileDescriptor, &stats) != 0) {
        terminateSession(&shmctx, fileDescriptor, "File could not be opened or stats could not be fetched");
    }
    
    int requiredBlocks = ((size_t)stats.st_size + BLOCK_SIZE-1) / BLOCK_SIZE;
    if (requiredBlocks == 0 || requiredBlocks > REQUEST_BLOCKS) terminateSession(&shmctx, fileDescriptor, "Empty file received or file too big");

    MessageHeader message;
    unsigned char fileBuffer[BLOCK_SIZE];
    for (int block_index = 0; block_index < requiredBlocks; block_index++) {
        // Initiate step 2
        ssize_t fileSize = readChunk(fileDescriptor, fileBuffer, BLOCK_SIZE);
        if (fileSize <= 0) terminateSession(&shmctx, fileDescriptor, "Provided file could not be read");
        
        int block_id = writeRequestSharedMemory(shmctx.sharedMemory, fileBuffer, fileSize, semID);
        if (block_id == -1) terminateSession(&shmctx, fileDescriptor, "No blocks currently available");
        
        // Initiate step 3
        message = createRequest(block_id, fileSize, block_index, requiredBlocks);
        if (sendMessage(msqctx.msqid_req, &message) == -1) {
            terminateSession(&shmctx, fileDescriptor, "Message could not be sent");
        } else printf("Message sent by the client. Mtype: %ld\n", message.mtype);
    }

    // Initiate step 4
    if (receiveMessage(msqctx.msqid_ret, &message, getpid(), 0) == -1) {
        terminateSession(&shmctx, fileDescriptor, "Message could not be received");
    } else printf("Message received by the client. Mtype: %ld\n", message.mtype);

    close(fileDescriptor);

    if (message.size == 0) terminateSession(&shmctx, fileDescriptor, "Server sent an empty message");

    unsigned char digest[DIGEST_SLOT+1];
    ssize_t bytesRead = readReturnSharedMemory(shmctx.sharedMemory, message.block_id, digest, message.size, semID);
    
    // Initiate step 5
    if (bytesRead == -1) printf("Could not read from result section\n");
    else {
        digest[bytesRead] = '\0';
        printf("%s\n", digest);
    }

    disconnectSharedMemory(&shmctx);
    return 0;
}

/**
 * Checks if user has inputted a filename or too many arguments
 * @param argc Number of strings
 * @return 0 on success, -1 on failure
 */
int usageCheck (int argc) {
    if (argc == 2) return 0;
    else {
        printf("Usage: <file name with no spaces>\n");
        return -1;
    }
}

/**
 * Connects the client to the shared memory environment needed for IPC
 * @param context Shared memory context
 * @return 0 on success, -1 on failure
 */
int connectSharedMemory (SharedMemoryContext* context) {   
    context->shmid = getSharedMemoryID();
    if (context->shmid == -1) {
        printf("Shared memory was not found\n");
        return -1;
    }

    context->sharedMemory = attachToSharedMemory(context->shmid);
    if (context->sharedMemory == (void*)-1) {
        printf("Could not attach to shared memory\n");
        context->shmid = -1;
        context->sharedMemory = NULL;
        return -1;
    }

    return 0;
}

/**
 * Fetches and connects to the queues required for IPC
 * @param context Message queue context
 * @return 0 on success, -1 on failure
 */
int connectMessageQueues (MessageQueueContext* context) {
    context->msqid_req = getMessageQueueID(KEY_REQUEST_QUEUE);
    if (context->msqid_req == -1) {
        printf("Could not connect to request queue\n");
        return -1;
    }

    context->msqid_ret = getMessageQueueID(KEY_RETURN_QUEUE);
    if (context->msqid_ret == -1) {
        printf("Could not connect to return queue\n");
        return -1;
    }

    return 0;
}

/**
 * Fetches and connects client to the semaphore set created in the server
 * @param semID Semaphore set ID
 * @return 0 on success, -1 on failure
 */
int connectSemaphores (int* semID) {
    *semID = getSemaphoreSetID();
    if (*semID == -1) {
        printf("Could not find semaphore set\n");
        return -1;
    }
    return 0;
}

/**
 * Quietly terminates attachment to shared memory
 * @param context Shared memory context
 */
void disconnectSharedMemory (SharedMemoryContext* context) {
    if (detachFromSharedMemory(context->sharedMemory) == -1) printf("Could not detach from shared memory\n");
}

/**
 * Terminates current section due to an error explained by errorMessage
 * @param shmctx Shared memory context
 * @param fileDescriptor File descriptor related to read file
 * @param errorMessage Explains what the problem was
 */
void terminateSession (SharedMemoryContext* shmctx, int fileDescriptor, const char* errorMessage) {
    if (errorMessage != NULL) printf("%s\n", errorMessage);
    if (fileDescriptor >= 0) close(fileDescriptor);
    disconnectSharedMemory(shmctx);
    exit(1);
}