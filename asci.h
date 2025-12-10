/********************************************************************
*																	*
*	Filename:		asci.h											*
*	Originator:		Chris Lewis										*
*	Project:		EEG Battery Module TestJig Software				*
*	Description:													*
*																	*
********************************************************************/
#if !defined _asci_H
#define _asci_H 
/*======================================================================*/
/*							NESTED INCLUDE FILES						*/
/*======================================================================*/
#include "ATMtypes.h"

/*======================================================================*/
/*							PUBLIC TYPEDEFS								*/
/*======================================================================*/ 
/*------------------------------------------------------------------------
Name		:ASCICODES_ENUM
Description	:Holds all codes returned by ASCI interface functions
------------------------------------------------------------------------*/
/* define valid asci function return codes as described above */
typedef enum validascireturncodes {
	ASCI_EMPTY,
	ASCI_BUSY,
	ASCI_ERROR,
	ASCI_FULL,
	ASCI_OK,
	LAST_ASCI_CODE} ASCICODES_ENUM;

/*======================================================================*/
/*							PUBLIC DEFINES								*/
/*======================================================================*/
/* define ASCI buffer sizes */
#define ASCI_TX_BUF_LEN 180
/* define maximum allowed ASC_Asci string to be logged */
#define MAX_ASCI_STRING (ASCI_TX_BUF_LEN-2)

/*======================================================================*/ 
/*						PUBLIC FUNCTION PROTOTYPES 						*/
/*======================================================================*/ 
/* system interface functions */
extern void ASC_Init_asci(void);
extern void ASC_Shutdown_asci(void);
/* ASCI Interface functions */
extern int8 ASC_Asci_tx(int8 *ptr,int8 count);
extern int8 ASC_Asci_getchar(int8 *c);
extern int8 ASC_Asci_msg(int8 *msg);
extern int8 ASC_Get_asci_tx_space(void);
extern int8 ASC_Asci_tx_empty(void);
extern void ASC_Pause(void);

/************************************************************************
*						End of asci.h									*
************************************************************************/
#endif
