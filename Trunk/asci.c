/*$Header$*/
/********************************************************************
*																	*
*	Filename:		asci.c											*
*	Originator:		Chris Lewis										*
*	Project:		Interface Board I2C Test Software				*
*	Description:													*
*																	*
********************************************************************/


/*==================================================================*/
/*							INCLUDE FILES							*/
/*==================================================================*/
#include "ATMtypes.h"
#include "asci.h"
#include "romdata.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include <avr/pgmspace.h>

/*==================================================================*/
/*						LOCAL MACRO DEFINITIONS						*/
/*==================================================================*/
typedef enum validascierrs 
{
	ASCI_FRAMEERR,
	ASCI_OVERRUNERR,
	ASCI_PARITYERR,
	ASCI_TXBUFOVERFLOW,
	LAST_ASCI_ERR
} ASCIERRS;

char  *error_msgs[LAST_ASCI_ERR] =
{
	"\n\rSerial Framing Error",
	"\n\rSerial Data Overrun Error",
	"\n\rSerial Parity Error",
	"\n\rSerial TX buf overflow Error"
};


/* define BRR value set achieve 9600 baud rate, given Fosc is 16MHz

	BBR = ( Fosc /( 16 x baudrate)) - 1 )	*/
	
#define BRRVAL ((16000000L/(16L*9600L))-1L)
//#define BRRVAL 103
#define ASCI_N81_CODE 0x06
                                 
/* define ASCI buffer sizes */                   
#define ASCI_RX_BUF_LEN 10
/*==================================================================*/
/* 						LOCAL FUNCTION PROTOTYPES 					*/
/*==================================================================*/
static void ASC_err(ASCIERRS err);
static int16 Kbhit(void);

/*==================================================================*/
/*		LOCAL INITIALISED VARIABLES (initialized to 0 by default)	*/
/*==================================================================*/
/* declare asci buffer vars */
static int8 volatile rx_ip_pos = 0;    			
static int8 volatile rx_op_pos = 0;
static int8 volatile tx_ip_pos = 0;
static int8 volatile tx_op_pos = 0;
static int8 tx_buf[ASCI_TX_BUF_LEN];
static int8 rx_buf[ASCI_RX_BUF_LEN];

static int16 asci_err;

static PROGMEM int8 const pause_msg[] = {"\n\rPAUSED\n\rPress any key to continue\n\r"};
/*********************************************************************
*					SYSTEM INTERFACE FUNCTIONS					 	 *
*********************************************************************/
/*====================================================================
Name		:Init_asci
Parameters	:None
Returns		:NONE
Description	:Enables the USART for 9600,n,8,1
--------------------------------------------------------------------*/
void ASC_Init_asci(void)
{              
	/* reset the tx and rx buffer positions */
	rx_ip_pos = 0;
	rx_op_pos = 0;
	tx_ip_pos = 0;
	tx_op_pos = 0;
	
	/* set UBRR High and Low registers with the BRRVAL for 9600 baud rate */
	UBRR0H = (BRRVAL >> 8);
	UBRR0L = (BRRVAL & 0x0ff);
	
	/* now set no parity, 8 data bits and 1 stop bit */
	UCSR0C = ASCI_N81_CODE;

	/* now enable the transmitter, receiver and rx interrupt */
	UCSR0B = ((1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0));	
}
/*====================================================================
Name		:Shutdown_asci
Parameters	:NONE
Returns		:NONE
Description	:Disables ASCI 
--------------------------------------------------------------------*/
void ASC_Shutdown_asci(void)
{                         
	/* wait for TX buf to empty 
	while(tx_ip_pos != tx_op_pos);*/
	/* wait for last tx char to leave serial buf */
/*	while(!(UCSR0A & (1 << TXC0)));           
	 disable ASCI */
    UCSR0B = 0;
}	                        


/*********************************************************************
*					ASCI TX FUNCTIONS							 	 *
*********************************************************************/
/*====================================================================
Name		:ASC_Asci_tx
Parameters	:ptr to chars to be sent, no of chars to send
Returns		:ASCI_FULL Error code is not enough space in buf, else returns ASCI_OK code
Description	:Low level tx function 
			 Loads the TX buf with the data passed according to the no of chars
 			 passed.                                                  
 			 If asci TX is not already enabled it starts the tx process.
--------------------------------------------------------------------*/
int8 ASC_Asci_tx(int8 *ptr,int8 count)
{
	int8 x,ip;

	/* first check if enough space in tx buf */
	while(count > ASC_Get_asci_tx_space());
//	{
//		ASC_err(ASCI_TXBUFOVERFLOW);
//		return ASCI_FULL; 	/* return error code */
//	}

	/* now load buffer - adjusting i/p pos after buffer is loaded */
	ip = tx_ip_pos;				/* make copy of ip pos */
	for(x = 0; x < count;x++)	/* now load tx buf */			
	{
		tx_buf[ip] = ptr[x];
		if(++ip >= ASCI_TX_BUF_LEN) /* check and process buffer roll over */
			ip = 0; 
	}

	/* disable ints before adjusting ip pos */	
	cli();
	tx_ip_pos = ip;				/* adjust ip pos */

	/* Now check if TX INT is still active */
	if(!(UCSR0B & (1 <<UDRIE0)))
	{  
		/* TX INT not ENABLED so load first Char*/
		UDR0 = tx_buf[tx_op_pos];          

		/* check and process buffer roll over */
		if(++tx_op_pos >= ASCI_TX_BUF_LEN)	
			tx_op_pos = 0;
	                           
	   	UCSR0B |= (1 << UDRIE0);	/* enable TDRE int */
	}
	
	sei();				/* re-enable int before returning */     
	
	/* wait for message to be transmitted before returning */
//	while(tx_ip_pos != tx_op_pos);
	                  
	return ASCI_OK;		/* return success code */
} 
/*====================================================================
Name		:ISR(USART0_UDRE_vect)
Parameters	:NONE
Returns		:NONE
Description	:Local ISR routine called when an interrupt is generated
			as a result of the tx data reg becoming empty.
			 It checks to see if there is any more data in the TX buf
			 to send. If so it sends it. If not it disables the TX int.
--------------------------------------------------------------------*/
ISR(USART_UDRE_vect)
{                                                     
	/* check if buffer is empty */
	if(tx_ip_pos == tx_op_pos)	
	   	UCSR0B &= ~(1 << UDRIE0);	/* buffer is empty so disable TDRE int */
    else   /* buffer is not empty so send next char */
    {
		UDR0 = tx_buf[tx_op_pos];	/* put next char in TX reg */
		if(++tx_op_pos >= ASCI_TX_BUF_LEN)  /* check and process buffer rollover */
			tx_op_pos = 0;
    }
}
/*====================================================================
Name		:ASC_Asci_msg
Parameters	:ptr to ASCII msg to send
Returns		:No. of bytes sent or 0 if buffer full
Description	:Prints the NUL terminated string to the asci if enough
			 space is available in the buffer.
			 Determines the no. of bytes to send from the strlen function.
			 Does not transmit the NUL character
--------------------------------------------------------------------*/
int8 ASC_Asci_msg(int8 *msg)
{                 
	int8 n;
	
	/* get string lentgh */
	n = (int8)strlen((const char *)msg);
	 
	/* call low level Asci_tx func */
	if(ASC_Asci_tx(msg,n) == ASCI_OK)
		return n;  		/* return no of bytes sent if buffered OK */
	else
		return 0;       /* return 0 indicating not enough buffer space */
		
}          
/*====================================================================
Name		:Get_asci_tx_space
Parameters	:NONE
Returns		:No. of free bytes in TX buf 
Description	:Calculates and returns no. of free bytes in tx buf.
--------------------------------------------------------------------*/
int8 ASC_Get_asci_tx_space(void)
{                      
	int16 ip,op;     
	
	/* make copy of ip pos */
	cli();
	ip = tx_ip_pos;
	op = tx_op_pos;
	sei();
	if(ip < op)       	/* check and process buffer rollover */
		ip += ASCI_TX_BUF_LEN;               
		      
	 /* calc and return no of free tx buf bytes */		
	return ((ASCI_TX_BUF_LEN-1) - (ip - op));   
}  

/*====================================================================
Name		:ASC_Asci_tx_empty
Parameters	:NONE
Returns		:TRUE if tx buf is empty, FALSE if not
Description	:returns TRUE or FALSE according to state of TX buf
--------------------------------------------------------------------*/
int8 ASC_Asci_tx_empty(void)
{                         
	/* check if buffer is empty */
	if(tx_ip_pos == tx_op_pos)
		return TRUE;
	else
		return FALSE;
}
/*********************************************************************
*					ASCI RX FUNCTIONS							 	 *
*********************************************************************/
/*====================================================================
Name		:ISR(USART0_RX_vect)
Parameters	:NONE
Returns		:NONE
Description	:ISR routine called if RX empty int is detected.
			 The int is cleared automatically when the ISR is executed.
			 This isr check for any rx errors by reading the UCSD0A register.
			 If errors are detected it sets error flags. It then reads the 
			 received byte from the RX reg and stores it in the rx buffer 
			 then moves the buffer input to the next position
--------------------------------------------------------------------*/
ISR(USART_RX_vect)
{  
	int8	rxerr,rxdata;
	
	rxerr = UCSR0A;	/* read status reg to check err flags */
	                             
	/* read rx byte and store rx data in RX buffer */
	rxdata = UDR0; /* read data which clears RXC0 flag and any rx errors */
	rx_buf[rx_ip_pos] = rxdata;
	
	/* now process rx buffer */
	if(++rx_ip_pos >= ASCI_RX_BUF_LEN) /* check and process buffer rollover */
		rx_ip_pos = 0;

	/* now process error flag and set system error accordingly */
	if(rxerr & ((1 << FE0) | (1 << DOR0) | (1 << UPE0)))
	{
		if(rxerr & (1 << FE0)) /* check for frame error */
			ASC_err(ASCI_FRAMEERR);
		else if(rxerr & (1 << DOR0)) /* check for overrun error */
			ASC_err(ASCI_OVERRUNERR);
		else /* must be parity error */
			ASC_err(ASCI_PARITYERR);
	}
}
/*====================================================================
Name		:ASC_Asci_getchar
Parameters	:ptr to storage byte
Returns		:ASCI EMPTY Code if no chars received, else ASCI_OK
Description	:Stores the next char in the RX buf at the passed ptr if available,
			 then processes RX buf. If no data in then it return ASCI_EMPTY
--------------------------------------------------------------------*/
int8 ASC_Asci_getchar(int8 *char_ptr)
{              
	/* check for empty RX buf */
	if(rx_ip_pos == rx_op_pos)
		return ASCI_EMPTY;    /* return empty err code */
	else  /* store next char and process RX buf */
	{    
		       
		cli();								/* disable int before accessing RX ptrs */	
		*char_ptr = rx_buf[rx_op_pos];		/* copy next RX buf char to ptr */
		if(++rx_op_pos >= ASCI_RX_BUF_LEN)  /* check and process buffer rollover */
			rx_op_pos = 0;
		sei();								/* re-enable ints */
	}                
	return ASCI_OK;							/* return success code */
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void ASC_err(ASCIERRS err)
{	
	asci_err = err;	
	ASC_Shutdown_asci();
	ASC_Init_asci();
	
	sprintf((char *)tmpstr,"\n\n\rSYSTEM ERROR\n\r%s\n\r",error_msgs[err]);
	ASC_Asci_msg(tmpstr);	
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
int16 Kbhit(void)
{
	int8 c;
	
	if(ASC_Asci_getchar(&c) == ASCI_EMPTY)
		return FALSE;
	else
		return TRUE;
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
void ASC_Pause(void)
{
	ASC_Asci_msg((int8 *const) ROM_Read_romstr(pause_msg));
	while(!Kbhit());
}
/*********************************************************************
*						End of asci.c								 *
*********************************************************************/
