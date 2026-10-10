# include "MainClient_sha256.h"

/**
 * Must be run after the client activates for the software to work properly
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
    if (connectMessageQueues(&msqctx) == -1) {
        disconnectSharedMemory(&shmctx);
        exit(1);
    }

    int fileDescriptor = open(argv[1], O_RDONLY, 0);
    struct stat stats;
    if (fileDescriptor < 0 || fstat(fileDescriptor, &stats) != 0) {
        printf("File could not be opened or stats could not be fetched\n");
        disconnectSharedMemory(&shmctx);
        close(fileDescriptor);
        exit(1);
    }
    
    int requiredBlocks = ((size_t)stats.st_size + BLOCK_SIZE-1) / BLOCK_SIZE;
    if (requiredBlocks == 0 || requiredBlocks > 15) {
        printf("Empty file received or file too big\n");
        disconnectSharedMemory(&shmctx);
        close(fileDescriptor);
        exit(1);
    }

    MessageHeader message;
    unsigned char fileBuffer[BLOCK_SIZE];
    for (int block_index = 0; block_index < requiredBlocks; block_index++) {
        // Initiate step 2
        ssize_t fileSize = readChunk(fileDescriptor, fileBuffer, BLOCK_SIZE);
        if (fileSize == -1) {
            printf("Provided file could not be read\n");
            disconnectSharedMemory(&shmctx);
            close(fileDescriptor);
            exit(1);
        }

        int block_id = writeRequestSharedMemory(shmctx.sharedMemory, fileBuffer, fileSize);
        if (block_id == -1) {
            printf("No blocks currently available\n");
            disconnectSharedMemory(&shmctx);
            close(fileDescriptor);
            exit(1);
        }
        
        // Initiate step 3
        message = createMessage(block_id, fileSize, block_index, requiredBlocks);
        if (sendMessage(msqctx.msqid_req, &message, getMessageSize(&message)) == -1) {
            printf("Message could not be sent\n");
            disconnectSharedMemory(&shmctx);
            close(fileDescriptor);
            exit(1);
        } else printf("Message sent by the client. Mtype: %ld\n", message.mtype);
    }

    close(fileDescriptor);

    // Initiate step 4
    if (receiveMessage(msqctx.msqid_ret, &message, getMessageSize(&message), getpid()) == -1) {
        printf("Message could not be received\n");
        disconnectSharedMemory(&shmctx);
        exit(1);
    } else printf("Message received by the client. Mtype: %ld\n", message.mtype);

    if (message.size == 0) {
        printf("Server sent an empty message\n");
        disconnectSharedMemory(&shmctx);
        exit(1);
    }

    unsigned char digest[DIGEST_SLOT];
    ssize_t bytesRead = readReturnSharedMemory(shmctx.sharedMemory, message.block_id, digest, message.size);
    
    // Initiate step 5
    if (bytesRead == -1) printf("Could not read from result section\n");
    else printf("%s\n", digest);
    
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
 * Gracefully terminates attachment to shared memory
 * @param context Shared memory context
 */
void disconnectSharedMemory (SharedMemoryContext* context) {
    if (detachFromSharedMemory(context->sharedMemory) == -1) printf("Could not detach from shared memory\n");
}

/**
 * Initializes fields of the message to be sent
 * @param block_id ID number corresponding to the block used inside shared memory
 * @param size Size of the contents to write into shared memory
 * @param block_index If multiple blocks are needed, shows the correct order of reconstruction
 * @param block_total Total blocks required to write the entire file
 */
MessageHeader createMessage (unsigned int block_id, size_t size, unsigned int block_index, unsigned int block_total) {
    
    MessageHeader message;
    message.mtype = getpid();
    message.size = size;
    message.block_id = block_id;
    message.block_index = block_index;
    message.block_total = block_total;

    return message;
}