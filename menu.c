/********************************************************************
*                                                                   *
*   Filename:   menu.c                                             *
*   Originator: C.Lewis                                             *
*   Project:    6489-90 Universal Interface Board Test Jig          *
*                                                                   *
*   This module is responsible for the debug menu generation and    *
*   Functionality                                                   *
*                                                                   *
********************************************************************/

/*==================================================================*/
/*                          INCLUDE FILES                           */
/*==================================================================*/
#include "ATMtypes.h"
#include "romdata.h"
#include "menu.h"
#include "Asci.h"
#include "romdata.h"
#include "i2c.h"
#include "adc.h"
#include "romdata.h"
#include "version.h"
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>


/*==================================================================*/
/*                      LOCAL FUNCTION PROTOTYPES                   */
/*==================================================================*/
static void Debug_menu(void);
static void Start_menu(void);
static void I2C_menu(void);
static void Test_menu(void);
static void Adc_debug_menu(void);

static void I2C_test(void);


static void Test_msg_func(void);
static int8 Cmd_check(int8);

/*==================================================================*/
/*                      LOCAL TYPE DEFINITIONS                      */
/*==================================================================*/

/*==================================================================*/
/*                      LOCAL MACRO DEFINITIONS                     */
/*==================================================================*/
#define STM_I2C_ADDR 0x10
#define DISPLAY_AMP_ERROR_CODE 0x11


/* definition used to echo the terminal keys pressed */
#define CMD_ECHO        TRUE
#define NO_CMD_ECHO     FALSE

/* NVRAM definitions */
#define TFR_COUNT 1
#define TFR_ADDR_LOW 0x72
#define TFR_ADDR_HIGH 0xCC
#define WRITE_CHAR_LOW 0x5A
#define WRITE_CHAR_HIGH 0xA5

#define MAX_PROUCT_CODE_LEN 25
#define CHECKSUM_MSB_ADDR 0x7e
#define CHECKSUM_LSB_ADDR 0x7f

#define EEPROM_ERASE_CHAR 0xff
#define EEPROM_RESET_CHAR 0x00
#define PRODUCT_PN_EEPROM_ADDR 0
#define PRODUCT_PN_EEPROM_LEN 9
#define PRODUCT_SN_EEPROM_ADDR 9
#define PRODUCT_SN_MAX_LEN 14

//Trigger func definitions
#define PULSE_WIDTH_100		100
#define PULSE_WIDTH_150		150
#define PULSE_WIDTH_1000	1000
#define PULSE_WIDTH_2000	2000

#define DELAY_COUNT 1000

/*==================================================================*/
/*                      GLOBAL CONSTANT DEFINITIONS                  */
/*==================================================================*/
int8 const NEWLINE_MSG[] PROGMEM =		{"\n\r"};
	
/*==================================================================*/
/*                      LOCAL CONSTANT DEFINITIONS                  */
/*==================================================================*/
 // Opening Menu Msgs
 int8 const COPYRIGHT_MSG[]  PROGMEM =	{"\n\n\n\n\n\n\n\n\n\n\n\r(c) Copyright The Magstim Company Ltd. 2025\n\n\r"};
 
 int8 const OPENING_MENU_MSG[] PROGMEM = {"\n\n\rEEG Battery Module - I2C Test Software\n\r"
                                          "======================================\n\r"
										  "TJ000004 Firmware Id: "};
 /* common menu messages */
 static int8 const CMD_NOT_IMPLEMENTED_MSG[] PROGMEM =	{" Command not implemented\n\r"};
 

/* declare Terminal Menus text*/
//*********************  Start Menu  ***************************
int8 const START_MENU_MSG[] PROGMEM =
{
	"\n\n\r"
	"Start MENU\n\r"
	"============\n\r"
	"T - Board tests\n\r"
	"D - Debug Menu\n\r"
	"<ENTER>- To refresh screen\n\n\r"
};
int8 const TEST_MENU_MSG[] PROGMEM =
{
	"\n\n\n\n\r"
	"TEST MENU\n\r"
	"============\n\r"
	"I - I2C test\n\r"
	"X - Exit to Start Menu\n\r"
};
int8 const DEBUG_MENU_MSG[] PROGMEM =
{
	"\n\n\n\n\r"
	"DEBUG MENU\n\r"
	"=============\n\r"
	"A - ADC test\n\r"
	"X - Exit to Start Menu\n\r"
};

int8 const I2C_MENU_MSG[] PROGMEM =
{
	"\n\n\n\n\rI2C Test"
	"\n\r========\n\n\r"
	"Ensure the Module is Programmed as a Master\n\r"
	"Connect the Touch Panel LCD to the Module and switch ON\n\n\r"
	"The '+' key will increment the AMP CODE value displayed by 1\n\r"
	"The '-' key will decrement the AMP CODE value displayed by 1\n\r"
	"Press 'X' to return to Test Menu\n\n\r"
};

int8 const ADC_DEBUG_MSG[] PROGMEM =
{
	"\n\n\n\n\rADC Debug"
	"\n\r=========\n\n\r"
	"\n\rPress X to exit\n\n\r"
};

/*==================================================================*/
/*      LOCAL INITIALISED VARIABLES (initialised to 0 by default)   */
/*==================================================================*/
/* general test menu vars */
static void (*cmd_bk_func)(void);
static void (*next_cmd_func)(void);

static const int8 *MEN_MSG_PTR;

/*********************************************************************
*                               FUNCTIONS                            *
*********************************************************************/
/*====================================================================
Name        :MEN_Init
Parameters  :NONE
Returns     :NONE
Description :Initializes some of the test module variables just in case
            :test menu is run latter. Sets the test state machine inactive
--------------------------------------------------------------------*/
void MEN_Init(void)
{
	ASC_Asci_msg((int8 *const)ROM_Read_romstr(COPYRIGHT_MSG));	//display Copyright msg
	ASC_Asci_msg((int8 *const)ROM_Read_romstr(OPENING_MENU_MSG));//display Opening msg
	sprintf(tmpstr,"%s\n\n\r", ROM_Read_romstr(VER_Get_sw_ver_str()));
	ASC_Asci_msg((int8 *const)tmpstr);//display Opening msg
	MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu); // Set start menu
}


/*====================================================================
Name        :MEN_bkproc
Parameters  :NONE
Returns     :NONE
Description :Called from the main loop, it executes the current cmd func if set
--------------------------------------------------------------------*/
void MEN_bkproc(void)
{
    /* check for and execute cmd func */
    if(cmd_bk_func)
        (*cmd_bk_func)();
}


/*====================================================================
Name        :MEN_Shutdown
Parameters  :NONE
Returns     :NONE
Description :Called from main() during system shutdown.
             It invalidates the cmd func
--------------------------------------------------------------------*/
void MEN_Shutdown(void)
{
    cmd_bk_func = NULL;
}

/*====================================================================
Name        :MEN_Set_cmd_bk_func
Parameters  :msg to display, next cmd func to execute
Returns     :NONE
Description :Loads the passed vars to local vars and sets the cmd func to display
             the passed message and set the passed cmd func
--------------------------------------------------------------------*/
void MEN_Set_cmd_bk_func(const int8  *MSG_PTR, void * const next_cmd_ptr)
{
    /* store passed params locally */
    MEN_MSG_PTR = MSG_PTR;
    next_cmd_func = next_cmd_ptr;
    /* set cmd func */
    cmd_bk_func = Test_msg_func;
}

/*====================================================================
Name        :Test_msg_func
Parameters  :NONE
Returns     :NONE
Description :Displays the message, stored in the program space,  pointed
             to by msg_ptr var.
             If the message length exceeds the MAX_ASCI_STRING length, it
             displays MAX_ASCI_STRING characters of the message and then
             exits. When the background process calls this function again
             it displays the next MAX_ASCI_STRING characters. It repeats
             this until the whole string has been displayed.
             Once the message has been displayed it sets the cmd func
             according to the next_cmd_func var.
--------------------------------------------------------------------*/
static void Test_msg_func(void)
{
    int16 len;
    int8 *ram_ptr;

    /* check for valid msg */
    if(MEN_MSG_PTR)
    {
        /* first set the len var according to the length of the string */
        len = ROM_Romstrlen(MEN_MSG_PTR);       /* get length of ROM string */
        if(len > MAX_ASCI_STRING)       /* check if len exceeds max length */
            len = MAX_ASCI_STRING;      /* limit string length */

        /*   wait until enough space in ASCI buffer */
        while(ASC_Get_asci_tx_space() < len);

        /* now move data from ROM to RAM and send to asci */
        ram_ptr = ROM_Read_romdata(MEN_MSG_PTR,len);
        MEN_MSG_PTR += len;                 /* adjust msg ptr */

        ASC_Asci_tx((const int8 *)ram_ptr,len);       /* send 'len' bytes of data */

        /* reset msg ptr if NUL char detected */
        if(pgm_read_byte(MEN_MSG_PTR) == 0)         /* check for '\0' char */
            MEN_MSG_PTR = NULL;                     /* reset msg ptr */
    }
    else  /* msg ptr is NULL so set new cmd func */
        cmd_bk_func = next_cmd_func;
}

/*====================================================================
Name        :Cmd_check
Parameters  :echo ON/OFF stat
Returns     :RS232 RX char, or 0 if empty
Description :Checks the ASC_Asci buffer for RX chars
             If detected it echos the char if the echo_stat is TRUE
--------------------------------------------------------------------*/
static int8 Cmd_check(int8 echo_stat)
{
    char c;

    /* return if no RX chars available */
    if(ASC_Asci_getchar((int8 *)&c) == ASCI_EMPTY)
        return 0;

    /* check echo_stat */
    if(echo_stat)
    {
        /* echo RX char */
        if(c == '\r')
            ASC_Asci_msg(ROM_Read_romstr(NEWLINE_MSG));
        else
            ASC_Asci_tx((const int8 *)&c,1);
    }
    /* return RX char */
    return c;
}

//========================================================================================================================================
//	Product Specific Menus
//========================================================================================================================================

/*====================================================================
Name        :Start_menu
Parameters  :NONE
Returns     :NONE
Description :Set as the cmd_func when command menu is invoked
             It gets any RX char and processes them according to their value
--------------------------------------------------------------------*/

static void Start_menu(void)
{
	int8 rx_byte;

	/* get any RX chars */
	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
	return;

	/* now process RX char */
	switch(rx_byte)
	{
		case 'T':
		case 't':
			MEN_Set_cmd_bk_func(TEST_MENU_MSG,Test_menu);
			break;
		case 'D':
		case 'd':
			MEN_Set_cmd_bk_func(DEBUG_MENU_MSG,Debug_menu);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
		break;
	}
}
/*====================================================================
Name        :Debug_menu
Parameters  :NONE
Returns     :NONE
Description :Set as the cmd_func when command menu is invoked
             It gets any RX char and processes them according to their value
--------------------------------------------------------------------*/
static void Debug_menu(void)
{
    int8 rx_byte;

    /* get any RX chars */
    rx_byte = Cmd_check(CMD_ECHO);
    /* return if none available */
    if(!rx_byte)
        return;

    /* now process RX char */
    switch(rx_byte)
    {
		case 'A':
		case 'a':
			ADC_Init();
			MEN_Set_cmd_bk_func(ADC_DEBUG_MSG,Adc_debug_menu);
			break;
		case 'x':
		case 'X':
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
			break;
	    default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(DEBUG_MENU_MSG,Debug_menu);
		    break;
    }
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void Test_menu(void)
{
	int8 rx_byte;

	/* get any RX chars */
	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
	return;

	/* now process RX char */
	switch(rx_byte)
	{
		case 'I':
		case 'i':
			MEN_Set_cmd_bk_func(I2C_MENU_MSG,I2C_menu);
			break;
		case 'x':
		case 'X':
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
			break;
		default:
			MEN_Set_cmd_bk_func(DEBUG_MENU_MSG,Debug_menu);
			break;
	}


	
}

/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static int8 cur_i2c_err_code;

static void I2C_menu(void)
{
	int8 buf[5];


	I2C_Init();
	
	// Set initial AMP CODE to 1
	cur_i2c_err_code = 1;
	// Display Current AMP CODE on terminal
	sprintf((char *)tmpstr,"Displaying %02x\r",cur_i2c_err_code);
	ASC_Asci_msg(tmpstr);
				
	//Set i2c data buffer
	buf[0] = DISPLAY_AMP_ERROR_CODE;	// set code to display AMP CODE on LCD
	buf[1] = cur_i2c_err_code;	// Set data to display as AMP CODE on LCD
				
	// Send 2 bytes of data stored in buf, to STM I2C interface
	I2C_Write(STM_I2C_ADDR,2,buf);
				
	MEN_Set_cmd_bk_func(NULL,I2C_test);
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
	
static void I2C_test(void)
{
	int8 rx_byte;
	int8 buf[5];


	/* get any RX chars */
	rx_byte = Cmd_check(NO_CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
		return;

	switch(rx_byte)
	{
		case 'X':
		case 'x':	
			I2C_Shutdown();
			MEN_Set_cmd_bk_func(TEST_MENU_MSG,Test_menu);
			return;
		case '+':
			cur_i2c_err_code++;
			break;
		case '-':
			cur_i2c_err_code--;
			break;
	}
	// Send 2 bytes of data stored in buf, to STM I2C interface
	sprintf((char *)tmpstr,"Displaying %02x\r",cur_i2c_err_code);
	ASC_Asci_msg(tmpstr);
	//Set i2c data buffer
	buf[0] = DISPLAY_AMP_ERROR_CODE;	// set code to display AMP CODE on LCD
	buf[1] = cur_i2c_err_code;	// Set data to display as AMP CODE on LCD
	I2C_Write(STM_I2C_ADDR,2,buf);
		
} 
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/

static void Adc_debug_menu(void)
{
	int32 adcval;
	int8 rx_byte;
	int32 adcvolts;
	
	if(ADC_Get_average(&adcval))
	{
		
		adcvolts = ADC_Get_adc_millivolts(adcval);
		
//		sprintf(tmpstr,"CH7 = %f %f %f\r",(double)adcval,(double)CONV_FACTOR, (double)adcvolts);
		sprintf(tmpstr,"CH7 = %umV    \r",adcvolts);
		ASC_Asci_msg(tmpstr);
	}
	

	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
	return;

	/* now process RX char */
	switch(rx_byte)
	{
		case 'x':
		case 'X':
			ADC_Shutdown();
			MEN_Set_cmd_bk_func(DEBUG_MENU_MSG,Debug_menu);
			break;
	}
}

/*********************************************************************
*                       End of menu.c                                *
*********************************************************************/
