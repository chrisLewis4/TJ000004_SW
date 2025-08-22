/*$Header$*/
/********************************************************************
*																	*
*	Filename:		i2c.h											*
*	Originator:		Chris Lewis										*
*	Project:		Interface Board I2C Test Software				*
*	Description:													*
*																	*
********************************************************************/
#if !defined _i2c_H
#define _i2c_H

/*==================================================================*/
/*							INCLUDE FILES							*/
/*==================================================================*/
#include "ATMtypes.h"
#include <avr/io.h>

/*==================================================================*/
/*						LOCAL MACRO DEFINITIONS						*/
/*==================================================================*/
// Functions for i2c communication

#define I2C_SDA                     PORTC4
#define I2C_SDA_PORT                PORTC
#define I2C_SDA_PIN                 PINC
#define I2C_SDA_DDR                 DDRC

#define I2C_SCL                     PORTC5
#define I2C_SCL_PORT                PORTC
#define I2C_SCL_PIN                 PINC
#define I2C_SCL_DDR                 DDRC

// MACRO definitions for reading and writing to an I2C ADDR
#define I2C_RD_BIT					0x01
#define I2C_RD_ADDR(addr)			((addr<<1)|I2C_RD_BIT)
#define I2C_WR_ADDR(addr)			((addr<<1)&~(I2C_RD_BIT))


/*==================================================================*/
/*						LOCAL CONSTANT DEFINITIONS					*/
/*==================================================================*/
/*==================================================================*/
/*		LOCAL INITIALISED VARIABLES (initialised to 0 by default)	*/
/*==================================================================*/

/*==================================================================*/
/* 						public FUNCTION PROTOTYPES 					*/
/*==================================================================*/


extern void	I2C_Init(void);
extern void I2C_Write(int8 addr, int8 byte_count,int8 *buf);
extern void I2C_Read(int8 addr, int8 byte_count,int8 *buf);
extern void I2C_Shutdown(void);

/************************************************************************
*						End of I2C.h									*
************************************************************************/
#endif
