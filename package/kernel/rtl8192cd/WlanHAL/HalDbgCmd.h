#ifndef __HALDBGCMD_H__
#define __HALDBGCMD_H__


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


#define DBG_OFF					0

//
//	Deprecated! Don't use it! 
//	TODO: fix related debug message!
//
//#define DBG_SEC					1

//
//	Fatal bug. 
//	For example, Tx/Rx/IO locked up, OS hangs, memory access violation, 
//	resource allocation failed, unexpected HW behavior, HW BUG and so on.
//
#define DBG_SERIOUS				2

//
//	Abnormal, rare, or unexpeted cases.
//	For example, IRP/Packet/OID canceled, device suprisely unremoved and so on.
//
#define DBG_WARNING				3

//
//	Normal case with useful information about current SW or HW state. 
//	For example, Tx/Rx descriptor to fill, Tx/Rx descriptor completed status, 
//	SW protocol state change, dynamic mechanism state change and so on.
//
#define DBG_LOUD				4

//
//	Normal case with detail execution flow or information.
//
#define DBG_TRACE				5

//-----------------------------------------------------------------------------
// Define the tracing components
//
//-----------------------------------------------------------------------------
#define COMP_TRACE				BIT0		// For function call tracing.
#define COMP_DBG				BIT1		// Only for temporary debug message.
#define COMP_INIT				BIT2		// during driver initialization / halt / reset.
#define COMP_OID_QUERY				BIT3		// Query OID.
#define COMP_OID_SET				BIT4		// Set OID.
#define COMP_RECV				BIT5		// Reveive part data path.
#define COMP_SEND				BIT6		// Send part path.
#define COMP_IO					BIT7		// I/O Related. Added by Annie, 2006-03-02.
#define COMP_POWER				BIT8		// 802.11 Power Save mode or System/Device Power state related.
#define COMP_MLME				BIT9		// 802.11 link related: join/start BSS, leave BSS.
#define COMP_SCAN				BIT10	// For site survey.
#define COMP_SYSTEM				BIT11	// For general platform function.
#define COMP_SEC				BIT12	// For Security.
#define COMP_AP					BIT13	// For AP mode related.
#define COMP_TURBO				BIT14	// For Turbo Mode related. By Annie, 2005-10-21.
#define COMP_QOS				BIT15	// For QoS.
#define COMP_AUTHENTICATOR			BIT16	// For AP mode Authenticator. Added by Annie, 2006-01-30.
#define COMP_BEACON				BIT17	// For Beacon related, by rcnjko.
#define COMP_ANTENNA				BIT18	// For Antenna diversity related, by rcnjko. 
#define COMP_RATE				BIT19	// For Rate Adaptive mechanism, 2006.07.02, by rcnjko. #define COMP_EVENTS				0x00000080	// Event handling
#define COMP_EVENTS				BIT20	// Event handling
#define COMP_FPGA				BIT21	// For FPGA verfication 
#define COMP_RM					BIT22	// For Radio Measurement. 
#define COMP_MP					BIT23	// For mass production test, by shien chang, 2006.07.13
#define COMP_RXDESC				BIT24	// Show Rx desc information for SD3 debug. Added by Annie, 2006-07-15.
#define COMP_CKIP				BIT25	// For CCX 1 S13: CKIP. Added by Annie, 2006-08-14.
#define COMP_DIG				BIT26	// For DIG, 2006.09.25, by rcnjko.
#define COMP_TXAGC				BIT27	// For Tx power, 060928, by rcnjko. 
#define COMP_HIPWR				BIT28	// For High Power Mechanism, 060928, by rcnjko. 
#define COMP_HALDM				BIT29	// For HW Dynamic Mechanism, 061010, by rcnjko. 
#define COMP_RSNA				BIT30	// For RSNA IBSS , 061201, by CCW. 
#define COMP_INDIC				BIT31	// For link indication
#define COMP_LED				BIT32	// For LED.
#define COMP_RF					BIT33	// For RF.
//1!!!!!!!!!!!!!!!!!!!!!!!!!!!
//1//1Attention Please!!!<11n or 8190 specific code should be put below this line>
//1!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define COMP_HT					BIT34	// For 802.11n HT related information. by Emily 2006-8-11
#define COMP_POWER_TRACKING			BIT35	//FOR 8190 TX POWER TRACKING
#define COMP_RX_REORDER				BIT36	// 8190 Rx Reorder
#define COMP_AMSDU				BIT37	// For A-MSDU Debugging
#define COMP_WPS				BIT38   //WPS Debug Message 
#define COMP_RATR				BIT39	
#define COMP_RESET				BIT40
// For debug command to print on dbgview!!
#define COMP_CMD				BIT41
#define COMP_EFUSE				BIT42
#define COMP_MESH_INTERWORKING 			BIT43
#define COMP_CCX				BIT44	//CCX Debug Flag
#define	COMP_IOCTL				BIT45	// IO Control
#define COMP_GP					BIT46	// For generic parser.
#define COMP_TXAGG				BIT47
#define COMP_HVL                            	BIT48     // For Ndis 6.2 Context Swirch and Hardware Virtualiztion Layer
#define COMP_TEST				BIT49
#define COMP_BB_POWERSAVING	BIT50
#define COMP_SWAS				BIT51	// For SW Antenna Switch
#define COMP_P2P				BIT52
#define COMP_MUX				BIT53
#define COMP_FUNC				BIT54
#define COMP_TDLS				BIT55
#define COMP_OMNIPEEK			BIT56
#define COMP_MULTICHANNEL		BIT57
#define COMP_DUALMACSWITCH	BIT60	// 2010/12/27 Add for Dual mac mode debug
#define COMP_EASY_CONCURRENT	BIT61	// 2010/12/27 Add for easy cncurrent mode debug
#define COMP_PSD				BIT63	//2011/3/9 Add for WLAN PSD for BT AFH
#define COMP_TCPCHECKSUM		BIT58
#define COMP_DFS                                BIT62

#define COMP_RATE_ADAPTIVE	BIT57

#define	COMP_ALL				UINT64_C(0xFFFFFFFFFFFFFFFF) // All components

#define	HALDBG					(&(priv->pmib->qc_debug))
#define RT_QC_TRACE(_Comp, _Level, Fmt)												\
			if(((_Comp) & HALDBG->HalDbgCom) && (_Level <= HALDBG->HalDbgLev))      \
			{																    	\
				printk Fmt;										                    \
			}


#if CFG_HAL_DBG
#define RT_TRACE(_Comp, _Level, Fmt)												\
			if(((_Comp) & GlobalDebugComponents) && (_Level <= GlobalDebugLevel))	\
			{																	    \
				HalDbgPrint Fmt;													\
			}

// Print Debug Message with prefix function name.
#define RT_TRACE_F(_Comp, _Level, Fmt)												\
			if(((_Comp) & GlobalDebugComponents) && (_Level <= GlobalDebugLevel))	\
			{																	\
				HalDbgPrint("%s(): ", __FUNCTION__);									\
				HalDbgPrint Fmt;														\
			}

// TODO: Filen, add ASSERT marco code
#define RT_ASSERT(_Exp,Fmt)														\
			if(!(_Exp))															\
			{																	\
				HalDbgPrint("Assertion for exp(%s) at file(%s), line(%d), function[%s]\n", #_Exp, __FILE__, __LINE__, __FUNCTION__);												\
				HalDbgPrint Fmt;														\
				ASSERT(FALSE);													\
			}
			
#define PRINT_DATA(_TitleString, _HexData, _HexDataLen)						\
{												\
	char			*szTitle = _TitleString;					\
	pu1Byte		pbtHexData = _HexData;							\
	u4Byte		u4bHexDataLen = _HexDataLen;						\
	u4Byte		__i;									\
	HalDbgPrint("%s", szTitle);								\
	for (__i=0;__i<u4bHexDataLen;__i++)								\
	{											\
		if ((__i & 15) == 0) 								\
		{										\
			HalDbgPrint("\n");								\
		}										\
		HalDbgPrint("%02X%s", pbtHexData[__i], ( ((__i&3)==3) ? "  " : " ") );			\
	}											\
	HalDbgPrint("\n");										\
}

// RT_PRINT_XXX macros: implemented for debugging purpose.
// Added by Annie, 2005-11-21.
#define RT_PRINT_DATA(_Comp, _Level, _TitleString, _HexData, _HexDataLen)			\
			if(((_Comp) & GlobalDebugComponents) && (_Level <= GlobalDebugLevel))	\
			{									\
				int __i = 0, __j = 0, __k = 54;								\
				pu1Byte	ptr = (pu1Byte)_HexData;				\
				HalDbgPrint("Rtl819x: ");						\
				HalDbgPrint(_TitleString);						\
				for( __i=0; __i<(int)_HexDataLen; __i++ )				\
				{								\
					HalDbgPrint("%02X%s", ptr[__i], (((__i + 1) % 4) == 0)? "  " : " ");	\
					__k -= (3 + ((((__i + 1) % 4) == 0) ? 1 : 0));								\
					if ((((__i + 1) % 16) == 0) || ((__i + 1) == _HexDataLen))	\
					{								\
						if(FALSE) { \
						for( ; __k > 0; __k --)		\
							HalDbgPrint(" ");			\
						__k = 54;					\
						for(__j = ((__i / 16) * 16); __j <= __i; __j ++)	\
							HalDbgPrint("%c", (ptr[__j] < 31 || ptr[__j] > 127) ? '.' : ptr[__j]);		\
						} \
						HalDbgPrint("\n");			\
					}							\
				}								\
			}

#define RT_PRINT_ADDR(_Comp, _Level, _TitleString, _Ptr)					\
			if(((_Comp) & GlobalDebugComponents) && (_Level <= GlobalDebugLevel))	\
			{									\
				int __i;								\
				pu1Byte	ptr = (pu1Byte)_Ptr;					\
				HalDbgPrint("Rtl819x: ");						\
				HalDbgPrint(_TitleString);						\
				HalDbgPrint(" ");							\
				for( __i=0; __i<6; __i++ )						\
					HalDbgPrint("%02X%s", ptr[__i], (__i==5)?"":"-");		\
				HalDbgPrint("\n");							\
			}

#define RT_PRINT_ADDRS(_Comp, _Level, _TitleString, _Ptr, _AddNum)				\
			if(((_Comp) & GlobalDebugComponents) && (_Level <= GlobalDebugLevel))	\
			{									\
				int __i, __j;							\
				pu1Byte	ptr = (pu1Byte)_Ptr;					\
				HalDbgPrint("Rtl819x: ");						\
				HalDbgPrint(_TitleString);						\
				HalDbgPrint("\n");							\
				for( __i=0; __i<(int)_AddNum; __i++ )					\
				{								\
					for( __j=0; __j<6; __j++ )					\
						HalDbgPrint("%02X%s", ptr[__i*6+__j], (__j==5)?"":"-");	\
					HalDbgPrint("\n");						\
				}								\
			}

// Added by Annie, 2005-11-22.
#define	MAX_STR_LEN	64
#define	PRINTABLE(_ch)	(_ch>=' ' &&_ch<='~' )	// I want to see ASCII 33 to 126 only. Otherwise, I print '?'. Annie, 2005-11-22.

#define RT_PRINT_STR(_Comp, _Level, _TitleString, _Ptr, _Len)					\
			if(((_Comp) & GlobalDebugComponents) && (_Level <= GlobalDebugLevel))	\
			{									\
				int		__i;						\
				u1Byte	buffer[MAX_STR_LEN];					\
				int	length = (_Len<MAX_STR_LEN)? _Len : (MAX_STR_LEN-1) ;	\
				PlatformZeroMemory( buffer, MAX_STR_LEN );			\
				PlatformMoveMemory( buffer, (pu1Byte)_Ptr, length );		\
				for( __i=0; __i<MAX_STR_LEN; __i++ )					\
				{								\
					if( !PRINTABLE(buffer[__i]) )	buffer[__i] = '?';	\
				}								\
				buffer[length] = '\0';						\
				HalDbgPrint("Rtl819x: ");						\
				HalDbgPrint(_TitleString);						\
				HalDbgPrint(": %d, <%s>\n", _Len, buffer);				\
			}

#define	RT_PRINT_UUID(_Comp, _Level, _TitleString, _UUID)		\
{		\
	RT_TRACE_F(_Comp, _Level, (" %s ", _TitleString));		\
	RT_TRACE(_Comp, _Level, (" %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X\n",	\
		(_UUID).Data1, (_UUID).Data2, (_UUID).Data3, (_UUID).Data4[0], (_UUID).Data4[1],	\
		(_UUID).Data4[2], (_UUID).Data4[3], (_UUID).Data4[4], (_UUID).Data4[5], (_UUID).Data4[6], (_UUID).Data4[7]));	\
}
			
#else	// of #if DBG
// 2009/06/22 MH Allow fre build to print info test.
#if 1
#define RT_TRACE(_Comp, _Level, Fmt)
#define RT_TRACE_F(_Comp, _Level, Fmt)
#else
#define RT_TRACE(_Comp, _Level, Fmt)												\
			if(((_Comp) & GlobalDebugComponents) && (_Level <= GlobalDebugLevel))	\
			{																	\
				HalDbgPrint Fmt;														\
			}

// Print Debug Message with prefix function name.
#define RT_TRACE_F(_Comp, _Level, Fmt)												\
			if(((_Comp) & GlobalDebugComponents) && (_Level <= GlobalDebugLevel))	\
			{																	\
				HalDbgPrint("%s(): ", __FUNCTION__);									\
				HalDbgPrint Fmt;														\
			}
#endif
#define RT_ASSERT(_Exp,Fmt)
#define PRINT_DATA(_TitleString, _HexData, _HexDataLen)
#define RT_PRINT_DATA(_Comp, _Level, _TitleString, _HexData, _HexDataLen)
#define RT_PRINT_ADDR(_Comp, _Level, _TitleString, _Ptr)
#define RT_PRINT_ADDRS(_Comp, _Level, _TitleString, _Ptr, _AddNum)
#define RT_PRINT_STR(_Comp, _Level, _TitleString, _Ptr, _Len)
#define	RT_PRINT_UUID(_Comp, _Level, _TitleString, _UUID)
#endif	// of #if DBG


extern u4Byte GlobalDebugLevel;
extern u8Byte GlobalDebugComponents;


#endif  //__HALDBGCMD_H__
