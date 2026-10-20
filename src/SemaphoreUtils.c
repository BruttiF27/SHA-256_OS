# include "SemaphoreUtils.h"

/**
 * Initializes the semaphores needed for shared memory protection
 * @param semID Semaphore set ID
 * @return 0 on success, -1 on failure
 */
int initializeSemaphores (int* semID) {
    *semID = createSemaphoreSet();
    if (*semID == -1) { printf("Semaphore set could not be created\n"); return -1; }

    if (initializeSemaphoreSet((*semID)) == -1) {
        printf("Semaphores could not be initialized\n");
        removeSemaphoreSet((*semID));
        *semID = -1;
        return -1;
    }

    return 0;
}

/**
 * Terminates the semaphores
 * @param semID Semaphore set ID
 */
void terminateSemaphores (int* semID) { if (removeSemaphoreSet((*semID)) == -1) printf("Semaphore set could not be removed\n"); }

/**
 * Instantiates the two semaphores to handle concurrent requests and returns
 * @return Semaphore set ID on success, -1 on failure
 */
int createSemaphoreSet () {
    key_t semKey = ftok("src", KEY_SEMAPHORE_SET);
    return semget(semKey, SEM_TOTAL, IPC_CREAT | IPC_EXCL | 0666);
}

/**
 * Removes semaphore set
 * @param semID Semaphore set ID
 * @return 0 on success, -1 on failure
 */
int removeSemaphoreSet (int semID) { return semctl(semID, 0, IPC_RMID); }

/**
 * Initializes all semaphore values inside the set
 * @param semID Semaphore set ID
 * @return 0 on success, -1 on failure
 */
int initializeSemaphoreSet (int semID) {
    union semun arg;
    for (int i = 0; i < SEM_TOTAL; i++) {
        if (i == SEM_REQUEST_FREE) arg.val = REQUEST_BLOCKS;
        else if (i == SEM_RETURN_FREE) arg.val = RESULT_SLOTS;
        else arg.val = 1;
        
        if (semctl(semID, i, SETVAL, arg) == -1) return -1;
    }
    return 0;
}

/**
 * Getter function to fetch semaphore set ID
 * @return Semaphore set ID on success, -1 on failure
 */
int getSemaphoreSetID () {
    key_t semKey = ftok("src", KEY_SEMAPHORE_SET);
    return semget(semKey, SEM_TOTAL, 0666);
}

/**
 * Generic wrapper helper function for semop
 * @param semID Semaphore set ID
 * @param semNum Specific semaphore number
 * @param valueChange Value added to the semaphore (-1 to wait, +1 to post)
 * @param flags Other flags required by the operation
 * @return 0 on success, -1 on failure
 */
int semaphoreOperation (int semID, int semNum, int valueChange, int flags) {
    struct sembuf op;
    op.sem_num = semNum;
    op.sem_op = valueChange;
    op.sem_flg = flags;
    return semop(semID, &op, 1);
}

/**
 * Waits on a semaphore: decrements it, blocking while its value is 0
 * @param semID Semaphore set id
 * @param semNum Specific semaphore number
 * @return 0 on success, -1 on failure (errno == EINTR if interrupted by a signal)
 */
int semaphoreWait (int semID, int semNum) { return semaphoreOperation(semID, semNum, -1, 0); }

/**
 * Like semaphoreWait, but never blocks
 * @param semID Semaphore set id
 * @param semNum Specific semaphore number
 * @return 0 on success, -1 on failure (errno == EAGAIN if the semaphore was unavailable)
 */
int semaphoreTryWait (int semID, int semNum) { return semaphoreOperation(semID, semNum, -1, IPC_NOWAIT); }

/**
 * Releases a semaphore: increments it, which wakes one waiting process if there is at least one in the queue
 * @param semID Semaphore set id
 * @param semNum Specific semaphore number
 * @return 0 on success, -1 on failure
 */
int semaphorePost (int semID, int semNum) { return semaphoreOperation(semID, semNum, 1, 0); }