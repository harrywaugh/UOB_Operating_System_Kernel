#include "waiter.h"
#include <stdlib.h>

#define PHILOSOPHERS 16

extern void philosopher();

void putStr( char* x, int n ) {
  for( int i = 0; i < n; i++ ) {
    PL011_putc( UART1, x[ i ], true );
  }
}


void waiter() {
    pid_t philosophers[PHILOSOPHERS];
    int   philosopherFds[PHILOSOPHERS];

    /////////////////////////////////////FORK children
    for (int i = 0; i < PHILOSOPHERS; i++)  {
        philosophers[ i ] = fork();
        if ( philosophers[ i ] == 0 )  {  //SUCCESS
            exec(&philosopher);
        } else if ( philosophers[ i ] == -1) { //FAILURE
            putStr( "Execution attempt failed: No stack memory exception.\n", 54);
        }
    }


    ///////////////////////////////////CREATE PIPES AND OPEN THEM
    char *pipeText = "/pipe";
    for (int i = 0; i < PHILOSOPHERS; i++)  {
        char *pipeNo;
        itoa(pipeNo, philosophers[i] );

        char *pipeStr = (char *)malloc(strlen(pipeText)+strlen(pipeNo)+1);
        memcpy(pipeStr, pipeNo, strlen(pipeNo));
        memcpy(pipeStr + strlen(pipeNo), pipeText, strlen(pipeText)+1);

        mkfifo (pipeStr, 0240);
        philosopherFds[i] = open (pipeStr, O_WRONLY);
        int *sig;
        *sig = 1;
        write(philosopherFds[i], sig, sizeof(int));
    }

    exit( EXIT_SUCCESS );
}
