
/********************************************************************
*																	*
*	Filename:		ADC.h											*
*	Originator:		Chris Lewis										*
*	Project:		EEG Battery Module TestJig Software				*
*	Description:													*
*																	*
********************************************************************/
#if !defined _ADC_H
#define _ADC_H

/*==================================================================*/
/*							INCLUDE FILES							*/
/*==================================================================*/
#include "ATMtypes.h"


/*==================================================================*/
/*						LOCAL MACRO DEFINITIONS						*/
/*==================================================================*/
typedef enum _adc_chan_id
{
	ADC_EXTBAT,		// 0, ADC chan 1
	ADC_INTBAT,		// 1, ADC chan 2
	ADC_POLYFUSE,	// 2, ADC chan 3
	ADC_VBAT,		// 3, ADC chan 6
	ADC_3V3,		// 4, ADC chan 7
	ADC_CHAN_COUNT

}ADC_CHAN_ID;

typedef struct _chan_average
{
	int16 max;
	int16 min;
	int16 avg;
	int32 sum;
	int8 cnt;
}CHAN_AVERAGE;
/*==================================================================*/
/*						LOCAL CONSTANT DEFINITIONS					*/
/*==================================================================*/
/*==================================================================*/
/*		LOCAL INITIALISED VARIABLES (initialised to 0 by default)	*/
/*==================================================================*/

/*==================================================================*/
/* 						public FUNCTION PROTOTYPES 					*/
/*==================================================================*/
void ADC_Shutdown(void);
void ADC_Init(void);
int8 ADC_Get_average_millivolts(int16 *millivolt_res, ADC_CHAN_ID chan);
int8* ADC_Get_chan_name(ADC_CHAN_ID chan);
CHAN_AVERAGE *ADC_Get_chan_average(ADC_CHAN_ID chan,int8 avg_size);


/************************************************************************
*						End of ADC.h									*
************************************************************************/
#endif
