#include "diningPhilosophersProgram.h"

#define PHILOSOPHERS 2

extern void philosopher();

void putStr( char* x, int n ) {
  for( int i = 0; i < n; i++ ) {
    PL011_putc( UART1, x[ i ], true );
  }
}


void main_philosophers_program() {
    pid_t philosophers[PHILOSOPHERS];
    /////////////////////////////////////FORK child 1
    philosophers[ 0 ] = fork();
    if ( philosophers[ 0 ] == 0 )  {  //SUCCESS

        exec(&philosopher);
    } else if ( philosophers[ 0 ] == -1) { //FAILURE
        putStr( "Execution attempt failed: No stack memory exception.\n", 54);
    }
    exit( EXIT_SUCCESS );
}
