# include "AdminClient.h"

/**
 * Must be run after server activation in order for it to work properly
 * 1. Parses a number provided in argv[1] and converts it to int
 * 2. Validates provided maximum process number
 * 3. Sends an update message to the server through request queue (mtype forced to 1) à la "best-effort"
 */
int main (int argc, char* argv[]) {
    if (usageCheck(argc) == -1) exit(1);
    
    int maxProcessNumber = requestValidation(argv[1]);
    if (maxProcessNumber == -1) exit(1);

    MessageQueueContext msqctx = {-1, -1};
    if (connectMessageQueue(&msqctx) == -1) exit(1);

    MessageHeader message = createAdminRequest(maxProcessNumber);
    if (sendMessage(msqctx.msqid_req, &message) == -1) {
        printf("Message could not be sent\n");
        exit(1);
    }

    printf("Maximum concurrent process value sent to server\n");
    exit(0);
}

/**
 * Checks if user has correctly inputted a correct argument number (2)
 * @param argc Number of provided CLI params
 * @return 0 on success, -1 on failure
 */
int usageCheck (int argc) {
    if (argc != 2) {
        printf("Usage: ./build/AdminClient <number>\n");
        return -1;
    }
    return 0;
}

/**
 * Validates the input provided by the user
 * @param argv String to be converted to int
 * @return parsed requested number on success, -1 on failure
 */
int requestValidation (char* argv) {
    int requestedNumber = atoi(argv);
    if (requestedNumber < 1 || requestedNumber > MAX_CHILD_PROCESSES) {
        printf("Illegal number inserted\n");
        return -1;
    }
    return requestedNumber;
}

/**
 * Fetches and connects to the queue required for IPC
 * @param context Message queue context
 * @return 0 on success, -1 on failure
 */
int connectMessageQueue (MessageQueueContext* context) {
    context->msqid_req = getMessageQueueID(KEY_REQUEST_QUEUE);
    if (context->msqid_req == -1) {
        printf("Could not connect to request queue\n");
        return -1;
    }

    return 0;
}

/**
 * Creates an admin request to edit max number of children processes
 * @praram Requested number of processes
 * @return Message to be sent
 */
MessageHeader createAdminRequest (int requestedValue) {
    MessageHeader message;
    message.mtype = ADMIN_MTYPE;
    message.size = requestedValue;
    message.block_id = 0;
    message.block_index = 0;
    message.block_total = 0;

    return message;
}