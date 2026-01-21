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
#include "romdata.h"
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>

/*======================================================================*/
/*							LOCAL TYPEDEFS								*/
/*======================================================================*/
/*------------------------------------------------------------------------
Name		:
Description	:
------------------------------------------------------------------------*/
typedef struct _adc_data 
{
	int32 volatile sum;
	int16 volatile avg;
	int16 volatile sample_count;
	int8 volatile data_ready;
}ADC_DATA;

/*==================================================================*/
/*						LOCAL MACRO DEFINITIONS						*/
/*==================================================================*/
// Define millivolt conversion factor based on a reference voltage of 4.0V
// millivolts = (VREF/(ADC Resolution-1)) * 0x10000
// => (4000/2023) * 65536 = 256250.244
//#define CONV_FACTOR 256250L // 0x100 samples
//#define CONV_FACTOR 128125L	// 0x80 samples
#define CONV_FACTOR 1001L	// 0x80 samples

// Now define Channel scaling based on HW gains
#define ADC_CH1_CONV_FACTOR (CONV_FACTOR * 4)
#define ADC_CH2_CONV_FACTOR (CONV_FACTOR * 4)
#define ADC_CH3_CONV_FACTOR (CONV_FACTOR * 2)
#define ADC_CH6_CONV_FACTOR (CONV_FACTOR * 4) //960937L
#define ADC_CH7_CONV_FACTOR (CONV_FACTOR * 1)


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



/*==================================================================*/
/*						LOCAL CONSTANT DEFINITIONS					*/
/*==================================================================*/
//#define ADC_SAMPLE_COUNT 0x100L
#define ADC_SAMPLE_COUNT 0x80L
int8 CH1_NAME[] =	{"EXTBAT"};
int8 CH2_NAME[] =	{"INTBAT"};
int8 CH3_NAME[] =	{"+5V IN"};
int8 CH6_NAME[] =	{"VBAT OUT"};
int8 CH7_NAME[] =	{"3V3 OUT"};
int8 *chan_names[ADC_CHAN_COUNT] = {CH1_NAME,CH2_NAME,CH3_NAME,CH6_NAME,CH7_NAME};

/*==================================================================*/
/*		LOCAL INITIALISED VARIABLES (initialised to 0 by default)	*/
/*==================================================================*/
// define variables for ADC channel data collection
static int8 const adc_mux_chans[ADC_CHAN_COUNT] = {0x01,0x02,0x03,0x06,0x07};
//	static int8 const adc_mux_chans[ADC_CHAN_COUNT] = {0x07,0x06};
static int32 const adc_conv_factor[ADC_CHAN_COUNT] = {ADC_CH1_CONV_FACTOR,ADC_CH2_CONV_FACTOR,ADC_CH3_CONV_FACTOR,ADC_CH6_CONV_FACTOR,ADC_CH7_CONV_FACTOR};	
//	static int32 const adc_conv_factor[ADC_CHAN_COUNT] = {ADC_CH7_CONV_FACTOR,ADC_CH6_CONV_FACTOR};	
static ADC_DATA adcdata[ADC_CHAN_COUNT];		
static int8 volatile cur_adc_mux_ix = 0;			// current mux channel
static CHAN_AVERAGE cur_avg;
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
void ADC_Init(void)
{
	int8 x;
	
	cur_adc_mux_ix = 0; //reset index for mux chans
	
	ADMUX = adc_mux_chans[cur_adc_mux_ix];	// Set External VREF, Right Justified, 1st mux chan
	ADCSRB = 0;		// Sets free running Mode
	
	//initialise ADC result buffer

	for(x = 0; x < ADC_CHAN_COUNT; x++)
	{
		adcdata[x].sum = 0;
		adcdata[x].avg = 0;
		adcdata[x].sample_count = 0;
		adcdata[x].data_ready = FALSE;
	}

	// Start ADC conv
	//ADCSRA = (ADC_ENABLE|ADC_START|ADC_AUTO_TRIG|ADC_INT_ENABLE|ADCCLK_128);
	ADCSRA = (ADC_ENABLE|ADC_START|ADC_INT_ENABLE|ADCCLK_128);
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
ISR(ADC_vect)
{
	int16 adc_res;
	int8 cur_chan;
	
	adc_res = (int16)ADC;		// get result for current chan
	cur_chan = cur_adc_mux_ix;	// copy cur chan
	// now set next conv channel
	if(++cur_adc_mux_ix >= ADC_CHAN_COUNT)
		cur_adc_mux_ix = 0;

	//set next mux chan
	ADMUX = adc_mux_chans[cur_adc_mux_ix];	// Set External VREF, Right Justified, set next mux chan
	ADCSRA = (ADC_ENABLE|ADC_START|ADC_INT_ENABLE|ADCCLK_128);

	// next mux channel is set so process current adc data
	adcdata[cur_chan].sum += (int32)adc_res;	// add current data to sum
	
	// now see if ready to calc average
	if(++(adcdata[cur_chan].sample_count) >= ADC_SAMPLE_COUNT)
	{
		// sample count has expired so get average
		adcdata[cur_chan].sample_count = 0;
		adcdata[cur_chan].avg = adcdata[cur_chan].sum / ADC_SAMPLE_COUNT;
		adcdata[cur_chan].sum = 0;	
		adcdata[cur_chan].data_ready = TRUE;
	}
			
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
int8 ADC_Get_average_millivolts(int16 *millivolt_res, ADC_CHAN_ID chan)
{
	if(adcdata[chan].data_ready)
	{
		*millivolt_res = (int16)(((int32)adcdata[chan].avg * adc_conv_factor[chan]) >> 8);
		adcdata[chan].data_ready = FALSE;
		return TRUE;
	}
	return FALSE;
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
int8* ADC_Get_chan_name(ADC_CHAN_ID chan)
{
	return chan_names[chan];
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
CHAN_AVERAGE *ADC_Get_chan_average(ADC_CHAN_ID chan,int8 avg_size)
{
	int16 avg;
	// initialise average struct
	cur_avg.max = 0;
	cur_avg.min = 0xffff;
	cur_avg.avg = 0;
	cur_avg.sum = 0;
	
	for(cur_avg.cnt = 0; cur_avg.cnt < avg_size;cur_avg.cnt++)
	{
		// wait for averagee to complete
		while(!ADC_Get_average_millivolts(&avg,chan));
		
		if(avg < cur_avg.min)
			cur_avg.min = avg;
		if(avg > cur_avg.max)
			cur_avg.max = avg;
		cur_avg.sum += (int32)avg;
	}
	// avg_size samples have been summed so calc average
	cur_avg.avg = (int16)(cur_avg.sum/(int32)avg_size);
	return &cur_avg;
	
}
/*********************************************************************
*						End of ADC.c								 *
*********************************************************************/
