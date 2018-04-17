#include "philosopher.h"
#include <stdlib.h>

void main_philosopher() {
    write( STDOUT_FILENO, "\nPhilospher 1 Started...\n", 25 );

    //Create pipe and open
    mkfifo ("4/pipe", 0240);
    int writePipeFd = open ("4/pipe", O_WRONLY);

    //Write to pipe
    write( STDOUT_FILENO, "Philospher 1 Writing to pipe...\n", 32 );
    write( writePipeFd, "\nphi1 data\n", 11);


    char *readString = (char *)malloc((size_t)11);
    int readPipeFd = -1;
    while (readPipeFd == -1)  {
        readPipeFd = open("3/pipe", O_RDONLY);
        read(readPipeFd, readString, 11);
    }
    write(STDOUT_FILENO, readString, 11);

    unlink("3/pipe");


    exit( EXIT_SUCCESS );
}
