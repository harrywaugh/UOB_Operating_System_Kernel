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
    int   whoHasFork[PHILOSOPHERS];

    /////////////////////////////////////FORK children
    for (int i = 0; i < PHILOSOPHERS; i++)  {
        whoHasFork[i] = -1;
        philosophers[ i ] = fork();
        if ( philosophers[ i ] == 0 )  {  //SUCCESS
            exec(&philosopher);
        } else if ( philosophers[ i ] == -1) { //FAILURE
            putStr( "Execution attempt failed: No stack memory exception.\n", 54);
        }
    }


    ///////////////////////////////////CREATE PIPES, OPEN THEM and get FDs
    char *pipeText = "/pipe";
    char *pipeStr;
    for (int i = 0; i < PHILOSOPHERS; i++)  {
        char *pipeNo;
        itoa(pipeNo, philosophers[i] );

        pipeStr = (char *)malloc(strlen(pipeText)+strlen(pipeNo)+1);
        memcpy(pipeStr, pipeNo, strlen(pipeNo));
        memcpy(pipeStr + strlen(pipeNo), pipeText, strlen(pipeText)+1);

        mkfifo (pipeStr, 0240);
        philosopherFds[i] = open (pipeStr, O_WRONLY);
    }
    /////////////////////////////CREATE PUBLIC PIPE
    mkfifo("-1/waiterpipe", 0402 );
    int waiterReadFd = open("-1/waiterpipe", O_RDONLY);


    int *requestingPid = (int*)malloc(sizeof(int));
    while (1)  {
        *requestingPid = 0;
        if(read(waiterReadFd, requestingPid, sizeof(int) ) > 0)  { //READ PID
            int *sig = (int *)malloc(sizeof(int));
            *sig = 0;
            if(read(waiterReadFd, sig, 1) > 0)  {
                if(*sig == 1)  {  //REQUESTING PID WANTS TO PICK UP FORKS
                    if(whoHasFork[(*requestingPid-3 + PHILOSOPHERS) % PHILOSOPHERS] == -1 &&
                       whoHasFork[(*requestingPid-2 + PHILOSOPHERS) % PHILOSOPHERS] == -1)  {
                        *sig = 1;  //YES
                        whoHasFork[(*requestingPid-3 + PHILOSOPHERS) % PHILOSOPHERS] = *requestingPid;
                        whoHasFork[(*requestingPid-2 + PHILOSOPHERS) % PHILOSOPHERS] = *requestingPid;
                    } else {
                        *sig = 0;  //NO
                    }
                    write(philosopherFds[(*requestingPid-2+PHILOSOPHERS)%PHILOSOPHERS], sig, 1);
                } else if(*sig == 0)  {  //REQUESTING PID WANTS TO PUT DOWN FORKS
                    whoHasFork[(*requestingPid-3 + PHILOSOPHERS) % PHILOSOPHERS] = -1;
                    whoHasFork[(*requestingPid-2 + PHILOSOPHERS) % PHILOSOPHERS] = -1;
                }
            }
            free(sig);
        }
    }

    exit( EXIT_SUCCESS );
}
