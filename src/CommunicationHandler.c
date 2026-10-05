# include "CommunicationHandler.h"

/* ----- Shared memory functions ----- */

/**
 * Initializes the shared memory segment required for inter-process communication
 * @return Shared memory ID on success, -1 on failure
 */
int createSharedMemory () {
    key_t shmKey = ftok("src", KEY_SHARED_MEMORY);
    return shmget(shmKey, MEMORY_SIZE, IPC_CREAT | IPC_EXCL | 0666);
}

/**
 * Marks shared memory segment for deletion. It will be removed once every actor has disconnected from it.
 * @param shmid Shared memory ID
 * @return 0 if success, -1 if failure
 */
int closeSharedMemory (int shmid) { return shmctl(shmid, IPC_RMID, NULL); }

/**
 * Getter method to fetch currently existing shared memory
 * @return Shared memory ID on success, -1 on failure
 */
int getSharedMemoryID () {
    key_t shmKey = ftok("src", KEY_SHARED_MEMORY);
    return shmget(shmKey, MEMORY_SIZE, 0666);
}

/**
 * Attaches client to an existing shared memory
 * @param shmid Shared memory ID
 * @return Shared memory address on success, -1 on failure
 */
int* attachToSharedMemory (int shmid) { return (int*) shmat(shmid, NULL, 0); }

/**
 * Detaches client from an existing shared memory
 * @param shmaddr Shared memory address
 * @return 0 on success, -1 on failure
 */
int detachFromSharedMemory (int* shmaddr) { return shmdt(shmaddr); }

/* ----- Message queues functions ----- */

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
 * Sends a message to the requested queue, found by msqid. Contract requires mtype and msgsz to be
 * determined before calling this function.
 * @param msqid Message queue ID
 * @param msg Message to be sent
 * @param msgsz Message size
 * @return 0 on success, -1 on failure
 */
int sendMessage (int msqid, MessageHeader* msg, size_t msgsz) { return msgsnd(msqid, msg, msgsz, 0); }

/**
 * Receives a message from the requested queue, found by msqid. Contract requires msgsz and msgtype
 * to be determined before calling this function.
 * @param msquid Message queue ID
 * @param msg Message to be received
 * @param msgsz Message size
 * @param msgtype Message selector, keep to 0 for FCFS
 */
ssize_t receiveMessage (int msqid, MessageHeader* msg, size_t msgsz, long msgtype) {
    return msgrcv(msqid, msg, msgsz, msgtype, 0);
}

/**
 * Calculates a specific message's size
 * @param msg Requested message
 * @return Message size
 */
size_t getMessageSize (MessageHeader* msg) { return sizeof(MessageHeader) - sizeof(long); }

/**
 * Called by both client and server, fetches the requested ID based on the provided value
 * @param msqVal Value of the constant required
 * @return message queue ID on success, -1 on failure
 */
int getMessageQueueID (int msqVal) {
    key_t msqKey = ftok("src", msqVal);
    return msgget(msqKey, 0666);
}

/**
 * Called by both client and server, marks message queue for removal
 * @param msqid Message queue ID
 * @return 0 on success, -1 on failure
 */
int closeMessageQueue (int msqid) { return msgctl(msqid, IPC_RMID, NULL); }