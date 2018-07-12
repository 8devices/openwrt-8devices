#ifndef __HAL8814AE_DEF_H__
#define __HAL8814AE_DEF_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8814AEDef.h
	
Abstract:
	Defined HAL 8814AE data structure & Define
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2013-05-28 Filen            Create.	
--*/


/*RT_STATUS
InitPON8814AE(
    IN  HAL_PADAPTER    Adapter,
    IN  u4Byte          ClkSel        
);

RT_STATUS
StopHW8814AE(
    IN  HAL_PADAPTER    Adapter
);


RT_STATUS
ResetHWForSurprise8814AE(
    IN  HAL_PADAPTER Adapter
);
*/

RT_STATUS	
hal_Associate_8814AE(
	struct rtl8192cd_priv *priv,
	BOOLEAN             IsDefaultAdapter
);

VOID
C2HPacketHandler_8814A(
	struct rtl8192cd_priv *priv,
		pu1Byte			Buffer,
		u1Byte			Length
);


#if (BEAMFORMING_SUPPORT == 1)

struct _RT_BEAMFORMING_INFO;

BOOLEAN
beamforming_setiqgen_8814a (
	struct rtl8192cd_priv *priv
	);

u1Byte
halTxbf8814A_GetNtx(
	struct rtl8192cd_priv *priv
);

VOID
SetBeamformRfMode8814A(
	struct rtl8192cd_priv *priv,
	PRT_BEAMFORMING_INFO 	pBeamformingInfo,
	u1Byte			idx
);

VOID
SetBeamformEnter8814A(
	struct rtl8192cd_priv *priv,
	u1Byte	BFerBFeeIdx
);

VOID
SetBeamformLeave8814A(
	struct rtl8192cd_priv *priv,
	u1Byte	Idx
);

VOID
SetBeamformStatus8814A(
	struct rtl8192cd_priv *priv,
	 u1Byte	Idx
);

VOID
C2HTxBeamformingHandler_8814A(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
);

VOID
Beamforming_NDPARate_8814A(
	struct rtl8192cd_priv *priv,
	BOOLEAN		Mode,
	u1Byte		BW,
	u1Byte		Rate
);

VOID
_C2HContentParsing8814A(
	struct rtl8192cd_priv *priv,
		u1Byte			c2hCmdId, 
		u1Byte			c2hCmdLen,
		pu1Byte 			tmpBuf
);


VOID HW_VAR_HW_REG_TIMER_START_8814A(struct rtl8192cd_priv *priv);

VOID HW_VAR_HW_REG_TIMER_INIT_8814A(struct rtl8192cd_priv *priv, int t);

VOID HW_VAR_HW_REG_TIMER_STOP_8814A(struct rtl8192cd_priv *priv);

#endif 


#endif  //__HAL8814AE_DEF_H__

