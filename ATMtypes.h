/*$Header$*/
/********************************************************************
*																	*
*	Filename:		ATMtypes.h										*
*	Originator:		Chris Lewis										*
*	Project:		Interface Board I2C Test Software				*
*	Description:													*
*																	*
********************************************************************/

/* this header defines types for the ATMEGA328P processors */


#if !defined _ATMtypes_H
#define _ATMtypes_H    

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
/*----------------------------------------------------------------------*/
/*						DEFINE MAGSTIM DATA TYPES						*/
/*----------------------------------------------------------------------*/

typedef int8_t		sint8;		/* define signed char (8 bits) */   
typedef uint16_t	int16;		/* define unsigned integer (16 bits) */
typedef int16_t 	sint16;		/* define signed integer (16 bits) */
typedef uint32_t  	int32;		/* define unsigned long integer (32 bits) */
typedef int32_t 	sint32;		/* define signed long integer (32 bits) */
typedef bool 		boolean;	/* define boolean unsigned char (8 bits) */
typedef uint8_t 	int8;		/* define unsigned char (8 bits) */

/*----------------------------------------------------------------------*/
/*						PUBLIC MACRO DEFINITIONS						*/
/*----------------------------------------------------------------------*/
/* Global definitions */    

/* define NULL if not already defined */
#ifndef	NULL
#define	NULL	(void *)0	
#endif

/* define TRUE and FALSE */
#define	FALSE	0		
//#define false	FALSE

#define	TRUE	!FALSE
//#define true	TRUE


/* Global bit definitions */
#define BIT0	0x00000001
#define BIT1	0x00000002
#define BIT2	0x00000004
#define BIT3	0x00000008
#define BIT4	0x00000010
#define BIT5	0x00000020
#define BIT6	0x00000040
#define BIT7	0x00000080
#define BIT8	0x00000100
#define BIT9	0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000

	/* define global last argument for variable agument parameters */
#define LAST_VARG 0
                  
	/* define global enums */
typedef enum yesnotypes {NO,YES} YESNO_ENUM;
typedef enum onofftypes {OFF,ON} ONOFF_ENUM;
typedef enum enabledisable {DISABLE,ENABLE} ENABLEDISABLE_ENUM;
typedef enum hilotypes {LO,HI} HILO_ENUM;
               
	/* global Port Data Direction definitions */
#define ALL_OUTPUTS 0xff
#define ALL_INPUTS	0x00

#define F_CPU 16000000


	/* external functions used for debugging only */
extern void Db(int8 db2stat, int8 db1stat);
extern void Debug_msg(char *msg);
extern void Set_debug_data(int dbdata);

/* global definitions */
#define TMPSTR_LEN 50
extern int8 tmpstr[TMPSTR_LEN];

/* Program dependant declarations */
typedef enum board_under_test {IFACE_BD, ROTOR_BD, COIL_MEM_BD, COIL_SC_BD, COIL_SC_BD_T, DATA_LOGGER_SHIELD, LAST_BD}BUT;
BUT MAI_Get_but(void);
void MAI_Set_but(BUT But);

#define DEBUG_HI (PORTD |= BIT7)
#define DEBUG_LO (PORTD &= ~BIT7)

/************************************************************************
*						End of ATMtypes.h								*
************************************************************************/
#endif

