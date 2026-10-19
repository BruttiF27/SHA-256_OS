# ifndef ADMINCLIENT_H
# define ADMINCLIENT_H

# include <stdio.h>
# include <stdlib.h>

# include "MessageQueueUtils.h"

# define MAX_CHILD_PROCESSES 64

int usageCheck (int);
int requestValidation (char*);
int connectMessageQueue (MessageQueueContext*);
MessageHeader createAdminRequest (int);

# endif