#include "philosopher.h"


void main_philosopher() {
    write( STDOUT_FILENO, "Philospher Started...\n", 22 );
    int id;
    int count = 0;
    mkfifo ("My first pipe", 0666);
    while( 1 ) {
        count++;
    }

    exit( EXIT_SUCCESS );
}
