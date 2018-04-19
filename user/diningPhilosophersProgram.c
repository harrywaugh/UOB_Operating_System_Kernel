#include "diningPhilosophersProgram.h"

#define PHILOSOPHERS 16

extern void philosopher();

void putStr( char* x, int n ) {
  for( int i = 0; i < n; i++ ) {
    PL011_putc( UART1, x[ i ], true );
  }
}


void main_philosophers_program() {
    pid_t philosophers[PHILOSOPHERS];
    /////////////////////////////////////FORK children
    for (int i = 0; i < PHILOSOPHERS; i++)  {
        philosophers[ i ] = fork();
        if ( philosophers[ i ] == 0 )  {  //SUCCESS
            exec(&philosopher);
        } else if ( philosophers[ i ] == -1) { //FAILURE
            putStr( "Execution attempt failed: No stack memory exception.\n", 54);
        }
    }

    exit( EXIT_SUCCESS );
}
