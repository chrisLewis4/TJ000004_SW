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
void MAI_Set_but(BUT But)
{
	but  = But;
}


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
	DDRB = (BIT0 | BIT1 | BIT2); // Set INTBAT EXTBAT & PolyFuse Load control
	PORTB &= ~(BIT0 | BIT1 | BIT2); // Set ops low
//	PORTB |= BIT1; // Set INTBAT on
	DDRD = BIT2;	// Set =5V enable as O/P
	PORTD |= BIT2; // Set +5V Control on
	
}


/*********************************************************************
*						End of main.c								 *
*********************************************************************/
