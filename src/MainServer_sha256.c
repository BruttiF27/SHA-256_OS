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

    // Define behaviour when receiving CTRL+C
    signal(SIGINT, sigintHandler);

    // Initiate step 1
    SharedMemoryContext shmctx = {-1, NULL};
    if (initializeSharedMemory(&shmctx) == -1) exit(1);
    MessageQueueContext msqctx = {-1, -1};
    if (initializeMessageQueues(&msqctx) == -1) {
        terminateSharedMemory(&shmctx);
        exit(1);
    }

    MessageHeader message;
    do {
        // Initiate step 2
        if (receiveMessage(msqctx.msqid_req, &message, getMessageSize(&message), 0) == -1) {
            if (termination_flag == 1) break;
            else {
                printf("Message could not be received\n");
                sendReply(msqctx.msqid_ret, message.mtype, 0, 0);
                continue;
            }
        } else printf("Message received by the server. Mtype: %ld\n", msqctx.msqid_req, message.mtype);

        long mtype = message.mtype;
        unsigned char rawData[BLOCK_SIZE * message.block_total];
        ssize_t total = collectBytes(shmctx.sharedMemory, msqctx.msqid_req, &message, rawData);
        if (total == -1) {
            if (termination_flag == 1) break;
            printf("Could not collect request data\n");
            sendReply(msqctx.msqid_ret, mtype, 0, 0);
            continue;
        }

        // Initiate step 3
        unsigned char digest[DIGEST_SLOT];
        if (digestContent(digest, rawData, (size_t) total)) {
            printf("Could not digest data\n");
            sendReply(msqctx.msqid_ret, mtype, 0, 0);
            continue;
        }

        // Initiate step 4
        int slot_id;
        do {
            slot_id = writeReturnSharedMemory(shmctx.sharedMemory, digest, 65);
            if (slot_id == -1) sleep(1);
        } while (slot_id == -1 && termination_flag != 1);
        if (slot_id == -1) break;   // interrupted while waiting
        
        // Initiate step 5
        if (sendReply(msqctx.msqid_ret, mtype, slot_id, 65) == -1) printf("Message could not be sent\n");
        else printf("Message sent by the server. Mtype: %ld\n", mtype);
    } while (termination_flag != 1);
    
    terminateSharedMemory(&shmctx);
    terminateMessageQueues(&msqctx);

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
 * Wrapper to fetch all bytes, even if written in more than one block.
 * @param sharedMemory Shared memory
 * @param msqid_req Message queue ID of the request queue
 * @param first First message of the sequence
 * @param out Actual read data
 * @return Bytes read on success, -1 on failure
 */
ssize_t collectBytes (SharedMemory* sharedMemory, int msqid_req, MessageHeader* first, unsigned char* out) {
    long mtype = first->mtype;
    unsigned int total = first->block_total;

    if (total == 0 || total > REQUEST_BLOCKS) return -1;

    MessageHeader current = *first;
    size_t bytes = 0;

    for (unsigned int got = 0; got < total; got++) {
        // The first chunk is already in `current`; fetch the others, filtered by mtype
        if (got > 0) if (receiveMessage(msqid_req, &current, getMessageSize(&current), mtype) == -1) return -1;

        // block_index says where the chunk BELONGS, so it must be in range
        if (current.block_index >= total) return -1;
        unsigned char* dest = out + (size_t) current.block_index * BLOCK_SIZE;

        // block_id says where the chunk IS in shared memory
        ssize_t n = readRequestSharedMemory(sharedMemory, current.block_id, dest, current.size);
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
 * @return 0 on success, 1 on failure
 */
int sendReply (int msqid_ret, long mtype, unsigned int slot_id, size_t size) {
    MessageHeader reply = createMessage(mtype, slot_id, size);
    return sendMessage(msqid_ret, &reply, getMessageSize(&reply));
}

/**
 * Initializes fields applied to message to be sent
 * @param mtype Message type
 * @param slot_id Slot used to write digest in return section
 * @param size Digest size
 * @return New message struct
 */
MessageHeader createMessage (long mtype, unsigned int slot_id, size_t size) {
    
    MessageHeader message;

    message.mtype = mtype;
    message.block_id = slot_id;
    message.size = size;

    // Basically useless for the client, so give init values.
    message.block_index = 0;
    message.block_total = 1;

    return message;
}

/**
 * Signal handler for SIGINT, marks process for termination.
 * @param sigNum Signal number
 */
void sigintHandler (int sigNum) { termination_flag = 1; }