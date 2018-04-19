#include "philosopher.h"
#include <stdlib.h>

void philosopher() {
    bool hasLeft = false;
    bool hasRight = false;

    /////////////////Get PID and Print Init Message
    int pid = getpid();
    char *pidStr;
    itoa(pidStr, pid-3);
    write( STDOUT_FILENO, "\nStarted Philospher ", 20 );
    write( STDOUT_FILENO, pidStr, 2);

    ///////////////Create Pipe Str
    char *pipeText = "/pipe";
    itoa(pidStr, pid );
    char *pipeStr = (char *)malloc(strlen(pipeText)+strlen(pidStr)+1);
    memcpy(pipeStr, pidStr, strlen(pidStr));
    memcpy(pipeStr + strlen(pidStr), pipeText, strlen(pipeText)+1);

    ///Open Pipe
    int readWaiterFd = -1;
    while(readWaiterFd == -1)  {
        readWaiterFd = open(pipeStr, O_RDONLY);
    }

    ///Read from pipe
    int *readSig = (int*)malloc(sizeof(int));
    *readSig = 0;
    read(readWaiterFd, readSig, sizeof(int));

    //Print pipe signal
    char* readSigStr;
    itoa(readSigStr, *readSig);
    write( STDOUT_FILENO, readSigStr, 2);

    exit(EXIT_SUCCESS);
}
