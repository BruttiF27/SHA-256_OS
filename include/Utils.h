# ifndef UTILS_H
# define UTILS_H

# define MEMORY_SIZE 65536
# define BLOCK_SIZE 4096
# define MEMORY_CLIENT_SECTION MEMORY_SIZE - BLOCK_SIZE
# define MEMORY_SERVER_SECTION BLOCK_SIZE

typedef struct MessageHeader {
    long mtype;                 // Contains message type, identifies client/server messages (value must be mtype > 0)
    unsigned int block_id;      // Keeps block id inside shared memory or block slot inside the result section (15th)
    unsigned int block_index;   // Keeps block index for specific request's data (default=0)
    unsigned int block_total;   // Keeps total blocks required for specific request's data (default=1)
    size_t size;                // Keeps message data size
} MessageHeader;

typedef struct Block {
    unsigned int usable;        // Tells if a block is usable for writing (0=no, 1=yes)
    size_t filled_size;         // Keeps track of how much memory of the block is used by the file contents
} Block;

# endif