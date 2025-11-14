/*$Header$*/
/********************************************************************
*																	*
*	Filename:		ADC.c											*
*	Originator:		Chris Lewis										*
*	Project:		EEG Battery Module TestJig Software				*
*	Description:													*
*																	*
********************************************************************/
/*======================================================================*/
/*							NESTED INCLUDE FILES						*/
/*======================================================================*/
#include "ATMtypes.h"
#include "adc.h"
#include "asci.h"
#include <avr/interrupt.h>
#include <avr/io.h>

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
#define ADC_AVERAGE_COUNT 0x100L
#define ADC_SAMPLE_COUNT 0x100L


/*==================================================================*/
/*		LOCAL INITIALISED VARIABLES (initialised to 0 by default)	*/
/*==================================================================*/
static int32 adc_sample_cnt;
static int32 volatile adc_sample_sum;
static int8 volatile adc_data_ready;

static int32 adc_avg_cnt;
static int32 adc_avg_sum;
static int8 adc_avg_ready;
/*==================================================================*/
/* 						LOCAL FUNCTION PROTOTYPES 					*/
/*==================================================================*/


/*==================================================================*/
/* 								FUNCTIONS 							*/
/*==================================================================*/


/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
#define ADC_ENABLE BIT7
#define ADC_START BIT6
#define ADC_AUTO_TRIG BIT5
#define ADC_INT_FLAG BIT4
#define ADC_INT_ENABLE BIT3
#define ADCCLK_128 0x07
#define ADCCLK_64 0x06
#define ADCCLK_32 0x05
#define ADCCLK_16 0x04
#define ADCCLK_8 0x03
#define ADCCLK_4 0x02
#define ADCCLK_2 0x01

void ADC_Init(void)
{
	ADMUX = 0x07;	// Set External VREF, Right Justified, Chan 7
	DIDR0 = 0x00;	// No need to disable digital pins for CH7
	ADCSRB = 0;		// Sets free running Mode
	
	//initialise ADC result buffer

	adc_sample_cnt = 0;
	adc_sample_sum = 0;
	adc_data_ready = FALSE;
		
	adc_avg_cnt = 0;
	adc_avg_sum = 0;
	adc_avg_ready = FALSE;

	ADCSRA = (ADC_ENABLE|ADC_START|ADC_AUTO_TRIG|ADC_INT_ENABLE|ADCCLK_128);
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
void ADC_Shutdown(void)
{
	ADCSRA = 0;
	
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static int16 adc_res;
static int32 adc_val;

ISR(ADC_vect)
{
	if(adc_sample_cnt & 0x01)
		DEBUG_HI;
	else
		DEBUG_LO;
		
//	adc_res = (int16)ADCL & 0xff;
//	adc_res |= (((int16)ADCH << 8) & 0xff00);
	adc_res = (int16)ADC;
	adc_sample_sum += (int32)adc_res;
	if(++adc_sample_cnt >= ADC_SAMPLE_COUNT)
	{
//		adc_val = adc_sample_sum;
		adc_val = adc_sample_sum >> 8;
		adc_sample_cnt = 0;
		adc_sample_sum = 0;
		adc_data_ready = TRUE;
	}
			
}
int8 ADC_Get_average(int32 *res)
{
	int32 avg_val;
	
	if(adc_data_ready)
	{
		adc_avg_sum += adc_val;
		if(++adc_avg_cnt >= ADC_AVERAGE_COUNT)
		{
			avg_val = adc_avg_sum >> 8;
			adc_avg_cnt = 0;
			adc_avg_sum = 0;
			*res = avg_val;
			return TRUE;
		}
		
	}
	return FALSE;
}
/*********************************************************************
*						End of ADC.c								 *
*********************************************************************/
