# ifndef UTILS_H
# define UTILS_H

typedef struct MessageHeader {
    long mtype;                 // Contains message type, identifies client/server messages (value must be mtype > 0)
    unsigned int block_id;      // Keeps block id inside shared memory
    unsigned int block_index;   // Keeps block index for specific request's data (default=0)
    unsigned int block_total;   // Keeps total blocks required for specific request's data (default=1)
    size_t size;                // Keeps message data size
} MessageHeader;

# endif