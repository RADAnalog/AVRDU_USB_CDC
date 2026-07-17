//*****************************************************************************
//	USB Private Inline Functions - AVR_DU series
//
//	This header should only be included in usb.c to maintain private scope
//
//	Author: Richard
//	Date:	2026-04-15
//
//*****************************************************************************
#ifndef USB_PRIVATE_H
#define USB_PRIVATE_H

//*****************************************************************************
//	USB Device inline functions
//*****************************************************************************

// Set device address
static inline void usbSetDeviceAddress(uint8_t addr)
{
	// Mask address bit7
	USB0_ADDR = addr & 0x7F;
}

// Enable Resume interrupt
static inline void usbEnableResume(void)
{
	USB0.INTCTRLA |= USB_RESUME_bm;
}

// Disable Resume interrupt
static inline void usbDisableResume(void)
{
	USB0.INTCTRLA &= ~USB_RESUME_bm;
}

// Enable Suspend interrupt
static inline void usbEnableSuspend(void)
{
	USB0.INTCTRLA |= USB_SUSPEND_bm;
}

// Disable Suspend interrupt
static inline void usbDisableSuspend(void)
{
	USB0.INTCTRLA &= ~USB_SUSPEND_bm;
}

//*****************************************************************************
//	USB Endpoint inline functions
//*****************************************************************************	
// Wait for Read-Modify-Write operation to complete (Datasheet 27.3.3.1)
static inline void waitUntilRMWDone(void)
{
	// Wait while busy flag is set
	while ((USB0.INTFLAGSB & USB_RMWBUSY_bm) != 0)
	{
		// Wait loop
	}
}

// Stalls the Control endpoint until cleared
static inline void usbEnableStallRequest(void)
{
	endpointTable.EP[EP0].IN.CTRL |= USB_DOSTALL_bm;
	endpointTable.EP[EP0].OUT.CTRL |= USB_DOSTALL_bm;
}

// Clears the stall request on the Control endpoint
static inline void usbClearStallRequest(void)
{
	endpointTable.EP[EP0].IN.CTRL &= ~USB_DOSTALL_bm;
	endpointTable.EP[EP0].OUT.CTRL &= ~USB_DOSTALL_bm;
}

// Wait for IN transaction on specified endpoint to complete
static inline void usbWaitInTransactionComplete(uint8_t ep)
{
	// Wait until Transaction Complete flag is set
	while ((endpointTable.EP[ep].IN.STATUS & USB_TRNCOMPL_bm) == 0)
	{
		// Wait loop
	}
}

// Wait for OUT transaction on specified endpoint to complete
static inline void usbWaitOutTransactionComplete(uint8_t ep)
{
	// Wait until Transaction COmplete flag is set
	while ((endpointTable.EP[ep].OUT.STATUS & USB_TRNCOMPL_bm) == 0)
	{
		// Wait loop
	}
}

// Clear Setup on Control endpoint
static inline void usbClearSetup(void)
{
	// Clear IN SETUP flag
	waitUntilRMWDone();
	USB0.STATUS[EP0].INCLR = USB_EPSETUP_bm;

	// Clear OUT SETUP flag
	waitUntilRMWDone();
	USB0.STATUS[EP0].OUTCLR = USB_EPSETUP_bm;
}

// Acknowledge transaction complete on specified IN endpoint 
static inline void usbClearInTransaction(uint8_t ep)
{
	waitUntilRMWDone();
	USB0.STATUS[ep].INCLR = USB_TRNCOMPL_bm;
}

// Acknowledge transaction complete on specified OUT endpoint
static inline void usbClearOutTransaction(uint8_t ep)
{
	waitUntilRMWDone();
	USB0.STATUS[ep].OUTCLR = USB_TRNCOMPL_bm;
}

// Acknowledge IN transaction on specified Endpoint
static inline void usbAckIn(uint8_t ep)
{
	waitUntilRMWDone();
	USB0.STATUS[ep].INCLR = USB_BUSNAK_bm;	// Strobe
}

// Acknowledge OUT transaction on specified Endpoint
static inline void usbAckOut(uint8_t ep)
{
	waitUntilRMWDone();
	USB0.STATUS[ep].OUTCLR = USB_BUSNAK_bm;	// Strobe
}

#endif