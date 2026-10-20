# ifndef JOBSCHEDULER_H
# define JOBSCHEDULER_H

# include <stdlib.h>
# include <signal.h>

# include "SharedMemoryUtils.h"
# include "MessageQueueUtils.h"
# include "Converter_sha256.h"

# define BACKLOG_SIZE 64

typedef struct Job {
    long mtype;                // who to reply to
    size_t size;               // Size of the job in bytes, used as a key for SJF
    unsigned char* data;       // Firstly malloc-ed, owned by the parent until forked, when it'll be freed.
} Job;

Job backlog[BACKLOG_SIZE];
int backlogCount = 0;

volatile __sig_atomic_t activeChildren = 0;

int addJob (int, size_t, char*);
int findShortestJob ();
void removeJob (int);

void dispatchJob (Job*, SharedMemory*, MessageQueueContext*, int);
void initializeSignalSet (sigset_t*, sigset_t*);
void signalError (MessageQueueContext*, Job*, char*);

# endif