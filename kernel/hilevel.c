/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"
#include "queue.h"
#define PROGRAMS 5

extern void     main_P1();
extern uint32_t tos_P1;
extern void     main_P2();
extern uint32_t tos_P2;
extern void     main_P3();
extern uint32_t tos_P3;
extern void     main_P4();
extern uint32_t tos_P4;
extern void     main_P5();
extern uint32_t tos_P5;

void (*p_mains[PROGRAMS])(void) = {&main_P1, &main_P2, &main_P3, &main_P4, &main_P5};
uint32_t *p_stacks[PROGRAMS]    = {&tos_P1,  &tos_P2,  &tos_P3,  &tos_P4,  &tos_P5  };

pcb_t pcb[ PROGRAMS ];
int executing = 0;
queue_t *queue_level1;
pcb_t *curr_prog;

void put_str( char* str )  {
    for (int i = 0; str[i] != '\0'; i++ )  PL011_putc( UART0, str[i], true);
    return;
}

void scheduler( ctx_t* ctx ) {
    bool switched = false;

    memcpy( &curr_prog->ctx, ctx, sizeof( ctx_t ) );                                           // preserve current program
    if(curr_prog->status != STATUS_TERMINATED){
        curr_prog->status = STATUS_READY;                                                      // update program status
        push(queue_level1, curr_prog);                                                         // Repush unfinished program to queue
    }

    pop (queue_level1, curr_prog);
    if (curr_prog != NULL && curr_prog->status == STATUS_READY)  {                    //Check if program is ready
        memcpy( ctx, &curr_prog->ctx, sizeof( ctx_t ) );                              // Restore new current program
        curr_prog->status = STATUS_EXECUTING;
    } else put_str("\nAll programs finished.\0");                                     // If program wasn't switched, they're all done
    return;
}

void hilevel_handler_rst( ctx_t* ctx              ) {
  /* Initialise PCBs representing processes stemming from execution of
   * the two user programs.  Note in each case that
   *
   * - the CPSR value of 0x50 means the processor is switched into USR
   *   mode, with IRQ interrupts enabled, and
   * - the PC and SP values matche the entry point and top of stack.
   */
   queue_level1 = newQueue();

   for ( int i = 0; i < PROGRAMS; i++ )  {
       memset( &pcb[ i ], 0, sizeof( pcb_t ) );
       pcb[ i ].pid      = i+1;
       pcb[ i ].priority = i * 10;
       pcb[ i ].status   = STATUS_READY;
       pcb[ i ].ctx.cpsr = 0x50;
       pcb[ i ].ctx.sp   = ( uint32_t )( p_stacks[i] );
       pcb[ i ].ctx.pc   = ( uint32_t )( p_mains[i]  );
       push(queue_level1, &pcb[ i ]);
   }
    /* Once the PCBs are initialised, we (arbitrarily) select one to be
    * restored (i.e., executed) when the function then returns.
    */
    curr_prog = (pcb_t *)malloc(sizeof(pcb_t));
    pop(queue_level1, curr_prog);
    memcpy( ctx, &curr_prog->ctx, sizeof( ctx_t ) );
    curr_prog->status = STATUS_EXECUTING;



    // pop (queue_level1, curr_prog);
    // memcpy( ctx, &curr_prog->ctx, sizeof( ctx_t ) );
    // curr_prog->status = STATUS_EXECUTING;
    // executing = 0;

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
        // for (int i = executing + 1; i < PROGRAMS + executing; i++)
        //     pcb[ i % PROGRAMS].priority.age++;  //increase all priority age except executing program
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
    case 0x04: { //0x04 => exit(x)
        curr_prog->status = STATUS_TERMINATED;
        put_str("\nProgram finished.\0");
        scheduler(ctx);
        break;
    }

    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
