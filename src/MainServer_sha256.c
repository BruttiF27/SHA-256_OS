# include "MainServer_sha256.h"

/**
 * Must be run first in order for the software to work properly
 * 1. Creates shared memory segment and waits for requests, sent through message queues
 * 2. Upon receiving a message, reads the requested block where the client wrote file data
 * 3. Data is then converted
 * 4. Writes converted data on a usable block in its half of the memory
 * 5. Sends message to client that the data can be fetched from shared memory
 */
int main (int argc, char *argv[]) {

    // Define behaviour when receiving CTRL+C
    signal(SIGINT, sigintHandler);

    // TODO Remove once client is able to send messages to server
    if (argc < 2) {
        printf("Usage: %s <string with no spaces>\n", argv[0]);
        exit(1);
    }

    // TODO Step 1
    int shmid, msqid_req, msqid_ret;
    MessageHeader message;
    if (initializeSharedMemory(&shmid) == -1 || initializeMessageQueues(&msqid_req, &msqid_ret) == -1) exit(1);

    // TODO This is temporary handling, needs to be removed once server reads from memory
    char* rawString = argv[1];
    char digestedString[65];

    do {
        // TODO Step 2
        if (receiveMessage(msqid_req, &message, getMessageSize(&message), 0) == -1) {
            if (termination_flag == 1) break;
            else printf("Message could not be received\n");
        } else printf("Message received by the server. Queue ID: %d, mtype: %d\n", msqid_req, message.mtype);

        // TODO Step 3
        if (digestContent(digestedString, rawString)) printf("Something's wrong\n");

        // TODO Step 4

        // TODO Step 5
        printf("%s\n", digestedString);
        if (sendMessage(msqid_ret, &message, getMessageSize(&message)) == -1) printf("Message could not be sent\n");
        else printf("Message sent by the server. Queue ID: %d, mtype: %d\n", msqid_ret, message.mtype);
        
    } while (termination_flag != 1);
    
    terminateSharedMemory(shmid);
    terminateMessageQueues(msqid_req, msqid_ret);

    return 0;
}

/**
 * Initializes the shared memory environment required for IPC
 * @param shmid Shared memory ID
 * @return 0 on success, -1 on failure
 */
int initializeSharedMemory (int* shmid) {
    int result = 0;
    *shmid = createSharedMemory();
    if (*shmid == -1) {
        printf("Shared memory could not be created\n");
        result = -1;
    }

    if (result == -1) terminateSharedMemory(*shmid);
    return result;
}

/**
 * Initializess the message queues environment needed for IPC
 * @param msqid_req Request message queue ID
 * @param msqid_ret Return message queue ID
 * @return 0 on success, -1 on failure
 */
int initializeMessageQueues (int* msqid_req, int* msqid_ret) {
    int result = 0;
    *msqid_req = createMessageQueue(KEY_REQUEST_QUEUE);
    if (*msqid_req == -1) {
        printf("Request message queue could not be created\n");
        result = -1;
    }

    *msqid_ret = createMessageQueue(KEY_RETURN_QUEUE);
    if (*msqid_ret == -1) {
        printf("Return message queue could not be created\n");
        result = -1;
    }

    if (result == -1) terminateMessageQueues(*msqid_req, *msqid_ret);
    return result;
}

/**
 * Terminates the IPC shared memory environment
 * @param shmid Shared memory ID
 */
void terminateSharedMemory (int shmid) { if (closeSharedMemory(shmid) == -1) printf("Shared memory could not be closed\n"); }

/**
 * Terminates the IPC message queues
 * @param msqid_req Request message queue ID
 * @param msqid_ret Return message queue ID
 */
void terminateMessageQueues (int msqid_req, int msqid_ret) {
    if (closeMessageQueue(msqid_req) == -1) printf("Could not close request message queue\n");
    if (closeMessageQueue(msqid_ret) == -1) printf("Could not close return message queue\n");
}

/**
 * Initializes fields applied to message to be sent
 */
MessageHeader createMessage (long mtype/*unsigned int block_id, size_t size, unsigned int block_index, unsigned int block_total*/) {
    MessageHeader message;
    
    // Re-use sender message's mtype to discern requests
    message.mtype = mtype;
    
    // calc message file size

    // message.block_id = block_id; // Block ID is used for slots in block #15
    // message.size = size;
    // message.block_index = block_index;
    // message.block_total = block_total;

    return message;
}

/**
 * Signal handler for SIGINT, marks process for termination.
 * @param sigNum Signal number
 */
void sigintHandler (int sigNum) { termination_flag = 1; }