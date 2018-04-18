#include "philosopher.h"
#include <stdlib.h>

void main_philosopher() {
    write( STDOUT_FILENO, "\nPhilospher 1 Started...\n", 25 );

    //Create pipe and open
    mkfifo ("4/pipe", 0240);
    int writePipeFd = open ("4/pipe", O_WRONLY);

    //Write to pipe
    write( STDOUT_FILENO, "Philospher 1 Writing to pipe...\n", 32 );
    write( writePipeFd, "phi1 data", 9);


    char *readString = (char *)malloc((size_t)9);
    int readPipeFd = -1;
    while (readPipeFd == -1)  {
        readPipeFd = open("3/pipe", O_RDONLY);
        read(readPipeFd, readString, 9);
    }
    write(STDOUT_FILENO, readString, 9);

    unlink("3/pipe");


    exit( EXIT_SUCCESS );
}
