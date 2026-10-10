# ifndef FILEHANDLER_H
# define FILEHANDLER_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>

ssize_t readChunk (int, unsigned char*, int);

# endif