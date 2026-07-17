//*****************************************************************************
//	USB Protocol Functions - AVR_DU series
//
//	Author: Richard
//	Date:	2026-04-13
//
//*****************************************************************************
#include "config.h"			// F_CPU
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>			// printf
#include <stdbool.h>
#include <string.h>			// memcpy
#include <stddef.h>			// wchar_t
#include "usb.h"			// USB Common functions
#include "usb_private.h"	// USB helper private functions
#include "led.h"			// LED functions
#include "descriptor_cdc.h"

//	Endpoint Table - must be word aligned (datasheet 27.5.7)
Usb_Endpoint_Table_t endpointTable __attribute__((aligned(2)));

//	Endpoint buffers
Usb_SetupPacket_t setupPacket;	// EP0.OUT 8 byte Setup packet
uint8_t controlPacket[64];		// EP0.IN/OUT Control packet
uint8_t ep1InPacket[8];			// EP1.IN packet - NOTIFICATION
uint8_t ep2InPacket[64];		// EP2.IN packet - data
uint8_t ep2OutPacket[64];		// EP2.OUT packet - data

// Variables
volatile bool usbStateConfigured = false;
volatile bool usbDataReady = false;

//*****************************************************************************
//	Initialize USB interface
//*****************************************************************************
void usbInit(void)
{
	// Enable internal 3.3 V regulator
	SYSCFG.VUSBCTRL = SYSCFG_USBVREG_bm;
	
	// Enable USB to request PLL, set the maximum endpoint address
	USB0.CTRLA = USB_ENABLE_bm | EP_MAX_ADDR;

	// Wait for PLL lock
	while ((CLKCTRL.USBPLLSTATUS & CLKCTRL_PLLS_bm) == 0);		


	// Setup Endpoint Table pointer
	USB0.EPPTR = (uint16_t)endpointTable.EP;
	
	// -- Initialize Endpoint 0 (Control) --
	
	// Control EP0 Setup (Host -> Device)
	endpointTable.EP[EP0].OUT.STATUS	=	0;
	endpointTable.EP[EP0].OUT.CTRL		=	USB_TYPE_CONTROL_gc |	
											USB_TCDSBL_bm |					// Disable global interrupt
											USB_BUFSIZE_DEFAULT_BUF8_gc;	// Setup packet is 8 bytes
	endpointTable.EP[EP0].OUT.CNT		=	0;
	endpointTable.EP[EP0].OUT.DATAPTR	=	(uint16_t)&setupPacket;			// Setup packet OUT buffer
	endpointTable.EP[EP0].OUT.MCNT		=	0;
	
	// Control EP0 IN (Device -> Host)
	endpointTable.EP[EP0].IN.STATUS		=	0;
	endpointTable.EP[EP0].IN.CTRL		=	USB_TYPE_CONTROL_gc |
											USB_TCDSBL_bm |					// Disable global interrupt
											USB_MULTIPKT_bm | USB_AZLP_bm |	// Multipacket and Automatic ZLP
											USB_BUFSIZE_DEFAULT_BUF64_gc;	// Buffer size must match DeviceDescriptor.MaxPacketSize0
	endpointTable.EP[EP0].IN.CNT		=	0;
	endpointTable.EP[EP0].IN.DATAPTR	=	(uint16_t)controlPacket;		// Control IN/OUT buffer
	endpointTable.EP[EP0].IN.MCNT		=	0;

	// -- Initialize Endpoint 1 --

	// Interrupt EP1.IN (Device -> Host)
	endpointTable.EP[EP1].IN.STATUS		=	0;
	endpointTable.EP[EP1].IN.CTRL		=	USB_TYPE_BULKINT_gc |
											USB_BUFSIZE_DEFAULT_BUF8_gc;
	endpointTable.EP[EP1].IN.CNT		=	0;
	endpointTable.EP[EP1].IN.DATAPTR	=	(uint16_t)ep1InPacket;
	endpointTable.EP[EP1].IN.MCNT		=	0;	
	
	// -- Initialize Endpoint 2
	
	// Interrupt EP2.IN (Device -> Host)
	endpointTable.EP[EP2].IN.STATUS		=	0;
	endpointTable.EP[EP2].IN.CTRL		=	USB_TYPE_BULKINT_gc |
											USB_BUFSIZE_DEFAULT_BUF64_gc;
	endpointTable.EP[EP2].IN.CNT		=	0;
	endpointTable.EP[EP2].IN.DATAPTR	=	(uint16_t)ep2InPacket;
	endpointTable.EP[EP2].IN.MCNT		=	0;	
	
	// Interrupt EP2.OUT (Host -> Device)
	endpointTable.EP[EP2].OUT.STATUS	=	0;
	endpointTable.EP[EP2].OUT.CTRL		=	USB_TYPE_BULKINT_gc |
											USB_BUFSIZE_DEFAULT_BUF64_gc;
	endpointTable.EP[EP2].OUT.CNT		=	0;
	endpointTable.EP[EP2].OUT.DATAPTR	=	(uint16_t)ep2OutPacket;
	endpointTable.EP[EP2].OUT.MCNT		=	0;	
	
	// -- Enable USB Bus and Endpoint interrupts --
	
	// Enable Bus interrupts
	USB0.INTCTRLA = USB_RESET_bm | USB_STALLED_bm;

	// Enable Endpoint interrupts
	USB0.INTCTRLB = USB_SETUP_bm | USB_TRNCOMPL_bm;
}

//*****************************************************************************
// USB Bus (Device) Interrupt Routine - INTFLAGSA
//*****************************************************************************
ISR(USB0_BUSEVENT_vect)
{
	// Reset
	if (USB0.INTFLAGSA & USB_RESET_bm)
	{
		USB0.INTFLAGSA = USB_RESET_bm;
		
		// Reset the address
		usbSetDeviceAddress(0x00);
		
		usbEnableSuspend();
		
		// Clear Status LEDs
		ledSuspended(OFF);
		ledConfigured(OFF);
		
		// Clear USB state
		usbStateConfigured = false;
	}

	// Suspended
	if (USB0.INTFLAGSA & USB_SUSPEND_bm)
	{
		USB0.INTFLAGSA = USB_SUSPEND_bm;

		usbDisableSuspend();
		usbEnableResume();

		ledSuspended(ON);
	}

	// Resume
	if (USB0.INTFLAGSA & USB_RESUME_bm)
	{
		USB0.INTFLAGSA = USB_RESUME_bm;

		usbDisableResume();
		usbEnableSuspend();

		ledSuspended(OFF);
	}

	// Stall
	if (USB0.INTFLAGSA & USB_STALLED_bm)
	{
		USB0.INTFLAGSA = USB_STALLED_bm;
		
		usbClearStallRequest();		
	}
}

//*****************************************************************************
// USB Transaction (Endpoint) Interrupt Routine - INTFLAGSB
//*****************************************************************************
ISR(USB0_TRNCOMPL_vect)
{
	// Setup
	if (USB0.INTFLAGSB & USB_SETUP_bm)
	{
		USB0.INTFLAGSB = USB_SETUP_bm;
		
		// Clear Setup before decoding request (Datasheet 27.7.2)
		usbClearSetup();
		usbHandleSetupRequest();
	}
	
	// Handle EP IN/OUT Transfers
	if (USB0.INTFLAGSB & USB_TRNCOMPL_bm)
	{
		USB0.INTFLAGSB = USB_TRNCOMPL_bm;
		
		// Notification packet EP1.IN (Device -> Host)
		if (endpointTable.EP[EP1].IN.STATUS & USB_TRNCOMPL_bm) 
		{	
			// Clear interrupt flag
			usbClearInTransaction(EP1);
		}
		
		// Device to Host packet EP2.IN
		if (endpointTable.EP[EP2].IN.STATUS & USB_TRNCOMPL_bm)
		{
			usbClearInTransaction(EP2);		
			
			if (usbDataReady == true)
			{
				usbDataReady = false;
				
				ep2InPacket[0] = ep2OutPacket[0];
				endpointTable.EP[EP2].IN.CNT = 1;
				
				printf("EP2.IN=%02x\n", ep2InPacket[0]);
			}

			usbAckIn(EP2);
		}		
		
		// Host to Device packet EP2.OUT
		if (endpointTable.EP[EP2].OUT.STATUS & USB_TRNCOMPL_bm)
		{	
			usbClearOutTransaction(EP2);
			
			printf("EP2.OUT=%02x\n", ep2OutPacket[0]);
			
			usbDataReady = true;
			
			usbAckOut(EP2);
		}	
	}
}

//*****************************************************************************
//	Process the Setup Request
//*****************************************************************************
void usbHandleSetupRequest(void)
{
	// Debug - print Setup Packet
	printf("%02x %02x %04x %04x %04x\n", setupPacket.bmRequestType, setupPacket.bRequest, setupPacket.wValue, setupPacket.wIndex, setupPacket.wLength);
	
	switch (setupPacket.bmRequestType & BM_REQUEST_TYPE_MASK) // Type is bits 5 & 6
	{
		case Device_Standard:
			switch (setupPacket.bRequest)
			{
				case Standard_GetDescriptor:
					usbGetDescriptor();
					break;
				case Standard_SetAddress:
					usbSetAddress();
					break;
				case Standard_SetConfiguration:
					usbSetConfiguration();
					break;
				default:
					// Unsupported Standard_Request (bRequest)
					usbEnableStallRequest();
					break;
			}
			break;
		case Device_Class:
			switch (setupPacket.bRequest)
			{
				case Hid_GetReport:
					usbHidGetReport();
					break;
				case Hid_SetReport:
					usbHidSetReport();
					break;
				case Hid_SetIdle:
					usbHidSetIdle();
					break;
				default:
					// Unsupported Class_Request (bRequest)
					usbEnableStallRequest();
					break;
			}
			break;					
		default:
			// Unsupported Request_Type (bmRequestType)
			usbEnableStallRequest();
			break;
	}
}

//*****************************************************************************
//	Get Descriptor (bRequest = 0x06)
//*****************************************************************************
void usbGetDescriptor(void)
{
	switch (setupPacket.wValueH) // Descriptor type
	{
		case Descriptor_Device:
			usbSendDescriptor(&DeviceDescriptor, DeviceDescriptor.bLength);
			break;
		case Descriptor_Configuration:
			usbSendDescriptor(&ConfigurationDescriptor, ConfigurationDescriptor.Configuration.wTotalLength);
			break;
		case Descriptor_String:
			usbGetStringDescriptor();
			break;			
		default:
			// Unsupported Descriptor_Type (wValueH)
			usbEnableStallRequest();
			break;
	}
}

//*****************************************************************************
//	Get String Descriptor (bRequest = 0x06, wValueH = 0x03)
//*****************************************************************************
void usbGetStringDescriptor(void)
{
	switch (setupPacket.wValueL) // String index
	{
		case String_Language:
			usbSendDescriptor(&LanguageString, LanguageString.bLength);
			break;
		case String_Manufacturer:
			usbSendDescriptor(&ManufacturerString, ManufacturerString.bLength);
			break;
		case String_Product:
			usbSendDescriptor(&ProductString, ProductString.bLength);
			break;
		case String_SerialNumber:
			usbSendDescriptor(&SerialNumberString, SerialNumberString.bLength);
			break;
		default:
			// Unsupported String Index (wValueL)
			usbEnableStallRequest();
		break;
	}
}

//*****************************************************************************
//	Send Descriptor to Host - Device -> Host
//*****************************************************************************
void usbSendDescriptor(const void *descriptor, uint16_t descriptorLength)
{
	// Data Stage
	
	// Host may request more or less bytes than the full descriptor
	uint16_t length = (setupPacket.wLength < descriptorLength) ? setupPacket.wLength : descriptorLength;
	
	// Copy descriptor to the EP0 buffer
	memcpy(controlPacket, descriptor, length);
	
	// Set packet length
	endpointTable.EP[EP0].IN.CNT = length;
	endpointTable.EP[EP0].IN.MCNT = 0;
	
	// Ack Data IN Stage
	usbAckIn(EP0);
	usbWaitInTransactionComplete(EP0);
	
	// Ack Status OUT Stage
	usbAckOut(EP0);
	usbWaitOutTransactionComplete(EP0);
}

//*****************************************************************************
//	Set Address (bRequest = 0x05) Host -> Device
//*****************************************************************************
void usbSetAddress(void)
{
	// Save the address before a new Setup packet arrives
	uint8_t address = setupPacket.wValueL;

	// No Data Stage

	// Status Stage: reply with a ZLP
	endpointTable.EP[EP0].IN.CNT = 0;
	endpointTable.EP[EP0].IN.MCNT = 0;

	// Ack Status IN Stage	
	usbAckIn(EP0);
	usbWaitInTransactionComplete(EP0);

	// Set the new address
	usbSetDeviceAddress(address);
}

//*****************************************************************************
//	Set Configuration (bRequest = 0x09) Host -> Device
//*****************************************************************************
void usbSetConfiguration(void)
{
	// No Data Stage

	// Status Stage: reply with a ZLP
	endpointTable.EP[EP0].IN.CNT = 0;
	endpointTable.EP[EP0].IN.MCNT = 0;
	
	// Ack Status IN Stage	
	usbAckIn(EP0);
	usbWaitInTransactionComplete(EP0);

	// Enumeration is complete
	usbStateConfigured = true;
	ledConfigured(ON);
}

//*****************************************************************************
//	Clear Feature (bRequest = 0x01) Host -> Device
//*****************************************************************************
void usbClearFeature(void)
{
	// No Data Stage

	// Status Stage: reply with a ZLP
	endpointTable.EP[EP0].IN.CNT = 0;
	endpointTable.EP[EP0].IN.MCNT = 0;

	// Ack Status IN Stage
	usbAckIn(EP0);
	usbWaitInTransactionComplete(EP0);
}

//*****************************************************************************
//	Hid Get Report (bRequest = 0x01) Device -> Host
//*****************************************************************************
void usbHidGetReport(void)
{
	// Data Stage 
	
	controlPacket[0] = 0x01;
	controlPacket[1] = 0x02;
	
	printf("GetReport Tx=%02x %02x\n", controlPacket[0], controlPacket[1]);

	endpointTable.EP[EP0].IN.CNT = 2;
	endpointTable.EP[EP0].IN.MCNT = 0;

	// Ack Data IN Stage
	usbAckIn(EP0);
	usbWaitInTransactionComplete(EP0);

	// Ack Status OUT Stage
	usbAckOut(EP0);
	usbWaitOutTransactionComplete(EP0);
}

//*****************************************************************************
//	Hid Set Report (bRequest = 0x09) Host -> Device
//*****************************************************************************
void usbHidSetReport(void)
{
	// Ack Data OUT Stage
	usbAckOut(EP0);
	usbWaitOutTransactionComplete(EP0);
		
	// For EP0 Reports, the IN endpoint is used for both IN and OUT transactions.
	printf("SetReport Rx=%02x %02x\n", controlPacket[0], controlPacket[1]);

	// Status Stage: reply with a ZLP
	endpointTable.EP[EP0].IN.CNT = 0;
	endpointTable.EP[EP0].IN.MCNT = 0;

	// Ack Status IN Stage
	usbAckIn(EP0);
	usbWaitInTransactionComplete(EP0);
}

//*****************************************************************************
//	Set Idle (bRequest = 0x0a) Host -> Device
//*****************************************************************************
void usbHidSetIdle(void)
{
	// No Data Stage

	//uint8_t duration = setupPacket.wValueH;
	//uint8_t reportId = setupPacket.wValueL;

	// Status Stage: reply with a ZLP
	endpointTable.EP[EP0].IN.CNT = 0;
	endpointTable.EP[EP0].IN.MCNT = 0;

	// Ack Status IN Stage
	usbAckIn(EP0);
	usbWaitInTransactionComplete(EP0);
}