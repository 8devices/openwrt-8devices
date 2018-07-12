#ifndef __HAL8192EE_DEF_H__
#define __HAL8192EE_DEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8192EEDef.h
	
Abstract:
	Defined HAL 8192EE data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

/*
RT_STATUS
InitPON8192EE(
    IN  HAL_PADAPTER Adapter,
    IN  u4Byte   	ClkSel        
);

RT_STATUS
StopHW8192EE(
    IN  HAL_PADAPTER Adapter
);


RT_STATUS	
hal_Associate_8192EE(
    HAL_PADAPTER            Adapter,
    BOOLEAN			    IsDefaultAdapter
);

*/

struct _RT_BEAMFORMING_INFO;

VOID
SetBeamformRfMode92E(
	struct rtl8192cd_priv *priv,
	struct _RT_BEAMFORMING_INFO *pBeamformingInfo
);



VOID
SetBeamformEnter92E(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
);

VOID
SetBeamformLeave92E(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
);

VOID
SetBeamformStatus92E(
	struct rtl8192cd_priv *priv,
	 u1Byte				Idx
);


VOID 
Beamforming_NDPARate_92E(
	struct rtl8192cd_priv *priv,
	BOOLEAN		Mode,
	u1Byte		BW,
	u1Byte		Rate
);

VOID
C2HTxBeamformingHandler_92E(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
);

VOID
_C2HContentParsing92E(
	struct rtl8192cd_priv *priv,
		u1Byte			c2hCmdId, 
		u1Byte			c2hCmdLen,
		pu1Byte 			tmpBuf
);

VOID
C2HPacketHandler_92E(
	struct rtl8192cd_priv *priv,
		pu1Byte			Buffer,
		u1Byte			Length
);

VOID HW_VAR_HW_REG_TIMER_START_92E(
	struct rtl8192cd_priv *priv
);

VOID HW_VAR_HW_REG_TIMER_INIT_92E(
	struct rtl8192cd_priv *priv,
	u2Byte t
);

VOID HW_VAR_HW_REG_TIMER_STOP_92E(
	struct rtl8192cd_priv *priv
);

RT_STATUS	
hal_Associate_8192ES(
	struct rtl8192cd_priv *priv,
	BOOLEAN IsDefaultAdapter
);

#endif  //__HAL8192EE_DEF_H__

