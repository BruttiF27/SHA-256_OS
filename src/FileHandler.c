# include "FileHandler.h"

/**
 * Reads a .txt file with steps of 4096B; assigns contents to a buffer and calculates file size.
 * @param fileDescriptor FD of the requested file
 * @param buffer Will hold the content that will be read.
 * @param blockCapacity Maximum size the function can read per iteration.
 * @return File size on success, -1 on failure.
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