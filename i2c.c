/********************************************************************
*																	*
*	Filename:		i2c.c											*
*	Originator:		Chris Lewis										*
*	Project:		EEG Battery Module TestJig Software				*
*	Description:													*
*																	*
********************************************************************/
/*======================================================================*/
/*							NESTED INCLUDE FILES						*/
/*======================================================================*/
#include "ATMtypes.h"
#include "i2c.h"
#include "asci.h"
#include "romdata.h"
#include <avr/pgmspace.h>


/*======================================================================*/
/*							LOCAL TYPEDEFS								*/
/*======================================================================*/
/*------------------------------------------------------------------------
Name		:
Description	:
------------------------------------------------------------------------*/


/*==================================================================*/
/*						LOCAL MACRO DEFINITIONS						*/
/*==================================================================*/

/*==================================================================*/
/*						LOCAL CONSTANT DEFINITIONS					*/
/*==================================================================*/


/*==================================================================*/
/*		LOCAL INITIALISED VARIABLES (initialised to 0 by default)	*/
/*==================================================================*/

/*==================================================================*/
/* 						LOCAL FUNCTION PROTOTYPES 					*/
/*==================================================================*/

static void	I2C_WaitForComplete(void);
static void	I2C_Start(void);
static void	I2C_Stop(void);
static void	I2C_Send(uint8_t data);
static uint8_t I2C_ReadAck(void);
static uint8_t I2C_ReadNoAck(void);

/*==================================================================*/
/* 								FUNCTIONS 							*/
/*==================================================================*/


/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
void I2C_Init(void) 
{
	/* set pullups for SDA, SCL lines */
	//I2C_SDA_PORT |= ((1 << I2C_SDA) | (1 << I2C_SCL));
	TWBR = 12;   /* set bit rate (p.242): 8MHz / (16+2*TWBR*1) ~= 100kHz */
	TWCR |= (1 << TWEN);                                       /* enable */
		
 }
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
int8 I2C_Write(int8 addr, int8 byte_count,int8 *buf)
{
	int8 count,tmp;
	
	I2C_Start();
	I2C_Send(I2C_WR_ADDR(addr));
	
	for(count = 0; count < byte_count;count++)
		I2C_Send(buf[count]);
	tmp = TWSR;
	I2C_Stop();
	return tmp;
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
void I2C_Read(int8 addr, int8 byte_count,int8 *buf)
{
	int8 count;
	
	I2C_Start();

	I2C_Send(I2C_RD_ADDR(addr));
	for(count = 0; count < byte_count;count++)
	{
		if(count == (byte_count - 1))
			buf[count] = I2C_ReadNoAck();
		else
			buf[count] = I2C_ReadAck();
	}
	I2C_Stop();
	
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void I2C_WaitForComplete(void) 
{
	int8 volatile tmp;
	int16 timeout;
	
	for(timeout = 0; timeout < 0xffff; timeout++)
	{
		tmp = TWCR;
		if((tmp  & (1 <<TWINT)))
			break;
	} 
	
/*	loop_until_bit_is_set(TWCR, TWINT);*/
}

/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void I2C_Start(void) 
{
	TWCR = (_BV(TWINT) | _BV(TWEN) | _BV(TWSTA));
	I2C_WaitForComplete();
	
}

/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void I2C_Stop(void) 
{
  TWCR = (_BV(TWINT) | _BV(TWEN) | _BV(TWSTO));
}

/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static uint8_t I2C_ReadAck(void) 
{
  TWCR = (_BV(TWINT) | _BV(TWEN) | _BV(TWEA));
  I2C_WaitForComplete();
  return (TWDR);
}

/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static uint8_t I2C_ReadNoAck(void) 
{
  TWCR = (_BV(TWINT) | _BV(TWEN));
  I2C_WaitForComplete();
  return (TWDR);
}

/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void I2C_Send(uint8_t data) 
{
	TWDR = data;
	TWCR = (_BV(TWINT) | _BV(TWEN));                  /* init and enable */
//	ASC_Asci_msg((int8 *)"\n\rDB 2b\n\r");
	I2C_WaitForComplete();
//	ASC_Asci_msg((int8 *)"\n\rDB 2c\n\r");
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
void I2C_Shutdown(void)
{
	TWBR = 0;   /* set bit rate (p.242): 8MHz / (16+2*TWBR*1) ~= 100kHz */
	TWCR &= ~(1 << TWEN);                                       /* enable */
}
/*********************************************************************
*						End of I2C.c								 *
*********************************************************************/
