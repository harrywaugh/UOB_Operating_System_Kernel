#include "philosopher.h"
#include <stdlib.h>

void philosopher() {
    bool hasForks = false;

    /////////////////Get PID and Print Init Message
    int pid = getpid();
    char *pidStr = (char*)malloc(2*sizeof(char));
    itoa(pidStr, pid-3);
    write( STDOUT_FILENO, "\nStarted Philospher ", 20 );
    write( STDOUT_FILENO, pidStr, 2);

    ///////////////Create Pipe Str
    char *pipeText = "/pipe";
    itoa(pidStr, pid );
    char *pipeStr = (char *)malloc(strlen(pipeText)+strlen(pidStr)+1);
    memcpy(pipeStr, pidStr, strlen(pidStr));
    memcpy(pipeStr + strlen(pidStr), pipeText, strlen(pipeText)+1);

    ///Open Read Pipe
    int readWaiterFd = -1;
    while(readWaiterFd == -1)  {
        readWaiterFd = open(pipeStr, O_RDONLY);
    }
    ///Open Write Pipe
    int writeWaiterFd = -1;
    while(writeWaiterFd == -1)  {
        writeWaiterFd = open("-1/waiterpipe", O_WRONLY);
    }


    itoa(pidStr, pid - 3);
    int state = 0;
    int *sig = (int*)malloc(sizeof(int));
    while(1)  {
        ///Read from pipe

        *sig = 0;
        if(read(readWaiterFd, sig, 1) > 0)  {   //Any replies?
            state = *sig;
        }


        if(state == 0)  {  //PHILOSOPHER THINKING -> Make eating request
            write( STDOUT_FILENO, "\nPhilosopher ", 13 );
            //itoa(pidStr, pid-3);
            write( STDOUT_FILENO, pidStr, 2 );
            write( STDOUT_FILENO, " Thinking\n", 10 );
            write(writeWaiterFd, &pid, sizeof(int));
            *sig = 1;
            write(writeWaiterFd, sig, (size_t)1);
            state = 2;
        } else if(state == 1) {           //PHILOSOPHER EATING  -> Put down fork
            write( STDOUT_FILENO, "\nPhilosopher ", 13 );
            //itoa(pidStr, pid-3);
            write( STDOUT_FILENO, pidStr, 2 );
            write( STDOUT_FILENO, " Eating\n", 8 );
            write(writeWaiterFd, &pid, sizeof(int));
            *sig = 0;
            write(writeWaiterFd, sig, (size_t)1);
            state = 0;
        }

    }

    exit(EXIT_SUCCESS);
}
