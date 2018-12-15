// #include "philosopher.h"
// #include <stdlib.h>
//
// void main_philosopher2() {
//     write( STDOUT_FILENO, "\nPhilospher 2 Started...\n", 25 );
//
//     //Create pipe and open
//     mkfifo ("3/pipe", 0240);
//     int writePipeFd = open ("3/pipe", O_WRONLY);
//
//     //Write to pipe
//     write( STDOUT_FILENO, "Philospher 2 Writing to pipe...", 32 );
//     write( writePipeFd, "phi2 data", 9);
//
//
//     char *readString = (char *)malloc((size_t)9);
//     int readPipeFd = -1;
//     while (readPipeFd == -1)  {
//         readPipeFd = open("4/pipe", O_RDONLY);
//         read(readPipeFd, readString, 9);
//     }
//     write(STDOUT_FILENO, readString, 9);
//
//     unlink("4/pipe");
//
//     exit( EXIT_SUCCESS );
// }
