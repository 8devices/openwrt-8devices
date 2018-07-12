#ifndef __WLAN_QOSTYPE_H__
#define __WLAN_QOSTYPE_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Wlan_QoSType.h
	
Abstract:
	Defined QoS Self-defined Wlan Marco & Enumeration & Define & ....
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-04-09 Filen            Create.	
--*/

#ifdef  CONFIG_WLAN_HAL
#define AC0_BE	0		// ACI: 0x00	// Best Effort
#define AC1_BK	1		// ACI: 0x01	// Background
#define AC2_VI	2		// ACI: 0x10	// Video
#define AC3_VO	3		// ACI: 0x11	// Voice
#define AC_MAX	4		// Max: define total number; Should not to be used as a real enum.

	
#define AC_PARAM_SIZE	4









#endif  //CONFIG_WLAN_HAL
#endif // __WLAN_QOSTYPE_H__

