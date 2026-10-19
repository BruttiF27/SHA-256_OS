# include "JobScheduler.h"

/**
 * Adds a job to a the backlog array if there's a free cell.
 * @param mtype Message type
 * @param size Data size
 * @param data Actual data of the job
 * @return 0 on success, -1 on failure
 */
int addJob (int mtype, size_t size, char* data) {
    if (backlogCount >= BACKLOG_SIZE) return -1;
    backlog[backlogCount].mtype = mtype;
    backlog[backlogCount].size  = size;
    backlog[backlogCount].data  = data;
    backlogCount++;
    return 0;
}

/**
 * Returns the index of the job with the smallest size
 * @return shortest job index on success, -1 on empty array
 */
int findShortestJob () {
    int best = -1;
    for (int i = 0; i < backlogCount; i++) {
        if (best == -1 || backlog[i].size < backlog[best].size) best = i;
    }
    return best;
}

/**
 * Removes a job from the backlog
 * @param bestIndex index of the job that was chosen from the backlog
 */
void removeJob (int bestIndex) {
    backlog[bestIndex] = backlog[backlogCount-1];
    backlogCount--;
}