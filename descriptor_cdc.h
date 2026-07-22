//*****************************************************************************
//	USB HID Descriptor - CDC for Virtual COM Port operation
//
//	Descriptors were generated using Copilot
//
//	Notification Endpoint (Interrupt IN, EP1.IN)
//	This endpoint is used by the device to send asynchronous status notifications to the host.
//	E.g., Serial state changes (DCD, DSR, break, ring indicator, etc.)
//	
//	Bulk OUT Endpoint (EP2.OUT)
//	The data pipe from the host to your device containing what the PC application writes 
//	to the COM port.
//
//	Bulk IN Endpoint (EP2.IN)
//	The data pipe from your device to the host containing everything your firmware writes 
//	to the virtual COM port
//
//	Author: Richard
//	Date:	2026-04-14
//
//*****************************************************************************
#ifndef DESCRIPTOR_CDC_H_
#define DESCRIPTOR_CDC_H_

#include <stddef.h>		// wchar_t

// USB Device Identifier
#define USB_VID					0x04D8		// Microchip
#define USB_PID					0x002D		// 0x0010 - 0x002F reserved for testing/non-public

// Unicode strings, no commas permitted
#define MANUFACTURER_STR 		L"USB Project-CDC"
#define PRODUCT_STR 			L"AVR16DU14"
#define SERIAL_NUMBER_STR 		L"0001"

//*****************************************************************************
//	Struct Typedefs
//*****************************************************************************
// Device Descriptor - 18 bytes
typedef struct {
	uint8_t     bLength;            // Size of this descriptor (18 bytes)
	uint8_t     bDescriptorType;    // DEVICE descriptor (0x01)
	uint16_t    bcdUSB;             // USB specification version (e.g., 0x0200)
	uint8_t     bDeviceClass;       // Class code (0 = defined per interface, or 0x02 for CDC)
	uint8_t     bDeviceSubClass;    // Subclass (CDC = 0x02)
	uint8_t     bDeviceProtocol;    // Protocol (CDC ACM = 0x01)
	uint8_t     bMaxPacketSize0;    // EP0 max packet size (8, 16, 32, or 64)
	uint16_t    idVendor;           // Vendor ID
	uint16_t    idProduct;          // Product ID
	uint16_t    bcdDevice;          // Device release number
	uint8_t     iManufacturer;      // String index for manufacturer
	uint8_t     iProduct;           // String index for product
	uint8_t     iSerialNumber;      // String index for serial number
	uint8_t     bNumConfigurations; // Number of configurations (usually 1)
} Usb_DeviceDescriptor_t;

// Configuration Descriptor Header - 9 bytes
typedef struct {
	uint8_t     bLength;            // Size of this descriptor (9 bytes)
	uint8_t     bDescriptorType;    // CONFIGURATION descriptor (0x02)
	uint16_t    wTotalLength;       // Total length of ALL descriptors for this configuration
	uint8_t     bNumInterfaces;     // Number of interfaces in this configuration
	uint8_t     bConfigurationValue;// Identifier for SetConfiguration()
	uint8_t     iConfiguration;     // String index for configuration (0 = none)
	uint8_t     bmAttributes;       // Power attributes (0x80 = bus powered)
	uint8_t     bMaxPower;          // Max power in 2mA units (50 = 100mA)
} Usb_ConfigurationDescriptorHeader_t;

// Interface Descriptor - 9 bytes
typedef struct {
	uint8_t     bLength;            // Size of this descriptor (9 bytes)
	uint8_t     bDescriptorType;    // INTERFACE descriptor (0x04)
	uint8_t     bInterfaceNumber;   // Index of this interface
	uint8_t     bAlternateSetting;  // Alternate setting number
	uint8_t     bNumEndpoints;      // Number of endpoints (excluding EP0)
	uint8_t     bInterfaceClass;    // Class code (CDC Comm = 0x02, Data = 0x0A)
	uint8_t     bInterfaceSubClass; // Subclass (ACM = 0x02, Data = 0x00)
	uint8_t     bInterfaceProtocol; // Protocol (AT commands = 0x01)
	uint8_t     iInterface;         // String index (0 = none)
} Usb_InterfaceDescriptor_t;

// CDC Header Functional Descriptor
typedef struct {
	uint8_t  bFunctionLength;		// = 5
	uint8_t  bDescriptorType;		// = 0x24 (CS_INTERFACE)
	uint8_t  bDescriptorSubType;	// = 0x00 (Header)
	uint16_t bcdCDC;				// CDC spec version (0x0110)
} Usb_CdcHeaderFunctionalDescriptor_t;

// CDC-ACM Functional Descriptor
typedef struct {
	uint8_t  bFunctionLength;		// = 4
	uint8_t  bDescriptorType;		// = 0x24
	uint8_t  bDescriptorSubType;	// = 0x02 (ACM)
	uint8_t  bmCapabilities;		// Bit mask of ACM capabilities
} Usb_CdcAcmFunctionalDescriptor_t;

// CDC Union Functional Descriptor
typedef struct {
	uint8_t  bFunctionLength;		// = 5
	uint8_t  bDescriptorType;		// = 0x24
	uint8_t  bDescriptorSubType;	// = 0x06 (Union)
	uint8_t  bMasterInterface;		// Comm interface #
	uint8_t  bSlaveInterface0;		// Data interface #
} Usb_CdcUnionFunctionalDescriptor_t;

// CDC Call Management Descriptor
typedef struct {
	uint8_t  bFunctionLength;		// = 5
	uint8_t  bDescriptorType;		// = 0x24
	uint8_t  bDescriptorSubType;	// = 0x01 (Call Management)
	uint8_t  bmCapabilities;		// 0x00 = no Call Management
	uint8_t  bDataInterface;		// Data interface #
} Usb_CdcCallManagementDescriptor_t;

// Endpoint Descriptor - 7 bytes
typedef struct {
	uint8_t     bLength;            // = 7
	uint8_t     bDescriptorType;    // = 0x05 (Endpoint)
	uint8_t     bEndpointAddress;   // IN/OUT + endpoint number
	uint8_t     bmAttributes;       // Transfer type (Bulk/Interrupt/Iso)
	uint16_t    wMaxPacketSize;     // Max packet size
	uint8_t     bInterval;          // Polling interval (ignored for Bulk)
} Usb_EndpointDescriptor_t;

// String Descriptor
typedef struct {
	uint8_t     bLength;			// = Length
	uint8_t     bDescriptorType;	// = 0x03 (String)
	wchar_t     bString[];			// Unicode string
} Usb_StringDescriptor_t;

//*****************************************************************************
// CDC Control Line State struct
//*****************************************************************************
typedef struct
{
	uint32_t dwDTERate;   // baud
	uint8_t  bCharFormat; // stop bits
	uint8_t  bParityType; // parity
	uint8_t  bDataBits;   // data bits
} USB_CDC_LineCoding_t;

//*****************************************************************************
// Configuration Descriptor Assembly
//*****************************************************************************
typedef struct {
	Usb_ConfigurationDescriptorHeader_t     Configuration;

	// Interface 0 — CDC Communication
	Usb_InterfaceDescriptor_t               CdcCommInterface;

	// CDC Functional Descriptors
	Usb_CdcHeaderFunctionalDescriptor_t     CdcHeaderFuncDesc;
	Usb_CdcAcmFunctionalDescriptor_t        CdcAcmFuncDesc;
	Usb_CdcUnionFunctionalDescriptor_t      CdcUnionFuncDesc;
	Usb_CdcCallManagementDescriptor_t       CdcCallMgmtFuncDesc;

	// Notification Endpoint (Interrupt EP1.IN)
	Usb_EndpointDescriptor_t                CdcNotificationEndpoint;

	// Interface 1 — CDC Data
	Usb_InterfaceDescriptor_t               CdcDataInterface;

	// Data Endpoints (Bulk EP2.OUT + Bulk EP2.IN)
	Usb_EndpointDescriptor_t                CdcOutEndpoint;
	Usb_EndpointDescriptor_t                CdcInEndpoint;
	
} Usb_ConfigurationDescriptor_t;


//*****************************************************************************
//	Descriptor Initializers
//*****************************************************************************

// Initialize Device Descriptor
const Usb_DeviceDescriptor_t DeviceDescriptor = {
	.bLength            = sizeof(Usb_DeviceDescriptor_t),
	.bDescriptorType    = 0x01,         // DEVICE descriptor
	.bcdUSB             = 0x0200,       // USB 2.00
	.bDeviceClass       = 0x02,         // CDC (Comm)
	.bDeviceSubClass    = 0x02,         // Abstract Control Model
	.bDeviceProtocol    = 0x01,         // AT commands (required by Windows)
	.bMaxPacketSize0    = 64,           // EP0 max packet
	.idVendor           = USB_VID,
	.idProduct          = USB_PID,
	.bcdDevice          = 0x0001,       // Device version
	.iManufacturer      = 0,
	.iProduct           = 0,
	.iSerialNumber      = 0,
	.bNumConfigurations = 1
};

// Initialize Configuration Descriptor Assembly
const Usb_ConfigurationDescriptor_t ConfigurationDescriptor = {

	// Configuration Header
	.Configuration = {
		.bLength            = sizeof(Usb_ConfigurationDescriptorHeader_t),
		.bDescriptorType    = 0x02,     // CONFIGURATION
		.wTotalLength       = sizeof(Usb_ConfigurationDescriptor_t),
		.bNumInterfaces     = 2,
		.bConfigurationValue= 1,
		.iConfiguration     = 0,
		.bmAttributes       = USB_SELF_POWERED,
		.bMaxPower          = 100 / 2   // 100 mA
	},

	// Interface 0 — CDC Communication
	.CdcCommInterface = {
		.bLength            = sizeof(Usb_InterfaceDescriptor_t),
		.bDescriptorType    = 0x04,     // INTERFACE
		.bInterfaceNumber   = 0,
		.bAlternateSetting  = 0,
		.bNumEndpoints      = 1,
		.bInterfaceClass    = 0x02,     // CDC Comm
		.bInterfaceSubClass = 0x02,     // ACM
		.bInterfaceProtocol = 0x01,     // AT commands
		.iInterface         = 0
	},

	// CDC Functional Descriptors
	.CdcHeaderFuncDesc = {
		.bFunctionLength    = sizeof(Usb_CdcHeaderFunctionalDescriptor_t),
		.bDescriptorType    = 0x24,
		.bDescriptorSubType = 0x00,     // Header
		.bcdCDC             = 0x0110
	},

	.CdcAcmFuncDesc = {
		.bFunctionLength    = sizeof(Usb_CdcAcmFunctionalDescriptor_t),
		.bDescriptorType    = 0x24,
		.bDescriptorSubType = 0x02,     // ACM
		.bmCapabilities     = 0x02
	},

	.CdcUnionFuncDesc = {
		.bFunctionLength    = sizeof(Usb_CdcUnionFunctionalDescriptor_t),
		.bDescriptorType    = 0x24,
		.bDescriptorSubType = 0x06,     // Union
		.bMasterInterface   = 0,
		.bSlaveInterface0   = 1
	},

	.CdcCallMgmtFuncDesc = {
		.bFunctionLength    = sizeof(Usb_CdcCallManagementDescriptor_t),
		.bDescriptorType    = 0x24,
		.bDescriptorSubType = 0x01,     // Call Management
		.bmCapabilities     = 0x00,
		.bDataInterface     = 1
	},

	// Notification Endpoint (Interrupt EP1.IN)
	.CdcNotificationEndpoint = {
		.bLength            = sizeof(Usb_EndpointDescriptor_t),
		.bDescriptorType    = 0x05,     // ENDPOINT
		.bEndpointAddress   = (USB_EP_DIR_IN | EP1),     // IN, EP1
		.bmAttributes       = 0x03,     // Interrupt
		.wMaxPacketSize     = 8,
		.bInterval          = 16
	},

	// Interface 1 — CDC Data
	.CdcDataInterface = {
		.bLength            = sizeof(Usb_InterfaceDescriptor_t),
		.bDescriptorType    = 0x04,
		.bInterfaceNumber   = 1,
		.bAlternateSetting  = 0,
		.bNumEndpoints      = 2,
		.bInterfaceClass    = 0x0A,     // Data
		.bInterfaceSubClass = 0x00,
		.bInterfaceProtocol = 0x00,
		.iInterface         = 0
	},

	// Bulk OUT Endpoint (EP2.OUT)
	.CdcOutEndpoint = {
		.bLength            = sizeof(Usb_EndpointDescriptor_t),
		.bDescriptorType    = 0x05,
		.bEndpointAddress   = (USB_EP_DIR_OUT | EP2),
		.bmAttributes       = 0x02,     // Bulk
		.wMaxPacketSize     = 64,
		.bInterval          = 0			// n/a for Bulk
	},

	// Bulk IN Endpoint (EP2.IN)
	.CdcInEndpoint = {
		.bLength            = sizeof(Usb_EndpointDescriptor_t),
		.bDescriptorType    = 0x05,
		.bEndpointAddress   = (USB_EP_DIR_IN | EP2),
		.bmAttributes       = 0x02,     // Bulk
		.wMaxPacketSize     = 64,
		.bInterval          = 0			// n/a for Bulk
	}
};

// Initialize String Descriptors

// String Index 0 - Language
const Usb_StringDescriptor_t LanguageString = {
	.bLength = 4,
	.bDescriptorType = 0x03,
	.bString = { 0x0409 }	// US English
};

// String Index 1 - Manufacturer
const Usb_StringDescriptor_t ManufacturerString = {
	.bLength = 2 + (sizeof(MANUFACTURER_STR) - 2),	// Subtract null bytes
	.bDescriptorType	= 0x03,
	.bString			= MANUFACTURER_STR
};

// String Index 2 - Product
const Usb_StringDescriptor_t ProductString = {
	.bLength = 2 + (sizeof(PRODUCT_STR) - 2),		// Subtract null bytes
	.bDescriptorType	= 0x03,
	.bString			= PRODUCT_STR
};

// String Index 3 - Serial Number
const Usb_StringDescriptor_t SerialNumberString = {
	.bLength = 2 + (sizeof(SERIAL_NUMBER_STR) - 2),	// Subtract null bytes
	.bDescriptorType	= 0x03,
	.bString			= SERIAL_NUMBER_STR
};

#endif
