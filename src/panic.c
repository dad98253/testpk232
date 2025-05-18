#define _POSIX_SOURCE 1
#include <stdlib.h>
#include <stdio.h>

void panic(char *filename,int line)
{
     (void)fprintf(stderr,"\n?Panic in line %d of file %s\n"
                      ,line,filename);
     (void)perror("Unexpected library error");
     abort();
}

