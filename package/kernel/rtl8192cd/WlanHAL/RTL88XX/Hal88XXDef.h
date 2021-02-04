#ifndef __HAL88XX_DEF_H__
#define __HAL88XX_DEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXDef.h
	
Abstract:
	Defined HAL 88XX common data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

#ifdef  WLAN_HAL_INTERNAL_USED

MIMO_TR_STATUS
GetChipIDMIMO88XX(
    IN  HAL_PADAPTER        Adapter
);


VOID
CAMEmptyEntry88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u1Byte          index
);


u4Byte
CAMFindUsable88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          for_begin
);


VOID
CAMReadMACConfig88XX
(
    IN  HAL_PADAPTER    Adapter,
    IN  u1Byte          index, 
    OUT pu1Byte         pMacad,
    OUT PCAM_ENTRY_CFG  pCfg
);


VOID
CAMProgramEntry88XX(
    IN	HAL_PADAPTER		Adapter,
    IN  u1Byte              index,
    IN  pu1Byte             macad,
    IN  pu1Byte             key128,
    IN  u2Byte              config
);


VOID
SetHwReg88XX(
    IN	HAL_PADAPTER		Adapter,
    IN	u1Byte				variable,
    IN	pu1Byte				val
);


VOID
GetHwReg88XX(
    IN      HAL_PADAPTER    Adapter,
    IN      u1Byte          variable,
    OUT     pu1Byte         val
);

RT_STATUS
GetMACIDQueueInTXPKTBUF88XX(
    IN      HAL_PADAPTER          Adapter,
    OUT     pu1Byte               MACIDList
);


RT_STATUS
SetMACIDSleep88XX(
    IN  HAL_PADAPTER Adapter,
    IN  BOOLEAN      bSleep,   
    IN  u4Byte       aid
);

#if (IS_RTL8881A_SERIES || IS_RTL8192E_SERIES)
RT_STATUS
InitLLT_Table88XX(
    IN  HAL_PADAPTER    Adapter
);
#endif //#if (IS_RTL8881A_SERIES || IS_RTL8192E_SERIES)

#if (IS_RTL8814A_SERIES || IS_RTL8197F_SERIES || IS_RTL8822B_SERIES)
RT_STATUS
InitLLT_Table88XX_V1(
    IN  HAL_PADAPTER    Adapter
);
#endif //#if (IS_RTL8814A_SERIES || IS_RTL8197F_SERIES || IS_RTL8822B_SERIES)

RT_STATUS
InitPON88XX(
    IN  HAL_PADAPTER Adapter
);

RT_STATUS
InitMAC88XX(
    IN  HAL_PADAPTER Adapter
);

VOID
InitIMR88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  RT_OP_MODE      OPMode
);

VOID
InitVAPIMR88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          VapSeq
);


RT_STATUS      
InitHCIDMAMem88XX(
    IN      HAL_PADAPTER    Adapter
);  

RT_STATUS
InitHCIDMAReg88XX(
    IN      HAL_PADAPTER    Adapter
);  

VOID
StopHCIDMASW88XX(
    IN  HAL_PADAPTER Adapter
);

VOID
StopHCIDMAHW88XX(
    IN  HAL_PADAPTER Adapter
);

#if CFG_HAL_SUPPORT_MBSSID
VOID
InitMBSSID88XX(
    IN  HAL_PADAPTER Adapter
);

VOID
InitMBIDCAM88XX(
    IN  HAL_PADAPTER Adapter
);

VOID
StopMBSSID88XX(
    IN  HAL_PADAPTER Adapter
);
#endif  //CFG_HAL_SUPPORT_MBSSID

RT_STATUS
SetMBIDCAM88XX(
    IN  HAL_PADAPTER Adapter,
    IN  u1Byte       MBID_Addr,    
    IN  u1Byte       IsRoot
);

RT_STATUS
StopMBIDCAM88XX(
    IN  HAL_PADAPTER Adapter,
    IN  u1Byte       MBID_Addr
);

RT_STATUS
ResetHWForSurprise88XX(
    IN  HAL_PADAPTER Adapter
);

#if CFG_HAL_MULTI_MAC_CLONE
VOID
McloneSetMBSSID88XX(
    IN  HAL_PADAPTER Adapter,
    IN	pu1Byte 	 macAddr,
    IN	int          entIdx
);

VOID
McloneStopMBSSID88XX(
    IN  HAL_PADAPTER Adapter,
    IN	int          entIdx
);
#endif // #if CFG_HAL_MULTI_MAC_CLONE

RT_STATUS
StopHW88XX(
    IN  HAL_PADAPTER Adapter
);

RT_STATUS
StopSW88XX(
    IN  HAL_PADAPTER Adapter
);

VOID
DisableVXDAP88XX(
    IN  HAL_PADAPTER Adapter
);

VOID
Timer1Sec88XX(
    IN  HAL_PADAPTER Adapter
);

RT_STATUS 
GetTxRPTBuf88XX(
    IN	HAL_PADAPTER        Adapter,
    IN	u4Byte              macID,
    IN  u1Byte              variable,   
    IN 	u1Byte				byteoffset,
    OUT pu1Byte             val
);

RT_STATUS 
SetTxRPTBuf88XX(
    IN	HAL_PADAPTER        Adapter,
    IN	u4Byte              macID,
    IN  u1Byte              variable,
    IN  pu1Byte             val    
);

u4Byte
CheckHang88XX(
    IN	HAL_PADAPTER        Adapter
);

VOID
SetCRC5ToRPTBuffer88XX(
    IN	HAL_PADAPTER        Adapter,
    IN	u1Byte              val,
    IN	u4Byte              macID,
    IN  u1Byte              bValid
);

VOID
SetCRC5ValidBit88XX(
    IN	HAL_PADAPTER        Adapter,
    IN	u1Byte              group,
    IN  u1Byte              bValid
    
);

VOID
SetCRC5EndBit88XX(
    IN	HAL_PADAPTER        Adapter,
    IN	u1Byte              group,
    IN  u1Byte              bEnd    
);

VOID
InitMACIDSearch88XX(
    IN	HAL_PADAPTER        Adapter    
);


RT_STATUS
CheckHWMACIDResult88XX(
    IN	HAL_PADAPTER        Adapter,    
    IN  u4Byte              MacID,
    OUT pu1Byte             result
);


RT_STATUS 
InitDDMA88XX(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte	source,
    IN  u4Byte	dest,
    IN  u4Byte 	length
);

#endif  //WLAN_HAL_INTERNAL_USED

#endif  //__HAL88XX_DEF_H__
