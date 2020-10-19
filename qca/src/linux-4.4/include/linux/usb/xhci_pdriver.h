/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 */

#ifndef __USB_CORE_XHCI_PDRIVER_H
#define __USB_CORE_XHCI_PDRIVER_H

/**
 * struct usb_xhci_pdata - platform_data for generic xhci platform driver
 *
 * @usb3_lpm_capable:	determines if this xhci platform supports USB3
 *			LPM capability
 *
 */
struct usb_xhci_pdata {
	unsigned	usb3_lpm_capable:1;
	unsigned	usb2_susphy_quirk:1;
	unsigned	usb3_dev_reset_quirk:1;
	struct usb_susphy       *susphy;
};

#endif /* __USB_CORE_XHCI_PDRIVER_H */
