/*$Header$*/
#if !defined _MENU_H
#define _MENU_H
/********************************************************************
*                                                                   *
*   Filename:   menu.h                                             *
*   Originator: C.Lewis                                             *
*   Project:    6489-90 Universal Interface Board Test Jig          *
*                                                                   *
*   This module is responsible for the debug menu generation and    *
*   Functionality                                                   *
*                                                                   *
********************************************************************/

/*======================================================================*/
/*                          NESTED INCLUDE FILES                        */
/*======================================================================*/
#include "ATMtypes.h"

/*======================================================================*/
/*                      PUBLIC Messages				                    */
/*======================================================================*/

extern int8 const COPYRIGHT_MSG[];
extern int8 const FIRMWARE_ID_MSG[];
extern int8 const CHECKSUM_MSG[];
extern int8 const HARDWARE_ID_MSG[];
extern int8 const NEWLINE_MSG[];
extern int8 const DEBUG_MENU_MSG[];
extern int8 const START_MENU_MSG[];

/*======================================================================*/
/*                      PUBLIC FUNCTION PROTOTYPES                      */
/*======================================================================*/
extern void MEN_Init(void);
extern void MEN_bkproc(void);

extern void MEN_Test_bkproc(void);
extern void MEN_Shutdown_test(void);
extern int8 MEN_Test_active(void);

extern void MEN_Set_cmd_bk_func(const int8 *MSG_PTR, void * const next_cmd_ptr);

extern void MEN_Test_start_menu(void);


/************************************************************************
*                       End of menu.h                                  *
************************************************************************/
#endif
