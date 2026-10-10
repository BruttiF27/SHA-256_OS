# ifndef MESSAGEQUEUEUTILS_H
# define MESSAGEQUEUEUTILS_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/msg.h>
# include <openssl/sha.h>

# define KEY_REQUEST_QUEUE 28
# define KEY_RETURN_QUEUE 29

typedef struct MessageHeader {
    long mtype;                 // Identifies client/server messages (value must be mtype > 0)
    unsigned int block_id;      // Keeps block id inside shared memory or block slot inside the result section (15th)
    unsigned int block_index;   // Keeps block index for specific request's data (default=0)
    unsigned int block_total;   // Keeps total blocks required for specific request's data (default=1)
    size_t size;                // Keeps message data size
} MessageHeader;

typedef struct MessageQueueContext {
    int msqid_req;
    int msqid_ret;
} MessageQueueContext;

int createMessageQueue (int);
int sendMessage (int, MessageHeader*, size_t);
ssize_t receiveMessage (int, MessageHeader*, size_t, long);
size_t getMessageSize (MessageHeader*);
int getMessageQueueID (int);
int closeMessageQueue (int);

# endif