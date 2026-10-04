# ifndef MAINSERVER_SHA256_H
# define MAINSERVER_SHA256_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/shm.h>
# include <openssl/sha.h>

# include "Converter_sha256.h"
# include "CommunicationHandler.h"
# include "FileHandler.h"
# include "Utils.h"

int initialize (int*);
int terminate (int);

# endif