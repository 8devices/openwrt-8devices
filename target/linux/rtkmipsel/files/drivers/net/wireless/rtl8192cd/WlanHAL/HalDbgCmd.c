/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalDbgCmd.h
	
Abstract:
	Defined HAL Debug Command
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-29 Filen            Create.	
--*/

#include "HalPrecomp.h"

u4Byte GlobalDebugLevel			= 	DBG_WARNING;

#if CFG_HAL_DBG
u8Byte GlobalDebugComponents	= 	\
//									COMP_TRACE				|
//									COMP_DBG				|
//									COMP_INIT				|
//									COMP_OID_QUERY			|
//									COMP_OID_SET			|
//									COMP_RECV				|
//									COMP_SEND				|
//									COMP_IO					|
//									COMP_POWER				|
//									COMP_MLME				|
//									COMP_SCAN				|
//									COMP_SYSTEM			|
//									COMP_SEC				|
//									COMP_AP				|
//									COMP_TURBO				|
//									COMP_QOS				|
//									COMP_AUTHENTICATOR	|
//									COMP_BEACON			|
//									COMP_ANTENNA			|
//									COMP_RATE				|
//									COMP_EVENTS			|
//									COMP_FPGA				|
//									COMP_RM				|
//									COMP_MP				|
//									COMP_RXDESC			|
//									COMP_CKIP				|
//									COMP_DIG				|
//									COMP_TXAGC				|
//									COMP_HIPWR				|
//									COMP_HALDM				|
//									COMP_RSNA				|
//									COMP_INDIC				|
//									COMP_LED				|
//									COMP_RF					|
//									COMP_DUALMACSWITCH	|
//									COMP_EASY_CONCURRENT	|

//1!!!!!!!!!!!!!!!!!!!!!!!!!!!
//1//1Attention Please!!!<11n or 8190 specific code should be put below this line>
//1!!!!!!!!!!!!!!!!!!!!!!!!!!!

//									COMP_HT				|
//									COMP_POWER_TRACKING 	|
//									COMP_RX_REORDER		|
//									COMP_AMSDU 			|
//									COMP_WPS				|
//									COMP_RATR				|
//									COMP_RESET 				|
//									COMP_CMD				|
//									COMP_EFUSE				|
//									COMP_MESH_INTERWORKING |
//									COMP_CCX				|	
//									COMP_IOCTL				|
//									COMP_GP 				|
//									COMP_TXAGG				|
//									COMP_BB_POWERSAVING	|
//									COMP_SWAS				|
//									COMP_P2P				|
//									COMP_MUX				|
//									COMP_FUNC				|
//									COMP_TDLS				|
//									COMP_OMNIPEEK			|
//									COMP_MULTICHANNEL		|
//									COMP_PSD				|
//									COMP_RATE_ADAPTIVE		|
									0;


#else
u8Byte GlobalDebugComponents	= 0;
#endif



