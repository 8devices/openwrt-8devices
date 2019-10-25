/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal88XXGen.c
	
Abstract:
	Defined RTL8192EE HAL Function
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

#include "../../../HalPrecomp.h"
#include "../../../../8192cd.h"

typedef enum _RTL8881A_C2H_EVT
{
	C2H_8881A_DBG = 0,
	C2H_8881A_LB = 1,
	C2H_8881A_TXBF = 2,
	C2H_8881A_TX_REPORT = 3,
	C2H_8881A_TX_RATE =4,
	C2H_8881A_BT_INFO = 9,
	C2H_8881A_BT_MP = 11,
	C2H_8881A_RA_RPT = 12,
#ifdef TXRETRY_CNT
	C2H_8881A_TX_RETRY = 13, //0x0D
#endif
	C2H_8881A_RA_PARA_RPT = 14,
	C2H_8881A_DEBUG_CODE = 0xFE,
	C2H_8881A_EXTEND_IND = 0xFF,
	MAX_8881A_C2HEVENT
}RTL8881A_C2H_EVT;

typedef enum _RTL8881A_EXTEND_C2H_EVT
{
	EXTEND_C2H_8881A_DBG_PRINT = 0

}RTL8881A_EXTEND_C2H_EVT;

VOID
C2HTxTxReportHandler_8881A(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
)
{
	struct tx_rpt rpt1;
	int k=0, j=0;
	for(j=0; j<2; j++) {
		rpt1.macid= CmdBuf[k];
		rpt1.txok = CmdBuf[k+1] | ((short)CmdBuf[k+2]<<8);
		rpt1.txfail = CmdBuf[k+3] | ((short)CmdBuf[k+4]<<8);
		rpt1.initil_tx_rate = CmdBuf[k+5];
		if(rpt1.macid != 0xff)
			txrpt_handler(priv, &rpt1);
		k+=6;
	}
}


VOID
_C2HContentParsing8881A(
	struct rtl8192cd_priv *priv,
		u1Byte			c2hCmdId, 
		u1Byte			c2hCmdLen,
		pu1Byte 			tmpBuf
)
{
	switch(c2hCmdId)
	{
		case C2H_8881A_TX_RATE:
#ifdef TXREPORT	
			C2HTxTxReportHandler_8881A(priv, tmpBuf, c2hCmdLen);

#ifdef TXRETRY_CNT
			requestTxRetry88XX(priv);
#else
			requestTxReport88XX(priv);
#endif			
#endif		
			break;
		
#ifdef TXRETRY_CNT
		case C2H_8881A_TX_RETRY:
			C2HTxTxRetryHandler(priv, tmpBuf);
			requestTxReport88XX(priv);	
			break;
#endif

		default:
			if(!(phydm_c2H_content_parsing(ODMPTR, c2hCmdId, c2hCmdLen, tmpBuf))) {
				printk("[C2H], Unkown packet!! CmdId(%#X)!\n", c2hCmdId);
			}
			break;
	}
}



VOID
C2HPacketHandler_8881A(
	struct rtl8192cd_priv *priv,
		pu1Byte			Buffer,
		u1Byte			Length
	)
{
	u1Byte	c2hCmdId=0, c2hCmdSeq=0, c2hCmdLen=0;
	pu1Byte tmpBuf=NULL;
	c2hCmdId = Buffer[0];
	c2hCmdSeq = Buffer[1];
	c2hCmdLen = Length -2;
	tmpBuf = Buffer+2;
	_C2HContentParsing8881A(priv, c2hCmdId, c2hCmdLen, tmpBuf);
}


#if (BEAMFORMING_SUPPORT == 1)
VOID
SetBeamformEnter8881A(
	struct rtl8192cd_priv *priv,
	u1Byte				BFerBFeeIdx
	)
{
	u1Byte					i = 0;
	u1Byte					BFerIdx = (BFerBFeeIdx & 0xF0)>>4;
	u4Byte					CSI_Param = 0;
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMER_ENTRY	BeamformerEntry;

	pBeamformingInfo->BeamformCap = Beamforming_GetBeamCap(priv, pBeamformingInfo);

	RTL_W32( 0x9b4, 0x01081008);

	//I am Bfee
	if ((pBeamformingInfo->BeamformCap & BEAMFORMEE_CAP) && (BFerIdx < BEAMFORMER_ENTRY_NUM)) {
		BeamformerEntry = pBeamformingInfo->BeamformerEntry[BFerIdx];
	
		RTL_W8( REG_SND_PTCL_CTRL, 0xCB);	

		// MAC addresss/Partial AID of Beamformer
		if (BFerIdx == 0) {
			for (i = 0; i < 6 ; i++)
				RTL_W8( (REG_ASSOCIATED_BFMER0_INFO+i), BeamformerEntry.MacAddr[i]);
		} else {
			for (i = 0; i < 6 ; i++)
				RTL_W8( (REG_ASSOCIATED_BFMER1_INFO+i), BeamformerEntry.MacAddr[i]);
		}

		// CSI report parameters of Beamformer
		if(BeamformerEntry.BeamformEntryCap & BEAMFORMEE_CAP_VHT_SU)
			CSI_Param = 0x01080108;	
		else 
			CSI_Param = 0x03080308;	

		RTL_W32(REG_TX_CSI_RPT_PARAM_BW20, CSI_Param);
		RTL_W32(REG_TX_CSI_RPT_PARAM_BW40, CSI_Param);
		RTL_W32(REG_TX_CSI_RPT_PARAM_BW80, CSI_Param);

		// Timeout value for MAC to leave NDP_RX_standby_state 60 us
		RTL_W8( REG_SND_PTCL_CTRL+3, 0x50);				//ndp_rx_standby_timer, 8814 need > 0x56, suggest from Dvaid
	}
	
}


VOID
SetBeamformLeave8881A(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
	)
{
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMER_ENTRY	BeamformerEntry;

	if (Idx < BEAMFORMER_ENTRY_NUM) {
		BeamformerEntry = pBeamformingInfo->BeamformerEntry[Idx];
	} else
		return;
	
	/*	Clear P_AID of Beamformee
	* 	Clear MAC addresss of Beamformer
	*	Clear Associated Bfmee Sel
	*/

	if (BeamformerEntry.BeamformEntryCap == BEAMFORMING_CAP_NONE) {
		if(Idx == 0) {
			RTL_W32( REG_ASSOCIATED_BFMER0_INFO, 0);
			RTL_W16( REG_ASSOCIATED_BFMER0_INFO+4, 0);
			RTL_W16( REG_TX_CSI_RPT_PARAM_BW20, 0);
			RTL_W16( REG_TX_CSI_RPT_PARAM_BW40, 0);
			RTL_W16( REG_TX_CSI_RPT_PARAM_BW80, 0);
		} else {
			RTL_W32( REG_ASSOCIATED_BFMER1_INFO, 0);
			RTL_W16( REG_ASSOCIATED_BFMER1_INFO+4, 0);
			RTL_W16( REG_TX_CSI_RPT_PARAM_BW20+2, 0);
			RTL_W16( REG_TX_CSI_RPT_PARAM_BW40+2, 0);
			RTL_W16( REG_TX_CSI_RPT_PARAM_BW80+2, 0);
		}	
	}

	if (((pBeamformingInfo->BeamformerEntry[0]).BeamformEntryCap == BEAMFORMING_CAP_NONE)
		&& ((pBeamformingInfo->BeamformerEntry[1]).BeamformEntryCap == BEAMFORMING_CAP_NONE))
			RTL_W8( REG_SND_PTCL_CTRL, 0xC8);

}

#endif 


