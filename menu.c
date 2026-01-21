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
static void Switch_5v_on_menu(void);
static void Check_intbat(void);
static void Check_5v(void);
static void Check_lower_threshold_menu(void);
static void Check_upper_threshold_menu(void);
static void Display_tracking_voltages(int16 *extbat_ptr, int16 *intbat_ptr, int16 *vbat_ptr);

static void Prog_slave_menu(void);
static void Slave_display_menu(void);
static void Slave_display_menu2(void);
static void Prog_master_menu(void);
static void Prog_master_menu2(void);
static void Base_iface_menu(void);
static void Base_iface_menu2(void);
static void Polyfuse_test_menu(void);
static void Polyfuse_test(void);
static void Test_done(void);



static void Debug_menu(void);
static void I2C_Debug_menu(void);
static void Adc_debug_menu(void);
static void Control_debug_menu(void);
static void Display_control_status(void);
static void Adc_display_values(void);
static void Set_verif_thresh_menu(void);
static void Display_current_thresholds(void);


//static void I2C_test(void);

static void Test_msg_func(void);
static int8 Cmd_check(int8);
static void Retry_or_exit_menu(void);
static void Set_retry_func( void * const next_cmd_ptr);

/*==================================================================*/
/*                      LOCAL MACRO DEFINITIONS                     */
/*==================================================================*/
// *** Test Voltage Limits and Tolerances **
#define EXTBAT_VOLTAGE 14000		// 14.0V nominal
#define EXTBAT_LIIMIT 100			// ± 200mV tolerance
#define INTBAT_VOLTAGE 15000		// 15.0V nominal
#define INTBAT_LIIMIT 100			// ± 200mV tolerance
#define _3V3_VOLTAGE 3300			// 3.3V nominal
#define _3V3_LIIMIT 150				// ± 150mV tolerance
#define _5V_VOLTAGE 5000			// 5.0V nominal
#define _5V_LIIMIT 150				// ± 200mV tolerance
#define VBAT_TRACKING_LIMIT 150		// ± 200mV tolerance

#define LOWER_SWITCH_THRESH		11200	// 11.2V Nominal
#define UPPER_SWITCH_THRESH		13000	// 23.0V Nominal
#define LOWER_THRESH_VERIF_HIGH	12000	// 11.6V for verification
#define LOWER_THRESH_VERIF_LOW	10400	// 110.8V for verification
#define UPPER_THRESH_VERIF_HIGH 13800	// 13.4V for verification
#define UPPER_THRESH_VERIF_LOW	12200	// 12.6V for verification

#define SWITCH_THRESH_TOL 400		// ± 400mV tolerance
#define POLYFUSE_TRIP_THRESHOLD		1500// 1.5V trip threshold
#define POLYFUSE_TRIP_THRESH_VERIF	100// 0.1V trip threshold for verification
#define POLYFUSE_TRIP_TIME 3		// define trip time is seconds

// I²C Definitions
#define STM_I2C_ADDR				0x10 // Master interface address
#define DISPLAY_AMP_ERROR_CODE		0x11 // Code to display error code on Master display
#define AMP_ERROR_CODE_MSB			0xff // MSB of error code (always 0xff)
#define AMP_ERROR_CODE_BLUE_LSB		0xff // LSB of error code to set BLUE LED ON (0xffff)
#define AMP_ERROR_CODE_GREEN_LSB	0xfe // LSB of error code to set GREEN LED ON (0xfffe)
#define AMP_ERROR_CODE_RED_LSB		0xfd // LSB of error code to set RED LED ON (0xfffd)
#define DISPLAY_AMP_NO_ERROR_CODE	0x00

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

#define BELL '\x7'
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
	"Ensure the EXTBAT PSU is set to 14.0V +/- 0.1V\n\r"
	"Ensure the INTBAT PSU is set to 15.0V +/- 0.1V\n\r"
	"Connect the Board Under Test (BUT) to the Jig\n\r"
	"\n*** Ensure the +5V Power Switch is OFF at this stage ***\n\n\r"
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

int8 const SWICTH_5V_ON_MSG[] PROGMEM =
{
	"\n\r*** TURN THE +5V POWER SWITCH ON ***\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};


/*int8 const POLYFUSE_VOLTAGE_ERR_MSG[] PROGMEM =
{
	"\n\n\r*** +5V Out of tolerance ***\n\n\r"
};*/

int8 const CHECK_THRESHOLD1_MSG[] PROGMEM =
{
	"\n\rSlowly Decrease the EXTBAT voltage until a Message appears\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
	"  EXTBAT  VBAT    INTBAT\n\r"
};
int8 const CHECK_THRESHOLD2_MSG[] PROGMEM =
{
	"\n\rSlowly Increase the EXTBAT voltage until a Message appears\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
	"  EXTBAT  VBAT    INTBAT\n\r"
};

int8 const RETRY_OR_EXIT_MSG[] PROGMEM =
{
	"\n\rPress 'X' to exit or ENTER to Retry\n\r"
};

int8 const VOLTAGE_TITLE_MSG[] PROGMEM =
{
	"\n\r  EXTBAT  VBAT    INTBAT\n\r"
};

int8 const EXTBAT_TRACKING_ERROR_MSG[] PROGMEM =
{
	"\n\n\r*** EXTBAT TRACKING ERROR ***\n\n\r"
};
int8 const INTBAT_TRACKING_ERROR_MSG[] PROGMEM =
{
	"\n\n\r*** INTBAT TRACKING ERROR ***\n\n\r"
};
int8 const LOWER_THRESHOLD_ERROR_MSG[] PROGMEM =
{
	"\n\n\r*** LOWER SWITCHING THRESHOLD ERROR ***\n\n\r"
};
int8 const UPPER_THRESHOLD_ERROR_MSG[] PROGMEM =
{
	"\n\n\r*** UPPER SWITCHING THRESHOLD ERROR ***\n\n\r"
};

int8 const PROG_SLAVE_MENU_MSG[] PROGMEM =
{
	"\n\n\rLeave the '5V' power switch ON and follow the instructions in the Test Method\n\r"
	"to program the BUT into SLAVE MODE\n\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};
	/*
	"Open the STM Programming application and ensure the following ST-LINK configuration is set:\n\r"
	" - Port: SWD\n\r"
	" - Reset Mode: Hardware Reset\n\r"
	" - Ensure the 'Verify Programming' check box is selected\n\n\r"
	"Ensure the Jumper is not fitted to the 'Master/Slave' header, P3\n\r"
	"Switch the 5V power switch ON\n\r"
	"Select the 'Connect' option and verify the programmer successfully connects to the BUT\n\r"
	"** NOTE: On Older firmware versions, use the 'SLAVE' version of the firmware **\n\r"
	"Open the appropriate '.ELF' file detailed in the Test Method, and program the Device\n\n\r"
	"Verify the BUT programs and verifies successfully\n\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};
*/

int8 const SLAVE_DISPLAY_MENU_MSG[] PROGMEM =
{
	"\n\n\rCycle the power to the BUT using the '5V' Power switch\n\r"
	"VERIFY on power up, the slave displays briefly shows the correct firmware version\n\n\r"
	"VERIFY after a short delay the following is shown on the slave display:\n\r"
	" - The battery charge status in the form 'XX%'\n\r"
	" - The message 'EXT PWR' is displayed\n\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};
int8 const SLAVE_DISPLAY_MENU2_MSG[] PROGMEM =
{
	"VERIFY the 'EXT PWR' message has been replaced with the following:\n\n\r"
	"                 '-:--' \n\n\r"
	"VERIFY an 'IDLE' message appears above the battery charge status\n\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};


int8 const PROG_MASTER_MENU_MSG[] PROGMEM =
{
	"Follow the instructions in the Test Method to program the BUT into MASTER MODE\n\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};
int8 const PROG_MASTER_MENU2_MSG[] PROGMEM =
{
	"\n\n\rCycle the power to the BUT using the +5V power switch\n\n\r"
	"VERIFY the slave display is now blank and the Master display shows an\n\r"
	"'ON/OFF' symbol, and the TRI-Colour LED on the Jig is OFF\n\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};
int8 const MASTER_SWITCH_TOGGLE_MENU_MSG[] PROGMEM =
{
	"\n\n\rVERIFY that, when the Master display is touched the following occurs:\n\r"
	" - an 'EXT PWR' message appears on the Master display\n\r"
	" - the TRI-Colour LED on the Jig is illuminated and is GREEN in colour\n\n\r"
	"VERIFY each time the Master display is touched the on/off status is toggled\n\n\r"
	"Set the display to the ON state\n\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};
int8 const BASE_IFACE_MENU_MSG[] PROGMEM =
{
	"\n\n\rVERIFY the Master display now shows the indicated Error Code and the\n\r"
	"TRI-Colour LED is now illuminated as idicated:\n\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};


int8 const MASTER_BATTERY_MSG[] PROGMEM =
{
	"\n\n\rVERIFY the Master display now shows the message 'BATTERY'\n\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};
int8 const POLYFUSE_TEST_MSG[] PROGMEM =
{
	"\n\n\rVERIFY that, after pressing the 'ENTER' key, the following occurs:\n\r"
	" - The +5V supply for the LED drive is reduced to less than 1.5V after 3 seconds\n\r"
	" - The TRI-Colour LED is briefly extinguished\n\r"
	" - The Master display is not affected and still displays 'BATTERY'\n\n\r"
	"Press 'X' to exit or ENTER to proceed\n\n\r"
};

int8 const TESTING_DONE_MSG[] PROGMEM =
{
	"\n\n\r\x7****************************\n\r"
	"* Testing is now completed *\n\r"
	"****************************\n\n\r"
	" - Turn OFF the 5V Power switch\n\r"
	" - Remove the Board Under Test & apply TESTED mark\n\r"
	" - Connect the next board to be tested to the Jig and press 'ENTER'\n\n\r"
	" - Or press Press 'X' to exit to start menu\n\n\r"
};

int8 const DEBUG_MENU_MSG[] PROGMEM =
{
	"\n\n\n\n\r"
	"DEBUG MENU\n\r"
	"=============\n\r"
	"A - ADC test\n\r"
	"C - I/O Control test\n\r"
	"I - I2C test\n\r"
	"T - Toggle Thresholds for Verification\n\r"
	"X - Exit to Start Menu\n\r"
};

int8 const I2C_MENU_MSG[] PROGMEM =
{
	"\n\n\n\n\rI2C Test"
	"\n\r========\n\n\r"
	"NOTE: Ensure the Module is Programmed as a Master\n\n\r"
	"B - LED is BLUE  - Error Code 0xffff\n\r"
	"G - LED is GREEN - Error Code 0xfffe\n\r"
	"R - LED is RED   - Error Code 0xfffd\n\n\r"

	"Press 'X' to return to Debug Menu\n\n\r"
};

int8 const ADC_DEBUG_MSG[] PROGMEM =
{
	"\n\n\n\n\rADC Debug"
	"\n\r=========\n\n\r"
	"\n\rPress X to exit\n\n\r"
	"\n\r     All Data in millivolts\n\r"
	"\n\r  EXT   INT   +5V   VBAT  3V3\n\r"
};
int8 const ADC_CHAN_LABELS_MSG[] PROGMEM =
{
	"\n\r  EXT   INT   +5V   VBAT  3V3\n\r"
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
	"\n\r  EXT   INT   +5V   VBAT  3V3\n\r"
};
int8 const SET_VERIF_THESHOLDS_MSG[] PROGMEM =
{
	"\n\n\n\rVerification Threshold modification Menu\n\r"
	"========================================\n\r"
	"Select option below to toggle value between actual and verification values\n\n\r"
	"1 - Increase Lower Threshold\n\r"
	"2 - Decrease Lower Threshold\n\r"
	"3 - Increase Upper Threshold\n\r"
	"4 - Decrease Upper Threshold\n\r"
	"5 - Lower Polyfuse Trip Threshold\n\r"
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
	
static int16 lower_switch_threshold = LOWER_SWITCH_THRESH;	// Set default lower switching threshold
static int16 upper_switch_threshold = UPPER_SWITCH_THRESH;	// Set default upper switching threshold
static int16 polyfuse_threshold = POLYFUSE_TRIP_THRESHOLD;

static int8 cur_err_code;


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

/*====================================================================
Name        :MEN_Set_cmd_bk_func
Parameters  :msg to display, next cmd func to execute
Returns     :NONE
Description :Loads the passed vars to local vars and sets the cmd func to display
             the passed message and set the passed cmd func
--------------------------------------------------------------------*/
void Set_retry_func( void * const next_cmd_ptr)
{
	retry_cmd_func = next_cmd_ptr;
	MEN_Set_cmd_bk_func(RETRY_OR_EXIT_MSG,Retry_or_exit_menu);
}
/*====================================================================
Name        :MEN_Set_cmd_bk_func
Parameters  :msg to display, next cmd func to execute
Returns     :NONE
Description :Loads the passed vars to local vars and sets the cmd func to display
             the passed message and set the passed cmd func
--------------------------------------------------------------------*/
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

	if(MAI_Get_control_status(EXTBAT_CNTRL) == ON)
		MAI_Set_control_status(EXTBAT_CNTRL,OFF);
	if(MAI_Get_control_status(INTBAT_CNTRL) == ON)
		MAI_Set_control_status(INTBAT_CNTRL,OFF);
	if(MAI_Get_control_status(POLYFUSE_CNTRL) == ON)
		MAI_Set_control_status(POLYFUSE_CNTRL,OFF);
		
	

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
		case 't':
		case 'T':
			MEN_Set_cmd_bk_func(SET_VERIF_THESHOLDS_MSG,Set_verif_thresh_menu);
			break;
		case 'I':
		case 'i':
			I2C_Init();
			MEN_Set_cmd_bk_func(I2C_MENU_MSG,I2C_Debug_menu);
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
	VOLTAGE_LIMITS vbat;
	
	vbat.nominal = extbat_lims.nominal;
	vbat.limit = 200;
	
	MAI_Set_control_status(INTBAT_CNTRL,OFF); // Turn oFF INTBAT supply to BUT
	TIM_Wait(250);	// set 0.5s delay
	MAI_Set_control_status(EXTBAT_CNTRL,ON); // Turn on EXTBAT supply to BUT
	TIM_Wait(250);	// set 0.5s delay
	if(!Check_chan_voltages(ADC_EXTBAT,&extbat_lims))
	{
		ASC_Asci_msg((int8 *const)ROM_Read_romstr(ADJUST_VOLTAGE_MSG));
		Set_retry_func(Check_extbat);
		
	}
	else
	{
		if(Check_chan_voltages(ADC_3V3,&_3v3_lims))
		{
			if(Check_chan_voltages(ADC_VBAT,&vbat))
				MEN_Set_cmd_bk_func(NULL,Check_intbat);
			else
				Set_retry_func(Check_extbat);
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
	VOLTAGE_LIMITS vbat;
	
	vbat.nominal = intbat_lims.nominal;
	vbat.limit = 200;
	
	MAI_Set_control_status(EXTBAT_CNTRL,OFF); // Turn off EXTBAT supply to BUT
	TIM_Wait(250);	// set 0.5s delay
	MAI_Set_control_status(INTBAT_CNTRL,ON); // Turn on INTBAT supply to BUT
	TIM_Wait(250);	// set 0.5s delay
	if(!Check_chan_voltages(ADC_INTBAT,&intbat_lims))
	{
		ASC_Asci_msg((int8 *const)ROM_Read_romstr(ADJUST_VOLTAGE_MSG));
		Set_retry_func(Check_intbat);
		
	}
	else
	{
		if(Check_chan_voltages(ADC_3V3,&_3v3_lims))
		{
			if(Check_chan_voltages(ADC_VBAT,&vbat))
				MEN_Set_cmd_bk_func(SWICTH_5V_ON_MSG,Switch_5v_on_menu);
			else
				Set_retry_func(Check_intbat);
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
static void Switch_5v_on_menu(void)
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
			MEN_Set_cmd_bk_func(SWICTH_5V_ON_MSG,Switch_5v_on_menu);
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
	if(!Check_chan_voltages(ADC_POLYFUSE,&_5v_lims))
	{
		Set_retry_func(Check_5v);
	}
	else
	{
		if(Check_chan_voltages(ADC_3V3,&_3v3_lims))
		{
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			MEN_Set_cmd_bk_func(CHECK_THRESHOLD1_MSG,Check_lower_threshold_menu);
			MAI_Set_control_status(EXTBAT_CNTRL,ON); // Turn on EXTBAT supply to BUT
			TIM_Wait(250);
			MAI_Set_control_status(INTBAT_CNTRL,ON); // Turn on INTBAT supply to BUT
			TIM_Wait(250);
			return;
			
		}
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
static void Check_lower_threshold_menu(void)
{
	int8 rx_byte;
	int16 extbat,intbat,vbat;

	//	ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWLINE_MSG));

	/* get any RX chars */
	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
	{
		Display_tracking_voltages(&extbat,&intbat,&vbat);
		
		// check if we are tracking extbat
		if((vbat >= (extbat - VBAT_TRACKING_LIMIT)) && (vbat <= (extbat + VBAT_TRACKING_LIMIT)))
		{
			// we are still tracking extbat - Check if we should have switched
			if((vbat < (lower_switch_threshold - SWITCH_THRESH_TOL)))
			{
				//Display error
				ASC_Asci_msg((int8 *const)ROM_Read_romstr(LOWER_THRESHOLD_ERROR_MSG));
				sprintf((char *)tmpstr,"\n\rThreshold = %dmV +/- %dmV\n\rEXTBAT = %dmV\n\r",lower_switch_threshold,SWITCH_THRESH_TOL,extbat );
				ASC_Asci_msg(tmpstr);
				Set_retry_func(Check_lower_threshold_menu);
			}
			return;
		}
		// we are no longer tracking extbat so if threshold is ok
		else if((extbat > (lower_switch_threshold - SWITCH_THRESH_TOL)) && (extbat < (lower_switch_threshold + SWITCH_THRESH_TOL)))
		{
			// in tolerance so display result
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			sprintf((char *)tmpstr, "\n\n\r\x7Lower Threshold OK @ %umV\n\r",extbat);
			ASC_Asci_msg(tmpstr);
			MEN_Set_cmd_bk_func(CHECK_THRESHOLD2_MSG,Check_upper_threshold_menu);
			TIM_Set_delay(500);
			while(!TIM_Get_delay_flag());
			return;
		}
		else
		{
			//Display error
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(LOWER_THRESHOLD_ERROR_MSG));
			sprintf((char *)tmpstr,"\n\rThreshold = %dmV +/- %dmV\n\rEXTBAT = %dmV\n\r",lower_switch_threshold,SWITCH_THRESH_TOL,extbat );
			ASC_Asci_msg(tmpstr);
			//				ASC_Asci_msg((int8 *const)ROM_Read_romstr(VOLTAGE_TITLE_MSG));
			//				Display_tracking_voltages(&extbat,&intbat,&vbat);
			Set_retry_func(Check_lower_threshold_menu);
			return;
		}

	}
	/* now process RX char */
	switch(rx_byte)
	{
		case 'x':
		case 'X':
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(CHECK_THRESHOLD1_MSG,Check_lower_threshold_menu);
		break;
	}
	
		
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void Check_upper_threshold_menu(void)
{
	int8 rx_byte;
	int16 extbat,intbat,vbat;

	//	ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWLINE_MSG));

	/* get any RX chars */
	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
	{
		Display_tracking_voltages(&extbat,&intbat,&vbat);
		
		// check if we are tracking intbat
		if(vbat >= (intbat - VBAT_TRACKING_LIMIT) && vbat <= (intbat + VBAT_TRACKING_LIMIT))
		{
			// we are still tracking - Check if we should have switched
			if(extbat > (upper_switch_threshold + SWITCH_THRESH_TOL))
			{
				//Display error
				ASC_Asci_msg((int8 *const)ROM_Read_romstr(UPPER_THRESHOLD_ERROR_MSG));
				sprintf((char *)tmpstr,"\n\rThreshold = %dmV +/- %dmV\n\rEXTBAT = %dmV\n\r",upper_switch_threshold,SWITCH_THRESH_TOL,extbat );
				ASC_Asci_msg(tmpstr);
				Set_retry_func(Check_upper_threshold_menu);
			}
			return;
		}
		// we are no longer tracking intbat so check if thresh is ok
		else if(extbat > (upper_switch_threshold - SWITCH_THRESH_TOL) && extbat < (upper_switch_threshold + SWITCH_THRESH_TOL))
		{
			// in tolerance so display result
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			sprintf((char *)tmpstr, "\n\n\r\x7Upper Threshold OK @ %umV\n\r",extbat);
			ASC_Asci_msg(tmpstr);
			// Turn off extbat and intbat supplies
			MAI_Set_control_status(EXTBAT_CNTRL,OFF);
			MAI_Set_control_status(INTBAT_CNTRL,OFF);
			MEN_Set_cmd_bk_func(PROG_SLAVE_MENU_MSG,Prog_slave_menu);
			return;
		}
		else
		{
			//Display error
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(UPPER_THRESHOLD_ERROR_MSG));
			sprintf((char *)tmpstr,"\n\rThreshold = %dmV +/- %dmV\n\rEXTBAT = %dmV\n\r",upper_switch_threshold,SWITCH_THRESH_TOL,extbat );
			ASC_Asci_msg(tmpstr);
			Set_retry_func(Check_upper_threshold_menu);
			return;
		}
	}

	/* now process RX char */
	switch(rx_byte)
	{
		case 'x':
		case 'X':
			MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(CHECK_THRESHOLD1_MSG,Check_lower_threshold_menu);
			break;
	}
		
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	: Retrieves and displays averaged EXTBAT, INTABAT and VBAT voltages
--------------------------------------------------------------------*/
static void Display_tracking_voltages(int16 *extbat_ptr, int16 *intbat_ptr, int16 *vbat_ptr)
{
	while(!ADC_Get_average_millivolts(extbat_ptr,ADC_EXTBAT));
	while(!ADC_Get_average_millivolts(intbat_ptr,ADC_INTBAT));
	while(!ADC_Get_average_millivolts(vbat_ptr,ADC_VBAT));
	sprintf((char *)tmpstr,"  %05u   %05u   %05u\r",*extbat_ptr,*vbat_ptr,*intbat_ptr);
	ASC_Asci_msg(tmpstr);
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
	sprintf((char *)tmpstr,"%s = %umV - ",ADC_Get_chan_name(chan),ca->avg);
	ASC_Asci_msg(tmpstr);
	if(ca->avg > (vlim->nominal + vlim->limit))
	{
		ASC_Asci_msg((int8 *const)ROM_Read_romstr(FAIL_MSG));
		sprintf((char *)tmpstr,"\n\n\r*** %s Voltage too high - %umV +/- %umV ***\n\r",ADC_Get_chan_name(chan), vlim->nominal,vlim->limit);
		ASC_Asci_msg(tmpstr);
	}
	else if(ca->avg < (vlim->nominal - vlim->limit))
	{
		ASC_Asci_msg((int8 *const)ROM_Read_romstr(FAIL_MSG));
		sprintf((char *)tmpstr,"\n\n\r*** %s Voltage too low - %umV +/- %umV ***\n\r",ADC_Get_chan_name(chan), vlim->nominal,vlim->limit);
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
static void Prog_slave_menu(void)
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
			MAI_Set_control_status(EXTBAT_CNTRL,OFF); // Turn on EXTBAT supply
			MAI_Set_control_status(INTBAT_CNTRL,OFF); // Turn off INTBAT supply
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			MEN_Set_cmd_bk_func(SLAVE_DISPLAY_MENU_MSG,Slave_display_menu);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(PROG_SLAVE_MENU_MSG,Prog_slave_menu);
		break;
	}
	
		
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void Slave_display_menu(void)
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
			MAI_Set_control_status(EXTBAT_CNTRL,OFF); // Turn off EXTBAT supply
			MAI_Set_control_status(INTBAT_CNTRL,ON); // Turn on EXTBAT supply
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			MEN_Set_cmd_bk_func(SLAVE_DISPLAY_MENU2_MSG,Slave_display_menu2);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(SLAVE_DISPLAY_MENU_MSG,Slave_display_menu);
			break;
	}
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void Slave_display_menu2(void)
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
			MAI_Set_control_status(EXTBAT_CNTRL,OFF); // Turn on EXTBAT supply
			MAI_Set_control_status(INTBAT_CNTRL,OFF); // Turn on EXTBAT supply
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			MEN_Set_cmd_bk_func(PROG_MASTER_MENU_MSG,Prog_master_menu);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(SLAVE_DISPLAY_MENU2_MSG,Slave_display_menu2);
			break;
	}
}
static void Prog_master_menu(void)
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
			MAI_Set_control_status(EXTBAT_CNTRL,OFF); // Turn off EXTBAT supply
			MAI_Set_control_status(INTBAT_CNTRL,OFF); // Turn off EXTBAT supply
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			MEN_Set_cmd_bk_func(PROG_MASTER_MENU2_MSG,Prog_master_menu2);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(PROG_MASTER_MENU_MSG,Prog_master_menu);
			break;
	}
	
}
static void Prog_master_menu2(void)
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
			MAI_Set_control_status(EXTBAT_CNTRL,ON); // Turn on EXTBAT supply
			MAI_Set_control_status(INTBAT_CNTRL,OFF); // Turn on EXTBAT supply
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			MEN_Set_cmd_bk_func(MASTER_SWITCH_TOGGLE_MENU_MSG,Base_iface_menu);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(PROG_MASTER_MENU_MSG,Prog_master_menu2);
			break;
	}
	
}
static void Base_iface_menu(void)
{
	int8 rx_byte,buf[5];

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
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			// Now set up I²C iface to display AMP CODE 0xfffd - RED LED
			I2C_Init();
			buf[0] = DISPLAY_AMP_ERROR_CODE;// set code to display AMP CODE on LCD
			buf[1] = AMP_ERROR_CODE_MSB;	// Set data to display as AMP CODE on LCD
			buf[1] = AMP_ERROR_CODE_RED_LSB;// Set data to display as AMP CODE on LCD
			cur_err_code = AMP_ERROR_CODE_RED_LSB;
			I2C_Write(STM_I2C_ADDR,3,buf);
			TIM_Set_delay(2000);
			MEN_Set_cmd_bk_func(BASE_IFACE_MENU_MSG,Base_iface_menu2);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(MASTER_SWITCH_TOGGLE_MENU_MSG,Base_iface_menu);
			break;
	}
		
}

static void Base_iface_menu2(void)
{
	int8 rx_byte,buf[5],*msg_ptr;

	/* get any RX chars */
	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(rx_byte)
	{
		/* now process RX char */
		switch(rx_byte)
		{
			case 'x':
			case 'X':
				MEN_Set_cmd_bk_func(START_MENU_MSG,Start_menu);
				break;
			case '\r':
			case '\n':
				ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
				// set up to run from internal battery only
				MAI_Set_control_status(EXTBAT_CNTRL,OFF); // Turn off EXTBAT supply
				MAI_Set_control_status(INTBAT_CNTRL,ON); // Turn on EXTBAT supply
				// Now set up I²C iface to display 'BATTERY' instead of amp code
				buf[0] = DISPLAY_AMP_ERROR_CODE;	// set code to display AMP CODE on LCD
				buf[1] = 0x00; //AMP_ERROR_CODE_MSB;	// Set data to display as AMP CODE on LCD
				buf[2] = DISPLAY_AMP_NO_ERROR_CODE;	// Set LSB data to display as AMP CODE on LCD
				I2C_Write(STM_I2C_ADDR,3,buf);
				I2C_Shutdown();
				MEN_Set_cmd_bk_func(MASTER_BATTERY_MSG,Polyfuse_test_menu);
				break;
			default:
				ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
				break;
		}
	}
	else
	{
		if(!TIM_Get_delay_flag())
			return;
		buf[0] = DISPLAY_AMP_ERROR_CODE;// set code to display AMP CODE on LCD
		buf[1] = AMP_ERROR_CODE_MSB;	// Set data to display as AMP CODE on LCD
		if(++cur_err_code == 0)
		{
			cur_err_code = AMP_ERROR_CODE_RED_LSB;
			msg_ptr = (int8 *)"RED";
		}
		else if(cur_err_code == AMP_ERROR_CODE_BLUE_LSB)
			msg_ptr = (int8 *)"BLUE";
		else
			msg_ptr = (int8 *)"GREEN";

		buf[2] = cur_err_code;// Set data to display as AMP CODE on LCD
		I2C_Write(STM_I2C_ADDR,3,buf);
		sprintf((char *)tmpstr,"Displaying Error Code 0x%02x%02x LED is %s     \r",(int16)AMP_ERROR_CODE_MSB,(int16)cur_err_code,msg_ptr);
		ASC_Asci_msg(tmpstr);
		
		TIM_Set_delay(2000);
	}
		
}
static void Polyfuse_test_menu(void)
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
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			MEN_Set_cmd_bk_func(POLYFUSE_TEST_MSG,Polyfuse_test);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(MASTER_BATTERY_MSG,Polyfuse_test_menu);
			break;
	}
	
}
static void Polyfuse_test(void)
{
	int8 rx_byte,x;
	int16 polyfuse_millivolts;

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
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(NEWPAGE_MSG));
			MAI_Set_control_status(POLYFUSE_CNTRL,ON); // Turn on POLYFUSE Load
			ASC_Asci_msg((int8 *)"\n\n\rPolyfuse  Time\n\r");
			for(x = 0; x < POLYFUSE_TRIP_TIME; x++)
			{
				TIM_Set_delay(1000);
				while(!TIM_Get_delay_flag())
				{
					while(!ADC_Get_average_millivolts(&polyfuse_millivolts,ADC_POLYFUSE));
					sprintf((char *)tmpstr," %04dmV    %d\r",polyfuse_millivolts,x);
					ASC_Asci_msg(tmpstr);
				}		
			}
			if(polyfuse_millivolts < polyfuse_threshold)
				sprintf((char *)tmpstr," \n\n\rPOLYFUSE Test PASS: Voltage after %d seconds = %05dmV",POLYFUSE_TRIP_TIME,polyfuse_millivolts);
			else
			{
				sprintf((char *)tmpstr," \n\n\rPOLYFUSE Test FAIL: Voltage after %d seconds = %05dmV (Threshold = %05dmV)",POLYFUSE_TRIP_TIME,polyfuse_millivolts, polyfuse_threshold);
				ASC_Asci_msg(tmpstr);
				MEN_Set_cmd_bk_func(RETRY_OR_EXIT_MSG,Polyfuse_test);
			
				MAI_Set_control_status(POLYFUSE_CNTRL,OFF); // Turn off POLYFUSE Load
				return;
			}
			ASC_Asci_msg(tmpstr);
			
			MAI_Set_control_status(POLYFUSE_CNTRL,OFF); // Turn off POLYFUSE Load
			
			MAI_Set_control_status(EXTBAT_CNTRL,OFF);
			MAI_Set_control_status(INTBAT_CNTRL,OFF);
			
			MEN_Set_cmd_bk_func(TESTING_DONE_MSG,Test_done);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(POLYFUSE_TEST_MSG,Polyfuse_test);
			break;
	}
	
}

static void Test_done(void)
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
			MEN_Set_cmd_bk_func(TEST_MENU_MSG,Test_menu);
			break;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(TESTING_DONE_MSG,Test_done);
		break;
	}
	
}
/*====================================================================
Name		:
Parameters	:
Returns		:
Description	:
--------------------------------------------------------------------*/
static void I2C_Debug_menu(void)
{
	int8 rx_byte,errcode_lsb = 0xff,*msg_ptr = NULL;
	int8 buf[5];


	/* get any RX chars */
	rx_byte = Cmd_check(NO_CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
	return;

	switch(rx_byte)
	{
		case 'B':
		case 'b':
			errcode_lsb = AMP_ERROR_CODE_BLUE_LSB;
			msg_ptr = (int8 *)"BLUE";
			break;
		case 'G':
		case 'g':
			errcode_lsb = AMP_ERROR_CODE_GREEN_LSB;
			msg_ptr = (int8 *)"GREEN";
			break;
		case 'R':
		case 'r':
			errcode_lsb = AMP_ERROR_CODE_RED_LSB;
			msg_ptr = (int8 *)"RED";
			break;
		case 'X':
		case 'x':
			buf[0] = DISPLAY_AMP_ERROR_CODE;	// set code to display AMP CODE on LCD
			buf[1] = 0x00; //AMP_ERROR_CODE_MSB;	// Set data to display as AMP CODE on LCD
			buf[2] = DISPLAY_AMP_NO_ERROR_CODE;	// Set LSB data to display as AMP CODE on LCD
			I2C_Write(STM_I2C_ADDR,3,buf);
			I2C_Shutdown();
			MEN_Set_cmd_bk_func(DEBUG_MENU_MSG,Debug_menu);
			return;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			MEN_Set_cmd_bk_func(I2C_MENU_MSG,I2C_Debug_menu);
			return;	
	}
	// Display Current AMP CODE on terminal
	sprintf((char *)tmpstr,"Displaying Error Code 0x%02x%02x LED is %s     \r",(int16)AMP_ERROR_CODE_MSB,(int16)errcode_lsb,msg_ptr);
	ASC_Asci_msg(tmpstr);
				
	//Set i2c data buffer
	buf[0] = DISPLAY_AMP_ERROR_CODE;	// set code to display AMP CODE on LCD
	buf[1] = AMP_ERROR_CODE_MSB;	// Set data to display as AMP CODE on LCD
	buf[2] = errcode_lsb;	// Set LSB data to display as AMP CODE on LCD
				
	// Send 2 bytes of data stored in buf, to STM I2C interface
	I2C_Write(STM_I2C_ADDR,3,buf);
				
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
	int8 x;
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
static void Set_verif_thresh_menu(void)
{
	int8 rx_byte;

	/* get any RX chars */
	rx_byte = Cmd_check(CMD_ECHO);
	/* return if none available */
	if(!rx_byte)
		return;
	//	ASC_Asci_msg((int8 *const)ROM_Read_romstr(ADC_CHAN_LABELS_MSG));

	/* now process RX char */
	switch(rx_byte)
	{
		case '1':
			if(lower_switch_threshold == LOWER_SWITCH_THRESH)
				lower_switch_threshold = LOWER_THRESH_VERIF_HIGH;
			else
				lower_switch_threshold = LOWER_SWITCH_THRESH;
			break;
		case '2':
			if(lower_switch_threshold == LOWER_SWITCH_THRESH)
				lower_switch_threshold = LOWER_THRESH_VERIF_LOW;
			else
				lower_switch_threshold = LOWER_SWITCH_THRESH;
			break;
		case '3':
			if(upper_switch_threshold == UPPER_SWITCH_THRESH)
				upper_switch_threshold = UPPER_THRESH_VERIF_HIGH;
			else
				upper_switch_threshold = UPPER_SWITCH_THRESH;
			break;
		case '4':
			if(upper_switch_threshold == UPPER_SWITCH_THRESH)
				upper_switch_threshold = UPPER_THRESH_VERIF_LOW;
			else
				upper_switch_threshold = UPPER_SWITCH_THRESH;
			break;
		case '5':
			if(polyfuse_threshold == POLYFUSE_TRIP_THRESHOLD)
				polyfuse_threshold = POLYFUSE_TRIP_THRESH_VERIF;
			else
				polyfuse_threshold = POLYFUSE_TRIP_THRESHOLD;
			break;
		case 'X':
		case 'x':
			MEN_Set_cmd_bk_func(DEBUG_MENU_MSG,Debug_menu);
			return;
		default:
			ASC_Asci_msg((int8 *const)ROM_Read_romstr(CMD_NOT_IMPLEMENTED_MSG));
			break;
	
	}
	Display_current_thresholds();
	MEN_Set_cmd_bk_func(SET_VERIF_THESHOLDS_MSG,Set_verif_thresh_menu);
	
}
static void Display_current_thresholds(void)
{
	sprintf((char *)tmpstr,"\n\n\rLower Threshold = %05dmV\n\rUpper Threshold = %05dmV\n\rPolyfuse Threshold = %05dmV\n\n\r",lower_switch_threshold,upper_switch_threshold,polyfuse_threshold);
	ASC_Asci_msg(tmpstr);
}
/*********************************************************************
*                       End of menu.c                                *
*********************************************************************/
