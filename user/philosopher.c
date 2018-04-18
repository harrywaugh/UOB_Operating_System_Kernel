#include "philosopher.h"
#include <stdlib.h>

void philosopher() {
    bool hasLeft = false;
    bool hasRight = false;
    int pid = getpid();
    write( STDOUT_FILENO, "\nPhilospher 0 Started...\n", 25 );

    exit(pid);
}
