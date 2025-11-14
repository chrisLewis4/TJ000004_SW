/*$Header: romdata.h  Revision:1.0  28 July 2014 14:59:11  cal $*/
/********************************************************************
*																	*
*	Filename:	romdata.h											*
*	Originator:	C.Lewis												*
*	Project:	3026 Tesla UI control software						*
*																	*
*	Created by Chris Lewis, CAL Design, July 2014					*
*	to work with ATMEGA645 device									*
*																	*
********************************************************************/
#if !defined _romdata_H
#define _romdata_H

/*======================================================================*/
/*							NESTED INCLUDE FILES						*/
/*======================================================================*/

/*======================================================================*/
/*							PUBLIC DEFINES								*/
/*======================================================================*/

/*======================================================================*/
/*						PUBLIC FUNCTION PROTOTYPES 						*/
/*======================================================================*/
extern int8 *ROM_Read_romstr(int8 const * romstr);
extern int8 *ROM_Read_romdata(int8 const * rptr,int16 n);
extern int16 ROM_Romstrlen(int8 const * rptr);
extern int8 ROM_Read_byte(int8 const *byteaddr);
extern int16 ROM_Read_word(int16 const *wordaddr);

/************************************************************************
*						End of romdata.h								*
************************************************************************/
#endif

