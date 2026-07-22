//*****************************************************************************
//	AVR16DU14 - USB Test Program - Bare metal version
//	                        _____
//                    GND 1|*    |14 VDD+
//              RESET PF6 2|     |13 PD7 -> Configured LED
//                   UPDI 3|     |12 PD6 -> Suspend LED
//                    PA0 4|TxD  |11 PD5
//           0.47u*   PA1 5|RxD  |10 PD4
//		 GND --||--- VUSB 6|     |9  PC3
//                     D- 7|_____|8  D+
//
//	*Ceramic MLCC (Murata GRM21BR71H474KA88K - 0.47u 50V X7R)
//
//	TXD - on PA0, 115200 baud rate
//
//	NOTES: 
//	1) The AVRxxDU refers to a "transaction" as an IN or OUT packet followed by ACK/NAK.
//	USB refers to a "transaction" as the completion of Token->Data->Handshake sequence.
//
//	2) For control endpoint, DATAPTR for the OUT endpoint is used for SETUP transactions,
//	and DATAPTR for the IN endpoint is used for both OUT and IN Control transactions
//	(Datasheet section 27.7.7).
//
//	FUSE: SYSCFG.RSTPINCFG <- Reset Mode on PF6 has been enabled on the prototype
//
//	Created: 2026-01-08 - Inherit from AT90USB162 project
//	Version: 2026-02-26 - Enumeration completes
//			 2026-03-16 - Full HID implementation including Reset, Suspend, Resume
//			 2026-03-18 - FIFO testing and characterization - see lab notes.  FIFO features not used
//			 2026-03-20 - Add HID Get/Set Reports and test with LVR's Generic_Hid.cs.  EP1.IN and EP1.OUT
//						  added to the descriptor and tested.
//			 2026-03-30 - MULTIPKT cannot handle host-terminated Data IN during the *first* GetDescriptor
//						  request if BUFSIZE is smaller than the 18-byte Device_Descriptor. 
//						  I.e., the host will only request the first 8 byte packet if BUFSIZE=8 and
//						  waitInTransactionComplete() will block because TRNCOMPL never asserts.
//						  Solution: use a BUFSIZE larger than the Device_Descriptor (e.g., =32 or =64).
//			 2026-04-02 - When Microchip Studio is running, it causes a periodic roll of
//						  string descriptor requests. Set all string descriptor indices = 0 to avoid
//						  this annoyance.
//			 2026-04-14	  Some code refactoring, general clean-up
//  Author : Richard
//
//*****************************************************************************
#include "config.h"		// F_CPU
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>		// printf
#include <stdbool.h>
#include <string.h>		// memcpy
#include <stddef.h>		// wchar_t
#include "usb.h"
#include "led.h"
#include "usart.h"

//*****************************************************************************
//	Function Definitions
//*****************************************************************************
void init(void);


//*****************************************************************************
//	Main
//*****************************************************************************
int main(void)
{
	init();
	ledInit();
	usartInit(BAUD_115200);	// Debug
	
	printf("AVR16DU14\n");
	
	usbInit();
	
	sei();

	// Attach to bus
	usbAttach();
	
	while (1)
	{

	}
}

//*****************************************************************************
//	Initialize AVR
//*****************************************************************************
void init(void)
{
	// Set CPU clock to 24MHz
	_PROTECTED_WRITE(CLKCTRL.OSCHFCTRLA, CLKCTRL_FRQSEL_24M_gc | CLKCTRL_RUNSTBY_bm | CLKCTRL_AUTOTUNE_SOF_gc);
	
	// Wait for system clock to stabilize
	while((CLKCTRL.MCLKSTATUS & CLKCTRL_OSCHFS_bm) == 0);
}
