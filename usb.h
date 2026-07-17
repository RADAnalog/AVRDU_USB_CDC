//*****************************************************************************
//	USB Common elements and public inline functions - AVR_DU series
//	
//	Author: Richard
//	Date:	2026-03-10
//
//*****************************************************************************
#ifndef USB_H
#define USB_H

#include "config.h"		// F_CPU
#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>		// printf

// ConfigurationDescriptor.bmAttributes options
#define USB_REMOTE_WAKEUP		0x20
#define USB_BUS_POWERED    		0x80	// | USB_REMOTE_WAKEUP
#define USB_SELF_POWERED    	0xC0	// | USB_REMOTE_WAKEUP

// usb function parameters
#define EP_MAX_ADDR				2		// Maximum endpoint address number
#define EP0						0		// Endpoint 0
#define EP1						1		// Endpoint 1
#define EP2						2

// HidReport.bEndpointAddress options
#define USB_EP_DIR_OUT			0x00
#define USB_EP_DIR_IN			0x80

// bmRequestType mask
#define BM_REQUEST_TYPE_MASK	0x60

// Device Request_Type (bmRequestType)
typedef enum {
	Device_Standard				= 0x00,
	Device_Class				= 0x20,
	Device_Vendor				= 0x40,
	Device_Reserved				= 0x60
} Usb_Device_Request_t;

// Standard Device Request (bRequest)
typedef enum {
	Standard_SetAddress 		= 0x05,
	Standard_GetDescriptor		= 0x06,
	Standard_SetConfiguration	= 0x09,
} Usb_Standard_Request_t;

// HID Device Request (bRequest)
typedef enum {
	Hid_GetReport				= 0x01,
	Hid_SetReport				= 0x09,
	Hid_SetIdle					= 0x0a
} Usb_Hid_Request_t;

// Descriptor Request Type (wValueH)
typedef enum {
	Descriptor_Device			= 0x01,
	Descriptor_Configuration	= 0x02,
	Descriptor_String			= 0x03,
	Descriptor_DeviceQualifier	= 0x06,
	Descriptor_Hid				= 0x21,
	Descriptor_HidReport		= 0x22
} Usb_Descriptor_Type_t;

// String Requests (wValueL)
typedef enum {
	String_Language				= 0,
	String_Manufacturer			= 1,
	String_Product				= 2,
	String_SerialNumber			= 3
} Usb_String_Request_t;

//*****************************************************************************
//	Setup Packet struct - 8 bytes
//*****************************************************************************
typedef struct {
	uint8_t						bmRequestType;
	uint8_t						bRequest;
	union {
		uint16_t				wValue;
		struct { // Redefine wValue:
			uint8_t				wValueL;
			uint8_t				wValueH;
		};
	};

	union {
		uint16_t				wIndex;
		struct { // Redefine wIndex:
			uint8_t				wIndexL;
			uint8_t				wIndexH;
		};
	};
	uint16_t					wLength;
} Usb_SetupPacket_t;

//*****************************************************************************
//	Endpoint Table struct - FIFO and FRAMENUM not implemented
//*****************************************************************************
//Endpoint Table struct
typedef struct {
	//uint8_t					FIFO[(EP_MAX_ADDR + 1) * 2];	// Two FIFO registers per EP
	USB_EP_PAIR_t				EP[EP_MAX_ADDR + 1];			// In/Out Endpoint Register Pairs
	//uint16_t	 				FRAMENUM;						// Frame Number
} Usb_Endpoint_Table_t;

//*****************************************************************************
// External Declarations
//*****************************************************************************
extern Usb_Endpoint_Table_t endpointTable;	// endpointTable is a Global Object


//*****************************************************************************
// Function Prototypes
//*****************************************************************************
void usbInit(void);
void usbHandleSetupRequest(void);
void usbSetupEndpoints(void);
void usbGetDescriptor(void);
void usbSendDescriptor(const void *descriptor, uint16_t descriptorLength);
void usbGetStringDescriptor(void);
void usbSetAddress(void);
void usbSetConfiguration(void);
void usbClearFeature(void);
void usbSetIdle(void);
void usbHidGetReport(void);
void usbHidSetReport(void);
void usbHidSetIdle(void);

//*****************************************************************************
//	USB Device Inline functions
//*****************************************************************************
// Attach to bus
static inline void usbAttach(void)
{
	USB0.CTRLB |= USB_ATTACH_bm;
}

// Detach from bus
static inline void usbDetach(void)
{
	USB0.CTRLB &= ~USB_ATTACH_bm;
}

#endif