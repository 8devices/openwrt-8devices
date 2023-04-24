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

#if (BEAMFORMING_SUPPORT == 1)

VOID
SetBeamformRfMode8197F(
	struct rtl8192cd_priv *priv,
	PRT_BEAMFORMING_INFO 	pBeamformingInfo,
	u1Byte					idx
	)
{
	u1Byte					i, Nr_index = 0;
	BOOLEAN					bSelfBeamformer = FALSE;
	RT_BEAMFORMING_ENTRY	BeamformeeEntry;
	BEAMFORMING_CAP		BeamformCap = BEAMFORMING_CAP_NONE;

	if (idx < BEAMFORMEE_ENTRY_NUM)
		BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[idx];
	else
		return;

	BeamformCap = Beamforming_GetBeamCap(priv, pBeamformingInfo);
	pBeamformingInfo->BeamformCap = BeamformCap;
	bSelfBeamformer = BeamformeeEntry.BeamformEntryCap & (BEAMFORMER_CAP_HT_EXPLICIT);

	if(get_rf_mimo_mode(priv) == MIMO_1T1R)
		return;

	PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_WE_LUT, 0x80000,0x1);	/*RF Mode table write enable*/
	PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_WE_LUT, 0x80000,0x1);	/*RF Mode table write enable*/
	
	if (bSelfBeamformer) { 
		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s set TX IQ gen\n", __FUNCTION__));
		/*Paath_A*/
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_RCK_OS, 0xfffff, 0x08000);	/*Select Standby mode	0x30=0x08000*/
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G1, 0xfffff, 0x0005f);	/*Set Table data*/
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G2, 0xfffff, 0x01042);	/*Enable TXIQGEN in standby mode*/
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_RCK_OS, 0xfffff, 0x18000);	/*Select RX mode	0x30=0x18000*/
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G1, 0xfffff, 0x0004f);	/*Set Table data*/
		PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_TXPA_G2, 0xfffff, 0x71fc2);	/*Enable TXIQGEN in RX mode*/
		/*Path_B*/
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_RCK_OS, 0xfffff, 0x08000);
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_TXPA_G1, 0xfffff, 0x00050);
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_TXPA_G2, 0xfffff, 0x01042);
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_RCK_OS, 0xfffff, 0x18000);
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_TXPA_G1, 0xfffff, 0x00040);
		PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_TXPA_G2, 0xfffff, 0x71fc2);
	}
	
	PHY_SetRFReg(priv, ODM_RF_PATH_A, RF_WE_LUT, 0x80000,0x0);	/*RF Mode table write disable*/
	PHY_SetRFReg(priv, ODM_RF_PATH_B, RF_WE_LUT, 0x80000,0x0);	/*RF Mode table write disable*/

	if (bSelfBeamformer)
		PHY_SetBBReg(priv, 0x90c, 0xffffffff, 0x83321333);
}



VOID
SetBeamformEnter8197F(
	struct rtl8192cd_priv *priv,
	u1Byte				BFerBFeeIdx
	)
{
	u1Byte					i = 0;
	u1Byte					BFerIdx = (BFerBFeeIdx & 0xF0)>>4;
	u1Byte					BFeeIdx = (BFerBFeeIdx & 0xF);
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformeeEntry;
	RT_BEAMFORMER_ENTRY	BeamformerEntry;
	BEAMFORMING_CAP		BeamformCap = BEAMFORMING_CAP_NONE;
	u2Byte					STAid = 0;
	u1Byte					Nc_index = 0,Nr_index = 0, grouping = 0, codebookinfo = 0, coefficientsize = 0;

	BeamformCap = Beamforming_GetBeamCap(priv, pBeamformingInfo);

	pBeamformingInfo->BeamformCap = BeamformCap;

	RTL_W8( REG_SND_PTCL_CTRL+1, 0x02);	/*same offset with 8814A*/
	RTL_W8( REG_SND_PTCL_CTRL+2, 0x02);

	if ((pBeamformingInfo->BeamformCap & BEAMFORMEE_CAP) && (BFerIdx < BEAMFORMER_ENTRY_NUM)) {
		BeamformerEntry = pBeamformingInfo->BeamformerEntry[BFerIdx];

		RTL_W8( REG_SND_PTCL_CTRL, 0xDB);	

		/*MAC addresss/Partial AID of Beamformer*/
		if (BFerIdx == 0) {
			for (i = 0; i < 6 ; i++)
				RTL_W8((REG_ASSOCIATED_BFMER0_INFO+i), BeamformerEntry.MacAddr[i]);

			RTL_W16(REG_ASSOCIATED_BFMER0_INFO+6, BeamformerEntry.P_AID);
		} else {
			for (i = 0; i < 6 ; i++)
				RTL_W8((REG_ASSOCIATED_BFMER1_INFO+i), BeamformerEntry.MacAddr[i]);

			RTL_W16(REG_ASSOCIATED_BFMER1_INFO+6, BeamformerEntry.P_AID);
		}

		/*CSI report parameters of Beamformer*/
		
		if (BFerIdx == 0)
			RTL_W16(REG_TX_CSI_RPT_PARAM_BW20, 0x0309);
		else
			RTL_W16(REG_TX_CSI_RPT_PARAM_BW20+2, 0x0309);

		RTL_W8( REG_SND_PTCL_CTRL+3, 0x50);		/*ndp_rx_standby_timer*/
	}

	if ((pBeamformingInfo->BeamformCap & BEAMFORMER_CAP) && (BFeeIdx < BEAMFORMEE_ENTRY_NUM)) {

		BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[BFeeIdx];
		SetBeamformRfMode8197F(priv, pBeamformingInfo, BFeeIdx);
		
		if(OPMODE & WIFI_ADHOC_STATE)
			STAid = BeamformeeEntry.AID;
		else 
			STAid = BeamformeeEntry.P_AID;

		/*P_AID of Beamformee & enable NDPA transmission*/
		if (BFeeIdx == 0) {
			RTL_W16( REG_TXBF_CTRL, STAid);	
			RTL_W8( REG_TXBF_CTRL+3, RTL_R8( REG_TXBF_CTRL+3)|BIT6|BIT7|BIT4);
		} else
			RTL_W16( REG_TXBF_CTRL+2, STAid |BIT14| BIT15|BIT12);

		/*CSI report parameters of Beamformee*/
		if (BFeeIdx == 0) {
			/*Get BIT24 & BIT25*/
			u1Byte	tmp = RTL_R8( REG_ASSOCIATED_BFMEE_SEL+3) & 0x3;
			RTL_W8( REG_ASSOCIATED_BFMEE_SEL+3, tmp | 0x60);
			RTL_W16( REG_ASSOCIATED_BFMEE_SEL, STAid);
		} else {
			/*Set BIT25*/
			RTL_W16( REG_ASSOCIATED_BFMEE_SEL+2, STAid | 0xE200);
		}
		RTL_W16( REG_TXBF_CTRL, RTL_R16(REG_TXBF_CTRL)|BIT15);	/*disable NDP/NDPA packet use beamforming */
		
		Beamforming_Notify(priv);
	}

}


VOID
SetBeamformLeave8197F(
	struct rtl8192cd_priv *priv,
	u1Byte				Idx
	)
{
	PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformeeEntry;
	RT_BEAMFORMER_ENTRY	BeamformerEntry;

	if (Idx < BEAMFORMER_ENTRY_NUM) {
		BeamformeeEntry = pBeamformingInfo->BeamformeeEntry[Idx];
		BeamformerEntry = pBeamformingInfo->BeamformerEntry[Idx];
	} else
		return;
	
	/*	Clear P_AID of Beamformee
	* 	Clear MAC addresss of Beamformer
	*	Clear Associated Bfmee Sel
	*/
	if (BeamformeeEntry.BeamformEntryCap == BEAMFORMING_CAP_NONE) {
		if(Idx == 0) {
			RTL_W16( REG_TXBF_CTRL, 0|BIT15);
			RTL_W16( REG_ASSOCIATED_BFMEE_SEL, 0);
		} else {
			RTL_W16(REG_TXBF_CTRL+2, RTL_R16( REG_TXBF_CTRL+2) & 0xF000);
			RTL_W16(REG_ASSOCIATED_BFMEE_SEL+2,	RTL_R16( REG_ASSOCIATED_BFMEE_SEL+2) & 0x60);
		}	
	}
	
	if (BeamformerEntry.BeamformEntryCap == BEAMFORMING_CAP_NONE) {
		if(Idx == 0) {
			RTL_W32( REG_ASSOCIATED_BFMER0_INFO, 0);
			RTL_W16( REG_ASSOCIATED_BFMER0_INFO+4, 0);
			RTL_W16( REG_TX_CSI_RPT_PARAM_BW20, 0);
		} else {
			RTL_W32( REG_ASSOCIATED_BFMER1_INFO, 0);
			RTL_W16( REG_ASSOCIATED_BFMER1_INFO+4, 0);
			RTL_W16( REG_TX_CSI_RPT_PARAM_BW20+2, 0);
		}	
	}

	if (((pBeamformingInfo->BeamformerEntry[0]).BeamformEntryCap == BEAMFORMING_CAP_NONE)
		&& ((pBeamformingInfo->BeamformerEntry[1]).BeamformEntryCap == BEAMFORMING_CAP_NONE))
			RTL_W8( REG_SND_PTCL_CTRL, 0xD8);
}



VOID
SetBeamformStatus8197F(
	struct rtl8192cd_priv *priv,
	 u1Byte				Idx
	)
{
	u2Byte					BeamCtrlVal;
	u4Byte					BeamCtrlReg;
	PRT_BEAMFORMING_INFO pBeamformingInfo = &(priv->pshare->BeamformingInfo);
	RT_BEAMFORMING_ENTRY	BeamformEntry;

	if (Idx < BEAMFORMEE_ENTRY_NUM)
		BeamformEntry = pBeamformingInfo->BeamformeeEntry[Idx];
	else
		return;

	if (OPMODE & WIFI_ADHOC_STATE)
		BeamCtrlVal = BeamformEntry.MacId;
	else 
		BeamCtrlVal = BeamformEntry.P_AID;

	if (Idx == 0)
		BeamCtrlReg = REG_TXBF_CTRL;
	else {
		BeamCtrlReg = REG_TXBF_CTRL + 2;
		BeamCtrlVal |= BIT12 | BIT14 | BIT15;
	}

	if((BeamformEntry.BeamformEntryState == BEAMFORMING_ENTRY_STATE_PROGRESSED) && (priv->pshare->rf_ft_var.applyVmatrix)) {
		if (BeamformEntry.BW == HT_CHANNEL_WIDTH_20)
			BeamCtrlVal |= BIT9;
		else if (BeamformEntry.BW == HT_CHANNEL_WIDTH_20_40)
			BeamCtrlVal |= (BIT9 | BIT10);
		else if (BeamformEntry.BW == HT_CHANNEL_WIDTH_80)
			BeamCtrlVal |= (BIT9 | BIT10 | BIT11);
	} else 
		BeamCtrlVal &= ~(BIT9|BIT10|BIT11|BIT8|BIT7|BIT6|BIT5|BIT4|BIT3|BIT2|BIT1|BIT0);
	
	RTL_W16(BeamCtrlReg, BeamCtrlVal);
	RTL_W16( REG_TXBF_CTRL, RTL_R16(REG_TXBF_CTRL)|BIT15);	/*disable NDP/NDPA packet use beamforming */

	ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, 
		("%s Idx %d BeamCtrlReg %x BeamCtrlVal %x, bw=%d\n", __FUNCTION__, Idx, BeamCtrlReg, BeamCtrlVal, BeamformEntry.BW));

}


VOID Beamforming_NDPARate_8197F(
	struct rtl8192cd_priv *priv,
	BOOLEAN		Mode,
	u1Byte		BW,
	u1Byte		Rate
	)
{
	u2Byte	NDPARate = Rate;

	if (NDPARate == 0) {
		if(priv->pshare->rssi_min > 30) // link RSSI > 30%
			NDPARate = 0x8;				//MGN_24M
		else
			NDPARate = 0x4;				//MGN_6M
	}

	if(NDPARate < ODM_MGN_MCS0)
		BW = HT_CHANNEL_WIDTH_20;	

	RTL_W8(REG_NDPA_OPT_CTRL, BW & 0x3);
	RTL_W8(REG_NDPA_RATE, NDPARate);

}

VOID
C2HTxBeamformingHandler_8197F(
	struct rtl8192cd_priv *priv,
		pu1Byte			CmdBuf,
		u1Byte			CmdLen
)
{
	u1Byte 	status = CmdBuf[0] & BIT0;
	Beamforming_End(priv, status);
}

#endif


