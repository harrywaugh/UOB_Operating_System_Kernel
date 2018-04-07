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
int pipesLength = 1;

queue_t *queue;
pcb_t *curr_prog;

int getFreeStack()  {
    int i = 0;
    while ( fullStacks[ i ] ) i++;
    if ( i < STACKS )  {
        fullStacks[ i ] = true;
        return i;
    }
    return -1;
}

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

bool memStackAvailable()  {
    for ( int i = 0; i < STACKS ; i++ )
        if (!fullStacks[i])  return true;
    return false;
}

void allocateNewPipe( char *name )  {
    void *p = realloc(pipes, sizeof(pipe_t *) * (pipesLength+1));
    if (p)    pipes = p;
    pipes[ pipesLength ] = (pipe_t *) malloc(sizeof(pipe_t));
    *(pipes[ pipesLength ]) = (pipe_t){newQueue((size_t)1), name, currFd++};
    pipesLength++;

}

uint32_t *getStackAddress(int id)  {
    return (uint32_t*)(&tos_P - (0x00000400 * id));
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
    curr_prog->status = STATUS_READY;
    push(queue, curr_prog);
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
          char*  x = ( char* )( ctx->gpr[ 1 ] );
          int    n = ( int   )( ctx->gpr[ 2 ] );

          if( fd == 1)  {   //STDOUT_FILENO
              for( int i = 0; i < n; i++ ) {
                PL011_putc( UART0, *x++, true );
              }
          } else {

          }


          ctx->gpr[ 0 ] = n;
          break;
        }

        case 0x03: { //Fork
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

        case 0x04: { //Exit
            curr_prog->status = STATUS_TERMINATED;
            put_str("\nProgram finished.\n\0");
            scheduler(ctx);
            break;
        }
        case 0x05: { //Exec
            void *main_fn = ( void * )( ctx->gpr[ 0 ]);                                // Get new main fn for program
            uint32_t *stack = (uint32_t *) (getStackAddress(curr_prog->stack_id));     // Get stack location (Top)
            ctx->sp = (uint32_t) stack;                                                // Set sp to start from top of stack
            memset(stack - 0x00000400, 0, 0x00001000);                                              // Clear new programs stack
            ctx->pc = (uint32_t) main_fn;                                              // Update pc to beggining of new program
            memcpy(&curr_prog->ctx, ctx, sizeof(ctx_t));                               // Copy over ctx
            break;
        }
        case 0x06: { //Kill
            pid_t pid = ctx->gpr[ 0 ];
            ctx->gpr[ 0 ] = (int32_t) terminateProgram(pid);
            break;
        }
        case 0x08: { //Mkfifo
            char *name = (char *) ctx->gpr[ 0 ];
            int mode =   (int   ) ctx->gpr[ 1 ];
            if( pipesLength != 1 )  allocateNewPipe(name);
            else {
                pipes[ pipesLength ] = (pipe_t *) malloc(sizeof(pipe_t));
                *(pipes[ pipesLength - 1]) = (pipe_t){newQueue((size_t)1), name, currFd++};
            }
            ctx->gpr[ 0 ] = 0;
            break;
        }
        default   : { // 0x?? => unknown/unsupported
          break;
        }
        return;
    }
}
