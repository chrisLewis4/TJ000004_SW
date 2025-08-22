/*$Header$*/
/********************************************************************
*                                                                   *
*   Filename:   rts.c                                               *
*   Originator: C.Lewis                                             *
*   Project:    6489-90 Universal Interface Board Test Jig          *
*                                                                   *
* This module is responsible for the scheduling of real time tasks  *
* It uses the 2nd 8-bit timer module, Timer2 and is set to produce  *
* an interrupt every 200µs											*
********************************************************************/

/*==================================================================*/
/*                          INCLUDE FILES                           */
/*==================================================================*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include "atmtypes.h"
#include "rts.h"


/*==================================================================*/
/*                      LOCAL FUNCTION PROTOTYPES                   */
/*==================================================================*/

/*==================================================================*/
/*                      LOCAL MACRO DEFINITIONS                     */
/*==================================================================*/
/* timer2 registers setup definitions */
#define COMPMATCH_BIT   BIT1    /* sets compare match interrupt in TIMSK0 */
#define DIV8_PRESCALE   0x02    /* set pre-scaler to n/8 in TCCR0A */
#define CTC_MODE        0x08    /* Set compare mode in TCCR0A */

/* define COUNTS used by 200us interrupt to determine other timeouts */

#define RTS_TIME        50

/* define RTS 10ms timeouts */
#define RTS_50MS_TIME   5
#define RTS_ERROR_TIME  100

/* define COUNT required to produce 200us int */
/* @ phi = 8 MHz, with n/8 pre-scaler */
/* (1/8,000,000) x 8 x 200 = 200us */
#define TIMER0_INT_COUNT 200


/*==================================================================*/
/*      LOCAL INITIALISED VARIABLES (initialised to 0 by default)   */
/*==================================================================*/
/* define and reset RTS_TASK connected structure list */
static RTS_TASK *rts_task_list = NULL;

/* declare rts task ptr and active flag */
static int8 rts_active = FALSE;


/*********************************************************************
*                               FUNCTIONS                            *
*********************************************************************/
/*====================================================================
Name        :RTS_Init_rts
Parameters  :NONE
Returns     :NONE
Description :Called from main() during system initialisation.
             It sets up the 200us int, and configures the RTS
             task for initialisation.
--------------------------------------------------------------------*/
void RTS_Init_rts(void)
{
    /* Configure TIMER2 to produce the 200us RTS interrupt */
    TCCR2A = CTC_MODE;                  /* set to reset timer on compare match, stop timer */
    TCNT2 = 0;                          /* reset timer count */
    OCR2A = TIMER0_INT_COUNT;           /* set the output compare match value in us */
    TIMSK2 = COMPMATCH_BIT;             /* enable the compare match interrupt */
    TCCR2A = (DIV8_PRESCALE|CTC_MODE);  /* set prescaler to fosc/8, which starts timer */

    rts_active = FALSE;
  
	
}
/*====================================================================
Name		: RTS_load_task
Parameters	: ptr to RTS_TASK struct to load
Returns		: NONE
Description	: Adds the passed structure ptr to the list of RTS tasks
--------------------------------------------------------------------*/
void RTS_load_task(RTS_TASK *new_task)
{
	RTS_TASK **cur_task;
	
	// find end of task list
	for(cur_task = &rts_task_list;*cur_task;cur_task = &(*cur_task)->next_task);
	
	cli(); // disable ints
	*cur_task = new_task; //set next task as passed ptr
	(*cur_task)->cur_count = 0; // reset count
	(*cur_task)->next_task = NULL; // terminate end of list
	sei(); // re-enable ints
}
/*====================================================================
Name		: RTS_unload_task
Parameters	: ptr to RTS_TASK struct to unload
Returns		: none
Description	: removed the passed struct pointer from the task list
--------------------------------------------------------------------*/
void RTS_unload_task(RTS_TASK *task)
{
	RTS_TASK **cur_task;
	// search for passed task in list or end of list
	for(cur_task = &rts_task_list;*cur_task && *cur_task != task;cur_task = &(*cur_task)->next_task);
	if(*cur_task) // check for valid task
	{
		// disable ints, remove task, re-anable ints
		cli();
		*cur_task = (*cur_task)->next_task;
		sei();
	}
}
/*====================================================================
Name        :RTS_Shutdown_rts
Parameters  :NONE
Returns     :NONE
Description :It disables the 200us int
--------------------------------------------------------------------*/
void RTS_Shutdown_rts(void)
{
    TIMSK2 = 0;         /* disable the compare match interrupt */
    TCCR2A = CTC_MODE;  /* reset prescaler bits which stops the timer */

}

/*====================================================================
Name        :ISR(TIMER2_COMP_vect)
Parameters  :NONE
Returns     :NONE
Description :Timer0 compare match interrupt service routine
            This is generated every 200us, and calls the current rts task
            after clearing the interrupt bit in the interrupt flag reg
--------------------------------------------------------------------*/
ISR(TIMER2_COMP_vect)
{
    RTS_TASK *cur_task;
	
	if(rts_active)
		ERR_Sys_err(SYSERR_RTSOVERRUN);
	else
		rts_active = TRUE;
		
	TIFR2 |= COMPMATCH_BIT; /* write a '1' to the compare match int flag to clear the int */

	
	for(cur_task = rts_task_list;cur_task;cur_task = cur_task->next_task)
	{
		if(++cur_task->cur_count >= cur_task->timeout_count)
		{
			cur_task->cur_count = 0;
			if(cur_task->rts_proc)
				(*cur_task->rts_proc)();
		}
	}	
	rts_active = FALSE;
}
/*********************************************************************
*                       End of rts.c                                 *
*********************************************************************/
