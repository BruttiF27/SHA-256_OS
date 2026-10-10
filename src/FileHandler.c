# include "FileHandler.h"

/**
 * Returns the contents read from a file
 * @param fileName Path to the requested file
 * @param buffer Contains the content that is to be read.
 * @return File size
 */
ssize_t readChunk (int fileDescriptor, unsigned char* buffer, int blockCapacity) {
    
    size_t fileSize = 0;
    ssize_t bytesRead;

    while (fileSize < blockCapacity && (bytesRead = read(fileDescriptor, buffer+fileSize, blockCapacity-fileSize)) > 0) {
        fileSize += bytesRead;
    }

    if (bytesRead == -1) {
        printf("Read function failed\n");
        return -1;
    }

    return fileSize;
}