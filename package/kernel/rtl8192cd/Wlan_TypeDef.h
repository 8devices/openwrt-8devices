#ifndef __WLAN_TYPEDEF_H__
#define __WLAN_TYPEDEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Wlan_TypeDef.h
	
Abstract:
	Defined Self-defined Wlan Marco & Enumeration & Define & ....
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-04-09 Filen            Create.	
--*/

typedef enum _MIMO_TR_STATUS {
	MIMO_1T2R = 1,
	MIMO_2T4R = 2,
	MIMO_2T2R = 3,	
	MIMO_1T1R = 4,
	MIMO_3T3R = 5,
	MIMO_4T4R = 6,
	MIMO_2T3R = 7,
	MIMO_3T4R = 8,
} MIMO_TR_STATUS;


#ifdef  CONFIG_WLAN_HAL
typedef enum _RT_OP_MODE{
	RT_OP_MODE_AP,
	RT_OP_MODE_INFRASTRUCTURE,
	RT_OP_MODE_IBSS,
	RT_OP_MODE_NO_LINK,
}RT_OP_MODE, *PRT_OP_MODE;

//Enumeration below is mapping to HW Operation
typedef enum _SECURITY_CONFIG_OPERATION_{
    SCO_TXUSEDK     = BIT(0),
    SCO_RXUSEDK     = BIT(1),   
    SCO_TXENC       = BIT(2),
    SCO_RXDEC       = BIT(3),  
    SCO_SKBYA2      = BIT(4),
    SCO_NOSKMC      = BIT(5),
    SCO_TXBCUSEDK   = BIT(6),
    SCO_RXBCUSEDK   = BIT(7),
    SCO_CHK_KEYID   = BIT(8)
}SECURITY_CONFIG_OPERATION, *PSECURITY_CONFIG_OPERATION;

#if 0
typedef enum _WIRELESS_MODE {
	WIRELESS_MODE_UNKNOWN   = 0x00,
	WIRELESS_MODE_A         = 0x01,
	WIRELESS_MODE_B         = 0x02,
	WIRELESS_MODE_G         = 0x04,
	WIRELESS_MODE_AUTO      = 0x08,
	WIRELESS_MODE_N_24G     = 0x10,
	WIRELESS_MODE_N_5G      = 0x20,
	WIRELESS_MODE_AC_5G     = 0x40
} WIRELESS_MODE;
#endif

#define MIMO_PS_STATIC			0
#define MIMO_PS_DYNAMIC			1
#define MIMO_PS_NOLIMIT			3




#endif  //CONFIG_WLAN_HAL
#endif // __WLAN_TYPEDEF_H__

