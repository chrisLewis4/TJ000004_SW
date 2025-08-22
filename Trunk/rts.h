/*$Header$*/
#if !defined _RTS_H
#define _RTS_H
/********************************************************************
*                                                                   *
*   Filename:   rts.h                                               *
*   Originator: C.Lewis                                             *
*   Project:    6489-90 Universal Interface Board Test Jig          *
*                                                                   *
* This module is responsible for the scheduling of real time tasks  *
*                                                                   *
********************************************************************/

/*======================================================================*/
/*                          NESTED INCLUDE FILES                        */
/*======================================================================*/
#include "ATMtypes.h"
/*======================================================================*/
/*                      PUBLIC MACRO DEFINITIONS                        */
/*======================================================================*/
/* define RTS timeout bitflags */

#define DEMO_10MS               BIT2
#define INIT_10MS_TIMEOUT       BIT3

typedef struct rts_task
{
	void	(*rts_proc)(void);
	int16	timeout_count;
	int16	cur_count;
	struct rts_task *next_task;
}RTS_TASK;



/*======================================================================*/
/*                      PUBLIC FUNCTION PROTOTYPES                      */
/*======================================================================*/
extern              void RTS_Init_rts(void);
extern           boolean RTS_Get_rts_timeout(int16 type);
extern              void RTS_Start_rts(void);
extern              void RTS_Stop_rts(void);
extern              void RTS_Shutdown_rts(void);
extern              void RTS_Reset_comms_timer(void);
extern              void RTS_start_error_timer(void);
extern              int8 RTS_has_error_timer_elapsed(void);


extern void RTS_load_task(RTS_TASK *);
extern void RTS_unload_task(RTS_TASK *);

/************************************************************************
*                       End of rts.h                                    *
************************************************************************/
#endif
