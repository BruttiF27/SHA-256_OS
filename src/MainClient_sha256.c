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

    int shmid, msqid_req, msqid_ret;
    int* shmaddr;

    if (connectToSharedMemory(&shmid, &shmaddr) == -1 || findMessageQueues(&msqid_req, &msqid_ret) == -1) exit(1);


    // TODO Step 2

    // TODO Step 3
    MessageHeader message = createMessage();
    if (sendMessage(msqid_req, &message, getMessageSize(&message)) == -1) printf("Message could not be sent\n");
    else printf("Message sent by the client. Queue ID: %d, mtype: %d\n", msqid_req, message.mtype);

    // TODO Step 4
    if (receiveMessage(msqid_ret, &message, getMessageSize(&message), 0) == -1) printf("Message could not be received\n");
    else printf("Message received by the client. Queue ID: %d, mtype: %d\n", msqid_ret, message.mtype);

    // TODO Step 5

    if (disconnectFromSharedMemory(shmaddr) == -1) exit(1);

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
 * @param shmid Shared memory ID
 * @param shmaddr Shared memory address
 * @return 0 on success, -1 on failure
 */
int connectToSharedMemory (int* shmid, int** shmaddr) {
    int result = 0;
    
    *shmid = getSharedMemoryID();
    if (*shmid == -1) {
        printf("Shared memory was not found\n");
        result = -1;
    }

    *shmaddr = attachToSharedMemory(*shmid);
    if (shmaddr == (void*)-1) {
        printf("Could not attach to shared memory\n");
        result = -1;
    }

    if (result == -1) disconnectFromSharedMemory(*shmaddr);
    return result;
}

/**
 * Fetches the queues required for IPC
 * @param msqid_req Request message queue ID
 * @param msqid_ret Return message queue ID
 * @return 0 on success, -1 on failure
 */
int findMessageQueues (int* msqid_req, int* msqid_ret) {
    int result = 0;
    *msqid_req = getMessageQueueID(KEY_REQUEST_QUEUE);
    if (*msqid_req == -1) {
        printf("Request message queue was not found\n");
        result = -1;
    }

    *msqid_ret = getMessageQueueID(KEY_RETURN_QUEUE);
    if (*msqid_ret == -1) {
        printf("Return message queue was not found\n");
        result = -1;
    }
    return result;
}

/**
 * Gracefully terminates attachment to shared memory
 * @param shmaddr Shared memory address
 * @return 0 on success, -1 on failure
 */
int disconnectFromSharedMemory (int* shmaddr) {
    if (detachFromSharedMemory(shmaddr) == -1) {
        printf("Could not detach from shared memory\n");
        return -1;
    }
    return 0;
}

/**
 * Initializes fields applied to message to be sent
 */
MessageHeader createMessage (/*unsigned int block_id, size_t size, unsigned int block_index, unsigned int block_total*/) {
    MessageHeader message;
    
    // use getpid() and give it to mtype
    message.mtype = getpid();
    
    // calc message file size

    // message.block_id = block_id;
    // message.size = size;
    // message.block_index = block_index;
    // message.block_total = block_total;

    return message;
}