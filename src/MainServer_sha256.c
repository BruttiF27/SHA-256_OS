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

    // TODO Remove once client is able to send messages to server
    if (argc < 2) {
        printf("Usage: %s <string with no spaces>\n", argv[0]);
        exit(1);
    }

    // TODO Step 1
    int shmid;
    if (initialize(&shmid) == -1) exit(1);
    

    // TODO Step 2


    // TODO Step 3
    char* rawString = argv[1];
    char digestedString[65];
    
    if(digestContent(digestedString, rawString)) printf("Something's wrong\n");

    sleep(10); // TODO Remove this once you get the server active until a signal is sent

    // TODO Step 4

    // TODO Step 5
    printf("%s\n", digestedString);

    if (terminate(shmid) == -1) exit(1);

    return 0;
}

int initialize (int* shmid) {
    *shmid = createSharedMemory();
    if (*shmid == -1) {
        printf("Shared memory could not be created\n");
        return -1;
    }
    return 0;
}

int terminate (int shmid) {
    if (closeSharedMemory(shmid)) {
        printf("Shared memory could not be closed\n");
        return -1;
    }
    return 0;
}