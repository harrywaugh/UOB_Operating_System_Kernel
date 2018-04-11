#include "philosopher.h"
#include <stdlib.h>

void main_philosopher2() {
    write( STDOUT_FILENO, "Philospher Started...\n", 22 );
    int id;
    int count = 0;
    int pipeFd = open("4/pipe", O_RDONLY);

    write( STDOUT_FILENO, "Reading from pipe...", 20 );
    char *readString = (char *)malloc((size_t)9);
    read(pipeFd, readString, 9);
    write(STDOUT_FILENO, readString, 9);


    exit( EXIT_SUCCESS );
}
