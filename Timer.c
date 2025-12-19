/*
 * Timer.c
 *
 * Created: 17/11/2025 16:27:51
 *  Author: chris.lewis
 */ 

/*==================================================================*/
/*							INCLUDE FILES							*/
/*==================================================================*/

#include "ATMtypes.h"
#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>

/*==================================================================*/
/*						LOCAL MACRO DEFINITIONS						*/
/*==================================================================*/

/*==================================================================*/
/*						LOCAL CONSTANT DEFINITIONS					*/
/*==================================================================*/
// 1ms timer tick definitions
// Aduino Nano A00005 runs at clock speed of 16MHz
// 16,000,000/64 = 250,000Hz = 4탎 Period
// for 1ms tick: 1ms/4탎 = 250

#define CLK_DIV64 0x03
#define COUNT_1MS 250

/*==================================================================*/
/*		LOCAL INITIALISED VARIABLES (initialised to 0 by default)	*/
/*==================================================================*/
static int16 volatile set_delay,delay_cnt;
static int8 volatile delay_flag;

/*==================================================================*/
/* 						LOCAL FUNCTION PROTOTYPES 					*/
/*==================================================================*/



/*==================================================================*/
/* 								FUNCTIONS 							*/
/*==================================================================*/



/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
void TIM_Init_timer(void)
{
	set_delay = 0;
	delay_cnt = 0;
	delay_flag = FALSE;
	
	//TCCR1B = 0;
	
	
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
void TIM_Set_delay(int16 millisecs)
{
	set_delay = millisecs;
	delay_cnt = 0;
	TCNT1 = 0;				// reset TCNT
	//TCCR1B |= (1 << WGM12);	// set CTC mode
	OCR1A = COUNT_1MS;		// Set 1ms Count (250 x 4탎 = 1.0ms)
	TIMSK1 |= (1 << OCIE1A);//Enable compare match int
	// Set clock speed to 250kHz (4탎) to start clock
	TCCR1B |= CLK_DIV64;
		
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
int8 TIM_Get_delay_flag(void)
{
	if(delay_flag)
	{
		delay_flag = FALSE;
		return TRUE;
	}
	return FALSE;
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
void TIM_Wait(int16 millisecs)
{
	TIM_Set_delay(millisecs);
	while(!TIM_Get_delay_flag());
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
ISR(TIMER1_COMPA_vect)
{

	if(++delay_cnt == set_delay)
	{
		// Count is done so stop timer
		TCCR1B = 0;
		TIMSK1 &= ~(1 << OCIE1A);//Disable compare match int
		delay_flag = true;
	}
	else
	{
		// count has not finished to add another 1ms to compare match
		OCR1A += COUNT_1MS;
	}
}
/*********************************************************************
*						End of Timer.c								 *
*********************************************************************/
