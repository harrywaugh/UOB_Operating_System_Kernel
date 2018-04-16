/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"
#define STACKS 18
#define QUEUENO 4

extern void      main_console();
extern uint32_t  tos_P;

int curr_pid = 1;
int currFd   = 3;

bool fullStacks[STACKS];
pipe_t **pipes;
int pipesLength = 0;

queue_t *queue;
pcb_t *curr_prog;

bool terminateProgram(pid_t pid) {
    node_t *curr_node = queue->head;
    while ( curr_node != NULL )  {
        pcb_t *pcb_temp = (pcb_t *)curr_node->item;
        if (pcb_temp->pid == pid)  {
            pcb_temp->status = STATUS_TERMINATED;
            return true;
        }
        curr_node = curr_node->previous;
    }
    return false;
}

bool checkPermissions(int user, pipe_t *pipe, int flags)  {
    int userCatergory = 0;
    if(curr_prog->pid == pipe->owner)  {
        userCatergory = 2;
    } else if(curr_prog->pid == pipe->group)  {
        userCatergory = 1;
    }
    if(((pipe->mode >> 3*userCatergory) & flags) != 0) return true;
    return false;
}

int getFreeStack()  {
    int i = 0;
    while ( fullStacks[ i ] ) i++;
    if ( i < STACKS )  {
        fullStacks[ i ] = true;
        return i;
    }
    return -1;
}
bool memStackAvailable()  {
    for ( int i = 0; i < STACKS ; i++ )
        if (!fullStacks[i])  return true;
    return false;
}
uint32_t *getStackAddress(int id)  {
    return (uint32_t*)(&tos_P - (0x00000400 * id));
}

void allocateNewPipe( char *name, int mode)  {
    if(pipesLength != 0)  {
        void *p = realloc(pipes, sizeof(pipe_t *) * (pipesLength+1));
        if (p)    pipes = p;
    }
    pipes[ pipesLength] = (pipe_t *) malloc(sizeof(pipe_t));
    char *fname = (char *)malloc(sizeof(name));
    memcpy(fname, name, strlen(name)+1);
    *(pipes[ pipesLength]) = (pipe_t){newQueue((size_t)1), name, currFd++, curr_prog->pid, NULL, mode};
    pipes[ pipesLength ]->group = atoi(strtok( fname, "/" ));
    pipesLength++;

}
int getPipeFromName(char *name)  {
    for (int i = 0; i < pipesLength; i++)  {
        if(strcmp(name, pipes[ i ]->name) == 0)  return i;
    }
    return -1;
}
int deallocatePipe(char *name)  {
    int pipeId = getPipeFromName(name);
    if (pipeId == -1)  return -1;
    else if ( pipesLength == 1 + pipeId )  {
        free(pipes[pipeId]);
    }
    else {
        free(pipes[pipeId]);
        memcpy(pipes[pipeId], pipes[pipesLength-1], sizeof(pipe_t));
    }
    void *p = realloc(pipes, sizeof(pipe_t *) * (pipesLength-1));
    if (p)    pipes = p;
    pipesLength--;
    return 0;
}
int openPipe( char *name, int flags)  {
    int pipeId = getPipeFromName(name);
    if (pipeId != -1 && !checkPermissions(curr_prog->pid, pipes[ pipeId ], flags ))  {
        pipeId = -1;
    }
    return pipeId;
}
int getPipeFromFd(int fd)  {
    for(int i = 0; i < pipesLength; i++)  {
        if(pipes[i]->fd == fd) return i;
    }
    return -1;
}
bool checkValidPipeName(char *name)  {
    for(int i = 0; i < pipesLength; i++) {
        if(strcmp(name, pipes[ i ]->name) == 0)  return false;
    }
    return true;
}

int writeBytesToQueue(queue_t *q, void *x, int n)  {
    for(int i = 0; i < n; i++)  {
        push(q, x++);
    }
    return n;
}
int readBytesFromQueue(queue_t *q, void *x, int n)  {
    for(int i = 0; i < n; i++)  {
        if(!pop(q, x++))  return i;
    }
    return n;
}

pcb_t *create_process(int id, void *main_fn)  {
    pcb_t *pcb = (pcb_t *)malloc(sizeof(pcb_t));
    memset( pcb, 0, sizeof( pcb_t ) );
    pcb->pid      = id;
    pcb->priority = 0;
    pcb->status   = STATUS_READY;
    pcb->ctx.cpsr = 0x50;
    pcb->stack_id = getFreeStack();
    pcb->ctx.sp   = ( uint32_t )( getStackAddress(pcb->stack_id) );
    pcb->ctx.pc   = ( uint32_t )( main_fn  );
    pcb->queue    = 0;
    return pcb;
}

void put_str( char* str )  {
    for (int i = 0; str[i] != '\0'; i++ )  PL011_putc( UART0, str[i], true);
    return;
}
int min (int a, int b)  {
    return (a < b) ? a : b;
}

// int activeQueue()  {
//     int i = 0;
//     while ( i < QUEUENO && isEmpty(queues[ i++ ]));
//
//     return i-1;
// }

void scheduler( ctx_t* ctx ) {
    memcpy( &curr_prog->ctx, ctx, sizeof( ctx_t ) );
    if(curr_prog->status != STATUS_TERMINATED){
        curr_prog->status = STATUS_READY;
        push(queue, curr_prog);
    }
    pop(queue, curr_prog);

    while (curr_prog->status != STATUS_READY)  {
        if ( curr_prog->status == STATUS_TERMINATED )  {
            fullStacks[curr_prog->stack_id] = false;
        } else {
            push( queue, curr_prog);
        }
        pop(  queue, curr_prog);
    }
    memcpy( ctx, &curr_prog->ctx, sizeof( ctx_t ) );                              // Restore new current program
    curr_prog->status = STATUS_EXECUTING;

    return;
}

void hilevel_handler_rst( ctx_t* ctx              ) {
    queue = newQueue(sizeof(pcb_t));
    curr_prog = create_process( curr_pid++, &main_console);
    memcpy( ctx, &curr_prog->ctx, sizeof( ctx_t ) );
    curr_prog->status = STATUS_EXECUTING;

    pipes = (pipe_t **)malloc(pipesLength * sizeof(pipe_t *));  //Allocate memory for pointer to array of pipe pointers



    TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
    TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
    TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
    TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
    TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

    GICC0->PMR          = 0x000000F0; // unmask all            interrupts
    GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
    GICC0->CTLR         = 0x00000001; // enable GIC interface
    GICD0->CTLR         = 0x00000001; // enable GIC distributor

    int_enable_irq();

    return;
}

void hilevel_handler_irq( ctx_t* ctx) {

  // Step 2: read  the interrupt identifier so we know the source.

    uint32_t id = GICC0->IAR;

    if( id == GIC_SOURCE_TIMER0 ) {
        scheduler( ctx );
        TIMER0->Timer1IntClr = 0x01;
    }

    GICC0->EOIR = id;
}

void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) {
  /* Based on the identified encoded as an immediate operand in the
   * instruction,
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */

      switch( id ) {
        case 0x01 : { // 0x01 => write( fd, x, n )
          int   fd = ( int   )( ctx->gpr[ 0 ] );
          int    n = ( int   )( ctx->gpr[ 2 ] );

          if( fd == 1)  {   //STDOUT_FILENO
              char*  x = ( char* )( ctx->gpr[ 1 ] );
              for( int i = 0; i < n; i++ ) {
                PL011_putc( UART0, *x++, true );
              }
              ctx->gpr[ 0 ] = n;
          } else {
              void*  x = ( void* )( ctx->gpr[ 1 ] );
              int pipeId = getPipeFromFd(fd);
              ctx->gpr [ 0 ] = pipeId == -1 ?
                                         -1 :
                                         writeBytesToQueue(pipes[ pipeId ]->queue, x, n);

          }
          break;
        }
        case 0x02 : { //Read
            int   fd = ( int   )( ctx->gpr[ 0 ] );
            int    n = ( int   )( ctx->gpr[ 2 ] );

            if(fd > 2)  {
                void *x = (void *) (ctx->gpr[ 1 ]);
                int pipeId = getPipeFromFd(fd);
                ctx->gpr [ 0 ] = pipeId == -1 ?
                                           -1 :
                                           readBytesFromQueue(pipes[ pipeId ]->queue, x, n);
            }
            break;
        }
        case 0x03 : { //Fork
            if ( memStackAvailable() )  {
                pcb_t *child = create_process(curr_pid, &main_console);  // Create process with new pid, and give it the console fn to run. Stack = NULL
                memcpy(&child->ctx, ctx, sizeof(ctx_t));                                                       // Copy context from console to new program.
                uint32_t *new_stack = (uint32_t *) (getStackAddress(child->stack_id) - 0x00000400);            // Get new stack location (Points to top)
                uint32_t *console_stack = (uint32_t *) (getStackAddress(curr_prog->stack_id) - 0x00000400);    // Get console stack location (Points to top)
                memcpy(new_stack, console_stack, 0x00001000);                                     // Copy stack from console to new program
                child->ctx.sp -= 0x00001000 * (child->stack_id - curr_prog->stack_id);  // Update new sp so it points to old stack location but in own stack
                child->ctx.gpr[ 0 ] = 0;                                                   // Set child return to 0.
                ctx->gpr[ 0 ] = curr_pid++;                                                // Set parent return to child pid.
                push(queue, child);                                                        // Add child to process queue.
                free(child);
            } else {
                ctx->gpr[ 0 ] = -1;
            }

            break;
        }
        case 0x04 : { //Exit
            curr_prog->status = STATUS_TERMINATED;
            put_str("\nProgram finished.\n\0");
            scheduler(ctx);
            break;
        }
        case 0x05 : { //Exec
            void *main_fn = ( void * )( ctx->gpr[ 0 ]);                                // Get new main fn for program
            uint32_t *stack = (uint32_t *) (getStackAddress(curr_prog->stack_id));     // Get stack location (Top)
            ctx->sp = (uint32_t) stack;                                                // Set sp to start from top of stack
            memset(stack - 0x00000400, 0, 0x00001000);                                              // Clear new programs stack
            ctx->pc = (uint32_t) main_fn;                                              // Update pc to beggining of new program
            memcpy(&curr_prog->ctx, ctx, sizeof(ctx_t));                               // Copy over ctx
            break;
        }
        case 0x06 : { //Kill
            pid_t pid = ctx->gpr[ 0 ];
            ctx->gpr[ 0 ] = (int32_t) terminateProgram(pid);
            break;
        }
        case 0x08 : { //Mkfifo
            char *name = (char *) ctx->gpr[ 0 ];
            int mode   = (int   ) ctx->gpr[ 1 ];
            if(checkValidPipeName(name))  {
                allocateNewPipe(name, mode);
                ctx->gpr[ 0 ] = 0;
            } else {
                ctx->gpr[ 0 ] = -1;
            }
            break;
        }
        case 0x0a : { //Open
            char *name = (char *) ctx->gpr[ 0 ];
            int   flags =   (int   ) ctx->gpr[ 1 ];
            int pipeId = openPipe(name, flags);
            ctx->gpr[ 0 ] = pipeId == -1 ?
                                      -1 :
                                      pipes[pipeId]->fd;
            break;
        }
        case 0x0b : {
            char *name = (char *) ctx->gpr[ 0 ];
            ctx->gpr[ 0 ] = deallocatePipe(name);
            break;
        }
        default   : { // 0x?? => unknown/unsupported
          break;
        }
        return;
    }
}
