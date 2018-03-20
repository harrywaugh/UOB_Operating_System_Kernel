/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"
#include "queue.h"
#define PROGRAMS 5
#define QUEUENO 4
int curr_pid = 1;
int curr_stack = 0;

extern void      main_console();
extern void      main_P1();
extern uint32_t  tos_P, tos_svc;
uint32_t *program_stack = &tos_P;


queue_t *queue;
pcb_t *curr_prog;

pcb_t *create_process(int id, uint32_t *stackp, void *main_fn)  {
    pcb_t *pcb = (pcb_t *)malloc(sizeof(pcb_t));
    memset( pcb, 0, sizeof( pcb_t ) );
    pcb->pid      = id;
    pcb->priority = 0;
    pcb->status   = STATUS_READY;
    pcb->ctx.cpsr = 0x50;
    pcb->ctx.sp   = ( uint32_t )( stackp );
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

uint32_t *getStackAddress(int pid)  {
    return (uint32_t*)(&tos_P - (0x00000400 * (pid - 1)));
}

// int activeQueue()  {
//     int i = 0;
//     while ( i < QUEUENO && isEmpty(queues[ i++ ]));
//
//     return i-1;
// }

void scheduler( ctx_t* ctx ) {
    memcpy( &curr_prog->ctx, ctx, sizeof( ctx_t ) );
    if (curr_prog->status != STATUS_TERMINATED)
        curr_prog->status = STATUS_READY;
    push(queue, curr_prog);
    pop(queue, curr_prog);
    if (curr_prog->status == STATUS_READY)  {                                         //Check if program is ready
         memcpy( ctx, &curr_prog->ctx, sizeof( ctx_t ) );                              // Restore new current program
         curr_prog->status = STATUS_EXECUTING;
     }

    return;
}

void hilevel_handler_rst( ctx_t* ctx              ) {

    queue = newQueue();

    curr_prog = create_process( curr_pid, getStackAddress(curr_pid++), &main_console);
    //pcb_t*p = create_process( curr_pid, getStackAddress(curr_pid++), &main_P1);
    //push(queue, p);
    memcpy( ctx, &curr_prog->ctx, sizeof( ctx_t ) );
    curr_prog->status = STATUS_EXECUTING;

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

          for( int i = 0; i < n; i++ ) {
            PL011_putc( UART0, *x++, true );
          }

          ctx->gpr[ 0 ] = n;
          break;
        }

        case 0x03: { //Fork
            pcb_t *child = create_process(curr_pid, NULL, &main_console);  // Create process with new pid, and give it the console fn to run. Stack = NULL
            memcpy(&child->ctx, ctx, sizeof(ctx_t));                                   // Copy context from console to new program.
            uint32_t *new_stack = (uint32_t *) (getStackAddress(curr_pid));            // Get new stack location (Points to top)
            uint32_t *console_stack = (uint32_t *) (getStackAddress(curr_prog->pid));  // Get console stack location (Points to top)
            new_stack -= 0x00000400;                                                   // Update so it points to bottom of stack
            console_stack -= 0x00000400;                                               // Update so it points to bottom of stack
            memcpy(new_stack, console_stack, 0x00001000);                              // Copy stack from console to new program
            child->ctx.sp -= 0x00001000 * (curr_pid - curr_prog->pid);                 // Update new sp so it points to old stack location but in own stack
            child->ctx.gpr[ 0 ] = 0;                                                   // Set child return to 0.
            ctx->gpr[ 0 ] = curr_pid++;                                                // Set parent return to child pid.
            push(queue, child);                                                        // Add child to process queue.
            break;
        }

        case 0x04: { //0x04 => exit(x)
            curr_prog->status = STATUS_TERMINATED;
            put_str("\nProgram finished.\n\0");
            scheduler(ctx);
            break;
        }
        case 0x05: { //Exec
            void *main_fn = ( void * )( ctx->gpr[ 0 ]);                                // Get new main fn for program
            uint32_t *stack = (uint32_t *) (getStackAddress(curr_prog->pid));          // Get stack location (Top)
            ctx->sp = (uint32_t) stack;                                                // Set sp to start from top of stack
            stack -= 0x00000400;                                                   // Move to bottom of stack
            memset(stack, 0, 0x00001000);                                              // Clear new programs stack
            ctx->pc = (uint32_t) main_fn;                                              // Update pc to beggining of new program
            memcpy(&curr_prog->ctx, ctx, sizeof(ctx_t));                               // Copy over ctx
            break;
        }
        default   : { // 0x?? => unknown/unsupported
          break;
        }
        return;
    }
}
