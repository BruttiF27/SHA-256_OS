# include "MessageQueueUtils.h"

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
 * Terminates the IPC message queues
 * @param context Message queue context
 */
void terminateMessageQueues (MessageQueueContext* context) {
    if (closeMessageQueue(context->msqid_req) == -1) printf("Could not close request message queue\n");
    if (closeMessageQueue(context->msqid_ret) == -1) printf("Could not close return message queue\n");
}

/**
 * Called by both client and server, instantiates the message queue based on the provided value
 * @param msqVal Value of the constant required for queue instantiation
 * @return message queue ID on success, -1 on failure
 */
int createMessageQueue (int msqVal) {
    key_t msgKey = ftok("src", msqVal);
    return msgget(msgKey, IPC_CREAT | IPC_EXCL | 0666);
}

/**
 * Called by both client and server, marks message queue for removal
 * @param msqid Message queue ID
 * @return 0 on success, -1 on failure
 */
int closeMessageQueue (int msqid) { return msgctl(msqid, IPC_RMID, NULL); }

/**
 * Called by both client and server, fetches the requested ID based on the provided value
 * @param msq_val Value of the constant required
 * @return message queue ID on success, -1 on failure
 */
int getMessageQueueID (int msq_val) {
    key_t msqKey = ftok("src", msq_val);
    return msgget(msqKey, 0666);
}

/**
 * Initializes fields of the request to be sent
 * @param block_id ID number corresponding to the block used inside shared memory
 * @param size Size of the contents to write into shared memory
 * @param block_index If multiple blocks are needed, shows the correct order of reconstruction
 * @param block_total Total blocks required to write the entire file
 */
MessageHeader createRequest (unsigned int block_id, size_t size, unsigned int block_index, unsigned int block_total) {
    MessageHeader message;
    message.mtype = getpid();
    message.size = size;
    message.block_id = block_id;
    message.block_index = block_index;
    message.block_total = block_total;

    return message;
}

/**
 * Initializes fields applied to message to be sent
 * @param mtype Message type
 * @param slot_id Slot used to write digest in return section
 * @param size Digest size
 * @return New message struct
 */
MessageHeader createReply (long mtype, unsigned int slot_id, size_t size) {
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
 * Sends a message to the requested queue, found by msqid. Contract requires mtype and msgsz to be
 * determined before calling this function.
 * @param msqid Message queue ID
 * @param msg Message to be sent
 * @return 0 on success, -1 on failure
 */
int sendMessage (int msqid, MessageHeader* msg) { return msgsnd(msqid, msg, getMessageSize(msg), 0); }

/**
 * Receives a message from the requested queue, found by msqid. Contract requires msgsz and msgtype
 * to be determined before calling this function.
 * @param msquid Message queue ID
 * @param msg Message to be received
 * @param msgtype Message selector
 * @param flags Flags required for msgrcv function
 * @return size of the message on success, -1 on failure
 */
ssize_t receiveMessage (int msqid, MessageHeader* msg, long msgtype, int flags) {
    return msgrcv(msqid, msg, getMessageSize(msg), msgtype, flags);
}

/**
 * Calculates a specific message's size
 * @param msg Requested message
 * @return Message size
 */
size_t getMessageSize (MessageHeader* msg) { return sizeof(MessageHeader) - sizeof(long); }