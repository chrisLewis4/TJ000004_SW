/********************************************************************
*																	*
*	Filename:		menu.c											*
*	Originator:		Chris Lewis										*
*	Project:		EEG Battery Module TestJig Software				*
*	Description:													*
*																	*
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
#include "Timer.h"
#include "romdata.h"
#include "version.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

/*==================================================================*/
/*                      LOCAL TYPE DEFINITIONS                      */
/*==================================================================*/

typedef struct _voltage_limits
{
	int16 nominal;
	int16 limit;
}VOLTAGE_LIMITS;

/*==================================================================*/
/*                      LOCAL FUNCTION PROTOTYPES                   */
/*==================================================================*/
static void Start_menu(void);
static void Test_menu(void);
static void Check_extbat(void);
static int8 Check_chan_voltages(ADC_CHAN_ID chan,VOLTAGE_LIMITS *vlim);
static void Connect_j8_menu(void);
static void Check_intbat(void);
static void Check_5v(void);
static void Check_threshold1_menu(void);



static void Debug_menu(void);
static void I2C_menu(void);
static void Adc_debug_menu(void);
static void Control_debug_menu(void);
static void Display_control_status(void);
static void Adc_display_values(void);


static void I2C_test(void);

static void Test_msg_func(void);
static int8 Cmd_check(int8);
static void Retry_or_exit_menu(void);
static void Set_retry_func( void * const next_cmd_ptr);

/*==================================================================*/
/*                      LOCAL MACRO DEFINITIONS                     */
/*==================================================================*/
#define EXTBAT_VOLTAGE 14000	// 14000mV nominal
#define EXTBAT_LIIMIT 100		//100mV tolerance
#define INTBAT_VOLTAGE 15000	// 15000mV nominal
#define INTBAT_LIIMIT 100		//100mV tolerance
#define _3V3_VOLTAGE 3300	// 3300 nominal
#define _3V3_LIIMIT 150		//150mV tolerance
#define _5V_VOLTAGE 5000	// 5000mV nominal
#define _5V_LIIMIT 200		//200mV tolerance



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
int8 const NEWLINE_MSG[] PROGMEM =	{"\n\r"};
int8 const ON_MSG[] PROGMEM =		{"ON"};
int8 const OFF_MSG[] PROGMEM =		{"OFF"};
int8 const NEWPAGE_MSG[] PROGMEM =		{"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\r"};
	
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
	"==========\n\r"
	"T - Board test\n\r"
	"D - Debug Menu\n\r"
	"<ENTER>- To refresh screen\n\n\r"
};
int8 const TEST_MENU_MSG[] PROGMEM =
{
	"\n\n\n\n\r"
	"Board Test\n\r"
	"==========\n\r"
	"Ensure the EXTBAT PSU is set to 14.0V ±0.1V\n\r"
	"Ensure the INTBAT PSU is set to 15.0V ±0.1V\n\r"
	"Connect the Board Under Test (BUT) to the Jig\n\r"
	"\n*** DO NOT Connect J8 at this stage ***\n\n\r"
	"Turn the External PSUs ON\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};
int8 const PASS_MSG[] PROGMEM =
{
	"PASS\n\r"
};
int8 const FAIL_MSG[] PROGMEM =
{
	"FAIL\n\r"
};

int8 const ADJUST_VOLTAGE_MSG[] PROGMEM =
{
	"\n\r*** Adjust Input Voltage ***\n\r"
};

int8 const CONNECT_J8_MSG[] PROGMEM =
{
	"\n\rConnect the test jig to J8 on the BUT\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};


int8 const POLYFUSE_VOLTAGE_ERR_MSG[] PROGMEM =
{
	"\n\n\r*** +5V Out of tolerance ***\n\n\r"
};

int8 const CHECK_THRESHOLD1_MSG[] PROGMEM =
{
	"\n\rSlowly Reduce the the EXTBAT voltage until Message appears\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};

int8 const RETRY_OR_EXIT_MSG[] PROGMEM =
{
	"\n\rPress 'X' to exit or ENTER to Retry\n\r"
};


int8 const DEBUG_MENU_MSG[] PROGMEM =
{
	"\n\n\n\n\r"
	"DEBUG MENU\n\r"
	"=============\n\r"
	"A - ADC test\n\r"
	"C - I/O Control test\n\r"
	"I - I2C test\n\r"
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
	"\n\r     All Data in millivolts\n\r"
	"\n\r  EXT   INT   +5V   OUT   3V3\n\r"
};
int8 const ADC_CHAN_LABELS_MSG[] PROGMEM =
{
	"\n\r  EXT   INT   +5V   OUT   3V3\n\r"
};

int8 const CONTROL_DEBUG_MSG[] PROGMEM =
{
	"\n\n\n\n\rI/O Control Debug Menu"
	"\n\r======================\n\n\r"
	"Current Status:\n\r"
};	

int8 const CONTROL_DEBUG_MENU_MSG[] PROGMEM =
{
	"\n\rE - Toggle EXTBAT status\n\r"
	"I - Toggle INTBAT status\n\r"
	"P - Toggle Polyfuse Load status\n\r"
	"\n\rPress X to exit\n\n\r"
};

/*==================================================================*/
/*      LOCAL INITIALISED VARIABLES (initialised to 0 by default)   */
/*==================================================================*/
/* general test menu vars */
static void (*cmd_bk_func)(void);
static void (*next_cmd_func)(void);
static void (*retry_cmd_func)(void);
static const int8 *MEN_MSG_PTR;

static VOLTAGE_LIMITS extbat_lims = {EXTBAT_VOLTAGE,EXTBAT_LIIMIT};
static VOLTAGE_LIMITS intbat_lims = {INTBAT_VOLTAGE,INTBAT_LIIMIT};
static VOLTAGE_LIMITS _3v3_lims = {_3V3_VOLTAGE,_3V3_LIIMIT};
static VOLTAGE_LIMITS _5v_lims = {_5V_VOLTAGE,_5V_LIIMIT};

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
	sprintf((char *)tmpstr,"%s\n\n\r", ROM_Read_romstr(VER_Get_sw_ver_str()));
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

void Set_retry_func( void * const next_cmd_ptr)
{
	retry_cmd_func = next_cmd_ptr;
	MEN_Set_cmd_bk_func(RETRY_OR_EXIT_MSG,Retry_or_exit_menu);
}
void Retry_or_exit_menu(void)
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
		case 'x':
		case 'X':
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
			break;
		case '\r':
		case '\n':
			MEN_Set_cmd_bk_func(NULL,retry_cmd_func);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(RETRY_OR_EXIT_MSG,Retry_or_exit_menu);
		break;
	}	
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

        ASC_Asci_tx((int8 *)ram_ptr,len);       /* send 'len' bytes of data */

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
            ASC_Asci_tx((int8 *)&c,1);
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
			MEN_Set_cmd_bk_func(ADC_DEBUG_MSG,Adc_debug_menu);
			break;
		case 'C':
		case 'c':
			MEN_Set_cmd_bk_func(CONTROL_DEBUG_MSG,Display_control_status);
			break;
		case 'x':
		case 'X':
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
			break;
		case 'I':
		case 'i':
			MEN_Set_cmd_bk_func(I2C_MENU_MSG,I2C_menu);
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
static void Display_control_status(void)
{
	int8 *msg;
	
	if(MAI_Get_control_status(EXTBAT_CNTRL) == ON)
		msg = ROM_Read_romstr(ON_MSG);
	else
		msg = ROM_Read_romstr(OFF_MSG);
	sprintf((char *)tmpstr,"EXTBAT = %s\n\r",msg);
	ASC_Asci_msg(tmpstr);
	
	if(MAI_Get_control_status(INTBAT_CNTRL) == ON)
		msg = ROM_Read_romstr(ON_MSG);
	else
		msg = ROM_Read_romstr(OFF_MSG);
	sprintf((char *)tmpstr,"INTBAT = %s\n\r",msg);
	ASC_Asci_msg(tmpstr);
	
	if(MAI_Get_control_status(POLYFUSE_CNTRL) == ON)
		msg = ROM_Read_romstr(ON_MSG);
	else
		msg = ROM_Read_romstr(OFF_MSG);

	sprintf((char *)tmpstr,"PFLOAD = %s\n\r",msg);
	ASC_Asci_msg(tmpstr);
	
	MEN_Set_cmd_bk_func(CONTROL_DEBUG_MENU_MSG,Control_debug_menu);

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
		case 'x':
		case 'X':
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
			break;
		case '\r':
		case '\n':
			MEN_Set_cmd_bk_func(NULL,Check_extbat);
			break;		
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(TEST_MENU_MSG,Test_menu);
			break;
	}	
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void Check_extbat(void)
{
	CHAN_AVERAGE *ca;
	
	MAI_Set_control_status(EXTBAT_CNTRL,ON); // Turn on EXTBAT supply to BUT
	TIM_Wait(500);	// set 0.5s delay
	if(!Check_chan_voltages(ADC_EXTBAT,&extbat_lims))
	{
		ASC_Asci_msg((int8 *const)ROM_Read_romstr(ADJUST_VOLTAGE_MSG));
		Set_retry_func(Check_extbat);
		
	}
	else
	{
		if(Check_chan_voltages(ADC_3V3,&_3v3_lims))
		{
			MEN_Set_cmd_bk_func(NULL,Check_intbat);
		}
		else
		{
			Set_retry_func(Check_extbat);
		}
	}
	MAI_Set_control_status(EXTBAT_CNTRL,OFF); // Turn on EXTBAT supply to BUT

}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/

static void Check_intbat(void)
{
	CHAN_AVERAGE *ca;
	
	MAI_Set_control_status(INTBAT_CNTRL,ON); // Turn on EXTBAT supply to BUT
	TIM_Wait(500);	// set 0.5s delay
	if(!Check_chan_voltages(ADC_INTBAT,&intbat_lims))
	{
		ASC_Asci_msg((int8 *const)ROM_Read_romstr(ADJUST_VOLTAGE_MSG));
		Set_retry_func(Check_intbat);
		
	}
	else
	{
		if(Check_chan_voltages(ADC_3V3,&_3v3_lims))
		{
			MEN_Set_cmd_bk_func(CONNECT_J8_MSG,Connect_j8_menu);
		}
		else
		{
			Set_retry_func(Check_intbat);
		}
	}
	MAI_Set_control_status(INTBAT_CNTRL,OFF); // Turn on EXTBAT supply to BUT

}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void Connect_j8_menu(void)
{
	int8 rx_byte;

//	ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWLINE_MSG));

	/* get any RX chars */
	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
		return;

	/* now process RX char */
	switch(rx_byte)
	{
		case 'x':
		case 'X':
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
			break;
		case '\r':
		case '\n':
			MEN_Set_cmd_bk_func(NULL,Check_5v);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(CONNECT_J8_MSG,Connect_j8_menu);
		break;
	}
	
	
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void Check_5v(void)
{
	CHAN_AVERAGE *ca;
	
//	MAI_Set_control_status(INTBAT_CNTRL,ON); // Turn on EXTBAT supply to BUT
//	TIM_Wait(500);	// set 0.5s delay
	if(!Check_chan_voltages(ADC_POLYFUSE,&_5v_lims))
	{
		Set_retry_func(Check_5v);
	}
	else
	{
		if(Check_chan_voltages(ADC_3V3,&_3v3_lims))
			MEN_Set_cmd_bk_func(CHECK_THRESHOLD1_MSG,Check_threshold1_menu);
		else
			Set_retry_func(Check_5v);
	}
	MAI_Set_control_status(INTBAT_CNTRL,OFF); // Turn on EXTBAT supply to BUT

}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void Check_threshold1_menu(void)
{
	int8 rx_byte;

	//	ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWLINE_MSG));

	/* get any RX chars */
	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
	return;

	/* now process RX char */
	switch(rx_byte)
	{
		case 'x':
		case 'X':
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
		break;
		case '\r':
		case '\n':
			ASC_Asci_msg("\n\n\r READY for Theshold Stuff\n\n\r");
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
		break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(CHECK_THRESHOLD1_MSG,Check_threshold1_menu);
		break;
	}
	
		
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static int8 Check_chan_voltages(ADC_CHAN_ID chan,VOLTAGE_LIMITS *vlim)
{
	CHAN_AVERAGE *ca;

	ca = ADC_Get_chan_average(chan,8);
//	sprintf(tmpstr,"%s: AVG=%umV MAX=%umV MIN=%umV - ",ADC_Get_chan_name(chan),ca->avg,ca->max,ca->min);
	sprintf(tmpstr,"%s = %umV - ",ADC_Get_chan_name(chan),ca->avg);
	ASC_Asci_msg(tmpstr);
	if(ca->avg > (vlim->nominal + vlim->limit))
	{
		ASC_Asci_msg((int8 *const)ROM_Read_romstr(FAIL_MSG));
		sprintf(tmpstr,"\n\n\r*** %s Voltage too high - %umV +/- %umV ***\n\r",ADC_Get_chan_name(chan), vlim->nominal,vlim->limit);
		ASC_Asci_msg(tmpstr);
	}
	else if(ca->avg < (vlim->nominal - vlim->limit))
	{
		ASC_Asci_msg((int8 *const)ROM_Read_romstr(FAIL_MSG));
		sprintf(tmpstr,"\n\n\r*** %s Voltage too low - %umV +/- %umV ***\n\r",ADC_Get_chan_name(chan), vlim->nominal,vlim->limit);
		ASC_Asci_msg(tmpstr);
	}
	else
	{
		ASC_Asci_msg((int8 *const)ROM_Read_romstr(PASS_MSG));
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
			MEN_Set_cmd_bk_func(DEBUG_MENU_MSG,Debug_menu);
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
	int8 rx_byte;

	Adc_display_values();
	
	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
		return;

	/* now process RX char */
	switch(rx_byte)
	{
		case 'x':
		case 'X':
//			ADC_Shutdown();
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
static void Control_debug_menu(void)
{
	int8 rx_byte;
	ONOFF_ENUM stat;

	/* get any RX chars */
	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
	{
		Adc_display_values();
		return;
	}
//	ASC_Asci_msg((int8 *const)ROM_Read_romstr(ADC_CHAN_LABELS_MSG));

	/* now process RX char */
	switch(rx_byte)
	{
		case 'E':
		case 'e':
			if(MAI_Get_control_status(EXTBAT_CNTRL) == ON)
				stat = OFF;
			else
				stat = ON;
			MAI_Set_control_status(EXTBAT_CNTRL,stat);
			break;
		case 'I':
		case 'i':
			if(MAI_Get_control_status(INTBAT_CNTRL) == ON)
				stat = OFF;
			else
				stat = ON;
			MAI_Set_control_status(INTBAT_CNTRL,stat);
			break;
		case 'P':
		case 'p':
			if(MAI_Get_control_status(POLYFUSE_CNTRL) == ON)
				stat = OFF;
			else
				stat = ON;
			MAI_Set_control_status(POLYFUSE_CNTRL,stat);
			break;
		case 'X':
		case 'x':
			MEN_Set_cmd_bk_func(DEBUG_MENU_MSG,Debug_menu);
			return;
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
		break;
	}
	MEN_Set_cmd_bk_func(CONTROL_DEBUG_MSG,Display_control_status);

}
static void Adc_display_values(void)
{
	int8 x,*chan_name;
	int16 adcvolts;

	for(x = 0; x < ADC_CHAN_COUNT;x++)
	{
		if(ADC_Get_average_millivolts(&adcvolts,x))
		{
			//			chan_name = ADC_Get_chan_name(x);
			//			sprintf((char *)tmpstr,"%d=%05umV ",x, adcvolts);
			sprintf((char *)tmpstr," %05u", adcvolts);
			ASC_Asci_msg(tmpstr);
			while(!ASC_Asci_tx_empty());
		}
	}
	sprintf((char *)tmpstr,"\r");
	ASC_Asci_msg(tmpstr);
}
/*********************************************************************
*                       End of menu.c                                *
*********************************************************************/
