/*$Header$*/
/********************************************************************
*																	*
*	Filename:		main.c											*
*	Originator:		Chris Lewis										*
*	Project:		Interface Board I2C Test Software				*
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
	int8 c;
	int8 buf[20];
	
	int8 x;
	
	initIO();
	ASC_Init_asci();
		
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
	/* Pin Mapping
	AVR - Arduino - Datalogger - LCD
	
	PD0 = D0	=      NC
	PD1 = D1	=      NC
	PD2 = D2	=      NC
	PD3 = D3	=      NC
	PD4 = D4	=      NC
	PD5 = D5	=      NC
	PD6 = D6	=      NC
	PD7 = D7	=      NC
	
	PB0 = D8	=      NC
	PB1 = D9	=      NC
	PB2 = D10	=      NC
	PB3 = D11	=      NC
	PB4 = D12	=      NC
	PB5 = D13	=      NC
	PB6 = X1
	PB7 = X2
	
	PC0 = A0   =     
	PC1 = A1
	PC2 = A2
	PC3 = A3
	PC4 = A4   =      SDA
	PC5 = A5   =      SCL
	
	
	DDRB = 0x00
	DDRC = 0x00
	DDRD = 0x00*/
}


/*********************************************************************
*						End of main.c								 *
*********************************************************************/
