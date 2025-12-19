/********************************************************************
*																	*
*	Filename:		main.c											*
*	Originator:		Chris Lewis										*
*	Project:		EEG Battery Module TestJig Software				*
*	Description:													*
*																	*
********************************************************************/
 
/*==================================================================*/
/*							INCLUDE FILES							*/
/*==================================================================*/

#include "ATMtypes.h"
#include "i2c.h"
#include "asci.h"
#include "menu.h"
#include "main.h"
#include "Timer.h"
#include "adc.h"
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>     

/*==================================================================*/
/*						LOCAL MACRO DEFINITIONS						*/
/*==================================================================*/
/*==================================================================*/
/*						LOCAL CONSTANT DEFINITIONS					*/
/*==================================================================*/
#define EXTBAT_CNTRL_BIT BIT2
#define INTBAT_CNTRL_BIT BIT1
#define POLYFUSE_CNTRL_BIT BIT0
#define IO_CNTRL_PORT_WR PORTB
#define IO_CNTRL_PORT_RD PINB

#define INT5V_CNTRL_BIT BIT2

	
/*==================================================================*/
/*		LOCAL INITIALISED VARIABLES (initialised to 0 by default)	*/
/*==================================================================*/
int8 tmpstr[TMPSTR_LEN];
/*==================================================================*/
/* 						LOCAL FUNCTION PROTOTYPES 					*/
/*==================================================================*/

void initIO(void);

/*==================================================================*/
/* 								FUNCTIONS 							*/
/*==================================================================*/
static BUT but;


/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
int main(void)
{
	initIO();
	TIM_Init_timer();
	ASC_Init_asci();
	ADC_Init();
		
// DISPLAY INSTRUCTIONS

	MEN_Init();


	//Enter Endless loop
	while(1) 
	{
		MEN_bkproc();	
	};
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
BUT MAI_Get_but(void)
{
	return but;
}

/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
void initIO(void)
{
	DDRD |= BIT7; //Set Port D Bit 7 as Debug port
	DEBUG_LO;
	DEBUG_HI;
	DEBUG_LO;
	DEBUG_HI;
	DEBUG_LO;
	DEBUG_HI;
	DEBUG_LO;
// Setup I/Os for TJ00004
	// Disable ADC digital inputs
	DIDR0 = (BIT1 | BIT2 | BIT3 );	// Disable digital inputs for ADCD channels used - No need to disable digital pins for CH6 or CH7

	// Set Output ports to default states (OFF)
	DDRB = (EXTBAT_CNTRL_BIT | INTBAT_CNTRL_BIT | POLYFUSE_CNTRL_BIT); // Set INTBAT EXTBAT & PolyFuse Load control
	IO_CNTRL_PORT_WR &= ~(EXTBAT_CNTRL_BIT | INTBAT_CNTRL_BIT | POLYFUSE_CNTRL_BIT); // Set ops low
//	PORTB |= (BIT1 | BIT2); // Set INTBAT on
	DDRD = INT5V_CNTRL_BIT;	// Set +5V enable as O/P
	PORTD |= INT5V_CNTRL_BIT; // Set +5V Control on
	
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
ONOFF_ENUM MAI_Get_control_status(IO_CONTROL cntrl_chan)
{
	int8 stat;

	stat = IO_CNTRL_PORT_RD;
	switch(cntrl_chan)
	{
		case EXTBAT_CNTRL:
			if(stat & EXTBAT_CNTRL_BIT)
				return ON;
			else
				return OFF;
			break;
		case INTBAT_CNTRL:
			if(stat & INTBAT_CNTRL_BIT)
				return ON;
			else
				return OFF;
			break;
		case POLYFUSE_CNTRL:
			if(stat & POLYFUSE_CNTRL_BIT)
				return ON;
			else
				return OFF;
			break;
		
	}
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
void MAI_Set_control_status(IO_CONTROL cntrl_chan, ONOFF_ENUM stat)
{
	switch(cntrl_chan)
	{
		case EXTBAT_CNTRL:
			if(stat == ON)
				IO_CNTRL_PORT_WR |= EXTBAT_CNTRL_BIT;
			else
				IO_CNTRL_PORT_WR &= ~EXTBAT_CNTRL_BIT;
			break;
		case INTBAT_CNTRL:
			if(stat == ON)
				IO_CNTRL_PORT_WR |= INTBAT_CNTRL_BIT;
			else
				IO_CNTRL_PORT_WR &= ~INTBAT_CNTRL_BIT;
			break;
		case POLYFUSE_CNTRL:
			if(stat == ON)
				IO_CNTRL_PORT_WR |= POLYFUSE_CNTRL_BIT;
			else
				IO_CNTRL_PORT_WR &= ~POLYFUSE_CNTRL_BIT;
			break;
	}
}

/*********************************************************************
*						End of main.c								 *
*********************************************************************/
