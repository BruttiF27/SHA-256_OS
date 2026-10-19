# ifndef JOBSCHEDULER_H
# define JOBSCHEDULER_H

# include <stdlib.h>

# define BACKLOG_SIZE 64

typedef struct Job {
    long mtype;                // who to reply to
    size_t size;               // Size of the job in bytes, used as a key for SJF
    unsigned char* data;       // Firstly malloc-ed, owned by the parent until forked, when it'll be freed.
} Job;

Job backlog[BACKLOG_SIZE];
int backlogCount = 0;

int addJob (int, size_t, char*);
int findShortestJob ();
void removeJob (int);

# endif