/*$Header: romdata.c  Revision:1.0  28 July 2014 15:02:42  cal $*/
/********************************************************************
*																	*
*	Filename:	romdata.c											*
*	Originator:	Chris Lewis											*
*	Project:	4535-90 Elate Coil Control SW						*
*																	*
*	This module was created as a result of the Elate coil project	*
*		The ATXMEGA256A3U has Harvard architecture, which has		*
*	seperate program and data memories. Therefore any constant		*
*	string defined in the normal way will automatically use RAM as	*
*	storage.
*	To store constant strings directly into the program space, the	*
*	'PROGMEM' macro (defined in <avr/pgmspace.h>) is used when		*
*	declaring the storage variable.									*
*	However, the data must then be accessed using the macro defined *
*	in pgmspace.h.													*
*		This module contains the functions which transfer data or	*
*	strings stored in the program space to a RAM buffer. Once in	*
*	the buffer the data can be processed using standard techniques	*
*	and functions													*
*																	*
********************************************************************/

/*==================================================================*/
/*							INCLUDE FILES							*/
/*==================================================================*/
#include <avr/pgmspace.h>
#include <string.h>
#include "ATMtypes.h"
#include "romdata.h"
#include "asci.h"


/*==================================================================*/
/*		LOCAL INITIALISED VARIABLES (initialized to 0 by default)	*/
/*==================================================================*/
/* define destination storage for rom data */
int8 rambuf[ASCI_TX_BUF_LEN+1];	

/*********************************************************************
*							 FUNCTIONS							 	 *
*********************************************************************/

/*====================================================================
Name		:ROM_Read_romstr
Parameters	:romstr - ptr to string stored in program space
Returns		:ptr to string copy, stored in RAM
Description	:This function reads the string stored in the program space, 
			 pointed to by the romstr parameter, and copies it to the 
			 RAM buffer 'rambuf', then return a ptr to rambuf.
--------------------------------------------------------------------*/
int8 *ROM_Read_romstr(int8 const * romstr)
{
	return ROM_Read_romdata(romstr,ROM_Romstrlen(romstr)+1);
}
/*====================================================================
Name		:ROM_Read_romdata
Parameters	:romstr - ptr to string data in program space, len - no. of bytes to read
Returns		:ptr to copied data, stored in RAM
Description	:This function reads 'len' bytes of data stored in the program space,
			 pointed to by the romptr parameter, and copies it to the
			 RAM buffer 'rambuf', then return a ptr to rambuf.
--------------------------------------------------------------------*/
int8 *ROM_Read_romdata(int8 const * romptr,int16 len)
{
	int16 x;

	/* loop for 'len' bytes */	
	for(x = 0; x < len; x++)
		rambuf[x] = pgm_read_byte(&romptr[x]); /* copy data from ROM to RAM */

	return rambuf;	/* return RAM ptr */
}
/*====================================================================
Name		:ROM_Romstrlen
Parameters	:rptr - pointer to string stored in the program space
Returns		:No. of bytes in the string (excluding string termination char) 
Description	:This function returns the no. of bytes in the string, excluding 
			 string termination char ('\0').
--------------------------------------------------------------------*/
int16 ROM_Romstrlen(int8 const * rptr)
{
	int16 n;
	
	/* loop and increment n until terminating char is detected */
	for(n = 0; pgm_read_byte(&rptr[n]) != 0; n++);
	
	return n;	/* return no. of bytes */
}

/*====================================================================
Name		:ROM_Read_byte
Parameters	:byteaddr - address of ROM Byte 
Returns		:copy of ROM byte
Description	:This function returns the value of the byte at the ROM address passed
--------------------------------------------------------------------*/
int8 ROM_Read_byte(int8 const *byteaddr)
{
	return pgm_read_byte(byteaddr);
}
/*====================================================================
Name		:ROM_Read_word
Parameters	:wordaddr - address of ROM word
Returns		:copy of ROM byte
Description	:This function returns the value of the byte at the ROM address passed
--------------------------------------------------------------------*/
int16 ROM_Read_word(int16 const *wordaddr)
{
	return pgm_read_word(wordaddr);
}
/*********************************************************************
*						End of romdata.c							 *
*********************************************************************/
