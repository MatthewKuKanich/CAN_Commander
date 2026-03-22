// Source: https://github.com/expected-ingot/flipper-xinput/blob/master/app.c

#include "controller.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_usb.h>
#include <applications/services/power/power_service/power.h>
#include <usb.h>
#include <usb_hid.h>
#include <stdlib.h>
#include <gui/gui.h>
#include <input/input.h>

#define XBOX_SURFACE_EP_IN 0x81 // IN Endpoint 1
#define HID_EP_SZ 0x10

typedef struct {
	uint8_t x, y;
} Position;
enum {
	UP = 1 << 0,
	DOWN = 1 << 1,
	LEFT = 1 << 2,
	RIGHT = 1 << 3,
	START = 1 << 4,
	BACK = 1 << 5,
	L3 = 1 << 6,
	R3 = 1 << 7,
	LEFT_BUMPER = 1 << 8,
	RIGHT_BUMPER = 1 << 9,
	LOGO = 1 << 10,
	THE_VOID_ONE = 1 << 11, // https://www.partsnotincluded.com/wp-content/uploads/2019/03/X360_ButtonPackets.jpg
	A = 1 << 12,
	B = 1 << 13,
	X = 1 << 14,
	Y = 1 << 15,
} xbox_buttons;

// "shit" variables are unknown variables. I couldn't find any other way to add those. /shrug

struct xbox_control_surface {
	// For the buttons, 0 means not pressed and 1 means pressed
	uint8_t message_type; // 0x00
	uint8_t length; // 20
	uint16_t buttons; // Refer to xbox_buttons enum for flags
	uint8_t left_trigger; // 0-255
	uint8_t right_trigger;
	int16_t left_x;
	int16_t left_y;
	int16_t right_x;
	int16_t right_y;
	uint8_t shit[6];
} FURI_PACKED;

struct xbox_control_surface current_surface;

struct usb_unknown0_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t shit1, shit2, shit3, shit4; // 0x00, 0x01, 0x01, 0x25
	uint8_t bEndpointAddress; // 0x81 IN endpoint 1
	uint8_t bMaxDataSize;
	uint8_t shit5, shit6, shit7, shit8, shit9; // 0x00, 0x00, 0x00, 0x00, 0x13
	uint8_t bEndpointAddress2; // 0x01 OUT endpoint 1
	uint8_t bMaxDataSize2;
	uint8_t shit10, shit11; // 0x00, 0x00
};

struct usb_unknown1_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t shit1, shit2, shit3, shit4;
	uint8_t bEndpointAddress;
	uint8_t bMaxDataSize;
	uint8_t shit5;
	uint8_t bEndpointAddress2;
	uint8_t bMaxDataSize2;
	uint8_t shit6;
	uint8_t bEndpointAddress3;
	uint8_t bMaxDataSize3;
	uint8_t shit7, shit8, shit9, shit10, shit11, shit12;
	uint8_t bEndpointAddress4;
	uint8_t bMaxDataSize4;
	uint8_t shit13, shit14, shit15, shit16, shit17;
};

struct usb_unknown2_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t shit1, shit2, shit3, shit4;
	uint8_t bEndpointAddress;
	uint8_t bMaxDataSize;
	uint8_t shit5;
};

struct usb_unknown3_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t shit1, shit2, shit3, shit4;
};

struct XboxIntf0Descriptor {
	struct usb_interface_descriptor hid;
	struct usb_unknown0_descriptor unknown;
	struct usb_endpoint_descriptor hid_ep_in;
	struct usb_endpoint_descriptor hid_ep_out;
};

struct XboxIntf1Descriptor {
	struct usb_interface_descriptor hid;
	struct usb_unknown1_descriptor unknown;
	struct usb_endpoint_descriptor hid_ep_in_2;
	struct usb_endpoint_descriptor hid_ep_out_2;
	struct usb_endpoint_descriptor hid_ep_in_3;
	struct usb_endpoint_descriptor hid_ep_out_3;
};

struct XboxIntf2Descriptor {
	struct usb_interface_descriptor hid;
	struct usb_unknown2_descriptor unknown;
	struct usb_endpoint_descriptor hid_ep_in_4;
};

struct XboxIntf3Descriptor {
	struct usb_interface_descriptor hid;
	struct usb_unknown3_descriptor unknown;
};

struct XboxConfigDescriptor {
	struct usb_config_descriptor config;
	struct XboxIntf0Descriptor intf_0;
	struct XboxIntf1Descriptor intf_1;
	struct XboxIntf2Descriptor intf_2;
	struct XboxIntf3Descriptor intf_3;
} FURI_PACKED;

struct usb_device_descriptor xbox_device_desc = {
	.bLength = 0x12,
	.bDescriptorType = 0x01,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0xFF, // Vendor specific
	.bDeviceSubClass = 0xFF,
	.bDeviceProtocol = 0xFF,
	.bMaxPacketSize0 = 0x08,
	.idVendor = 0x045E,
	.idProduct = 0x028E,
	.bcdDevice = 0x0114,
	.iManufacturer = 0x01,
	.iProduct = 0x02,
	.iSerialNumber = 0x03,
	.bNumConfigurations = 0x01,
};

const struct XboxConfigDescriptor xbox_cfg_desc = {
	.config = {
		.bLength = 9,
		.bDescriptorType = 0x02, // CONFIGURATION
		.wTotalLength = 153,
		.bNumInterfaces = 4,
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = 0b10100000, // Not self-powered, remote wake-up
		.bMaxPower = USB_CFG_POWER_MA(500),
	},
	.intf_0 = {
		.hid = {
			.bLength = 9,
			.bDescriptorType = 0x04, // INTERFACE
			.bInterfaceNumber = 0,
			.bAlternateSetting = 0,
			.bNumEndpoints = 2,
			.bInterfaceClass = 0xFF, // Vendor Specific
			.bInterfaceSubClass = 0x5D,
			.bInterfaceProtocol = 0x01,
			.iInterface = 0,
		},
		.unknown = { // Unknown descriptor
			.bLength = 17,
			.bDescriptorType = 0x21, // UNKNOWN
			.shit1 = 0x00, .shit2 = 0x01, .shit3 = 0x01,
			.shit4 = 0x25,
			.bEndpointAddress = 0x81, // IN Endpoint 1
			.bMaxDataSize = 20,
			.shit5 = 0x00, .shit6 = 0x00, .shit7 = 0x00, .shit8 = 0x00, .shit9 = 0x13,
			.bEndpointAddress2 = 0x01, // OUT Endpoint 1
			.bMaxDataSize2 = 8,
			.shit10 = 0x00, .shit11 = 0x00,
		},
		.hid_ep_in = {
			.bLength = 7,
			.bDescriptorType = 0x05, // ENDPOINT
			.bEndpointAddress = 0x81, // IN Endpoint 1
			.bmAttributes = 0x03,
			.wMaxPacketSize = 32,
			.bInterval = 4,
		},
		.hid_ep_out = {
			.bLength = 7,
			.bDescriptorType = 0x05, // ENDPOINT
			.bEndpointAddress = 0x01, // OUT Endpoint 1
			.bmAttributes = 0x03,
			.wMaxPacketSize = 32,
			.bInterval = 8,
		}
	},
	.intf_1 = {
		.hid = {
			.bLength = 9,
			.bDescriptorType = 0x04, // INTERFACE
			.bInterfaceNumber = 1,
			.bAlternateSetting = 0,
			.bNumEndpoints = 4,
			.bInterfaceClass = 0xFF, // Vendor Specific
			.bInterfaceSubClass = 0x5D,
			.bInterfaceProtocol = 0x03,
			.iInterface = 0,
		},
		.unknown = {
			.bLength = 27,
			.bDescriptorType = 0x21, // UNKNOWN
			.shit1 = 0x00,
			.shit2 = 0x01,
			.shit3 = 0x01,
			.shit4 = 0x01,
			.bEndpointAddress = 0x82, // IN Endpoint 2
			.bMaxDataSize = 64,
			.shit5 = 0x01,
			.bEndpointAddress2 = 0x02, // OUT Endpoint 2
			.bMaxDataSize2 = 32,
			.shit6 = 0x16,
			.bEndpointAddress3 = 0x83, // IN Endpoint 3
			.bMaxDataSize3 = 0,
			.shit7 = 0x00,
			.shit8 = 0x00,
			.shit9 = 0x00,
			.shit10 = 0x00,
			.shit11 = 0x00,
			.shit12 = 0x16,
			.bEndpointAddress4 = 0x03, // OUT Endpoint 3
			.bMaxDataSize4 = 0,
			.shit13 = 0x00,
			.shit14 = 0x00,
			.shit15 = 0x00,
			.shit16 = 0x00,
			.shit17 = 0x00,
		},
		.hid_ep_in_2 = {
			.bLength = 7,
			.bDescriptorType = 0x05, // ENDPOINT
			.bEndpointAddress = 0x82, // IN Endpoint 2
			.bmAttributes = 0x03,
			.wMaxPacketSize = 32,
			.bInterval = 2,
		},
		.hid_ep_out_2 = {
			.bLength = 7,
			.bDescriptorType = 0x05, // ENDPOINT
			.bEndpointAddress = 0x02, // OUT Endpoint 2
			.bmAttributes = 0x03,
			.wMaxPacketSize = 32,
			.bInterval = 4,
		},
		.hid_ep_in_3 = {
			.bLength = 7,
			.bDescriptorType = 0x05, // ENDPOINT
			.bEndpointAddress = 0x83, // IN Endpoint 3
			.bmAttributes = 0x03,
			.wMaxPacketSize = 32,
			.bInterval = 64,
		},
		.hid_ep_out_3 = {
			.bLength = 7,
			.bDescriptorType = 0x05, // ENDPOINT
			.bEndpointAddress = 0x03, // OUT Endpoint 3
			.bmAttributes = 0x03,
			.wMaxPacketSize = 32,
			.bInterval = 16,
		},
	},
	.intf_2 = {
		.hid = {
			.bLength = 9,
			.bDescriptorType = 0x04, // INTERFACE
			.bInterfaceNumber = 2,
			.bAlternateSetting = 0,
			.bNumEndpoints = 1,
			.bInterfaceClass = 0xFF, // Vendor Specific
			.bInterfaceSubClass = 0x5D,
			.bInterfaceProtocol = 0x02,
			.iInterface = 0,
		},
		.unknown = {
			.bLength = 9,
			.bDescriptorType = 0x21, // UNKNOWN
			.shit1 = 0x00,
			.shit2 = 0x01,
			.shit3 = 0x01,
			.shit4 = 0x22,
			.bEndpointAddress = 0x84, // IN Endpoint 4
			.bMaxDataSize = 7,
			.shit5 = 0x00,
		},
		.hid_ep_in_4 = {
			.bLength = 7,
			.bDescriptorType = 0x05, // ENDPOINT
			.bEndpointAddress = 0x84, // IN Endpoint 4
			.bmAttributes = 0x03,
			.wMaxPacketSize = 32,
			.bInterval = 16,
		}
	},
	.intf_3 = {
		.hid = {
			.bLength = 9,
			.bDescriptorType = 0x04, // INTERFACE
			.bInterfaceNumber = 3,
			.bAlternateSetting = 0,
			.bNumEndpoints = 0,
			.bInterfaceClass = 0xFF, // Vendor Specific
			.bInterfaceSubClass = 0xFD,
			.bInterfaceProtocol = 0x13,
			.iInterface = 4,
		},
		.unknown = {
			.bLength = 6,
			.bDescriptorType = 0x41, // UNKNOWN
			.shit1 = 0x00,
			.shit2 = 0x01,
			.shit3 = 0x01,
			.shit4 = 0x03,
		},
	},

};

bool boot_protocol = false;
usbd_device* usb_dev;
FuriSemaphore* hid_semaphore = NULL;
bool hid_connected = false;
bool started = false;
bool running = false;

void* hid_set_string_descr(char* str) {
	furi_assert(str);

	size_t len = strlen(str);
	struct usb_string_descriptor* dev_str_desc = malloc(len * 2 + 2);
	dev_str_desc->bLength = len * 2 + 2;
	dev_str_desc->bDescriptorType = USB_DTYPE_STRING;
	for(size_t i = 0; i < len; i++)
		dev_str_desc->wString[i] = str[i];

	return dev_str_desc;
}

void hid_txrx_ep_callback(usbd_device* dev, uint8_t event, uint8_t ep) {
	UNUSED(dev);
	UNUSED(ep);
	if(event == usbd_evt_eptx) {
		furi_semaphore_release(hid_semaphore);
	} else if(boot_protocol == true) {
		//uint8_t message_type;
		//usbd_ep_read(usb_dev, ep, &led_state, sizeof(led_state));
	} else {
		//struct HidReportLED leds;
		//usbd_ep_read(usb_dev, ep, &leds, sizeof(leds));
		//led_state = leds.led_state;
	}
}
usbd_respond hid_ep_config(usbd_device* dev, uint8_t cfg) {
	switch(cfg) {
	case 0:
		/* deconfiguring device */
		usbd_ep_deconfig(dev, XBOX_SURFACE_EP_IN);
		usbd_reg_endpoint(dev, XBOX_SURFACE_EP_IN, 0);
		return usbd_ack;
	case 1:
		/* configuring device */
		usbd_ep_config(dev, XBOX_SURFACE_EP_IN, USB_EPTYPE_INTERRUPT, 32);
		usbd_reg_endpoint(dev, XBOX_SURFACE_EP_IN, hid_txrx_ep_callback);
		//usbd_ep_write(dev, HID_EP_IN, 0, 0);
		boot_protocol = false; /* BIOS will SET_PROTOCOL if it wants this */
		return usbd_ack;
	default:
		return usbd_fail;
	}
}
usbd_respond hid_control(usbd_device* dev, usbd_ctlreq* req, usbd_rqc_callback* callback) {
	UNUSED(callback);
	/* HID control requests */
	if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
	        (USB_REQ_INTERFACE | USB_REQ_CLASS) &&
	        req->wIndex == 0) {
		switch(req->bRequest) {
		case USB_HID_SETIDLE:
			return usbd_ack;
		case USB_HID_GETREPORT:
			//if(boot_protocol == true) {
			//  dev->status.data_ptr = &hid_report.keyboard.boot;
			//  dev->status.data_count = sizeof(hid_report.keyboard.boot);
			//} else {
			//  dev->status.data_ptr = &hid_report;
			//  dev->status.data_count = sizeof(hid_report);
			//}
			return usbd_ack;
		case USB_HID_SETPROTOCOL:
			if(req->wValue == 0)
				boot_protocol = true;
			else if(req->wValue == 1)
				boot_protocol = false;
			else
				return usbd_fail;
			return usbd_ack;
		default:
			return usbd_fail;
		}
	}
	if(((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) ==
	        (USB_REQ_INTERFACE | USB_REQ_STANDARD) &&
	        req->wIndex == 0 && req->bRequest == USB_STD_GET_DESCRIPTOR) {
		switch(req->wValue >> 8) {
		case USB_DTYPE_HID:
			dev->status.data_ptr = (uint8_t*)&(xbox_cfg_desc.intf_0.hid);
			dev->status.data_count = sizeof(xbox_cfg_desc.intf_0.hid);
			return usbd_ack;
		default:
			return usbd_fail;
		}
	}
	return usbd_fail;
}

void hid_deinit(usbd_device* dev) {
	running = false;
	usbd_reg_config(dev, NULL);
	usbd_reg_control(dev, NULL);
}
void hid_on_wakeup(usbd_device* dev) {
	UNUSED(dev);
	if(!hid_connected) {
		hid_connected = true;
	}
}
void hid_on_suspend(usbd_device* dev) {
	UNUSED(dev);
	if(hid_connected) {
		hid_connected = false;
		running = false;
		furi_semaphore_release(hid_semaphore);
	}
}

bool hid_send_report() {
	if((hid_semaphore == NULL) || (hid_connected == false) || (running == false)) return false;
	FuriStatus status = furi_semaphore_acquire(hid_semaphore, 8 * 2);
	if(status == FuriStatusErrorTimeout) {
		return false;
	}
	furi_check(status == FuriStatusOk);
	if(hid_connected == false) {
		return false;
	}
	if (boot_protocol != true) {
		usbd_ep_write(usb_dev, XBOX_SURFACE_EP_IN, &current_surface, sizeof(current_surface));
	}
	return true;
}

void hid_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx);

FuriHalUsbInterface usb_xbox = {
	.init = hid_init,
	.deinit = hid_deinit,
	.wakeup = hid_on_wakeup,
	.suspend = hid_on_suspend,

	.dev_descr = (struct usb_device_descriptor*)&xbox_device_desc,

	.str_manuf_descr = NULL, //USB_STRING_DESC("Microsoft Corporation"),
	.str_prod_descr = NULL, //USB_STRING_DESC("Controller"),
	.str_serial_descr = NULL, //USB_STRING_DESC("08FEC93"),

	.cfg_descr = (void*)&xbox_cfg_desc
};

void hid_init(usbd_device* dev, FuriHalUsbInterface* intf, void* ctx) {
	UNUSED(intf);
	UNUSED(ctx);
	// FuriHalUsbHidConfig* cfg = (FuriHalUsbHidConfig*)ctx;
	// UNUSED(cfg);
	if(hid_semaphore == NULL) hid_semaphore = furi_semaphore_alloc(1, 1);
	usb_dev = dev;

	usb_xbox.dev_descr->iManufacturer = 1;
	usb_xbox.dev_descr->iProduct = 2;
	usb_xbox.dev_descr->iSerialNumber = 3;
	usb_xbox.dev_descr->idVendor = 0x045E; // Microsoft
	usb_xbox.dev_descr->idProduct = 0x028E; // Xbox 360 Controller

	usb_xbox.str_manuf_descr = hid_set_string_descr("Microsoft Corporation");
	usb_xbox.str_prod_descr = hid_set_string_descr("Controller");
	usb_xbox.str_serial_descr = hid_set_string_descr("08FEC93");

	usbd_reg_config(dev, hid_ep_config);
	usbd_reg_control(dev, hid_control);
	usbd_connect(dev, true);
}

////////////////////////////////////////////////////////////////////////////////

void controller_start() {
	if (started) return;
	started = true;
	running = false;

	furi_check(furi_hal_usb_set_config(&usb_xbox, NULL));
}

void controller_stop() {
	if (!started) return;
	started = false;
	running = false;

	furi_check(furi_hal_usb_set_config(NULL, NULL));
}

void controller_enable() {
	running = true;
}

bool controller_is_enabled() {
	return running;
}

void controller_handle(const CcEvent* event) {
    if(!event || event->type != CcEventTypeCanFrame) {
        return;
    }

	const uint8_t *data = event->data.can_frame.data;

	// TODO: Should parse this from DBC or similar and allow user customization
    switch (event->data.can_frame.id) {
        case 0x224: // Brake (range 0-512)
            // Take bytes 4 and 5
            uint16_t brake_value = (data[4] << 8) | data[5];
            
            // Divide by 2 to get 0-256
            brake_value >>= 1;
            if (brake_value > 255) brake_value = 255;

            current_surface.left_trigger = brake_value;
            break;
        
        
        case 0x361: // Gas (range 0-200)
            // Take byte 2
            uint8_t gas_value = data[2];

            // Multiply by 8 to make it easier to press
            gas_value <<= 3;

            current_surface.right_trigger = gas_value;
            break;
        
        
        case 0x025: // Steering (range -2048-2047)
            // Take the bottom 12 bits from bytes 0 and 1
            int16_t steer_value = (uint16_t)((data[0] << 12) | (data[1] << 4));

            // Multiply by 120 to make it easier to press
            if (steer_value > 0 && steer_value > (32767/120)) steer_value = 32767;
            else if (steer_value < 0 && steer_value < (-32768/120)) steer_value = -32768;
            else steer_value *= 120;

			// Reverse because my car uses negative=right
			steer_value = -steer_value;

            current_surface.left_x = steer_value;
            break;
        
    }

	// Don't send data to the computer too quickly
	static uint32_t last_ticks = 0;
	if (furi_get_tick() - last_ticks >= furi_ms_to_ticks(50)) {
		last_ticks = furi_get_tick();
        
    	hid_send_report();
	}
}