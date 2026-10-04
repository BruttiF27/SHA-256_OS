# include "MainClient_sha256.h"

/**
 * Must be run after the client activates for the software to work properly
 * 1. Connects to an existing shared memory segment, defined in the server.
 * 2. Fetches the filename from argv[1] and writes its contents inside a usable block of client shared memory section
 * 3. Sends a message to the server to advise him to fetch the written contents and waits for the answer
 * 4. Once the reply message is received, the converted data can be read from a slot inside the server shared memory section
 * 5. Print digest on stdout
 */
int main (int argc, char *argv[]) {
    if (usageCheck(argc) == -1) exit(1);

    int shmid;
    int* shmaddr;

    if (initialize(&shmid, &shmaddr) == -1) exit(1);
    if (terminate(shmaddr) == -1) exit(1);

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
 * Initializes the environment needed for IPC
 * @param shmid Shared memory ID
 * @param shmaddr Shared memory address
 */
int initialize (int* shmid, int** shmaddr) {
    *shmid = getSharedMemoryID();
    if (*shmid == -1) {
        printf("Shared memory was not found\n");
        return -1;
    }

    *shmaddr = attachToSharedMemory(*shmid);
    if (shmaddr == (void*)-1) {
        printf("Could not attach to shared memory\n");
        return -1;
    }
    return 0;
}

/**
 * Gracefully terminates the active environment
 * @param shmaddr Shared memory address
 */
int terminate (int* shmaddr) {
    if (detachFromSharedMemory(shmaddr) == -1) {
        printf("Could not detach from shared memory\n");
        return -1;
    }
    return 0;
}