#include "philosopher.h"
#include <stdlib.h>

void philosopher() {
    bool hasLeft = false;
    bool hasRight = false;
    int pid = getpid() - 3;
    char *pidStr;
    itoa(pidStr, pid);
    write( STDOUT_FILENO, "\nStarted Philospher ", 20 );
    write(STDOUT_FILENO, pidStr, 2);

    exit(pid);
}
