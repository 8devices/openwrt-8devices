/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

//============================================================
// include files
//============================================================
#include "mp_precomp.h"
#include "phydm_precomp.h"


VOID
ODM_ChangeDynamicInitGainThresh(
	IN	PVOID		pDM_VOID,
	IN	u4Byte		DM_Type,
	IN	u4Byte		DM_Value
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T			pDM_DigTable = &pDM_Odm->DM_DigTable;

	if (DM_Type == DIG_TYPE_THRESH_HIGH)
	{
		pDM_DigTable->RssiHighThresh = DM_Value;		
	}
	else if (DM_Type == DIG_TYPE_THRESH_LOW)
	{
		pDM_DigTable->RssiLowThresh = DM_Value;
	}
	else if (DM_Type == DIG_TYPE_ENABLE)
	{
		pDM_DigTable->Dig_Enable_Flag	= TRUE;
	}	
	else if (DM_Type == DIG_TYPE_DISABLE)
	{
		pDM_DigTable->Dig_Enable_Flag = FALSE;
	}	
	else if (DM_Type == DIG_TYPE_BACKOFF)
	{
		if(DM_Value > 30)
			DM_Value = 30;
		pDM_DigTable->BackoffVal = (u1Byte)DM_Value;
	}
	else if(DM_Type == DIG_TYPE_RX_GAIN_MIN)
	{
		if(DM_Value == 0)
			DM_Value = 0x1;
		pDM_DigTable->rx_gain_range_min = (u1Byte)DM_Value;
	}
	else if(DM_Type == DIG_TYPE_RX_GAIN_MAX)
	{
		if(DM_Value > 0x50)
			DM_Value = 0x50;
		pDM_DigTable->rx_gain_range_max = (u1Byte)DM_Value;
	}
}	// DM_ChangeDynamicInitGainThresh //

int 
getIGIForDiff(int value_IGI)
{
	#define ONERCCA_LOW_TH		0x30
	#define ONERCCA_LOW_DIFF		8

	if (value_IGI < ONERCCA_LOW_TH) {
		if ((ONERCCA_LOW_TH - value_IGI) < ONERCCA_LOW_DIFF)
			return ONERCCA_LOW_TH;
		else
			return value_IGI + ONERCCA_LOW_DIFF;
	} else {
		return value_IGI;
	}
}

VOID
odm_FAThresholdCheck(
	IN		PVOID			pDM_VOID,
	IN		BOOLEAN			bDFSBand,
	IN		BOOLEAN			bPerformance,
	IN		u4Byte			RxTp,
	IN		u4Byte			TxTp,
	OUT		u4Byte*			dm_FA_thres
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	
	if(pDM_Odm->bLinked && (bPerformance||bDFSBand))
	{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
		/*For AP*/
#if (DIG_HW == 1)
		dm_FA_thres[0] = pDM_Odm->priv->pshare->rf_ft_var.dbg_dig_th1;
		dm_FA_thres[1] = pDM_Odm->priv->pshare->rf_ft_var.dbg_dig_th2;
		dm_FA_thres[2] = pDM_Odm->priv->pshare->rf_ft_var.dbg_dig_th3;
#else
		if ((RxTp>>2) > TxTp && RxTp < 10000 && RxTp > 500) {			/*10Mbps & 0.5Mbps*/
			dm_FA_thres[0] = 0x080;
			dm_FA_thres[1] = 0x100;
			dm_FA_thres[2] = 0x200;
		} else {
			dm_FA_thres[0] = 0x100;
			dm_FA_thres[1] = 0x200;
			dm_FA_thres[2] = 0x300;
		}
#endif
#else
		/*For NIC*/
		dm_FA_thres[0] = DM_DIG_FA_TH0;
		dm_FA_thres[1] = DM_DIG_FA_TH1;
		dm_FA_thres[2] = DM_DIG_FA_TH2;
#endif
	} else {
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_CE))
		if(bDFSBand)
		{
			// For DFS band and no link
			dm_FA_thres[0] = 250;
			dm_FA_thres[1] = 1000;
			dm_FA_thres[2] = 2000;
		}
		else
#endif
		{
			dm_FA_thres[0] = 2000;
			dm_FA_thres[1] = 4000;
			dm_FA_thres[2] = 5000;
		}
	}
	return;
}

u1Byte
odm_ForbiddenIGICheck(
	IN		PVOID			pDM_VOID,
	IN		u1Byte			dym_min,
	IN		u1Byte			CurrentIGI
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	PFALSE_ALARM_STATISTICS 	pFalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	u1Byte						rx_gain_min = pDM_DigTable->rx_gain_range_min;

	if (pDM_DigTable->LargeFA_Timeout) {
		if (--pDM_DigTable->LargeFA_Timeout == 0)
			pDM_DigTable->LargeFAHit = 0;
	}

	if (pFalseAlmCnt->Cnt_all > 10000) {
		
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Abnormally false alarm case.\n"));

		if(pDM_DigTable->LargeFAHit != 3)
			pDM_DigTable->LargeFAHit++;
		
		if(pDM_DigTable->ForbiddenIGI < CurrentIGI)	{
			pDM_DigTable->ForbiddenIGI = CurrentIGI;
			pDM_DigTable->LargeFAHit = 1;
			pDM_DigTable->LargeFA_Timeout = LARGE_FA_TIMEOUT;
		}

		if(pDM_DigTable->LargeFAHit >= 3) {
			if((pDM_DigTable->ForbiddenIGI + 2) > pDM_DigTable->rx_gain_range_max)
				rx_gain_min = pDM_DigTable->rx_gain_range_max;
			else
				rx_gain_min = (pDM_DigTable->ForbiddenIGI + 2);
			pDM_DigTable->Recover_cnt = 1800;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Abnormally false alarm case: Recover_cnt = %d\n", pDM_DigTable->Recover_cnt));
		}
	}

	else if (pFalseAlmCnt->Cnt_all > 2000) {
		
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("Abnormally false alarm case.\n"));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("Cnt_all=%d, Cnt_all_pre=%d, CurrentIGI=0x%x, PreIGValue=0x%x\n", 
			pFalseAlmCnt->Cnt_all, pFalseAlmCnt->Cnt_all_pre, CurrentIGI, pDM_DigTable->PreIGValue));

		/* pFalseAlmCnt->Cnt_all = 1.1875*pFalseAlmCnt->Cnt_all_pre */
		if ((pFalseAlmCnt->Cnt_all > (pFalseAlmCnt->Cnt_all_pre + (pFalseAlmCnt->Cnt_all_pre >> 3) + (pFalseAlmCnt->Cnt_all_pre >> 4))) && (CurrentIGI < pDM_DigTable->PreIGValue)) {
			if (pDM_DigTable->LargeFAHit != 3)
				pDM_DigTable->LargeFAHit++;
			
			if (pDM_DigTable->ForbiddenIGI < CurrentIGI) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD,
					("Updating ForbiddenIGI by CurrentIGI, ForbiddenIGI=0x%x, CurrentIGI=0x%x\n",
					pDM_DigTable->ForbiddenIGI, CurrentIGI));
				
				pDM_DigTable->ForbiddenIGI = CurrentIGI;	/*pDM_DigTable->ForbiddenIGI = pDM_DigTable->CurIGValue;*/
				pDM_DigTable->LargeFAHit = 1;
				pDM_DigTable->LargeFA_Timeout = LARGE_FA_TIMEOUT;
			}
			
		}
		
		if (pDM_DigTable->LargeFAHit >= 3) {
			
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("FaHit is greater than 3, rx_gain_range_max=0x%x, rx_gain_range_min=0x%x, ForbiddenIGI=0x%x\n",
				pDM_DigTable->rx_gain_range_max, rx_gain_min, pDM_DigTable->ForbiddenIGI));

			if ((pDM_DigTable->ForbiddenIGI + 1) > pDM_DigTable->rx_gain_range_max)
				rx_gain_min = pDM_DigTable->rx_gain_range_max;
			else
				rx_gain_min = (pDM_DigTable->ForbiddenIGI + 1);

			pDM_DigTable->Recover_cnt = 1200;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD,
				("Abnormal fa case: Recover_cnt = %d,  rx_gain_min = 0x%x\n",
				pDM_DigTable->Recover_cnt, rx_gain_min));
		}
	} else {
		if (pDM_DigTable->Recover_cnt != 0) {
			
			pDM_DigTable->Recover_cnt --;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: Recover_cnt = %d\n", pDM_DigTable->Recover_cnt));
		} else {
		
			if(pDM_DigTable->LargeFAHit < 3) {
				if((pDM_DigTable->ForbiddenIGI - 2) < dym_min) {
					pDM_DigTable->ForbiddenIGI = dym_min;
					rx_gain_min = dym_min;
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: At Lower Bound\n"));
				} else {
					if (pDM_DigTable->LargeFAHit == 0) {
						pDM_DigTable->ForbiddenIGI -= 2;
						rx_gain_min = (pDM_DigTable->ForbiddenIGI + 2);
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Normal Case: Approach Lower Bound\n"));
					}
				}
			} else {
				pDM_DigTable->LargeFAHit = 0;
			}
		}
	}
	
	return rx_gain_min;

}


VOID
odm_InbandNoiseCalculate (	
	IN		PVOID		pDM_VOID
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	PDM_ODM_T			pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T				pDM_DigTable = &pDM_Odm->DM_DigTable;
	u1Byte				IGIBackup, TimeCnt = 0, ValidCnt = 0;
	BOOLEAN				bTimeout = TRUE;
	s1Byte				sNoise_A, sNoise_B;
	s4Byte				NoiseRpt_A = 0,NoiseRpt_B = 0;
	u4Byte				tmp = 0;
	static	u1Byte		failCnt = 0;

	if(!(pDM_Odm->SupportICType & (ODM_RTL8192E)))
		return;

	if(pDM_Odm->RFType == ODM_1T1R || *(pDM_Odm->pOnePathCCA) != ODM_CCA_2R)
		return;

	if(!pDM_DigTable->bNoiseEst)
		return;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_InbandNoiseEstimate()========>\n"));
	
	//1 Set initial gain.
	IGIBackup = pDM_DigTable->CurIGValue;
	pDM_DigTable->IGIOffset_A = 0;
	pDM_DigTable->IGIOffset_B = 0;
	ODM_Write_DIG(pDM_Odm, 0x24);

	//1 Update idle time power report	
	if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
		ODM_SetBBReg(pDM_Odm, ODM_REG_TX_ANT_CTRL_11N, BIT25, 0x0);

	delay_ms(2);

	//1 Get noise power level
	while(1)
	{
		//2 Read Noise Floor Report
		if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
			tmp = ODM_GetBBReg(pDM_Odm, 0x8f8, bMaskLWord);

		sNoise_A = (s1Byte)(tmp & 0xff);
		sNoise_B = (s1Byte)((tmp & 0xff00)>>8);

		//ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("sNoise_A = %d, sNoise_B = %d\n",sNoise_A, sNoise_B));

		if((sNoise_A < 20 && sNoise_A >= -70) && (sNoise_B < 20 && sNoise_B >= -70))
		{
			ValidCnt++;
			NoiseRpt_A += sNoise_A;
			NoiseRpt_B += sNoise_B;
			//ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("sNoise_A = %d, sNoise_B = %d\n",sNoise_A, sNoise_B));
		}

		TimeCnt++;
		bTimeout = (TimeCnt >= 150)?TRUE:FALSE;
		
		if(ValidCnt == 20 || bTimeout)
			break;

		delay_ms(2);
		
	}

	//1 Keep idle time power report	
	if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
		ODM_SetBBReg(pDM_Odm, ODM_REG_TX_ANT_CTRL_11N, BIT25, 0x1);

	//1 Recover IGI
	ODM_Write_DIG(pDM_Odm, IGIBackup);
	
	//1 Calculate Noise Floor
	if(ValidCnt != 0)
	{
		NoiseRpt_A  /= (ValidCnt<<1);
		NoiseRpt_B  /= (ValidCnt<<1);
	}
	
	if(bTimeout)
	{
		NoiseRpt_A = 0;
		NoiseRpt_B = 0;

		failCnt ++;
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("Noise estimate fail time = %d\n", failCnt));
		
		if(failCnt == 3)
		{
			failCnt = 0;
			pDM_DigTable->bNoiseEst = FALSE;
		}
	}
	else
	{
		NoiseRpt_A = -110 + 0x24 + NoiseRpt_A -6;
		NoiseRpt_B = -110 + 0x24 + NoiseRpt_B -6;
		pDM_DigTable->bNoiseEst = FALSE;
		failCnt = 0;
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("NoiseRpt_A = %d, NoiseRpt_B = %d\n", NoiseRpt_A, NoiseRpt_B));
	}

	//1 Calculate IGI Offset
	if(NoiseRpt_A > NoiseRpt_B)
	{
		pDM_DigTable->IGIOffset_A = NoiseRpt_A - NoiseRpt_B;
		pDM_DigTable->IGIOffset_B = 0;
	}
	else
	{
		pDM_DigTable->IGIOffset_A = 0;
		pDM_DigTable->IGIOffset_B = NoiseRpt_B - NoiseRpt_A;
	}

#endif
	return;
}

VOID
odm_DigForBtHsMode(
	IN		PVOID		pDM_VOID
	)
{
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T				pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T					pDM_DigTable=&pDM_Odm->DM_DigTable;
	u1Byte					digForBtHs=0;
	u1Byte					digUpBound=0x5a;
	
	if (pDM_Odm->bBtConnectProcess) {
		digForBtHs = 0x22;
	} else {
		//
		// Decide DIG value by BT HS RSSI.
		//
		digForBtHs = pDM_Odm->btHsRssi+4;
		
		//DIG Bound
		if(digForBtHs > digUpBound)
			digForBtHs = digUpBound;
		if(digForBtHs < 0x1c)
			digForBtHs = 0x1c;

		// update Current IGI
		pDM_DigTable->BT30_CurIGI = digForBtHs;
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DigForBtHsMode() : set DigValue=0x%x\n", digForBtHs));
#endif
}

VOID
phydm_setBigJumpStep(
	IN	PVOID			pDM_VOID,
	IN	u1Byte			CurrentIGI
)
{
#if (RTL8822B_SUPPORT == 1 || RTL8197F_SUPPORT == 1)
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T			pDM_DigTable = &pDM_Odm->DM_DigTable;
	u1Byte			step1[8] = {24, 30, 40, 50, 60, 70, 80, 90};
	u1Byte			i;

	if (pDM_DigTable->enableAdjustBigJump == 0)
		return;

	for (i = 0; i <= pDM_DigTable->bigJumpStep1; i++) {
		if ((CurrentIGI + step1[i]) > pDM_DigTable->bigJumpLmt[pDM_DigTable->agcTableIdx]) {
			if (i != 0)
				i = i - 1;
			break;
		} else if (i == pDM_DigTable->bigJumpStep1)
			break;
	}
	if (pDM_Odm->SupportICType & ODM_RTL8822B)
		ODM_SetBBReg(pDM_Odm, 0x8c8, 0xe, i);
	else if (pDM_Odm->SupportICType & ODM_RTL8197F)
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_AGC_SET_2_11N, 0xe, i);
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("phydm_setBigJumpStep(): bigjump = %d (ori = 0x%d), LMT=0x%x\n", i, pDM_DigTable->bigJumpStep1, pDM_DigTable->bigJumpLmt[pDM_DigTable->agcTableIdx]));
#endif
}

VOID
ODM_Write_DIG(
	IN	PVOID			pDM_VOID,
	IN	u1Byte			CurrentIGI
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T			pDM_DigTable = &pDM_Odm->DM_DigTable;

	if (pDM_DigTable->bStopDIG) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("ODM_Write_DIG(): Stop Writing IGI\n"));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_TRACE, ("ODM_Write_DIG(): ODM_REG(IGI_A,pDM_Odm)=0x%x, ODM_BIT(IGI,pDM_Odm)=0x%x\n",
		ODM_REG(IGI_A,pDM_Odm),ODM_BIT(IGI,pDM_Odm)));

	//1 Check initial gain by upper bound		
	if ((!pDM_DigTable->bPSDInProgress) && pDM_Odm->bLinked)
	{
		if(0)
		{
			if (CurrentIGI > pDM_DigTable->rx_gain_range_max) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_TRACE, ("ODM_Write_DIG(): CurrentIGI(0x%02x) is larger than upper bound !!\n", CurrentIGI));
				CurrentIGI = pDM_DigTable->rx_gain_range_max;
			}
		}
		if (pDM_Odm->SupportAbility & ODM_BB_ADAPTIVITY && pDM_Odm->adaptivity_flag == TRUE)
		{
			if(CurrentIGI > pDM_Odm->Adaptivity_IGI_upper)
				CurrentIGI = pDM_Odm->Adaptivity_IGI_upper;
	
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("ODM_Write_DIG(): Adaptivity case: Force upper bound to 0x%x !!!!!!\n", CurrentIGI));
		}
	}

	if(pDM_DigTable->CurIGValue != CurrentIGI)
	{

#if (RTL8822B_SUPPORT == 1 || RTL8197F_SUPPORT == 1)
		/* Modify big jump step for 8822B and 8197F */
		if (pDM_Odm->SupportICType & (ODM_RTL8822B|ODM_RTL8197F))
			phydm_setBigJumpStep(pDM_Odm, CurrentIGI);
#endif

#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
		/* Set IGI value of CCK for new CCK AGC */
		if (pDM_Odm->cck_new_agc) {
			if (pDM_Odm->SupportICType & ODM_IC_PHY_STATUE_NEW_TYPE)
				ODM_SetBBReg(pDM_Odm, 0xa0c, 0x00003f00, (CurrentIGI>>1));
		}
#endif

		/*Add by YuChen for USB IO too slow issue*/
		if ((pDM_Odm->SupportAbility & ODM_BB_ADAPTIVITY) && (CurrentIGI > pDM_DigTable->CurIGValue))
			Phydm_Adaptivity(pDM_Odm, CurrentIGI);

		//1 Set IGI value
		if (pDM_Odm->SupportPlatform & (ODM_WIN|ODM_CE)) {
			ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);

			if(pDM_Odm->RFType > ODM_1T1R)
				ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);

#if (RTL8814A_SUPPORT == 1)
			if (pDM_Odm->SupportICType & ODM_RTL8814A) {
				ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_C,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
				ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_D,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
			}
#endif
		} else if (pDM_Odm->SupportPlatform & (ODM_AP)) {
			switch(*(pDM_Odm->pOnePathCCA))
			{
				case ODM_CCA_2R:
					ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);

					if(pDM_Odm->RFType > ODM_1T1R)
						ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
#if (RTL8814A_SUPPORT == 1)					
					if (pDM_Odm->SupportICType & ODM_RTL8814A) {
						ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_C,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
						ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_D,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
					}
#endif
					break;
				case ODM_CCA_1R_A:
					ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
					if(pDM_Odm->RFType != ODM_1T1R)
						ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B,pDM_Odm), ODM_BIT(IGI,pDM_Odm), getIGIForDiff(CurrentIGI));
					break;
				case ODM_CCA_1R_B:
					ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm), getIGIForDiff(CurrentIGI));
					if(pDM_Odm->RFType != ODM_1T1R)
						ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B,pDM_Odm), ODM_BIT(IGI,pDM_Odm), CurrentIGI);
					break;
			}
		}

		pDM_DigTable->CurIGValue = CurrentIGI;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_TRACE, ("ODM_Write_DIG(): CurrentIGI(0x%02x).\n", CurrentIGI));
	
}

VOID
odm_PauseDIG(
	IN		PVOID					pDM_VOID,
	IN		PHYDM_PAUSE_TYPE		PauseType,
	IN		PHYDM_PAUSE_LEVEL		pause_level,
	IN		u1Byte					IGIValue
)
{
	PDM_ODM_T			pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T				pDM_DigTable = &pDM_Odm->DM_DigTable;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG()=========> level = %d\n", pause_level));

	if ((pDM_DigTable->pause_dig_level == 0) && (!(pDM_Odm->SupportAbility & ODM_BB_DIG) || !(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, 
			("odm_PauseDIG(): Return: SupportAbility DIG or FA is disabled !!\n"));
		return;
	}

	if (pause_level > DM_DIG_MAX_PAUSE_TYPE) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, 
			("odm_PauseDIG(): Return: Wrong pause level !!\n"));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): pause level = 0x%x, Current value = 0x%x\n", pDM_DigTable->pause_dig_level, IGIValue));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): pause value = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		pDM_DigTable->pause_dig_value[7], pDM_DigTable->pause_dig_value[6], pDM_DigTable->pause_dig_value[5], pDM_DigTable->pause_dig_value[4],
		pDM_DigTable->pause_dig_value[3], pDM_DigTable->pause_dig_value[2], pDM_DigTable->pause_dig_value[1], pDM_DigTable->pause_dig_value[0]));
	
	switch (PauseType) {
	/* Pause DIG */
	case PHYDM_PAUSE:
	{
		/* Disable DIG */
		ODM_CmnInfoUpdate(pDM_Odm, ODM_CMNINFO_ABILITY, pDM_Odm->SupportAbility & (~ODM_BB_DIG));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Pause DIG !!\n"));
		
		/* Backup IGI value */
		if (pDM_DigTable->pause_dig_level == 0) {
			pDM_DigTable->IGIBackup = pDM_DigTable->CurIGValue;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Backup IGI  = 0x%x, new IGI = 0x%x\n", pDM_DigTable->IGIBackup, IGIValue));
		}

		/* Record IGI value */
		pDM_DigTable->pause_dig_value[pause_level] = IGIValue;

		/* Update pause level */
		pDM_DigTable->pause_dig_level = (pDM_DigTable->pause_dig_level | BIT(pause_level));

		/* Write new IGI value */
		if (BIT(pause_level + 1) > pDM_DigTable->pause_dig_level) {
			ODM_Write_DIG(pDM_Odm, IGIValue);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): IGI of higher level = 0x%x\n",  IGIValue));
		}
		break;
	}
	/* Resume DIG */
	case PHYDM_RESUME:
	{
		/* check if the level is illegal or not */
		if ((pDM_DigTable->pause_dig_level & (BIT(pause_level))) != 0) {
			pDM_DigTable->pause_dig_level = pDM_DigTable->pause_dig_level & (~(BIT(pause_level)));
			pDM_DigTable->pause_dig_value[pause_level] = 0;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Resume DIG !!\n"));
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Wrong resume level !!\n"));
			break;
		}

		/* Resume DIG */
		if (pDM_DigTable->pause_dig_level == 0) {
			/* Write backup IGI value */
			ODM_Write_DIG(pDM_Odm, pDM_DigTable->IGIBackup);
			pDM_DigTable->bIgnoreDIG = TRUE;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Write original IGI = 0x%x\n", pDM_DigTable->IGIBackup));

			/* Enable DIG */
			ODM_CmnInfoUpdate(pDM_Odm, ODM_CMNINFO_ABILITY, pDM_Odm->SupportAbility | ODM_BB_DIG);	
			break;
		}

		if (BIT(pause_level) > pDM_DigTable->pause_dig_level) {
			s1Byte		max_level;
		
			/* Calculate the maximum level now */
			for (max_level = (pause_level - 1); max_level >= 0; max_level--) {
				if ((pDM_DigTable->pause_dig_level & BIT(max_level)) > 0)
					break;
			}
		
			/* write IGI of lower level */
			ODM_Write_DIG(pDM_Odm, pDM_DigTable->pause_dig_value[max_level]);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Write IGI (0x%x) of level (%d)\n",  
				 pDM_DigTable->pause_dig_value[max_level], max_level));
			break;
		}
		break;
	}
	default:
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): Wrong  type !!\n"));
		break;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): pause level = 0x%x, Current value = 0x%x\n", pDM_DigTable->pause_dig_level, IGIValue));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseDIG(): pause value = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		pDM_DigTable->pause_dig_value[7], pDM_DigTable->pause_dig_value[6], pDM_DigTable->pause_dig_value[5], pDM_DigTable->pause_dig_value[4],
		pDM_DigTable->pause_dig_value[3], pDM_DigTable->pause_dig_value[2], pDM_DigTable->pause_dig_value[1], pDM_DigTable->pause_dig_value[0]));

}

BOOLEAN 
odm_DigAbort(
	IN		PVOID			pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T			pDM_DigTable = &pDM_Odm->DM_DigTable;

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	prtl8192cd_priv	priv = pDM_Odm->priv;
#elif(DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PADAPTER		pAdapter	= pDM_Odm->Adapter; 
#endif

	//SupportAbility
	if(!(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: SupportAbility ODM_BB_FA_CNT is disabled\n"));
		return	TRUE;
	}

	//SupportAbility
	if(!(pDM_Odm->SupportAbility & ODM_BB_DIG))
	{	
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: SupportAbility ODM_BB_DIG is disabled\n"));
		return	TRUE;
	}

	//ScanInProcess
	if(*(pDM_Odm->pbScanInProcess))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: In Scan Progress \n"));
	    	return	TRUE;
	}

	if(pDM_DigTable->bIgnoreDIG)
	{
		pDM_DigTable->bIgnoreDIG = FALSE;
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: Ignore DIG \n"));
	    	return	TRUE;
	}

	//add by Neil Chen to avoid PSD is processing
	if(pDM_Odm->bDMInitialGainEnable == FALSE)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: PSD is Processing \n"));
		return	TRUE;
	}

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	#if OS_WIN_FROM_WIN7(OS_VERSION)
	if(IsAPModeExist( pAdapter) && pAdapter->bInHctTest)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: Is AP mode or In HCT Test \n"));
	    	return	TRUE;
	}
	#endif

	if(pDM_Odm->bBtHsOperation)
	{
		odm_DigForBtHsMode(pDM_Odm);
	}	

#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
	#ifdef CONFIG_SPECIAL_SETTING_FOR_FUNAI_TV	
	if((pDM_Odm->bLinked) && (pDM_Odm->Adapter->registrypriv.force_igi !=0))
	{	
		printk("pDM_Odm->RSSI_Min=%d \n",pDM_Odm->RSSI_Min);
		ODM_Write_DIG(pDM_Odm,pDM_Odm->Adapter->registrypriv.force_igi);
		return	TRUE;
	}
	#endif
#else
	if (!(priv->up_time > 5))
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Return: Not In DIG Operation Period \n"));
		return	TRUE;
	}
#endif

	return	FALSE;
}

VOID
odm_DIGInit(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	PFALSE_ALARM_STATISTICS 	FalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
#endif
	u4Byte						ret_value;
	u1Byte						i;

	pDM_DigTable->bStopDIG = FALSE;
	pDM_DigTable->bIgnoreDIG = FALSE;
	pDM_DigTable->bPSDInProgress = FALSE;
	pDM_DigTable->CurIGValue = (u1Byte) ODM_GetBBReg(pDM_Odm, ODM_REG(IGI_A,pDM_Odm), ODM_BIT(IGI,pDM_Odm));
	pDM_DigTable->PreIGValue = 0;
	pDM_DigTable->RssiLowThresh 	= DM_DIG_THRESH_LOW;
	pDM_DigTable->RssiHighThresh 	= DM_DIG_THRESH_HIGH;
	pDM_DigTable->FALowThresh	= DM_FALSEALARM_THRESH_LOW;
	pDM_DigTable->FAHighThresh	= DM_FALSEALARM_THRESH_HIGH;
	pDM_DigTable->BackoffVal = DM_DIG_BACKOFF_DEFAULT;
	pDM_DigTable->BackoffVal_range_max = DM_DIG_BACKOFF_MAX;
	pDM_DigTable->BackoffVal_range_min = DM_DIG_BACKOFF_MIN;
	pDM_DigTable->PreCCK_CCAThres = 0xFF;
	pDM_DigTable->CurCCK_CCAThres = 0x83;
	pDM_DigTable->ForbiddenIGI = DM_DIG_MIN_NIC;
	pDM_DigTable->LargeFAHit = 0;
	pDM_DigTable->LargeFA_Timeout = 0;
	pDM_DigTable->Recover_cnt = 0;
	pDM_DigTable->bMediaConnect_0 = FALSE;
	pDM_DigTable->bMediaConnect_1 = FALSE;
	pDM_DigTable->pre_rssi_min = 0;

	//To Initialize pDM_Odm->bDMInitialGainEnable == FALSE to avoid DIG error
	pDM_Odm->bDMInitialGainEnable = TRUE;

#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	pDM_DigTable->tdma_dig_cnt = 0;
	pDM_DigTable->pre_tdma_dig_cnt = 0;
	pDM_DigTable->tdma_igi[TDMA_DIG_HIGH_STATE] = pDM_DigTable->CurIGValue;
	pDM_DigTable->tdma_igi[TDMA_DIG_LOW_STATE] = pDM_DigTable->CurIGValue;
	pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE]  = DM_DIG_MAX_NIC;
	pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_HIGH_STATE] = DM_DIG_MIN_NIC;
	pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE] = 0x26;
	pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_LOW_STATE] = DM_DIG_MIN_NIC;

	for(i = 0; i <= 1; i++) {
		pDM_DigTable->stay_check_state_cnt[i] = 0;		// check state	
		pDM_DigTable->goto_lock_state_cnt[i] = 0; 		// check state
		pDM_DigTable->goto_release_state_cnt[i] = 0;	// lock state
		pDM_DigTable->stay_release_state_cnt[i] = 0;	// release state
	}
#endif	//#if (PHYDM_TDMA_DIG_SUPPORT == 1)

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	pDM_DigTable->DIG_Dynamic_MIN_0 = 0x25;
	pDM_DigTable->DIG_Dynamic_MIN_1 = 0x25;

	// For AP\ ADSL modified DIG
	pDM_DigTable->bTpTarget = FALSE;
	pDM_DigTable->bNoiseEst = TRUE;
	pDM_DigTable->IGIOffset_A = 0;
	pDM_DigTable->IGIOffset_B = 0;
	pDM_DigTable->TpTrainTH_min = 0;

	// For RTL8881A
	FalseAlmCnt->Cnt_Ofdm_fail_pre = 0;

	//Dyanmic EDCCA
	if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
	{
		ODM_SetBBReg(pDM_Odm, 0xC50, 0xFFFF0000, 0xfafd);
	}
#else
	pDM_DigTable->DIG_Dynamic_MIN_0 = DM_DIG_MIN_NIC;
	pDM_DigTable->DIG_Dynamic_MIN_1 = DM_DIG_MIN_NIC;

	//To Initi BT30 IGI
	pDM_DigTable->BT30_CurIGI=0x32;

	ODM_Memory_Set(pDM_Odm, pDM_DigTable->pause_dig_value, 0, (DM_DIG_MAX_PAUSE_TYPE + 1));
	pDM_DigTable->pause_dig_level = 0;
	ODM_Memory_Set(pDM_Odm, pDM_DigTable->pause_cckpd_value, 0, (DM_DIG_MAX_PAUSE_TYPE + 1));
	pDM_DigTable->pause_cckpd_level = 0;
#endif

	if(pDM_Odm->BoardType & (ODM_BOARD_EXT_PA|ODM_BOARD_EXT_LNA))
	{
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_NIC;
		pDM_DigTable->rx_gain_range_min = DM_DIG_MIN_NIC;
	}
	else
	{
		pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_NIC;
		pDM_DigTable->rx_gain_range_min = DM_DIG_MIN_NIC;
	}

#if (RTL8822B_SUPPORT == 1 || RTL8197F_SUPPORT == 1)
	pDM_DigTable->enableAdjustBigJump = 1;
	if (pDM_Odm->SupportICType & ODM_RTL8822B) {
		ret_value = ODM_GetBBReg(pDM_Odm, 0x8c8, bMaskLWord);
		pDM_DigTable->bigJumpStep1 = (u1Byte)(ret_value & 0xe) >> 1;
		pDM_DigTable->bigJumpStep2 = (u1Byte)(ret_value & 0x30)>>4;
		pDM_DigTable->bigJumpStep3 = (u1Byte)(ret_value & 0xc0)>>6;

	} else if (pDM_Odm->SupportICType & ODM_RTL8197F) {
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_BB_AGC_SET_2_11N, bMaskLWord);
		pDM_DigTable->bigJumpStep1 = (u1Byte)(ret_value & 0xe) >> 1;
		pDM_DigTable->bigJumpStep2 = (u1Byte)(ret_value & 0x30)>>4;
		pDM_DigTable->bigJumpStep3 = (u1Byte)(ret_value & 0xc0)>>6;
	}
	if (pDM_Odm->SupportICType & (ODM_RTL8822B|ODM_RTL8197F)) {
		for (i = 0; i < sizeof(pDM_DigTable->bigJumpLmt); i++) {
			if (pDM_DigTable->bigJumpLmt[i] == 0)
				pDM_DigTable->bigJumpLmt[i] = 0x64;		/* Set -10dBm as default value */
		}
	}
#endif


#if (DIG_HW == 1)
	pDM_DigTable->pre_rssi_min = 0;
#endif
}

u1Byte
phydm_igi_chkby_upper_lower_bond(
	u1Byte				igi,
	u1Byte				upper_bond,
	u1Byte				lower_bond
	)
{
	u1Byte			igi_tmp = igi;

	if (igi < lower_bond)
		igi_tmp = lower_bond;
	else if (igi > upper_bond)
		igi_tmp = upper_bond;

	return igi_tmp;
}

VOID 
odm_DIG(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PADAPTER					pAdapter	= pDM_Odm->Adapter;
	HAL_DATA_TYPE				*pHalData = GET_HAL_DATA(pDM_Odm->Adapter);
#elif (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	prtl8192cd_priv				priv = pDM_Odm->priv;
	PSTA_INFO_T   				pEntry;
#endif

	// Common parameters
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
//	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	u1Byte						CurrentIGI;
	BOOLEAN						FirstConnect,FirstDisConnect;
	u1Byte						DIG_MaxOfMin, DIG_Dynamic_MIN;
	u1Byte						dm_dig_max, dm_dig_min;
	u1Byte						offset;
	u1Byte						rssi_min = pDM_Odm->RSSI_Min;
	u1Byte						igi_upper_rssi_min;
	u4Byte						dm_FA_thres[3];
	u4Byte						TxTp = 0, RxTp = 0;
	BOOLEAN						bDFSBand = FALSE;
	BOOLEAN						bPerformance = TRUE, bFirstTpTarget = FALSE, bFirstCoverage = FALSE;
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	u4Byte						TpTrainTH_MIN = DM_DIG_TP_Target_TH0;
	static		u1Byte			TimeCnt = 0;
	u1Byte						i;
#endif
#if (DIG_HW == 1)
	BOOLEAN						DIG_GoUpCheck = TRUE;
	u1Byte						step_size[3] = {0};
#endif

	CurrentIGI = pDM_DigTable->CurIGValue;
	
	if(odm_DigAbort(pDM_Odm) == TRUE)
		return;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG()==>\n\n"));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("CurrentIGI=0x%x\n", CurrentIGI));

#if (DIG_HW == 1)
	if (pDM_Odm->bLinked) {
		if (pDM_DigTable->pre_rssi_min <= rssi_min) {
	step_size[2] = 2;
			step_size[1] = 1;
			step_size[0] = 2;
		} else {
			step_size[2] = 4;
			step_size[1] = 2;
			step_size[0] = 2;
		}
		pDM_DigTable->pre_rssi_min = rssi_min;
	} else {
		step_size[2] = 2;
		step_size[1] = 1;
		step_size[0] = 2;
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("RSSI_Min = %d, pre_rssi_min = %d\n",
		rssi_min, pDM_DigTable->pre_rssi_min));	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("step_size[0] = %d, step_size[1] = %d, step_size[2] = %d\n",
		step_size[0], step_size[1], step_size[2]));	
#endif

	//1 Update status
	{
		DIG_Dynamic_MIN = pDM_DigTable->DIG_Dynamic_MIN_0;
		FirstConnect = (pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == FALSE);
		FirstDisConnect = (!pDM_Odm->bLinked) && (pDM_DigTable->bMediaConnect_0 == TRUE);
	}

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	//1 Noise Floor Estimate
	//pDM_DigTable->bNoiseEst = (FirstConnect)?TRUE:pDM_DigTable->bNoiseEst;
	//odm_InbandNoiseCalculate (pDM_Odm);
	
	//1 Mode decision
	if(pDM_Odm->bLinked)
	{
		//2 Calculate total TP
		for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
		{
			pEntry = pDM_Odm->pODM_StaInfo[i];
			if(IS_STA_VALID(pEntry))
			{
				RxTp += (u4Byte)(pEntry->rx_byte_cnt_LowMAW>>7);
				TxTp += (u4Byte)(pEntry->tx_byte_cnt_LowMAW>>7);			//Kbps
			}
		}
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): TX TP = %dkbps, RX TP = %dkbps\n", TxTp, RxTp));
	}

	switch(pDM_Odm->priv->pshare->rf_ft_var.dig_cov_enable)
	{
		case 0:
		{
			bPerformance = TRUE;
			break;
		}
		case 1:
		{
			bPerformance = FALSE;
			break;
		}
		case 2:
		{
			if(pDM_Odm->bLinked)
			{
				if(pDM_DigTable->TpTrainTH_min > DM_DIG_TP_Target_TH0)
					TpTrainTH_MIN = pDM_DigTable->TpTrainTH_min;

				if(pDM_DigTable->TpTrainTH_min > DM_DIG_TP_Target_TH1)
					TpTrainTH_MIN = DM_DIG_TP_Target_TH1;

				ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): TP training mode lower bound = %dkbps\n", TpTrainTH_MIN));

				//2 Decide DIG mode by total TP
				if((TxTp + RxTp) > DM_DIG_TP_Target_TH1)			// change to performance mode
				{
					bFirstTpTarget = (!pDM_DigTable->bTpTarget)?TRUE:FALSE;
					pDM_DigTable->bTpTarget = TRUE;
					bPerformance = TRUE;
				}
				else if((TxTp + RxTp) < TpTrainTH_MIN)	// change to coverage mode
				{
					bFirstCoverage = (pDM_DigTable->bTpTarget)?TRUE:FALSE;
					
					if(TimeCnt < DM_DIG_TP_Training_Period)
					{
						pDM_DigTable->bTpTarget = FALSE;
						bPerformance = FALSE;
						TimeCnt++;
					}
					else
					{
						pDM_DigTable->bTpTarget = TRUE;
						bPerformance = TRUE;
						bFirstTpTarget = TRUE;
						TimeCnt = 0;
					}
				}
				else										// remain previous mode
				{
					bPerformance = pDM_DigTable->bTpTarget;

					if(!bPerformance)
					{
						if(TimeCnt < DM_DIG_TP_Training_Period)
							TimeCnt++;
						else
						{
							pDM_DigTable->bTpTarget = TRUE;
							bPerformance = TRUE;
							bFirstTpTarget = TRUE;
							TimeCnt = 0;
						}
					}
				}

				if(!bPerformance)
					pDM_DigTable->TpTrainTH_min = RxTp + TxTp;

			}
			else
			{
				bPerformance = FALSE;
				pDM_DigTable->TpTrainTH_min = 0;
			}
			break;
		}
		default:
			bPerformance = TRUE;
	}

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("====== DIG mode = %d  ======\n", pDM_Odm->priv->pshare->rf_ft_var.dig_cov_enable));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("====== bPerformance = %d ======\n", bPerformance));
#endif

	//1 Boundary Decision
	{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
		//2 For AP\ADSL
		if(!bPerformance)
		{
			dm_dig_max = DM_DIG_MAX_AP_COVERAGR;
			dm_dig_min = DM_DIG_MIN_AP_COVERAGE;
#if (DIG_HW == 1)
			DIG_MaxOfMin = DM_DIG_MIN_AP_COVERAGE;
#else
			DIG_MaxOfMin = DM_DIG_MAX_OF_MIN_COVERAGE;
#endif

		}
		else
		{
			if (pDM_Odm->RFType == ODM_1T1R)
				dm_dig_max = DM_DIG_MAX_AP - 6;
			else
				dm_dig_max = DM_DIG_MAX_AP;

			if ((*pDM_Odm->pBandType == ODM_BAND_2_4G) && (pDM_Odm->SupportICType & ODM_RTL8814A)) /* for 2G 8814 */
				dm_dig_min = 0x1c;
			else if (pDM_Odm->SupportICType & ODM_RTL8197F)		/*refine by YuChen 20160815 asked by RF Arthur*/
				dm_dig_min = 0x1e;
			else
				dm_dig_min = DM_DIG_MIN_AP;

#if (DIG_HW == 1)			
			DIG_MaxOfMin = DM_DIG_MIN_AP_COVERAGE;
#else
			DIG_MaxOfMin = DM_DIG_MAX_OF_MIN;
#endif

		}
		
		//4 TX2path
		if (priv->pmib->dot11RFEntry.tx2path && !bDFSBand && (*(pDM_Odm->pWirelessMode) == ODM_WM_B))
				dm_dig_max = 0x2A;

#if RTL8192E_SUPPORT
#ifdef HIGH_POWER_EXT_LNA
		if ((pDM_Odm->SupportICType & (ODM_RTL8192E)) && (pDM_Odm->ExtLNA))
			dm_dig_max = 0x42;						
#endif
#endif

#else	//#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

		//2 For WIN\CE
		if(pDM_Odm->SupportICType >= ODM_RTL8188E)
			dm_dig_max = 0x5A;
		else
			dm_dig_max = DM_DIG_MAX_NIC;
		
		if(pDM_Odm->SupportICType != ODM_RTL8821)
			dm_dig_min = DM_DIG_MIN_NIC;
		else
			dm_dig_min = 0x1C;

		DIG_MaxOfMin = DM_DIG_MAX_AP;
#endif


#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_CE))
		/* Modify lower bound for DFS band */
		if ((((*pDM_Odm->pChannel >= 52) && (*pDM_Odm->pChannel <= 64)) ||
			((*pDM_Odm->pChannel >= 100) && (*pDM_Odm->pChannel <= 140)))
			#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
			&& phydm_dfs_master_enabled(pDM_Odm) == TRUE
			#endif
		) {
			bDFSBand = TRUE;
			if (*pDM_Odm->pBandWidth == ODM_BW20M)
				dm_dig_min = DM_DIG_MIN_AP_DFS+2;
			else
				dm_dig_min = DM_DIG_MIN_AP_DFS;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): ====== In DFS band ======\n"));
		}
#endif
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("Absolutly upper bound = 0x%x, lower bound = 0x%x\n",dm_dig_max, dm_dig_min));

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
	if (pDM_Odm->pu1ForcedIgiLb && (0 < *pDM_Odm->pu1ForcedIgiLb)) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Force IGI lb to: 0x%02x !!!!!!\n", *pDM_Odm->pu1ForcedIgiLb));
		dm_dig_min = *pDM_Odm->pu1ForcedIgiLb;
		dm_dig_max = (dm_dig_min <= dm_dig_max) ? (dm_dig_max) : (dm_dig_min + 1);
	}
#endif

	//1 Adjust boundary by RSSI
	if(pDM_Odm->bLinked && bPerformance)
	{
		//2 Modify DIG upper bound
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
		offset = 15;
#else
		//4 Modify DIG upper bound for 92E, 8723A\B, 8821 & 8812 BT
		if ((pDM_Odm->SupportICType & (ODM_RTL8192E|ODM_RTL8723B|ODM_RTL8812|ODM_RTL8821)) && (pDM_Odm->bBtLimitedDig == 1))
		{
			offset = 10;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Coex. case: Force upper bound to RSSI + %d !!!!!!\n", offset));		
		}
		else
			offset = 15;
#endif
		igi_upper_rssi_min = rssi_min + offset;

		/*rx_gain_range_max*/
		if((igi_upper_rssi_min) > dm_dig_max)
			pDM_DigTable->rx_gain_range_max = dm_dig_max;
		else if((igi_upper_rssi_min) < dm_dig_min)
			pDM_DigTable->rx_gain_range_max = dm_dig_min;
		else
			pDM_DigTable->rx_gain_range_max = igi_upper_rssi_min;

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
		//2 Modify DIG lower bound
		//if(pDM_Odm->bOneEntryOnly)
		{
			/*DIG_Dynamic_MIN()*/
			if(rssi_min < dm_dig_min)
				DIG_Dynamic_MIN = dm_dig_min;
			else if (rssi_min > DIG_MaxOfMin)
				DIG_Dynamic_MIN = DIG_MaxOfMin;
			else
				DIG_Dynamic_MIN = rssi_min;

#if (DM_ODM_SUPPORT_TYPE & ODM_CE)
			if (bDFSBand) {
				DIG_Dynamic_MIN = dm_dig_min;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("DFS band: Force lower bound to 0x%x after link\n", dm_dig_min));
			}
#endif
		}
#else
		{
			//4 For AP
#ifdef __ECOS
			HAL_REORDER_BARRIER();
#else
			rmb();
#endif
			if (bDFSBand)
			{
				DIG_Dynamic_MIN = dm_dig_min;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("DFS band: Force lower bound to 0x%x after link\n", dm_dig_min));
			} else {

				/*DIG_Dynamic_MIN(rx_gain_range_min)*/
				if(rssi_min < dm_dig_min)
					DIG_Dynamic_MIN = dm_dig_min;
				else if (rssi_min > DIG_MaxOfMin)
					DIG_Dynamic_MIN = DIG_MaxOfMin;
				else
					DIG_Dynamic_MIN = rssi_min;				
			}
		}
#endif
	}
	else
	{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_CE))
		if(bPerformance && bDFSBand)
		{
			pDM_DigTable->rx_gain_range_max = 0x28;
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD,
				("DFS band:Force upper bound to 0x%x before link.\n", pDM_DigTable->rx_gain_range_max));
		}
		else
#endif
		{
			pDM_DigTable->rx_gain_range_max = DM_DIG_MAX_OF_MIN;
		}
		DIG_Dynamic_MIN = dm_dig_min;
	}
	
	//1 Force Lower Bound for AntDiv
	if(pDM_Odm->bLinked && !pDM_Odm->bOneEntryOnly)
	{
		if((pDM_Odm->SupportICType & ODM_ANTDIV_SUPPORT) && (pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
		{
			if (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV || pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV) {
				if (pDM_DigTable->AntDiv_RSSI_max > DIG_MaxOfMin) {
					DIG_Dynamic_MIN = DIG_MaxOfMin;
				}
				else {
					DIG_Dynamic_MIN = (u1Byte) pDM_DigTable->AntDiv_RSSI_max;				
				}
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("Antenna diversity case: Force lower bound to 0x%x\n",
					DIG_Dynamic_MIN));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("Antenna diversity case: RSSI_max=0x%x\n",
					pDM_DigTable->AntDiv_RSSI_max));
			}
		}
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD,
		("Adjust boundary by RSSI rx_gain_range_max=0x%x, DIG_Dynamic_MIN = 0x%x\n",
		pDM_DigTable->rx_gain_range_max, DIG_Dynamic_MIN));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD,
		("Link status: bLinked = %d, RSSI = %d, bFirstConnect = %d, bFirsrDisConnect = %d\n\n",
		pDM_Odm->bLinked, rssi_min, FirstConnect, FirstDisConnect));

	//1 Modify DIG lower bound, deal with abnormal case
	//2 Abnormal false alarm case
	if(!pDM_Odm->bLinked)
	{
		pDM_DigTable->rx_gain_range_min = DIG_Dynamic_MIN;

		if (FirstDisConnect)
			pDM_DigTable->ForbiddenIGI = DIG_Dynamic_MIN;
	}
	else {
		pDM_DigTable->rx_gain_range_min = odm_ForbiddenIGICheck(pDM_Odm, DIG_Dynamic_MIN, CurrentIGI);
	}
	//2 Abnormal # beacon case
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
	if(pDM_Odm->bLinked && !FirstConnect)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("Beacon Num (%d)\n", pDM_Odm->PhyDbgInfo.NumQryBeaconPkt));
		if((pDM_Odm->PhyDbgInfo.NumQryBeaconPkt < 5) && (pDM_Odm->bsta_state))
		{
			pDM_DigTable->rx_gain_range_min = 0x1c;
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Abnrormal #beacon (%d) case in STA mode: Force lower bound to 0x%x !!!!!!\n\n",
				pDM_Odm->PhyDbgInfo.NumQryBeaconPkt, pDM_DigTable->rx_gain_range_min));
		}
	}
#endif

	//2 Abnormal lower bound case
	if(pDM_DigTable->rx_gain_range_min > pDM_DigTable->rx_gain_range_max)
	{
		pDM_DigTable->rx_gain_range_min = pDM_DigTable->rx_gain_range_max;
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("Abnrormal rx_gain_range_min: force to 0x%x\n\n",
			pDM_DigTable->rx_gain_range_min));
	}

	
	//1 False alarm threshold decision
	odm_FAThresholdCheck(pDM_Odm, bDFSBand, bPerformance, RxTp, TxTp, dm_FA_thres);
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD,
		("odm_DIG(): False alarm threshold = %d, %d, %d \n\n",
		dm_FA_thres[0], dm_FA_thres[1], dm_FA_thres[2]));

	//1 Adjust initial gain by false alarm
	if(pDM_Odm->bLinked && bPerformance)
	{
		//2 After link
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("Adjust IGI after link\n"));

		if(bFirstTpTarget || (FirstConnect && bPerformance)) {	
			pDM_DigTable->LargeFAHit = 0;
			
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_CE))
			if(bDFSBand)
			{
				if(rssi_min > 0x28) {
					CurrentIGI = 0x28;
				}
				else {
					CurrentIGI = rssi_min;
				}
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("DFS band:One-shot to 0x28 upmost\n"));
			}
			else
#endif
			{

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
#if (RTL8812A_SUPPORT==1) 
				if(pDM_Odm->SupportICType == ODM_RTL8812)
					ODM_ConfigBBWithHeaderFile(pDM_Odm, CONFIG_BB_AGC_TAB_DIFF);
#endif
#endif
			}

			ODM_RT_TRACE(pDM_Odm,	ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): First connect case: IGI does on-shot to 0x%x\n", CurrentIGI));

		}
		else {

#if ((DM_ODM_SUPPORT_TYPE & (ODM_AP)) && (DIG_HW == 1))
			if (priv->pshare->rf_ft_var.dig_upcheck_enable)
				DIG_GoUpCheck = phydm_DIG_GoUpCheck(pDM_Odm);
#endif
			CurrentIGI = phydm_dig_current_igi_by_fa_th(pDM_Odm, CurrentIGI, dm_FA_thres, step_size, DIG_GoUpCheck, NORMAL_DIG);
			
			//4 Abnormal # beacon case
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
			if((pDM_Odm->PhyDbgInfo.NumQryBeaconPkt < 5) && (pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH1) && (pDM_Odm->bsta_state))
			{						
				CurrentIGI = pDM_DigTable->rx_gain_range_min;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Abnormal #beacon (%d) case: IGI does one-shot to 0x%x\n", 
					pDM_Odm->PhyDbgInfo.NumQryBeaconPkt, CurrentIGI));
			}
#endif
		}
	} else {
		//2 Before link
		ODM_RT_TRACE(pDM_Odm,	ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): Adjust IGI before link\n"));
		
		if(FirstDisConnect || bFirstCoverage) {
			CurrentIGI = dm_dig_min;
			ODM_RT_TRACE(pDM_Odm,	ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIG(): First disconnect case: IGI does on-shot to lower bound\n"));
		} else {
#if ((DM_ODM_SUPPORT_TYPE & (ODM_AP)) && (DIG_HW == 1))
			if (priv->pshare->rf_ft_var.dig_upcheck_enable)
				DIG_GoUpCheck = phydm_DIG_GoUpCheck(pDM_Odm);
#endif
			CurrentIGI = phydm_dig_current_igi_by_fa_th(pDM_Odm, CurrentIGI, dm_FA_thres, step_size, DIG_GoUpCheck, NORMAL_DIG);
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD,
		("before check igi by ULbond, CurrentIGI=0x%x\n", CurrentIGI));

	//1 Check initial gain by upper/lower bound
	CurrentIGI = phydm_igi_chkby_upper_lower_bond(CurrentIGI, pDM_DigTable->rx_gain_range_max, pDM_DigTable->rx_gain_range_min);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD,
		("after ULbond check,CurrentIGI=0x%x,\n", CurrentIGI));

	//1 Update status
	{
#if ((DM_ODM_SUPPORT_TYPE & ODM_WIN) || ((DM_ODM_SUPPORT_TYPE & ODM_CE) && (ODM_CONFIG_BT_COEXIST == 1)))
		if(pDM_Odm->bBtHsOperation)
		{
			if(pDM_Odm->bLinked)
			{
				if(pDM_DigTable->BT30_CurIGI > (CurrentIGI))
					ODM_Write_DIG(pDM_Odm, CurrentIGI);
				else
					ODM_Write_DIG(pDM_Odm, pDM_DigTable->BT30_CurIGI);
					
				pDM_DigTable->bMediaConnect_0 = pDM_Odm->bLinked;
				pDM_DigTable->DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
			}
			else
			{
				if(pDM_Odm->bLinkInProcess)
					ODM_Write_DIG(pDM_Odm, 0x1c);
				else if(pDM_Odm->bBtConnectProcess)
					ODM_Write_DIG(pDM_Odm, 0x28);
				else
					ODM_Write_DIG(pDM_Odm, pDM_DigTable->BT30_CurIGI);//ODM_Write_DIG(pDM_Odm, pDM_DigTable->CurIGValue);	
			}
		}
		else		// BT is not using
#endif
		{
			ODM_Write_DIG(pDM_Odm, CurrentIGI);
			pDM_DigTable->bMediaConnect_0 = pDM_Odm->bLinked;
			pDM_DigTable->DIG_Dynamic_MIN_0 = DIG_Dynamic_MIN;
		}
	}
}

void
phydm_lna_sat_chk_watchdog(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T				pDM_Odm = (PDM_ODM_T)pDM_VOID;	
	PLNA_SATURATION_INFO	pDM_LNA_info = &pDM_Odm->DM_LNA_SAT_INFO;
	u1Byte 					rssi_min = pDM_Odm->RSSI_Min;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD, ("\n%s ==>\n", __FUNCTION__));

	if(!(pDM_Odm->SupportAbility & ODM_BB_LNA_SAT_CHK))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD,
			("SupportAbility ODM_BB_LNA_SAT_CHK is disabled, return.\n"));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD, ("pre_timer_check_cnt=%d, cur_timer_check_cnt=%d\n",
		pDM_LNA_info->pre_timer_check_cnt,
		pDM_LNA_info->cur_timer_check_cnt));
	
	if(pDM_Odm->is_disable_lna_sat_chk) {
		phydm_lna_sat_chk_init(pDM_Odm);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD,
			("is_disable_lna_sat_chk=%d, return.\n", pDM_Odm->is_disable_lna_sat_chk));
		return;
	}

	if ((pDM_Odm->SupportICType & ODM_RTL8197F) == 0) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD,
			("SupportICType != ODM_RTL8197F, return.\n"));
		return;
	}

	if ((rssi_min == 0) || (rssi_min == 0xff)) {
		// adapt agc table 0
		phydm_set_ofdm_agc_tab(pDM_Odm, OFDM_AGC_TAB_0);
		phydm_lna_sat_chk_init(pDM_Odm);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD,
			("rssi_min=%d, return.\n", rssi_min));
		return;
	}

	if(pDM_LNA_info->cur_timer_check_cnt == pDM_LNA_info->pre_timer_check_cnt) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD, ("Timer check fail, restart timer.\n"));
		phydm_lna_sat_chk(pDM_Odm);
	}
	else {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD, ("Timer check pass.\n"));
	}
	pDM_LNA_info->pre_timer_check_cnt = pDM_LNA_info->cur_timer_check_cnt;
}

void
phydm_lna_sat_chk_init(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T				pDM_Odm = (PDM_ODM_T)pDM_VOID;	
	PLNA_SATURATION_INFO	pDM_LNA_info = &pDM_Odm->DM_LNA_SAT_INFO;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD, ("%s ==>\n", __FUNCTION__));

	pDM_LNA_info->check_time = 0;
	pDM_LNA_info->sat_cnt_acc_patha = 0;		
	pDM_LNA_info->sat_cnt_acc_pathb = 0;		
	pDM_LNA_info->cur_sat_status = 0;
	pDM_LNA_info->pre_sat_status = 0;
	pDM_LNA_info->cur_timer_check_cnt = 0;
	pDM_LNA_info->pre_timer_check_cnt = 0;
}

void
phydm_set_ofdm_agc_tab(
	IN		PVOID		pDM_VOID,
	IN		u1Byte		tab_sel
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	/* table sel:0/2, 1 is used for CCK */
	if (tab_sel == OFDM_AGC_TAB_0)
		ODM_SetBBReg(pDM_Odm, 0xc70, 0x1e00, OFDM_AGC_TAB_0);
	else if (tab_sel == OFDM_AGC_TAB_2)
		ODM_SetBBReg(pDM_Odm, 0xc70, 0x1e00, OFDM_AGC_TAB_2);
	else
		ODM_SetBBReg(pDM_Odm, 0xc70, 0x1e00, OFDM_AGC_TAB_0);
}

u1Byte
phydm_get_ofdm_agc_tab(
	IN		PVOID		pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	
	return 	(u1Byte)ODM_GetBBReg(pDM_Odm, 0xc70, 0x1e00);
}


void
phydm_lna_sat_chk(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;	
	pDIG_T			pDM_DigTable = &pDM_Odm->DM_DigTable;
	PLNA_SATURATION_INFO	pDM_LNA_info = &pDM_Odm->DM_LNA_SAT_INFO;
	u1Byte			igi_rssi_min, rssi_min = pDM_Odm->RSSI_Min;
	u4Byte			sat_status_patha, sat_status_pathb;
	u1Byte			igi_restore = pDM_DigTable->CurIGValue;
	u1Byte			i, lna_sat_chk_cnt = pDM_Odm->lna_sat_chk_cnt;
	u4Byte			lna_sat_cnt_thd = 0;
	u1Byte			agc_tab;
	u4Byte			max_check_time = 0;
//	u4Byte			func_start, func_end;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD, ("\n%s ==>\n", __FUNCTION__));

	if(!(pDM_Odm->SupportAbility & ODM_BB_LNA_SAT_CHK))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD,
			("SupportAbility ODM_BB_LNA_SAT_CHK is disabled, return.\n"));
		return;
	}

	if(pDM_Odm->is_disable_lna_sat_chk) {		
		phydm_lna_sat_chk_init(pDM_Odm);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD,
			("is_disable_lna_sat_chk=%d, return.\n", pDM_Odm->is_disable_lna_sat_chk));
		return;
	}

	//func_start = ODM_GetBBReg(pDM_Odm, 0x560, bMaskDWord);

	// move igi to target pin of rssi_min
	if ((rssi_min == 0) || (rssi_min == 0xff)) {
		// adapt agc table 0
		phydm_set_ofdm_agc_tab(pDM_Odm, OFDM_AGC_TAB_0);
		phydm_lna_sat_chk_init(pDM_Odm);
		return;
	}
	else if (rssi_min % 2 != 0)
		igi_rssi_min = rssi_min + RSSI_TO_IGI - 1;
	else
		igi_rssi_min = rssi_min + RSSI_TO_IGI;

	if ((pDM_Odm->lna_sat_chk_period_ms > 0) && (pDM_Odm->lna_sat_chk_period_ms <= ONE_SEC_MS))
		max_check_time = lna_sat_chk_cnt*(ONE_SEC_MS/(pDM_Odm->lna_sat_chk_period_ms))*5;
	else
		max_check_time = lna_sat_chk_cnt * 5;

	lna_sat_cnt_thd = (max_check_time * pDM_Odm->lna_sat_chk_duty_cycle)/100;
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD,
		("check_time=%d, rssi_min=%d, igi_rssi_min=0x%x\nlna_sat_chk_cnt=%d, lna_sat_chk_period_ms=%d, max_check_time=%d, lna_sat_cnt_thd=%d\n",
		pDM_LNA_info->check_time,
		rssi_min,
		igi_rssi_min,
		lna_sat_chk_cnt,
		pDM_Odm->lna_sat_chk_period_ms,
		max_check_time,
		lna_sat_cnt_thd));

	ODM_Write_DIG(pDM_VOID, igi_rssi_min);

	// adapt agc table 0 check saturation status
	phydm_set_ofdm_agc_tab(pDM_Odm, OFDM_AGC_TAB_0);
	// open rf power detection ckt & set detection range
	ODM_SetRFReg(pDM_Odm, RF_PATH_A, 0x86, 0x1f, 0x10);
	ODM_SetRFReg(pDM_Odm, RF_PATH_B, 0x86, 0x1f, 0x10);

	// check saturation status
	for(i = 0; i < lna_sat_chk_cnt; i++) {
		sat_status_patha = ODM_GetRFReg(pDM_Odm, RF_PATH_A, 0xae, 0xc0000);
		sat_status_pathb = ODM_GetRFReg(pDM_Odm, RF_PATH_B, 0xae, 0xc0000);
		if(sat_status_patha != 0)
			pDM_LNA_info->sat_cnt_acc_patha++;
		if(sat_status_pathb != 0)
			pDM_LNA_info->sat_cnt_acc_pathb++;

		if ((pDM_LNA_info->sat_cnt_acc_patha >= lna_sat_cnt_thd) ||
			(pDM_LNA_info->sat_cnt_acc_pathb >= lna_sat_cnt_thd)) {
			pDM_LNA_info->cur_sat_status= 1;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD,
			("cur_sat_status=%d, check_time=%d\n",
			pDM_LNA_info->cur_sat_status,
			pDM_LNA_info->check_time));
			break;
		} else
			pDM_LNA_info->cur_sat_status= 0;
	}
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD,
		("cur_sat_status=%d, pre_sat_status=%d, sat_cnt_acc_patha=%d, sat_cnt_acc_pathb=%d\n",
		pDM_LNA_info->cur_sat_status,
		pDM_LNA_info->pre_sat_status,
		pDM_LNA_info->sat_cnt_acc_patha,
		pDM_LNA_info->sat_cnt_acc_pathb));
	
	// agc table decision
	if (pDM_LNA_info->cur_sat_status) {
		if(!pDM_Odm->is_disable_gain_table_switch)
			phydm_set_ofdm_agc_tab(pDM_Odm, OFDM_AGC_TAB_2);
		pDM_LNA_info->check_time = 0;
		pDM_LNA_info->sat_cnt_acc_patha = 0;
		pDM_LNA_info->sat_cnt_acc_pathb = 0;
		pDM_LNA_info->pre_sat_status = pDM_LNA_info->cur_sat_status;

	} else if (pDM_LNA_info->check_time <= (max_check_time - 1)) {
		if (pDM_LNA_info->pre_sat_status && (!pDM_Odm->is_disable_gain_table_switch))
			phydm_set_ofdm_agc_tab(pDM_Odm, OFDM_AGC_TAB_2);
		pDM_LNA_info->check_time++;

	} else if (pDM_LNA_info->check_time == max_check_time) {
		if(!pDM_Odm->is_disable_gain_table_switch && (pDM_LNA_info->pre_sat_status == 1))
			phydm_set_ofdm_agc_tab(pDM_Odm, OFDM_AGC_TAB_0);
		pDM_LNA_info->check_time = 0;
		pDM_LNA_info->sat_cnt_acc_patha = 0;
		pDM_LNA_info->sat_cnt_acc_pathb = 0;
		pDM_LNA_info->pre_sat_status = pDM_LNA_info->cur_sat_status;
	}

	agc_tab = phydm_get_ofdm_agc_tab(pDM_Odm);
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD, ("use AGC table %d\n",	agc_tab));
	//func_end = ODM_GetBBReg(pDM_Odm, 0x560, bMaskDWord);

	//ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("function process time=%d\n",
	//	func_end - func_start));

	// restore previous igi
	ODM_Write_DIG(pDM_VOID, igi_restore);
	
	pDM_LNA_info->cur_timer_check_cnt++;
	ODM_SetTimer(pDM_Odm, &pDM_LNA_info->phydm_lna_sat_chk_timer, pDM_Odm->lna_sat_chk_period_ms);
}

void
phydm_lna_sat_chk_callback(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_LNA_SAT_CHK, ODM_DBG_LOUD, ("\n%s ==>\n", __FUNCTION__));
	phydm_lna_sat_chk(pDM_Odm);
}

void
phydm_lna_sat_chk_timers(
	IN		PVOID		pDM_VOID,
	u8		state
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PLNA_SATURATION_INFO	pDM_LNA_info = &pDM_Odm->DM_LNA_SAT_INFO;

	if (state == INIT_LNA_SAT_CHK_TIMMER) {
		ODM_InitializeTimer(pDM_Odm, &(pDM_LNA_info->phydm_lna_sat_chk_timer),
			(RT_TIMER_CALL_BACK)phydm_lna_sat_chk_callback, NULL, "phydm_lna_sat_chk_timer");
	} else if (state == CANCEL_LNA_SAT_CHK_TIMMER) {
		ODM_CancelTimer(pDM_Odm, &(pDM_LNA_info->phydm_lna_sat_chk_timer));
	} else if (state == RELEASE_LNA_SAT_CHK_TIMMER) {
		ODM_ReleaseTimer(pDM_Odm, &(pDM_LNA_info->phydm_lna_sat_chk_timer));
	}
}

#if (PHYDM_TDMA_DIG_SUPPORT == 1)

void
phydm_reset_state_cnt(
	IN		PVOID			pDM_VOID,
	bool					is_high_low_sel
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;

	pDM_DigTable->stay_check_state_cnt[is_high_low_sel] = 0;
	pDM_DigTable->goto_lock_state_cnt[is_high_low_sel] = 0;
	pDM_DigTable->goto_release_state_cnt[is_high_low_sel] = 0;
	pDM_DigTable->stay_release_state_cnt[is_high_low_sel] = 0;
}

u1Byte
phydm_rx_gain_min_check(
	IN		PVOID			pDM_VOID,
	IN		u1Byte			dym_min,
	IN		u1Byte			CurrentIGI,
	IN		u1Byte			rx_gain_min,
	bool					is_high_low_sel
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	PFALSE_ALARM_STATISTICS_ACC pFalseAlmCntAcc = &(pDM_Odm->FalseAlmCnt_Acc);
	u4Byte						fa_cca_ratio = pFalseAlmCntAcc->fa_cca_ratio[is_high_low_sel];
	u1Byte						ori_check_state = pDM_DigTable->rx_gain_min_check_state[is_high_low_sel];
//	u1Byte						ori_goto_lock_state_cnt = pDM_DigTable->goto_lock_state_cnt[is_high_low_sel];
	bool						is_enough_fa = (pFalseAlmCntAcc->Cnt_all_1sec[is_high_low_sel] >= pDM_Odm->tdma_is_enough_fa_th) ? 1 : 0;
	bool						high_fa_cca_ratio;
	bool						low_fa_cca_ratio;
	/*rgm = rx_gain_min*/
	bool						is_igi_less_eq_rgm = (CurrentIGI <= rx_gain_min) ? 1 : 0;
	u4Byte						debug_component;

	if (pDM_Odm->is_disable_rx_gain_min_check) {
			phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
			pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_CHECK_STATE;
			return dym_min;
		}

	if (is_high_low_sel) {
		high_fa_cca_ratio = ((fa_cca_ratio >= pDM_Odm->tdma_high_fa_cca_ratio_l2h) && (is_enough_fa==1)) ? 1 : 0;
		low_fa_cca_ratio = ((fa_cca_ratio <= pDM_Odm->tdma_high_fa_cca_ratio_h2l) || (is_enough_fa==0)) ? 1 : 0;

		if (pDM_Odm->rx_gain_min_dbg_switch == TDMA_DIG_HIGH_STATE)
			debug_component = ODM_COMP_TDMA_HDIG;
		else
			debug_component = 0;
	} else {
		high_fa_cca_ratio = ((fa_cca_ratio >= pDM_Odm->tdma_low_fa_cca_ratio_l2h) && (is_enough_fa==1)) ? 1 : 0;
		low_fa_cca_ratio = ((fa_cca_ratio <= pDM_Odm->tdma_low_fa_cca_ratio_h2l) || (is_enough_fa==0)) ? 1 : 0;
		if (pDM_Odm->rx_gain_min_dbg_switch == TDMA_DIG_LOW_STATE)
			debug_component = ODM_COMP_TDMA_LDIG;
		else
			debug_component = 0;
	}

	ODM_RT_TRACE(pDM_Odm, debug_component, ODM_DBG_LOUD,
		("dym_min=0x%x,CurrentIGI=0x%x,rx_gain_min=0x%x,fa_cca_ratio=%d,is_high_low_sel=%d,is_enough_fa=%d\n",
		dym_min,
		CurrentIGI,
		rx_gain_min,
		fa_cca_ratio,
		is_high_low_sel,
		is_enough_fa));

	ODM_RT_TRACE(pDM_Odm, debug_component, ODM_DBG_LOUD,
		("check_state=%d,stay_check_state_cnt=%d,goto_lock_cnt=%d,goto_release_cnt=%d,stay_release_state_cnt=%d\n",
		pDM_DigTable->rx_gain_min_check_state[is_high_low_sel],
		pDM_DigTable->stay_check_state_cnt[is_high_low_sel],
		pDM_DigTable->goto_lock_state_cnt[is_high_low_sel],
		pDM_DigTable->goto_release_state_cnt[is_high_low_sel],
		pDM_DigTable->stay_release_state_cnt[is_high_low_sel]));

	if (ori_check_state == OSC_CHECK_STATE) {

		if (high_fa_cca_ratio) {

			if (pDM_DigTable->goto_lock_state_cnt[is_high_low_sel] > 0)
				pDM_DigTable->goto_lock_state_cnt[is_high_low_sel]--;

			pDM_DigTable->stay_check_state_cnt[is_high_low_sel]++;

			if (pDM_DigTable->stay_check_state_cnt[is_high_low_sel] >= 3) {
				rx_gain_min = rx_gain_min + 2;
				phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
				pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_CHECK_STATE;
				ODM_RT_TRACE(pDM_Odm, debug_component, ODM_DBG_LOUD, ("stay check_state.\n"));
			}
		} else if (low_fa_cca_ratio) {

			/*"if (CurrentIGI <= rx_gain_min)" is to avoid osc.*/

			if (is_igi_less_eq_rgm) {
				if (pDM_DigTable->stay_check_state_cnt[is_high_low_sel] > 0)
					pDM_DigTable->stay_check_state_cnt[is_high_low_sel]--;

				pDM_DigTable->goto_lock_state_cnt[is_high_low_sel]++;
				if (pDM_DigTable->goto_lock_state_cnt[is_high_low_sel] >= 3) {
					rx_gain_min = rx_gain_min;
					phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
					pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_LOCK_STATE;					
				}
			}
		}
	}
	else if (ori_check_state == OSC_LOCK_STATE) {
		/*rgm = rx_gain_min*/
		if (is_igi_less_eq_rgm) {
			if (high_fa_cca_ratio) {
				rx_gain_min = rx_gain_min + 2;
				phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
				pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_CHECK_STATE;
			} else if (low_fa_cca_ratio) {
				if ((rx_gain_min - 2) >= dym_min) {
					pDM_DigTable->goto_release_state_cnt[is_high_low_sel]++;
					if (pDM_DigTable->goto_release_state_cnt[is_high_low_sel] >= 6) {
						
						/*translate to RELEASE_STATE*/
						rx_gain_min = rx_gain_min - 2;
						phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
						pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_RELEASE_STATE;
					}
				} else {
					rx_gain_min = dym_min;
					phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
					pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_LOCK_STATE;					
				}
			}
		}
		else {
			/*not update rx_gain_min, back to check state to find suitable one.*/
			phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
			pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_CHECK_STATE;
		}
	}

	else if (ori_check_state == OSC_RELEASE_STATE) {
		/*rgm = rx_gain_min*/
		if (is_igi_less_eq_rgm) {
			if (low_fa_cca_ratio) {

				pDM_DigTable->stay_release_state_cnt[is_high_low_sel]++;

				if (pDM_DigTable->stay_release_state_cnt[is_high_low_sel] >= 3) {
					if ((rx_gain_min - 2) >= dym_min) {
						/*test success, keep trying lower rx_gain_min*/
						rx_gain_min = rx_gain_min - 2;
						phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
						pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_RELEASE_STATE;					
					} else {
						rx_gain_min= dym_min;
						phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
						pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_LOCK_STATE;
					}
				}
			} else {
				/*test fail, resume rx_gain_min*/
				rx_gain_min = rx_gain_min + 2;
				phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
				pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_LOCK_STATE;
			}
		}
		else {
			/*wait igi comes down*/
			if (!low_fa_cca_ratio) {
				rx_gain_min = rx_gain_min + 2;
				phydm_reset_state_cnt(pDM_Odm, is_high_low_sel);
				pDM_DigTable->rx_gain_min_check_state[is_high_low_sel] = OSC_CHECK_STATE;
			}
		}
	}

	ODM_RT_TRACE(pDM_Odm, debug_component, ODM_DBG_LOUD,
		("check_state=%d,stay_check_state_cnt=%d,goto_lock_cnt=%d,goto_release_cnt=%d,stay_release_state_cnt=%d\n",
		pDM_DigTable->rx_gain_min_check_state[is_high_low_sel],
		pDM_DigTable->stay_check_state_cnt[is_high_low_sel],
		pDM_DigTable->goto_lock_state_cnt[is_high_low_sel],
		pDM_DigTable->goto_release_state_cnt[is_high_low_sel],
		pDM_DigTable->stay_release_state_cnt[is_high_low_sel]));

	ODM_RT_TRACE(pDM_Odm, debug_component, ODM_DBG_LOUD,
		("rx_gain_min=0x%x\n", rx_gain_min));
	
	return rx_gain_min;

}

void
phydm_tdma_dig_timers(
	IN		PVOID		pDM_VOID,
	u8		state
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;

//	if (pDM_Odm->original_dig_restore) 
	{
		if (state == INIT_TDMA_DIG_TIMMER) {
			ODM_InitializeTimer(pDM_Odm, &(pDM_DigTable->phydm_tdma_dig_timer),
				(RT_TIMER_CALL_BACK)phydm_tdma_dig_call_back, NULL, "phydm_tdma_dig_timer");
		} else if (state == CANCEL_TDMA_DIG_TIMMER) {
			ODM_CancelTimer(pDM_Odm, &(pDM_DigTable->phydm_tdma_dig_timer));
		} else if (state == RELEASE_TDMA_DIG_TIMMER) {
			ODM_ReleaseTimer(pDM_Odm, &(pDM_DigTable->phydm_tdma_dig_timer));
		}
	}
}

void 
phydm_tdma_low_dig(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	// Common parameters
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	u1Byte						tdma_low_igi = pDM_DigTable->tdma_igi[TDMA_DIG_LOW_STATE];
	u1Byte						DIG_MaxOfMin;
	u1Byte						tdma_low_dym_min;
	u1Byte						dm_dig_max, dm_dig_min;
	u1Byte						offset;
	u1Byte						rssi_min = pDM_Odm->RSSI_Min;
	u1Byte						igi_upper_rssi_min;
	u4Byte						dm_FA_thres[3];
	u4Byte						TxTp = 0, RxTp = 0;
	BOOLEAN						bDFSBand = FALSE;
	BOOLEAN						bPerformance = TRUE;
	BOOLEAN						DIG_GoUpCheck = TRUE;
	u1Byte						step_size[3] = {0};

	if(odm_DigAbort(pDM_Odm) == TRUE)
		return;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD, ("%s ==>\n",__FUNCTION__));

	step_size[2] = 2;
	step_size[1] = 2;
	step_size[0] = 2;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD, ("step_size[0] = %d, step_size[1] = %d, step_size[2] = %d\n",
		step_size[0], step_size[1], step_size[2]));	

	//1 Mode decision

	bPerformance = TRUE;

	//1 Boundary Decision
	{
		dm_dig_max = DM_DIG_MAX_AP_COVERAGR;
		dm_dig_min = DM_DIG_MIN_AP_COVERAGE;
		DIG_MaxOfMin = DM_DIG_MIN_AP_COVERAGE;

		/* Modify lower bound for DFS band */
		if ((((*pDM_Odm->pChannel >= 52) && (*pDM_Odm->pChannel <= 64)) ||
			((*pDM_Odm->pChannel >= 100) && (*pDM_Odm->pChannel <= 140)))
		) {
			bDFSBand = TRUE;
			if (*pDM_Odm->pBandWidth == ODM_BW20M)
				dm_dig_min = DM_DIG_MIN_AP_DFS+2;
			else
				dm_dig_min = DM_DIG_MIN_AP_DFS;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD, ("In DFS band\n"));
		}
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD, ("abs UB=0x%x,LB=0x%x\n", dm_dig_max, dm_dig_min));

	//1 Adjust boundary by RSSI
	if(pDM_Odm->bLinked && bPerformance)
	{
		//2 Modify DIG upper bound

		offset = 15;

		igi_upper_rssi_min = rssi_min + offset;

		/*rx_gain_range_max*/
		if((igi_upper_rssi_min) > dm_dig_max)
			pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE] = dm_dig_max;
		else if((igi_upper_rssi_min) < dm_dig_min)
			pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE] = dm_dig_min;
		else
			pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE] = igi_upper_rssi_min;

		if (bDFSBand)
		{
			tdma_low_dym_min = dm_dig_min;
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD, ("DFS band: Force LB to 0x%x after link\n", dm_dig_min));
		} else {
			tdma_low_dym_min = dm_dig_min;
		}
	}
	else
	{
		if(bPerformance && bDFSBand)
		{
			pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE] = 0x26;
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD,
				("DFS band:Force upper bound to 0x%x before link.\n", pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE]));
		}
		else
		{
			pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE] = 0x26;
		}
		tdma_low_dym_min = dm_dig_min;
	}
	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD, ("tdma_rx_gain_max=0x%x, tdma_low_dym_min=0x%x\n",
		pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE], tdma_low_dym_min));	

	//1 Modify DIG lower bound, deal with abnormal case
	//2 Abnormal false alarm case
	if(!pDM_Odm->bLinked)
	{
		pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_LOW_STATE] = tdma_low_dym_min;
	}
	else {
			pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_LOW_STATE]
				= phydm_rx_gain_min_check(pDM_Odm, tdma_low_dym_min, tdma_low_igi, pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_LOW_STATE], TDMA_DIG_LOW_STATE);
	}

	//2 Abnormal lower bound case
	/*tdma_high_rx_gain_min*/
	if(pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_LOW_STATE] > pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE])
	{
		pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_LOW_STATE] = pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE];
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD, ("Abnrormal tdma_rx_gain_min: force to 0x%x\n\n",
			pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_LOW_STATE]));
	}
	
	//1 False alarm threshold decision
	odm_FAThresholdCheck(pDM_Odm, bDFSBand, bPerformance, RxTp, TxTp, dm_FA_thres);
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD,
		("odm_DIG(): False alarm threshold = %d, %d, %d \n\n",
		dm_FA_thres[0], dm_FA_thres[1], dm_FA_thres[2]));

	//1 Adjust initial gain by false alarm
	if(pDM_Odm->bLinked && bPerformance)
	{
		//2 After link
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD, ("Adjust IGI after link\n"));
		tdma_low_igi = phydm_dig_current_igi_by_fa_th(pDM_Odm, tdma_low_igi, dm_FA_thres, step_size, DIG_GoUpCheck, TDMA_DIG_LOW_STATE);			

	} else {
		//2 Before link
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD, ("Adjust IGI before link\n"));
		
		tdma_low_igi = phydm_dig_current_igi_by_fa_th(pDM_Odm, tdma_low_igi, dm_FA_thres, step_size, DIG_GoUpCheck, TDMA_DIG_LOW_STATE);
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD,
		("before check igi by ULbond, tdma_low_igi=0x%x\n\n", tdma_low_igi));

	//1 Check initial gain by upper/lower bound
	pDM_DigTable->tdma_igi[TDMA_DIG_LOW_STATE] = phydm_igi_chkby_upper_lower_bond(tdma_low_igi, pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE], pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_LOW_STATE]);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD,
		("ULbond,tdma_rx_gain_max=0x%x, tdma_rx_gain_min=0x%x\n",
		pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_LOW_STATE],
		pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_LOW_STATE]
		));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_LDIG, ODM_DBG_LOUD,
		("after ULbond check, tdma_igi=0x%x\n",
		pDM_DigTable->tdma_igi[TDMA_DIG_LOW_STATE]
		));

	//1 Update status

	if (pDM_Odm->force_low_igi == 0xff)
		ODM_Write_DIG(pDM_Odm, pDM_DigTable->tdma_igi[TDMA_DIG_LOW_STATE]);
	else
		ODM_Write_DIG(pDM_Odm, pDM_Odm->force_low_igi);
}



VOID 
phydm_tdma_high_dig(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	prtl8192cd_priv				priv = pDM_Odm->priv;
	// Common parameters
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;	
	u1Byte						tdma_high_igi = pDM_DigTable->tdma_igi[TDMA_DIG_HIGH_STATE];
	u1Byte						DIG_MaxOfMin;
	u1Byte						tdma_high_dym_min;
	u1Byte						dm_dig_max, dm_dig_min;
	u1Byte						offset;
	u1Byte						rssi_min = pDM_Odm->RSSI_Min;
	u1Byte						igi_upper_rssi_min;
	u4Byte						dm_FA_thres[3];
	u4Byte						TxTp = 0, RxTp = 0;
	BOOLEAN						bDFSBand = FALSE;
	BOOLEAN						bPerformance = TRUE;
	BOOLEAN						DIG_GoUpCheck = TRUE;
	u1Byte						step_size[3] = {0};
	
	if(odm_DigAbort(pDM_Odm) == TRUE)
		return;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("%s ==>\n\n",__FUNCTION__));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("tdma_high_igi=0x%x\n", tdma_high_igi));

	step_size[2] = 2;
	step_size[1] = 2;
	step_size[0] = 2;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("step_size[0] = %d, step_size[1] = %d, step_size[2] = %d\n",
		step_size[0], step_size[1], step_size[2]));	

	//1 Mode decision

	bPerformance = TRUE;

	//1 Boundary Decision
	{
		if(!bPerformance)
		{
			dm_dig_max = DM_DIG_MAX_AP_COVERAGR;
			dm_dig_min = DM_DIG_MIN_AP_COVERAGE;
			DIG_MaxOfMin = DM_DIG_MIN_AP_COVERAGE;
		}
		else
		{
			if (pDM_Odm->RFType == ODM_1T1R)
				dm_dig_max = DM_DIG_MAX_AP - 6;
			else
				dm_dig_max = DM_DIG_MAX_AP;

			if ((*pDM_Odm->pBandType == ODM_BAND_2_4G) && (pDM_Odm->SupportICType & ODM_RTL8814A)) /* for 2G 8814 */
				dm_dig_min = 0x1c;
			else if (pDM_Odm->SupportICType & ODM_RTL8197F)		/*refine by YuChen 20160815 asked by RF Arthur*/
				dm_dig_min = 0x1e;
			else
				dm_dig_min = DM_DIG_MIN_AP;

			DIG_MaxOfMin = DM_DIG_MIN_AP_COVERAGE;
		}
		
		//4 TX2path
		if (priv->pmib->dot11RFEntry.tx2path && !bDFSBand && (*(pDM_Odm->pWirelessMode) == ODM_WM_B))
				dm_dig_max = 0x2A;

#if RTL8192E_SUPPORT
#ifdef HIGH_POWER_EXT_LNA
		if ((pDM_Odm->SupportICType & (ODM_RTL8192E)) && (pDM_Odm->ExtLNA))
			dm_dig_max = 0x42;
#endif
#endif
		/* Modify lower bound for DFS band */
		if ((((*pDM_Odm->pChannel >= 52) && (*pDM_Odm->pChannel <= 64)) ||
			((*pDM_Odm->pChannel >= 100) && (*pDM_Odm->pChannel <= 140)))
			#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
			&& phydm_dfs_master_enabled(pDM_Odm) == TRUE
			#endif
		) {
			bDFSBand = TRUE;
			if (*pDM_Odm->pBandWidth == ODM_BW20M)
				dm_dig_min = DM_DIG_MIN_AP_DFS+2;
			else
				dm_dig_min = DM_DIG_MIN_AP_DFS;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("====== In DFS band ======\n"));
		}
	}
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("Absolutly upper bound = 0x%x, lower bound = 0x%x\n",dm_dig_max, dm_dig_min));

	//1 Adjust boundary by RSSI
	if(pDM_Odm->bLinked && bPerformance)
	{
		//2 Modify DIG upper bound

		//offset = 15;

		offset = pDM_Odm->rssi_to_igi_upper_offset;
		
		igi_upper_rssi_min = rssi_min + offset;

		/*tdma_high_rx_gain_max*/
		if((igi_upper_rssi_min) < dm_dig_min)
			pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE] = dm_dig_min;
		else
			pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE]  = igi_upper_rssi_min;

		if (bDFSBand)
		{
			tdma_high_dym_min = dm_dig_min;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("DFS band: Force lower bound to 0x%x after link\n", dm_dig_min));
		} else {
			/*tdma_high_dym_min(tdma_high_rx_gain_min)*/
			if(rssi_min < dm_dig_min)
				tdma_high_dym_min = dm_dig_min;
			else {
				tdma_high_dym_min = (pDM_Odm->tdma_dig_turbo == 1) ? rssi_min : dm_dig_min;
			}
		}
	}
	else
	{
		if(bPerformance && bDFSBand)
		{
			pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE]  = 0x28;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD,
				("DFS band:Force upper bound to 0x%x before link.\n", pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE] ));
		}
		else
		{
			pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE]  = DM_DIG_MAX_OF_MIN;
		}
		tdma_high_dym_min = dm_dig_min;
	}
	
	//1 Force Lower Bound for AntDiv
	if(pDM_Odm->bLinked && !pDM_Odm->bOneEntryOnly)
	{
		if((pDM_Odm->SupportICType & ODM_ANTDIV_SUPPORT) && (pDM_Odm->SupportAbility & ODM_BB_ANT_DIV))
		{
			if (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV || pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV) {
				if (pDM_DigTable->AntDiv_RSSI_max > DIG_MaxOfMin) {
					tdma_high_dym_min = DIG_MaxOfMin;
				}
				else {
					tdma_high_dym_min = (u1Byte) pDM_DigTable->AntDiv_RSSI_max;
				}
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("Antenna diversity case: Force lower bound to 0x%x\n",
					tdma_high_dym_min));
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("Antenna diversity case: RSSI_max=0x%x\n",
					pDM_DigTable->AntDiv_RSSI_max));
			}
		}
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD,
		("tdma_high_rx_gain_max=0x%x, tdma_high_dym_min = 0x%x\n",
		pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE] , tdma_high_dym_min));

	//1 Modify DIG lower bound, deal with abnormal case
	//2 Abnormal false alarm case
	if(!pDM_Odm->bLinked)
	{
		pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_HIGH_STATE] = tdma_high_dym_min;
	}
	else {
			pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_HIGH_STATE]
				= phydm_rx_gain_min_check(pDM_Odm, tdma_high_dym_min, tdma_high_igi, pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_HIGH_STATE], TDMA_DIG_HIGH_STATE);
	}

	//2 Abnormal lower bound case
	/*tdma_high_rx_gain_min*/
	if(pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_HIGH_STATE] > pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE] )
	{
		pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_HIGH_STATE] = pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE] ;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("Abnrormal tdma_high_rx_gain_min: force to 0x%x\n\n",
			pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_HIGH_STATE]));
	}
	
	//1 False alarm threshold decision
	odm_FAThresholdCheck(pDM_Odm, bDFSBand, bPerformance, RxTp, TxTp, dm_FA_thres);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("False alarm threshold = %d, %d, %d \n\n",
		dm_FA_thres[0], dm_FA_thres[1], dm_FA_thres[2]));

	//1 Adjust initial gain by false alarm
	if(pDM_Odm->bLinked && bPerformance)
	{
		//2 After link
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("Adjust IGI after link\n"));

		tdma_high_igi = phydm_dig_current_igi_by_fa_th(pDM_Odm, tdma_high_igi, dm_FA_thres, step_size, DIG_GoUpCheck, TDMA_DIG_HIGH_STATE);
	} else {
		//2 Before link
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, ("odm_DIG(): Adjust IGI before link\n"));
		
		tdma_high_igi = phydm_dig_current_igi_by_fa_th(pDM_Odm, tdma_high_igi, dm_FA_thres, step_size, DIG_GoUpCheck, TDMA_DIG_HIGH_STATE);
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD,
		("before check igi by ULbond, tdma_high_igi=0x%x\n\n",tdma_high_igi));

	//1 Check initial gain by upper/lower bound
	pDM_DigTable->tdma_igi[TDMA_DIG_HIGH_STATE] = phydm_igi_chkby_upper_lower_bond(tdma_high_igi, pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE] , pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_HIGH_STATE]);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD,
		("ULbond, tdma, high_rx_gain_max=0x%x high_rx_gain_min=0x%x\n",
		pDM_DigTable->tdma_rx_gain_max[TDMA_DIG_HIGH_STATE] ,
		pDM_DigTable->tdma_rx_gain_min[TDMA_DIG_HIGH_STATE]
		));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD,
		("after ULbond check,tdma_high_igi=0x%x\n", pDM_DigTable->tdma_igi[TDMA_DIG_HIGH_STATE]));

}

void
phydm_tdma_dig_timer_check(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	prtl8192cd_priv				priv = pDM_Odm->priv;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;

	if ((odm_DigAbort(pDM_Odm) == TRUE) || (pDM_Odm->original_dig_restore == 1))
		return;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_DIG_CALL_BACK, ODM_DBG_LOUD,
		("tdma_dig_cnt=%d, pre_tdma_dig_cnt=%d, tdma_dig_timer_ms=%d\n",
		pDM_DigTable->tdma_dig_cnt,
		pDM_DigTable->pre_tdma_dig_cnt,
		pDM_Odm->tdma_dig_timer_ms));

	if((pDM_DigTable->tdma_dig_cnt == 0) || (pDM_DigTable->tdma_dig_cnt == pDM_DigTable->pre_tdma_dig_cnt)) {

		if(ODMPTR->SupportAbility & ODM_BB_DIG) {
			
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_DIG_CALL_BACK, ODM_DBG_LOUD,
			("\ntdma dig timer Check fail, restart tdma_dig_timer!\n\n"));
			
			ODM_SetTimer(pDM_Odm, &pDM_DigTable->phydm_tdma_dig_timer, pDM_Odm->tdma_dig_timer_ms);
		}
	} else
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_DIG_CALL_BACK, ODM_DBG_LOUD, ("Check pass, update pre_tdma_dig_cnt\n"));	
	
	pDM_DigTable->pre_tdma_dig_cnt = pDM_DigTable->tdma_dig_cnt;
}

void
phydm_tdma_dig_call_back(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	u1Byte						reg_c50;
	u1Byte						total_state_number = pDM_Odm->total_state_number;

	if((odm_DigAbort(pDM_Odm) == TRUE) || (pDM_Odm->original_dig_restore == TRUE))
		return;

	// prevent dumb
	if((total_state_number == 0) || (total_state_number == 1)) {
		pDM_Odm->total_state_number = 2;
		total_state_number = pDM_Odm->total_state_number;
	}

	reg_c50 = ODM_GetBBReg(pDM_Odm, 0xc50, bMaskByte0);
	
	pDM_DigTable->tdma_dig_state = (pDM_DigTable->tdma_dig_cnt % total_state_number);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_DIG_CALL_BACK, ODM_DBG_LOUD,
		("tdma_dig_state=%d, regc50=0x%x, tdma_high_igi=0x%x, tdma_low_igi=0x%x\n",
		pDM_DigTable->tdma_dig_state,
		reg_c50,
		pDM_DigTable->tdma_igi[TDMA_DIG_HIGH_STATE],
		pDM_DigTable->tdma_igi[TDMA_DIG_LOW_STATE]));

	pDM_DigTable->tdma_dig_cnt++;

	if (pDM_DigTable->tdma_dig_state == 1) {

		if (pDM_Odm->force_high_igi == 0xff)
			ODM_Write_DIG(pDM_Odm, pDM_DigTable->tdma_igi[TDMA_DIG_HIGH_STATE]);
		else
			ODM_Write_DIG(pDM_Odm, pDM_Odm->force_high_igi);
	} else if (pDM_DigTable->tdma_dig_state == 0) {

		if (pDM_Odm->force_low_igi == 0xff)
			ODM_Write_DIG(pDM_Odm, pDM_DigTable->tdma_igi[TDMA_DIG_LOW_STATE]);
		else
			ODM_Write_DIG(pDM_Odm, pDM_Odm->force_low_igi);
	}
	phydm_TDMA_FalseAlarmCounterCheck(pDM_Odm);
	ODM_SetTimer(pDM_Odm, &pDM_DigTable->phydm_tdma_dig_timer, pDM_Odm->tdma_dig_timer_ms);
}

VOID 
phydm_TDMA_FalseAlarmCounterCheck(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	BOOLEAN						one_sec_dump = pDM_DigTable->one_sec_dump;
	u1Byte						total_state_number = pDM_Odm->total_state_number;
	u1Byte						tdma_dig_state = pDM_DigTable->tdma_dig_state;
	u1Byte						i = 0;
	u4Byte						tdma_dig_block_period_ms = pDM_Odm->tdma_dig_timer_ms * total_state_number;

	/*caution!ONE_SEC_MS must be divisible by tdma_dig_block_period_ms, or FA will be fewer.*/ 
	u4Byte						tdma_dig_block_cnt_thd = ONE_SEC_MS / tdma_dig_block_period_ms;
	/*tdma_dig_state == 1, collect L-state FA, else, collect H-state FA*/
	bool						is_high_low_sel = (tdma_dig_state == TDMA_DIG_HIGH_STATE)? 0: 1;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TDMA_DIG_CALL_BACK, ODM_DBG_LOUD,
		("tdma_dig_state=%d,tdma_dig_block_cnt=%d,one_sec_dump=%d,tdma_dig_block_cnt_thd=%d\n",
		tdma_dig_state,
		pDM_DigTable->tdma_dig_block_cnt,
		pDM_DigTable->one_sec_dump,
		tdma_dig_block_cnt_thd
		));
	
	if (tdma_dig_state == 0) {
		
		odm_FalseAlarmCounterStatistics(pDM_Odm);
		/*collect to high/low FA by is_high_low_sel*/		
		odm_FalseAlarmCounterAcc(pDM_Odm, one_sec_dump, is_high_low_sel);
		odm_FalseAlarmCounterReset(pDM_Odm);

		if (one_sec_dump) { /*1sec H-state FA collect end*/
			
			phydm_NoisyDetection(pDM_Odm);
			odm_CCKPacketDetectionThresh(pDM_Odm);
			// tdma dig
			phydm_tdma_high_dig(pDM_Odm);
			phydm_tdma_low_dig(pDM_Odm);
			
			for(i=0;i<=1;i++)
				odm_FalseAlarmCounterAccReset(pDM_Odm, i);
			pDM_DigTable->one_sec_dump = 0;
			pDM_DigTable->tdma_dig_block_cnt = 0;
		}
		pDM_DigTable->tdma_dig_block_cnt++;

		/*1sec dump decision*/
		if (pDM_DigTable->tdma_dig_block_cnt >= tdma_dig_block_cnt_thd){
			pDM_DigTable->one_sec_dump = 1; /*collect FA till this block end*/
		}
	}else if (tdma_dig_state == 1) {
		odm_FalseAlarmCounterStatistics(pDM_Odm);
		/*collect to high/low FA by is_high_low_sel*/		
		odm_FalseAlarmCounterAcc(pDM_Odm, one_sec_dump, is_high_low_sel);
		odm_FalseAlarmCounterReset(pDM_Odm);
	}
}

VOID 
odm_FalseAlarmCounterAcc(
	IN		PVOID		pDM_VOID,
	IN 		BOOLEAN		one_sec_dump_en,
	bool				is_high_low_sel
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PFALSE_ALARM_STATISTICS 	FalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	PFALSE_ALARM_STATISTICS_ACC	FalseAlmCnt_Acc = &(pDM_Odm->FalseAlmCnt_Acc);
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
//	u4Byte						ret_value;
	u8							one_sec_numerator, one_sec_denumerator;
	u8							total_state_number = pDM_Odm->total_state_number;
	u32							Cnt_CCA_all_1sec, Cnt_all_1sec;
	
	FalseAlmCnt_Acc->Cnt_Parity_Fail[is_high_low_sel] += FalseAlmCnt->Cnt_Parity_Fail;
	FalseAlmCnt_Acc->Cnt_Rate_Illegal[is_high_low_sel] += FalseAlmCnt->Cnt_Rate_Illegal;
	FalseAlmCnt_Acc->Cnt_Crc8_fail[is_high_low_sel] += FalseAlmCnt->Cnt_Crc8_fail;
	FalseAlmCnt_Acc->Cnt_Mcs_fail[is_high_low_sel] += FalseAlmCnt->Cnt_Mcs_fail;
	FalseAlmCnt_Acc->Cnt_Ofdm_fail[is_high_low_sel] += FalseAlmCnt->Cnt_Ofdm_fail;
//	FalseAlmCnt_Acc->Cnt_Ofdm_fail_pre += FalseAlmCnt->Cnt_Parity_Fail;
	FalseAlmCnt_Acc->Cnt_Cck_fail[is_high_low_sel] += FalseAlmCnt->Cnt_Cck_fail;
	FalseAlmCnt_Acc->Cnt_all[is_high_low_sel] += FalseAlmCnt->Cnt_all;
//	FalseAlmCnt_Acc->Cnt_all_pre += FalseAlmCnt->Cnt_all_pre;
	FalseAlmCnt_Acc->Cnt_Fast_Fsync[is_high_low_sel] += FalseAlmCnt->Cnt_Fast_Fsync;
	FalseAlmCnt_Acc->Cnt_SB_Search_fail[is_high_low_sel] += FalseAlmCnt->Cnt_SB_Search_fail;
	FalseAlmCnt_Acc->Cnt_OFDM_CCA[is_high_low_sel] += FalseAlmCnt->Cnt_OFDM_CCA;
	FalseAlmCnt_Acc->Cnt_CCK_CCA[is_high_low_sel] += FalseAlmCnt->Cnt_CCK_CCA;
	FalseAlmCnt_Acc->Cnt_CCA_all[is_high_low_sel] += FalseAlmCnt->Cnt_CCA_all;
//	FalseAlmCnt_Acc->Cnt_BW_USC += FalseAlmCnt->Cnt_BW_USC;
//	FalseAlmCnt_Acc->Cnt_BW_LSC += FalseAlmCnt->Cnt_BW_LSC;
	FalseAlmCnt_Acc->cnt_cck_crc32_error[is_high_low_sel] += FalseAlmCnt->cnt_cck_crc32_error;
	FalseAlmCnt_Acc->cnt_cck_crc32_ok[is_high_low_sel] += FalseAlmCnt->cnt_cck_crc32_ok;
	FalseAlmCnt_Acc->cnt_ofdm_crc32_error[is_high_low_sel] += FalseAlmCnt->cnt_ofdm_crc32_error;
	FalseAlmCnt_Acc->cnt_ofdm_crc32_ok[is_high_low_sel] += FalseAlmCnt->cnt_ofdm_crc32_ok;
	FalseAlmCnt_Acc->cnt_ht_crc32_error[is_high_low_sel] += FalseAlmCnt->cnt_ht_crc32_error;
	FalseAlmCnt_Acc->cnt_ht_crc32_ok[is_high_low_sel] += FalseAlmCnt->cnt_ht_crc32_ok;
	FalseAlmCnt_Acc->cnt_vht_crc32_error[is_high_low_sel] += FalseAlmCnt->cnt_vht_crc32_error;
	FalseAlmCnt_Acc->cnt_vht_crc32_ok[is_high_low_sel] += FalseAlmCnt->cnt_vht_crc32_ok;
	FalseAlmCnt_Acc->cnt_crc32_error_all[is_high_low_sel] += FalseAlmCnt->cnt_crc32_error_all;
	FalseAlmCnt_Acc->cnt_crc32_ok_all[is_high_low_sel] += FalseAlmCnt->cnt_crc32_ok_all;

	if(one_sec_dump_en == 1) {
		
		if(is_high_low_sel == TDMA_DIG_HIGH_STATE) {
			one_sec_numerator = total_state_number;
			one_sec_denumerator = total_state_number-1;
		} else {
			one_sec_numerator = total_state_number;
			one_sec_denumerator = 1;
		}
		FalseAlmCnt_Acc->Cnt_all_1sec[is_high_low_sel] = FalseAlmCnt_Acc->Cnt_all[is_high_low_sel] * one_sec_numerator / one_sec_denumerator;
		FalseAlmCnt_Acc->Cnt_CCA_all_1sec[is_high_low_sel] = FalseAlmCnt_Acc->Cnt_CCA_all[is_high_low_sel] * one_sec_numerator / one_sec_denumerator;
		FalseAlmCnt_Acc->Cnt_Cck_fail_1sec[is_high_low_sel] = FalseAlmCnt_Acc->Cnt_Cck_fail[is_high_low_sel] * one_sec_numerator / one_sec_denumerator;
		FalseAlmCnt_Acc->Cnt_Ofdm_fail_1sec[is_high_low_sel] = FalseAlmCnt_Acc->Cnt_Ofdm_fail[is_high_low_sel] * one_sec_numerator / one_sec_denumerator;

		Cnt_all_1sec = FalseAlmCnt_Acc->Cnt_all_1sec[is_high_low_sel];
		Cnt_CCA_all_1sec = FalseAlmCnt_Acc->Cnt_CCA_all_1sec[is_high_low_sel];

		if (Cnt_CCA_all_1sec != 0)
			FalseAlmCnt_Acc->fa_cca_ratio[is_high_low_sel] = (FalseAlmCnt_Acc->Cnt_all_1sec[is_high_low_sel]*100) / FalseAlmCnt_Acc->Cnt_CCA_all_1sec[is_high_low_sel];
		else
			FalseAlmCnt_Acc->fa_cca_ratio[is_high_low_sel] = FalseAlmCnt_Acc->Cnt_all_1sec[is_high_low_sel];

		FalseAlmCnt_Acc->is_fa_bigger_cca[is_high_low_sel] = (Cnt_all_1sec > Cnt_CCA_all_1sec) ? 1 : 0;
	}

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_TDMA_DIG_CALL_BACK, ODM_DBG_LOUD,
		("[%s] ==> tdma_dig_state=%d,one_sec_dump_en=%d,is_high_low_sel=%d,Cnt_all=%d,Cnt_all_1sec=%d\n\n",
		__FUNCTION__,
		pDM_DigTable->tdma_dig_state,
		one_sec_dump_en,
		is_high_low_sel,
		FalseAlmCnt_Acc->Cnt_all[is_high_low_sel],
		FalseAlmCnt_Acc->Cnt_all_1sec[is_high_low_sel]
		));

	
}

VOID 
odm_FalseAlarmCounterAccReset(
	IN		PVOID		pDM_VOID,
	u8				high_low_idx
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PFALSE_ALARM_STATISTICS_ACC	FalseAlmCnt_Acc = &(pDM_Odm->FalseAlmCnt_Acc);

	//memset(FalseAlmCnt_Acc, 0, sizeof(pDM_Odm->FalseAlmCnt_Acc));

	// Cnt_all_for_rssi_dump & Cnt_CCA_all_for_rssi_dump do NOT need to be reset

	FalseAlmCnt_Acc->Cnt_Parity_Fail[high_low_idx] = 0;
	FalseAlmCnt_Acc->Cnt_Rate_Illegal[high_low_idx] = 0;
	FalseAlmCnt_Acc->Cnt_Crc8_fail[high_low_idx] = 0;
	FalseAlmCnt_Acc->Cnt_Mcs_fail[high_low_idx] = 0;
	FalseAlmCnt_Acc->Cnt_Ofdm_fail[high_low_idx] = 0;
//	FalseAlmCnt_Acc->Cnt_Ofdm_fail_pre = 0;
	FalseAlmCnt_Acc->Cnt_Cck_fail[high_low_idx] = 0;
	FalseAlmCnt_Acc->Cnt_all[high_low_idx] = 0;
//	FalseAlmCnt_Acc->Cnt_all_pre = 0;
	FalseAlmCnt_Acc->Cnt_Fast_Fsync[high_low_idx] = 0;
	FalseAlmCnt_Acc->Cnt_SB_Search_fail[high_low_idx] = 0;
	FalseAlmCnt_Acc->Cnt_OFDM_CCA[high_low_idx] = 0;
	FalseAlmCnt_Acc->Cnt_CCK_CCA[high_low_idx] = 0;
	FalseAlmCnt_Acc->Cnt_CCA_all[high_low_idx] = 0;
//	FalseAlmCnt_Acc->Cnt_BW_USC = 0;
//	FalseAlmCnt_Acc->Cnt_BW_LSC = 0;
	FalseAlmCnt_Acc->cnt_cck_crc32_error[high_low_idx] = 0;
	FalseAlmCnt_Acc->cnt_cck_crc32_ok[high_low_idx] = 0;
	FalseAlmCnt_Acc->cnt_ofdm_crc32_error[high_low_idx] = 0;
	FalseAlmCnt_Acc->cnt_ofdm_crc32_ok[high_low_idx] = 0;
	FalseAlmCnt_Acc->cnt_ht_crc32_error[high_low_idx] = 0;
	FalseAlmCnt_Acc->cnt_ht_crc32_ok[high_low_idx] = 0;
	FalseAlmCnt_Acc->cnt_vht_crc32_error[high_low_idx] = 0;
	FalseAlmCnt_Acc->cnt_vht_crc32_ok[high_low_idx] = 0;
	FalseAlmCnt_Acc->cnt_crc32_error_all[high_low_idx] = 0;
	FalseAlmCnt_Acc->cnt_crc32_ok_all[high_low_idx] = 0;

}


#endif	//#if (PHYDM_TDMA_DIG_SUPPORT == 1)

u1Byte
phydm_dig_current_igi_by_fa_th(
	IN		PVOID		pDM_VOID,
	u1Byte				CurrentIGI,
	pu4Byte				dm_FA_thres,
	pu1Byte				step_size,
	bool				DIG_GoUpCheck,
	u1Byte				is_high_low_sel
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;	
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);

#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	PFALSE_ALARM_STATISTICS_ACC pFalseAlmCntAcc = &(pDM_Odm->FalseAlmCnt_Acc);
	u1Byte						tdma_dig_state = pDM_DigTable->tdma_dig_state;
#endif

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG|ODM_COMP_TDMA_LDIG|ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD,
		("%s ==>\n",__FUNCTION__));

#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	if ((pDM_Odm->original_dig_restore == 0) && (is_high_low_sel != NORMAL_DIG)) {
		if((pFalseAlmCntAcc->Cnt_all_1sec[is_high_low_sel] > dm_FA_thres[2]))
			CurrentIGI = CurrentIGI + step_size[2];
		else if ((pFalseAlmCntAcc->Cnt_all_1sec[is_high_low_sel] > dm_FA_thres[1]))
			CurrentIGI = CurrentIGI + step_size[1];
		else if(pFalseAlmCntAcc->Cnt_all_1sec[is_high_low_sel] < dm_FA_thres[0])
			CurrentIGI = CurrentIGI - step_size[0];

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG|ODM_COMP_TDMA_LDIG|ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, 
			("tdma_dig_state=%d,is_high_low_sel=%d\n",
			tdma_dig_state,
			is_high_low_sel));
	} else
#endif	//#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	{
		if((pFalseAlmCnt->Cnt_all > dm_FA_thres[2]) && DIG_GoUpCheck)
			CurrentIGI = CurrentIGI + step_size[2];
		else if ((pFalseAlmCnt->Cnt_all > dm_FA_thres[1]) && DIG_GoUpCheck)
			CurrentIGI = CurrentIGI + step_size[1];
		else if(pFalseAlmCnt->Cnt_all < dm_FA_thres[0])
			CurrentIGI = CurrentIGI - step_size[0];
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG|ODM_COMP_TDMA_LDIG|ODM_COMP_TDMA_HDIG, ODM_DBG_LOUD, 
		("final igi=0x%x,fa=%d,th0=%d th1=%d th2=%d\n",
		CurrentIGI,
		pFalseAlmCnt->Cnt_all,
		dm_FA_thres[0], dm_FA_thres[1], dm_FA_thres[2]));
	
	return CurrentIGI;
		
}

VOID
odm_DIGbyRSSI_LPS(
	IN		PVOID		pDM_VOID
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);

	u1Byte	RSSI_Lower=DM_DIG_MIN_NIC;   //0x1E or 0x1C
	u1Byte	CurrentIGI=pDM_Odm->RSSI_Min;

	if(odm_DigAbort(pDM_Odm) == TRUE)
		return;

	CurrentIGI=CurrentIGI+RSSI_OFFSET_DIG;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIGbyRSSI_LPS()==>\n"));

	// Using FW PS mode to make IGI
	//Adjust by  FA in LPS MODE
	if(pFalseAlmCnt->Cnt_all> DM_DIG_FA_TH2_LPS)
		CurrentIGI = CurrentIGI+4;
	else if (pFalseAlmCnt->Cnt_all > DM_DIG_FA_TH1_LPS)
		CurrentIGI = CurrentIGI+2;
	else if(pFalseAlmCnt->Cnt_all < DM_DIG_FA_TH0_LPS)
		CurrentIGI = CurrentIGI-2;	


	//Lower bound checking

	//RSSI Lower bound check
	if((pDM_Odm->RSSI_Min-10) > DM_DIG_MIN_NIC)
		RSSI_Lower =(pDM_Odm->RSSI_Min-10);
	else
		RSSI_Lower =DM_DIG_MIN_NIC;

	//Upper and Lower Bound checking
	 if(CurrentIGI > DM_DIG_MAX_NIC)
	 	CurrentIGI = DM_DIG_MAX_NIC;
	 else if(CurrentIGI < RSSI_Lower)
		CurrentIGI = RSSI_Lower;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIGbyRSSI_LPS(): pFalseAlmCnt->Cnt_all = %d\n",pFalseAlmCnt->Cnt_all));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIGbyRSSI_LPS(): pDM_Odm->RSSI_Min = %d\n",pDM_Odm->RSSI_Min));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_DIGbyRSSI_LPS(): CurrentIGI = 0x%x\n",CurrentIGI));

	ODM_Write_DIG(pDM_Odm, CurrentIGI);//ODM_Write_DIG(pDM_Odm, pDM_DigTable->CurIGValue);
#endif
}


VOID 
odm_FalseAlarmCounterStatistics(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PFALSE_ALARM_STATISTICS 	FalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	#if (PHYDM_LA_MODE_SUPPORT == 1)
	PRT_ADCSMP					AdcSmp = &(pDM_Odm->adcsmp);
	#endif
	u4Byte 						ret_value;
	prtl8192cd_priv				priv = pDM_Odm->priv;

#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
//Mark there, and check this in odm_DMWatchDog
#if 0 //(DM_ODM_SUPPORT_TYPE == ODM_AP)
	prtl8192cd_priv priv		= pDM_Odm->priv;
	if( (priv->auto_channel != 0) && (priv->auto_channel != 2) )
		return;
#endif
#endif

	if(!(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))
		return;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics()======>\n"));

#if (ODM_IC_11N_SERIES_SUPPORT == 1) 
	if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
	{
			odm_FalseAlarmCounterRegHold(pDM_Odm);
	
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE1_11N, bMaskDWord);
		FalseAlmCnt->Cnt_Fast_Fsync = (ret_value&0xffff);
		FalseAlmCnt->Cnt_SB_Search_fail = ((ret_value&0xffff0000)>>16);		

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE2_11N, bMaskDWord);
		FalseAlmCnt->Cnt_OFDM_CCA = (ret_value&0xffff); 
		FalseAlmCnt->Cnt_Parity_Fail = ((ret_value&0xffff0000)>>16);	

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE3_11N, bMaskDWord);
		FalseAlmCnt->Cnt_Rate_Illegal = (ret_value&0xffff);
		FalseAlmCnt->Cnt_Crc8_fail = ((ret_value&0xffff0000)>>16);

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE4_11N, bMaskDWord);
		FalseAlmCnt->Cnt_Mcs_fail = (ret_value&0xffff);

		FalseAlmCnt->Cnt_Ofdm_fail = 	FalseAlmCnt->Cnt_Parity_Fail + FalseAlmCnt->Cnt_Rate_Illegal +
								FalseAlmCnt->Cnt_Crc8_fail + FalseAlmCnt->Cnt_Mcs_fail +
								FalseAlmCnt->Cnt_Fast_Fsync + FalseAlmCnt->Cnt_SB_Search_fail;

		/* read CCK CRC32 counter */
		FalseAlmCnt->cnt_cck_crc32_error = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_CRC32_ERROR_CNT_11N, bMaskDWord);
		FalseAlmCnt->cnt_cck_crc32_ok= ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_CRC32_OK_CNT_11N, bMaskDWord);

		/* read OFDM CRC32 counter */
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_CRC32_CNT_11N, bMaskDWord);
		FalseAlmCnt->cnt_ofdm_crc32_error = (ret_value & 0xffff0000) >> 16;
		FalseAlmCnt->cnt_ofdm_crc32_ok= ret_value & 0xffff;

		/* read HT CRC32 counter */
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_HT_CRC32_CNT_11N, bMaskDWord);
		FalseAlmCnt->cnt_ht_crc32_error = (ret_value & 0xffff0000) >> 16;
		FalseAlmCnt->cnt_ht_crc32_ok= ret_value & 0xffff;

		/* read VHT CRC32 counter */
		FalseAlmCnt->cnt_vht_crc32_error = 0;
		FalseAlmCnt->cnt_vht_crc32_ok= 0;

#if (RTL8188E_SUPPORT==1)
		if(pDM_Odm->SupportICType == ODM_RTL8188E)
		{
			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_SC_CNT_11N, bMaskDWord);
			FalseAlmCnt->Cnt_BW_LSC = (ret_value&0xffff);
			FalseAlmCnt->Cnt_BW_USC = ((ret_value&0xffff0000)>>16);
		}
#endif

		{
			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_FA_LSB_11N, bMaskByte0);
			FalseAlmCnt->Cnt_Cck_fail = ret_value;

			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_FA_MSB_11N, bMaskByte3);
			FalseAlmCnt->Cnt_Cck_fail +=  (ret_value& 0xff)<<8;

			ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_CCA_CNT_11N, bMaskDWord);
			FalseAlmCnt->Cnt_CCK_CCA = ((ret_value&0xFF)<<8) |((ret_value&0xFF00)>>8);
		}
	
		FalseAlmCnt->Cnt_all_pre = FalseAlmCnt->Cnt_all;

		FalseAlmCnt->time_fa_all = (FalseAlmCnt->Cnt_Fast_Fsync + FalseAlmCnt->Cnt_SB_Search_fail) * 12 +
					  (FalseAlmCnt->Cnt_Parity_Fail + FalseAlmCnt->Cnt_Rate_Illegal) * 28 +
					  FalseAlmCnt->Cnt_Crc8_fail * 36 +
					  FalseAlmCnt->Cnt_Mcs_fail * 32 +
					  FalseAlmCnt->Cnt_Cck_fail * 80;
		
		FalseAlmCnt->Cnt_all = (	FalseAlmCnt->Cnt_Fast_Fsync + 
							FalseAlmCnt->Cnt_SB_Search_fail +
							FalseAlmCnt->Cnt_Parity_Fail +
							FalseAlmCnt->Cnt_Rate_Illegal +
							FalseAlmCnt->Cnt_Crc8_fail +
							FalseAlmCnt->Cnt_Mcs_fail +
							FalseAlmCnt->Cnt_Cck_fail);	

		FalseAlmCnt->Cnt_CCA_all = FalseAlmCnt->Cnt_OFDM_CCA + FalseAlmCnt->Cnt_CCK_CCA;

				/* Get debug port 0 */
				ODM_SetBBReg(pDM_Odm, ODM_REG_DBG_RPT_11N, bMaskDWord, 0x0);
				FalseAlmCnt->dbg_port0 = ODM_GetBBReg(pDM_Odm, ODM_REG_RPT_11N, bMaskDWord);

				/* Get EDCCA flag */
				ODM_SetBBReg(pDM_Odm, ODM_REG_DBG_RPT_11N, bMaskDWord, 0x208);
				FalseAlmCnt->edcca_flag = (BOOLEAN)ODM_GetBBReg(pDM_Odm, ODM_REG_RPT_11N, BIT30);

		ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Fast_Fsync=%d, Cnt_SB_Search_fail=%d\n",
			FalseAlmCnt->Cnt_Fast_Fsync, FalseAlmCnt->Cnt_SB_Search_fail));
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Parity_Fail=%d, Cnt_Rate_Illegal=%d\n",
			FalseAlmCnt->Cnt_Parity_Fail, FalseAlmCnt->Cnt_Rate_Illegal));
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Crc8_fail=%d, Cnt_Mcs_fail=%d\n",
		FalseAlmCnt->Cnt_Crc8_fail, FalseAlmCnt->Cnt_Mcs_fail));
	}
#endif

#if (ODM_IC_11AC_SERIES_SUPPORT == 1) 
	if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES)
	{
		u4Byte CCKenable;

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE1_11AC, bMaskDWord);
		FalseAlmCnt->Cnt_Fast_Fsync = ((ret_value & 0xffff0000) >> 16);

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE2_11AC, bMaskDWord);
		FalseAlmCnt->Cnt_SB_Search_fail = (ret_value & 0xffff);

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE3_11AC, bMaskDWord);
		FalseAlmCnt->Cnt_Parity_Fail = (ret_value & 0xffff);
		FalseAlmCnt->Cnt_Rate_Illegal = ((ret_value & 0xffff0000) >> 16);

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE4_11AC, bMaskDWord);
		FalseAlmCnt->Cnt_Crc8_fail = (ret_value & 0xffff);
		FalseAlmCnt->Cnt_Mcs_fail = ((ret_value & 0xffff0000) >> 16);

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE5_11AC, bMaskDWord);
		FalseAlmCnt->cnt_crc8_fail_vht = (ret_value & 0xffff);

		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_TYPE6_11AC, bMaskDWord);
		FalseAlmCnt->cnt_mcs_fail_vht = (ret_value & 0xffff);

		
		/* read OFDM FA counter */
		FalseAlmCnt->Cnt_Ofdm_fail = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_FA_11AC, bMaskLWord);

		/* Read CCK FA counter */
		FalseAlmCnt->Cnt_Cck_fail = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_FA_11AC, bMaskLWord);

		/* read CCK/OFDM CCA counter */
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_CCA_CNT_11AC, bMaskDWord);
		FalseAlmCnt->Cnt_OFDM_CCA = (ret_value & 0xffff0000) >> 16;
		FalseAlmCnt->Cnt_CCK_CCA = ret_value & 0xffff;

		/* read CCK CRC32 counter */
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_CCK_CRC32_CNT_11AC, bMaskDWord);
		FalseAlmCnt->cnt_cck_crc32_error = (ret_value & 0xffff0000) >> 16;
		FalseAlmCnt->cnt_cck_crc32_ok= ret_value & 0xffff;

		/* read OFDM CRC32 counter */
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_OFDM_CRC32_CNT_11AC, bMaskDWord);
		FalseAlmCnt->cnt_ofdm_crc32_error = (ret_value & 0xffff0000) >> 16;
		FalseAlmCnt->cnt_ofdm_crc32_ok= ret_value & 0xffff;

		/* read HT CRC32 counter */
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_HT_CRC32_CNT_11AC, bMaskDWord);
		FalseAlmCnt->cnt_ht_crc32_error = (ret_value & 0xffff0000) >> 16;
		FalseAlmCnt->cnt_ht_crc32_ok= ret_value & 0xffff;

		/* read VHT CRC32 counter */
		ret_value = ODM_GetBBReg(pDM_Odm, ODM_REG_VHT_CRC32_CNT_11AC, bMaskDWord);
		FalseAlmCnt->cnt_vht_crc32_error = (ret_value & 0xffff0000) >> 16;
		FalseAlmCnt->cnt_vht_crc32_ok= ret_value & 0xffff;

#if (RTL8881A_SUPPORT==1) 
		/* For 8881A */
		if(pDM_Odm->SupportICType == ODM_RTL8881A)
		{
			u4Byte Cnt_Ofdm_fail_temp = 0;
		
			if(FalseAlmCnt->Cnt_Ofdm_fail >= FalseAlmCnt->Cnt_Ofdm_fail_pre)
			{
				Cnt_Ofdm_fail_temp = FalseAlmCnt->Cnt_Ofdm_fail_pre;
				FalseAlmCnt->Cnt_Ofdm_fail_pre = FalseAlmCnt->Cnt_Ofdm_fail;
				FalseAlmCnt->Cnt_Ofdm_fail = FalseAlmCnt->Cnt_Ofdm_fail - Cnt_Ofdm_fail_temp;
			}
			else
				FalseAlmCnt->Cnt_Ofdm_fail_pre = FalseAlmCnt->Cnt_Ofdm_fail;
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Ofdm_fail=%d\n",	FalseAlmCnt->Cnt_Ofdm_fail_pre));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Ofdm_fail_pre=%d\n",	Cnt_Ofdm_fail_temp));
			
			/* Reset FA counter by enable/disable OFDM */
			if(FalseAlmCnt->Cnt_Ofdm_fail_pre >= 0x7fff)
			{
				// reset OFDM
				ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_PATH_11AC, BIT29,0);
				ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_PATH_11AC, BIT29,1);
				FalseAlmCnt->Cnt_Ofdm_fail_pre = 0;
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Reset false alarm counter\n"));
			}
		}
#endif
		CCKenable =  ODM_GetBBReg(pDM_Odm, ODM_REG_BB_RX_PATH_11AC, BIT28);
		if(CCKenable)//if(*pDM_Odm->pBandType == ODM_BAND_2_4G)
		{
			FalseAlmCnt->Cnt_all = FalseAlmCnt->Cnt_Ofdm_fail + FalseAlmCnt->Cnt_Cck_fail;
			FalseAlmCnt->Cnt_CCA_all = FalseAlmCnt->Cnt_CCK_CCA + FalseAlmCnt->Cnt_OFDM_CCA;
		}
		else
		{
			FalseAlmCnt->Cnt_all = FalseAlmCnt->Cnt_Ofdm_fail;
			FalseAlmCnt->Cnt_CCA_all = FalseAlmCnt->Cnt_OFDM_CCA;
		}

		#if (PHYDM_LA_MODE_SUPPORT == 1)
		if (AdcSmp->ADCSmpState == ADCSMP_STATE_IDLE)
			#endif
			{
				/* Get debug port 0 */
				ODM_SetBBReg(pDM_Odm, ODM_REG_DBG_RPT_11AC, bMaskDWord, 0x0);
				FalseAlmCnt->dbg_port0 = ODM_GetBBReg(pDM_Odm, ODM_REG_RPT_11AC, bMaskDWord);

				/* Get EDCCA flag */
				ODM_SetBBReg(pDM_Odm, ODM_REG_DBG_RPT_11AC, bMaskDWord, 0x209);
				FalseAlmCnt->edcca_flag = (BOOLEAN)ODM_GetBBReg(pDM_Odm, ODM_REG_RPT_11AC, BIT30);
			}
	}
#endif

		odm_FalseAlarmCounterRegReset(pDM_Odm);

	FalseAlmCnt->time_fa_all = (FalseAlmCnt->Cnt_Fast_Fsync + FalseAlmCnt->Cnt_SB_Search_fail) * 12 +
					  (FalseAlmCnt->Cnt_Parity_Fail + FalseAlmCnt->Cnt_Rate_Illegal) * 28 +
					  (FalseAlmCnt->Cnt_Crc8_fail + FalseAlmCnt->cnt_crc8_fail_vht + FalseAlmCnt->cnt_mcs_fail_vht) * 36 +
					  FalseAlmCnt->Cnt_Mcs_fail * 32 +
					  FalseAlmCnt->Cnt_Cck_fail * 80;

	FalseAlmCnt->cnt_crc32_error_all = FalseAlmCnt->cnt_vht_crc32_error + FalseAlmCnt->cnt_ht_crc32_error + FalseAlmCnt->cnt_ofdm_crc32_error + FalseAlmCnt->cnt_cck_crc32_error;
	FalseAlmCnt->cnt_crc32_ok_all = FalseAlmCnt->cnt_vht_crc32_ok + FalseAlmCnt->cnt_ht_crc32_ok + FalseAlmCnt->cnt_ofdm_crc32_ok + FalseAlmCnt->cnt_cck_crc32_ok;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_OFDM_CCA=%d\n", FalseAlmCnt->Cnt_OFDM_CCA));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_CCK_CCA=%d\n", FalseAlmCnt->Cnt_CCK_CCA));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_CCA_all=%d\n", FalseAlmCnt->Cnt_CCA_all));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Ofdm_fail=%d\n", FalseAlmCnt->Cnt_Ofdm_fail));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Cck_fail=%d\n", FalseAlmCnt->Cnt_Cck_fail));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Cnt_Ofdm_fail=%d\n", FalseAlmCnt->Cnt_Ofdm_fail));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Total False Alarm=%d\n", FalseAlmCnt->Cnt_all));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): CCK CRC32 fail: %d, ok: %d\n", FalseAlmCnt->cnt_cck_crc32_error, FalseAlmCnt->cnt_cck_crc32_ok));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): OFDM CRC32 fail: %d, ok: %d\n", FalseAlmCnt->cnt_ofdm_crc32_error, FalseAlmCnt->cnt_ofdm_crc32_ok));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): HT CRC32 fail: %d, ok: %d\n", FalseAlmCnt->cnt_ht_crc32_error, FalseAlmCnt->cnt_ht_crc32_ok));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): VHT CRC32 fail: %d, ok: %d\n", FalseAlmCnt->cnt_vht_crc32_error, FalseAlmCnt->cnt_vht_crc32_ok));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): Total CRC32 fail: %d, ok: %d\n", FalseAlmCnt->cnt_crc32_error_all, FalseAlmCnt->cnt_crc32_ok_all));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_FA_CNT, ODM_DBG_LOUD, ("odm_FalseAlarmCounterStatistics(): dbg port 0x0 = 0x%x, EDCCA = %d\n\n", FalseAlmCnt->dbg_port0, FalseAlmCnt->edcca_flag));

}

// reset FA TSF also
VOID 
odm_FalseAlarmCounterReset(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PFALSE_ALARM_STATISTICS 	FalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	prtl8192cd_priv				priv = pDM_Odm->priv;
	u4Byte						TSF;

	memset(FalseAlmCnt, 0, sizeof(pDM_Odm->FalseAlmCnt));
	odm_FalseAlarmCounterRegReset(pDM_Odm);		
#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	if(pDM_DigTable->tdma_dig_state != 1)
		pDM_DigTable->fa_acc_1sec_tsf = 0;
	else
#endif		
		pDM_DigTable->fa_acc_1sec_tsf = pDM_DigTable->fa_acc_1sec_tsf;
	
	GET_HAL_INTERFACE(priv)->GetHwRegHandler(priv, HW_VAR_TSF_TIMER, (pu1Byte)&TSF);
	pDM_DigTable->fa_start_tsf = TSF;
	pDM_DigTable->fa_end_tsf = TSF;

}
	
VOID
odm_FalseAlarmCounterRegReset(
	IN		PVOID					pDM_VOID
)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;

#if (ODM_IC_11N_SERIES_SUPPORT == 1)	
	if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES) {
		if (pDM_Odm->SupportICType >= ODM_RTL8188E) {
			/*reset false alarm counter registers*/
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTC_11N, BIT31, 1);
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTC_11N, BIT31, 0);
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT27, 1);
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT27, 0);
		
			/*update ofdm counter*/
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_HOLDC_11N, BIT31, 0); /*update page C counter*/
			ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT31, 0);	/*update page D counter*/
		
			/*reset CCK CCA counter*/
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT13|BIT12, 0); 
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT13|BIT12, 2); 
		
			/*reset CCK FA counter*/
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT15|BIT14, 0); 
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT15|BIT14, 2); 
		
			/*reset CRC32 counter*/
			ODM_SetBBReg(pDM_Odm, ODM_REG_PAGE_F_RST_11N, BIT16, 1); 
			ODM_SetBBReg(pDM_Odm, ODM_REG_PAGE_F_RST_11N, BIT16, 0); 
		}
	}
#endif		// #if (ODM_IC_11N_SERIES_SUPPORT == 1)

#if (ODM_IC_11AC_SERIES_SUPPORT == 1)
	
	if(pDM_Odm->SupportICType & ODM_IC_11AC_SERIES) {
		/* reset OFDM FA coutner */
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RST_11AC, BIT17, 1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RST_11AC, BIT17, 0);
		
		/* reset CCK FA counter */
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11AC, BIT15, 0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11AC, BIT15, 1);
		
		/* reset CCA counter */
		ODM_SetBBReg(pDM_Odm, ODM_REG_RST_RPT_11AC, BIT0, 1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_RST_RPT_11AC, BIT0, 0);
	}
#endif		// #if (ODM_IC_11AC_SERIES_SUPPORT == 1)

}
	
VOID
odm_FalseAlarmCounterRegHold(
	IN		PVOID					pDM_VOID
)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	if(pDM_Odm->SupportICType & ODM_IC_11N_SERIES)
	{
		//hold ofdm counter
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_HOLDC_11N, BIT31, 1); //hold page C counter
		ODM_SetBBReg(pDM_Odm, ODM_REG_OFDM_FA_RSTD_11N, BIT31, 1); //hold page D counter

		//hold cck counter
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT12, 1); 
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT14, 1); 
	}
}

//3============================================================
//3 CCK Packet Detect Threshold
//3============================================================

VOID
odm_PauseCCKPacketDetection(
	IN		PVOID					pDM_VOID,
	IN		PHYDM_PAUSE_TYPE		PauseType,
	IN		PHYDM_PAUSE_LEVEL		pause_level,
	IN		u1Byte					CCKPDThreshold
)
{
	PDM_ODM_T			pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T				pDM_DigTable = &pDM_Odm->DM_DigTable;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection()=========> level = %d\n", pause_level));

	if ((pDM_DigTable->pause_cckpd_level == 0) && (!(pDM_Odm->SupportAbility & ODM_BB_CCK_PD) || !(pDM_Odm->SupportAbility & ODM_BB_FA_CNT))) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("Return: SupportAbility ODM_BB_CCK_PD or ODM_BB_FA_CNT is disabled\n"));
		return;
	}

	if (pause_level > DM_DIG_MAX_PAUSE_TYPE) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, 
			("odm_PauseCCKPacketDetection(): Return: Wrong pause level !!\n"));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): pause level = 0x%x, Current value = 0x%x\n", pDM_DigTable->pause_cckpd_level, CCKPDThreshold));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): pause value = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		pDM_DigTable->pause_cckpd_value[7], pDM_DigTable->pause_cckpd_value[6], pDM_DigTable->pause_cckpd_value[5], pDM_DigTable->pause_cckpd_value[4],
		pDM_DigTable->pause_cckpd_value[3], pDM_DigTable->pause_cckpd_value[2], pDM_DigTable->pause_cckpd_value[1], pDM_DigTable->pause_cckpd_value[0]));

	switch (PauseType) {
	/* Pause CCK Packet Detection Threshold */
	case PHYDM_PAUSE:
	{
		/* Disable CCK PD */
		ODM_CmnInfoUpdate(pDM_Odm, ODM_CMNINFO_ABILITY, pDM_Odm->SupportAbility & (~ODM_BB_CCK_PD));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Pause CCK packet detection threshold !!\n"));

		/* Backup original CCK PD threshold decided by CCK PD mechanism */
		if (pDM_DigTable->pause_cckpd_level == 0) {
			pDM_DigTable->CCKPDBackup = pDM_DigTable->CurCCK_CCAThres;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, 
				("odm_PauseCCKPacketDetection(): Backup CCKPD  = 0x%x, new CCKPD = 0x%x\n", pDM_DigTable->CCKPDBackup, CCKPDThreshold));
		}

		/* Update pause level */
		pDM_DigTable->pause_cckpd_level = (pDM_DigTable->pause_cckpd_level | BIT(pause_level));

		/* Record CCK PD threshold */
		pDM_DigTable->pause_cckpd_value[pause_level] = CCKPDThreshold;

		/* Write new CCK PD threshold */
		if (BIT(pause_level + 1) > pDM_DigTable->pause_cckpd_level) {
			ODM_Write_CCK_CCA_Thres(pDM_Odm, CCKPDThreshold);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): CCKPD of higher level = 0x%x\n", CCKPDThreshold));
		}
		break;
	}
	/* Resume CCK Packet Detection Threshold */
	case PHYDM_RESUME:
	{	
		/* check if the level is illegal or not */
		if ((pDM_DigTable->pause_cckpd_level & (BIT(pause_level))) != 0) {
			pDM_DigTable->pause_cckpd_level = pDM_DigTable->pause_cckpd_level & (~(BIT(pause_level)));
			pDM_DigTable->pause_cckpd_value[pause_level] = 0;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Resume CCK PD !!\n"));
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Wrong resume level !!\n"));
			break;
		}

		/* Resume DIG */
		if (pDM_DigTable->pause_cckpd_level == 0) {
			/* Write backup IGI value */
			ODM_Write_CCK_CCA_Thres(pDM_Odm, pDM_DigTable->CCKPDBackup);
			/* pDM_DigTable->bIgnoreDIG = TRUE; */
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Write original CCKPD = 0x%x\n", pDM_DigTable->CCKPDBackup));

			/* Enable DIG */
			ODM_CmnInfoUpdate(pDM_Odm, ODM_CMNINFO_ABILITY, pDM_Odm->SupportAbility | ODM_BB_CCK_PD);	
			break;
		}

		if (BIT(pause_level) > pDM_DigTable->pause_cckpd_level) {
			s1Byte	max_level;
		
			/* Calculate the maximum level now */
			for (max_level = (pause_level - 1); max_level >= 0; max_level--) {
				if ((pDM_DigTable->pause_cckpd_level & BIT(max_level)) > 0)
					break;
			}
		
			/* write CCKPD of lower level */
			ODM_Write_CCK_CCA_Thres(pDM_Odm, pDM_DigTable->pause_cckpd_value[max_level]);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Write CCKPD (0x%x) of level (%d)\n", 
				pDM_DigTable->pause_cckpd_value[max_level], max_level));
			break;
		}
		break;
	}
	default:
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): Wrong  type !!\n"));
		break;
	}	
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): pause level = 0x%x, Current value = 0x%x\n", pDM_DigTable->pause_cckpd_level, CCKPDThreshold));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_PauseCCKPacketDetection(): pause value = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", 
		pDM_DigTable->pause_cckpd_value[7], pDM_DigTable->pause_cckpd_value[6], pDM_DigTable->pause_cckpd_value[5], pDM_DigTable->pause_cckpd_value[4],
		pDM_DigTable->pause_cckpd_value[3], pDM_DigTable->pause_cckpd_value[2], pDM_DigTable->pause_cckpd_value[1], pDM_DigTable->pause_cckpd_value[0]));
}


VOID 
odm_CCKPacketDetectionThresh(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T				pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T					pDM_DigTable = &pDM_Odm->DM_DigTable;
#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	PFALSE_ALARM_STATISTICS_ACC	FalseAlmCntAcc = &(pDM_Odm->FalseAlmCnt_Acc);
#endif
	PFALSE_ALARM_STATISTICS 	FalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	u1Byte					CurCCK_CCAThres = pDM_DigTable->CurCCK_CCAThres, RSSI_thd = 35;
	u1Byte					pd_th = 0, cs_ration = 0;
	BOOLEAN					en_2rcca;

	en_2rcca = ((BOOLEAN)ODM_GetBBReg(pDM_Odm, 0xa2c, BIT(18)) && (BOOLEAN)ODM_GetBBReg(pDM_Odm, 0xa2c, BIT(22)));
	
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	prtl8192cd_priv				priv = pDM_Odm->priv;
#endif
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
//modify by Guo.Mingzhi 2011-12-29
	if (pDM_Odm->bDualMacSmartConcurrent == TRUE)
//	if (pDM_Odm->bDualMacSmartConcurrent == FALSE)
		return;
	if(pDM_Odm->bBtHsOperation)
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_CCKPacketDetectionThresh() write 0xcd for BT HS mode!!\n"));
		ODM_Write_CCK_CCA_Thres(pDM_Odm, 0xcd);
		return;
	}
#endif

	if((!(pDM_Odm->SupportAbility & ODM_BB_CCK_PD)) ||(!(pDM_Odm->SupportAbility & ODM_BB_FA_CNT)))
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_CCKPacketDetectionThresh()  return==========\n"));
#ifdef MCR_WIRELESS_EXTEND
		ODM_Write_CCK_CCA_Thres(pDM_Odm, 0x43);
#endif
		return;
	}

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
	if(pDM_Odm->ExtLNA)
		return;
#endif

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_CCKPacketDetectionThresh()  ==========>\n"));

#if (PHYDM_TDMA_DIG_SUPPORT == 1)
	if(pDM_Odm->original_dig_restore == 0) {		
		if (pDM_DigTable->cckFaMa == 0xffffffff)
			pDM_DigTable->cckFaMa = FalseAlmCntAcc->Cnt_Cck_fail_1sec[TDMA_DIG_HIGH_STATE];
		else
			pDM_DigTable->cckFaMa = ((pDM_DigTable->cckFaMa<<1) + pDM_DigTable->cckFaMa + FalseAlmCntAcc->Cnt_Cck_fail_1sec[TDMA_DIG_HIGH_STATE]) >> 2;
	} else
#endif
	{
		if (pDM_DigTable->cckFaMa == 0xffffffff)
			pDM_DigTable->cckFaMa = FalseAlmCnt->Cnt_Cck_fail;
		else
			pDM_DigTable->cckFaMa = ((pDM_DigTable->cckFaMa<<1) + pDM_DigTable->cckFaMa + FalseAlmCnt->Cnt_Cck_fail) >> 2;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_CCKPacketDetectionThresh(): CCK FA moving average = %d\n", pDM_DigTable->cckFaMa));

	if (pDM_Odm->bLinked) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
		if (pDM_Odm->RSSI_Min > (RSSI_thd + 14))
			CurCCK_CCAThres = 0xed;
		else if (pDM_Odm->RSSI_Min > (RSSI_thd + 6))
			CurCCK_CCAThres = 0xdd;
		else if (pDM_Odm->RSSI_Min > RSSI_thd)
			CurCCK_CCAThres = 0xcd;
		else if (pDM_Odm->RSSI_Min > 20) {
			if (pDM_DigTable->cckFaMa > ((DM_DIG_FA_TH1>>1) + (DM_DIG_FA_TH1>>3)))
				CurCCK_CCAThres = 0xcd;
			else if (pDM_DigTable->cckFaMa < (DM_DIG_FA_TH0>>1))
				CurCCK_CCAThres = 0x83;
		} else if (pDM_Odm->RSSI_Min > 7)
			CurCCK_CCAThres = 0x83;
		else
			CurCCK_CCAThres = 0x40;
#else
	if (pDM_Odm->SupportICType & ODM_RTL8197F) {
		if ((pDM_DigTable->CurIGValue > (0x24 + 14)) || (pDM_Odm->RSSI_Min > 26))
			CurCCK_CCAThres = 0xed;
		else if ((pDM_DigTable->CurIGValue > (0x24 + 6)) || (pDM_Odm->RSSI_Min > 26))
			CurCCK_CCAThres = 0xdd;
		else if ((pDM_DigTable->CurIGValue > 0x24) || (pDM_Odm->RSSI_Min > 20 && pDM_Odm->RSSI_Min <= 24))
			CurCCK_CCAThres = 0xcd;
		else if ((pDM_DigTable->CurIGValue <= 0x24) || (pDM_Odm->RSSI_Min < 18)){
			if (pDM_DigTable->cckFaMa > 0x400)
				CurCCK_CCAThres = 0x83;
			else if (pDM_DigTable->cckFaMa < 0x200)
				CurCCK_CCAThres = 0x40;
		}
	} else {
		if (pDM_DigTable->CurIGValue > (0x24 + 14))
			CurCCK_CCAThres = 0xed;
		else if (pDM_DigTable->CurIGValue > (0x24 + 6))
			CurCCK_CCAThres = 0xdd;
		else if (pDM_DigTable->CurIGValue > 0x24)
			CurCCK_CCAThres = 0xcd;
		else {
			if (pDM_DigTable->cckFaMa > 0x400)
				CurCCK_CCAThres = 0x83;
			else if (pDM_DigTable->cckFaMa < 0x200)
				CurCCK_CCAThres = 0x40;
		}
	}
#endif
	} else {
		if (pDM_DigTable->cckFaMa > 0x400)
			CurCCK_CCAThres = 0x83;
		else if (pDM_DigTable->cckFaMa < 0x200)
			CurCCK_CCAThres = 0x40;
	}
	
	/*Add by Yu Chen 20160902, pd_th for 0xa0a, cs_ration for 0xaaa*/
	if (pDM_Odm->SupportICType & ODM_RTL8197F) {
		switch (CurCCK_CCAThres) {
			case 0xed:
				cs_ration = pDM_DigTable->aaa_default + AAA_BASE + AAA_STEP*2;
				if (en_2rcca) {cs_ration = cs_ration - 5;}
				pd_th = 0xd;
				break;

			case 0xdd:
				cs_ration = pDM_DigTable->aaa_default + AAA_BASE + AAA_STEP;
				if (en_2rcca) {cs_ration = cs_ration - 4;}
				pd_th = 0xd;
				break;

			case 0xcd:
				cs_ration = pDM_DigTable->aaa_default + AAA_BASE;
				if (en_2rcca) {cs_ration = cs_ration - 3;}
				pd_th = 0xd;
				break;

			case 0x83:
				cs_ration = pDM_DigTable->aaa_default + AAA_STEP;
				if (en_2rcca) {cs_ration = cs_ration - 1;}
				pd_th = 0x3;
				break;

			case 0x40:
				cs_ration = pDM_DigTable->aaa_default;
				pd_th = 0x3;
				break;

			default:
				cs_ration = pDM_DigTable->aaa_default;
				pd_th = 0x3;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("cck pd use default\n"));
				break;
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("%s : cs_ration = 0x%x, pd_th = 0x%x\n", __func__, cs_ration, pd_th));

	if (pDM_Odm->SupportICType & ODM_RTL8197F) {
		ODM_SetBBReg(pDM_Odm, 0xa08, 0xf0000, pd_th);
		ODM_SetBBReg(pDM_Odm, 0xaa8, 0x1f0000, cs_ration);
	} else
		ODM_Write_CCK_CCA_Thres(pDM_Odm, CurCCK_CCAThres);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_CCKPacketDetectionThresh()  CurCCK_CCAThres = 0x%x\n", CurCCK_CCAThres));
}

VOID
ODM_Write_CCK_CCA_Thres(
	IN	PVOID			pDM_VOID,
	IN	u1Byte			CurCCK_CCAThres
	)
{
	PDM_ODM_T			pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T				pDM_DigTable = &pDM_Odm->DM_DigTable;

	if(pDM_DigTable->CurCCK_CCAThres!=CurCCK_CCAThres)		//modify by Guo.Mingzhi 2012-01-03
	{
		ODM_Write1Byte(pDM_Odm, ODM_REG(CCK_CCA,pDM_Odm), CurCCK_CCAThres);
		pDM_DigTable->cckFaMa = 0xffffffff;
	}
	pDM_DigTable->PreCCK_CCAThres = pDM_DigTable->CurCCK_CCAThres;
	pDM_DigTable->CurCCK_CCAThres = CurCCK_CCAThres;
}

BOOLEAN
phydm_DIG_GoUpCheck(
	IN		PVOID		pDM_VOID
	)
{
	PDM_ODM_T				pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PCCX_INFO				CCX_INFO = &pDM_Odm->DM_CCX_INFO;
	pDIG_T					pDM_DigTable = &pDM_Odm->DM_DigTable;
	u1Byte 					CurIGValue = pDM_DigTable->CurIGValue;
	u1Byte					max_DIG_cover_bond;
	u1Byte					current_IGI_MaxUp_resolution;
	u1Byte					rx_gain_range_max;
	u1Byte					i = 0;
	
	u4Byte					total_NHM_cnt;
	u4Byte					DIG_cover_cnt;
	u4Byte					over_DIG_cover_cnt;
	BOOLEAN					ret = TRUE;

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	prtl8192cd_priv				priv = pDM_Odm->priv;

	max_DIG_cover_bond = DM_DIG_MAX_AP - priv->pshare->rf_ft_var.dig_upcheck_initial_value;
	current_IGI_MaxUp_resolution = CurIGValue + 6;
	rx_gain_range_max = pDM_DigTable->rx_gain_range_max;
	
	phydm_getNHMresult(pDM_Odm);

	total_NHM_cnt = CCX_INFO->NHM_result[0] + CCX_INFO->NHM_result[1];

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("phydm_DIG_GoUpCheck(): *****Get NHM results*****\n"));
	
	if (total_NHM_cnt != 0) {

		/* CurIGValue < max_DIG_cover_bond - 6 */
		if (pDM_DigTable->DIG_GoUpCheck_Level == DIG_GOUPCHECK_LEVEL_0) {
			DIG_cover_cnt = CCX_INFO->NHM_result[1];
			ret = ((priv->pshare->rf_ft_var.dig_level0_ratio_reciprocal * DIG_cover_cnt) >= total_NHM_cnt) ? TRUE : FALSE;
		}

		/* (max_DIG_cover_bond - 6) <= CurIGValue < DM_DIG_MAX_AP */
		else if (pDM_DigTable->DIG_GoUpCheck_Level == DIG_GOUPCHECK_LEVEL_1) {
			over_DIG_cover_cnt = CCX_INFO->NHM_result[1];
			ret = (priv->pshare->rf_ft_var.dig_level1_ratio_reciprocal * over_DIG_cover_cnt < total_NHM_cnt) ? TRUE : FALSE;

			if (!ret) {
				/* update pDM_DigTable->rx_gain_range_max */
				pDM_DigTable->rx_gain_range_max = (rx_gain_range_max >= max_DIG_cover_bond - 6) ? (max_DIG_cover_bond - 6) : rx_gain_range_max;

				ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("phydm_DIG_GoUpCheck(): Noise power is beyond DIG can filter, lock rx_gain_range_max to 0x%x\n", 
					pDM_DigTable->rx_gain_range_max));
			}
		}
		
		/* CurIGValue > DM_DIG_MAX_AP, foolproof */
		else if (pDM_DigTable->DIG_GoUpCheck_Level == DIG_GOUPCHECK_LEVEL_2) {
			ret = TRUE;
		}

		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("phydm_DIG_GoUpCheck(): DIG_GoUpCheck_level = %d\n, current_IGI_MaxUp_resolution = 0x%x\n, max_DIG_cover_bond = 0x%x\n, rx_gain_range_max = 0x%x, ret = %d\n", 
			pDM_DigTable->DIG_GoUpCheck_Level, 
			current_IGI_MaxUp_resolution, 
			max_DIG_cover_bond,
			pDM_DigTable->rx_gain_range_max,
			ret));

		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("phydm_DIG_GoUpCheck(): NHM_result = %d, %d, %d, %d\n", 
			CCX_INFO->NHM_result[0], CCX_INFO->NHM_result[1], CCX_INFO->NHM_result[2], CCX_INFO->NHM_result[3]));
		
	}
	else
		ret = TRUE;

	for (i = 0 ; i <= 10 ; i ++) {
		CCX_INFO->NHM_th[i] = 0xFF;
	}
	
	if (CurIGValue < max_DIG_cover_bond - 6){
		CCX_INFO->NHM_th[0] = 2 * (CurIGValue - priv->pshare->rf_ft_var.dig_upcheck_initial_value);
		pDM_DigTable->DIG_GoUpCheck_Level = DIG_GOUPCHECK_LEVEL_0;
	}
	else if (CurIGValue <= DM_DIG_MAX_AP) {
		CCX_INFO->NHM_th[0] = 2 * max_DIG_cover_bond;
		pDM_DigTable->DIG_GoUpCheck_Level = DIG_GOUPCHECK_LEVEL_1;
	}
	/* CurIGValue > DM_DIG_MAX_AP, foolproof */
	else {
		pDM_DigTable->DIG_GoUpCheck_Level = DIG_GOUPCHECK_LEVEL_2;
		ret = TRUE;
	}
	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("phydm_DIG_GoUpCheck(): *****Set NHM settings*****\n"));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("phydm_DIG_GoUpCheck(): DIG_GoUpCheck_level = %d\n", 
		pDM_DigTable->DIG_GoUpCheck_Level));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("phydm_DIG_GoUpCheck(): NHM_th = 0x%x, 0x%x, 0x%x\n", 
		CCX_INFO->NHM_th[0], CCX_INFO->NHM_th[1], CCX_INFO->NHM_th[2]));
	
	CCX_INFO->NHM_inexclude_cca = NHM_EXCLUDE_CCA;	
	CCX_INFO->NHM_inexclude_txon = NHM_EXCLUDE_TXON;
	CCX_INFO->NHM_period = 0xC350;

	phydm_NHMsetting(pDM_Odm, SET_NHM_SETTING);
	phydm_NHMtrigger(pDM_Odm);
#endif

	return ret;
}

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)

// <20130108, Kordan> E.g., With LNA used, we make the Rx power smaller to have a better EVM. (Asked by Willis)
VOID
odm_RFEControl(
	IN	PDM_ODM_T	pDM_Odm,
	IN  u8Byte		RSSIVal
	)
{
	PADAPTER		Adapter = (PADAPTER)pDM_Odm->Adapter;
    HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	static u1Byte 	TRSW_HighPwr = 0;
	 
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("===> odm_RFEControl, RSSI = %d, TRSW_HighPwr = 0x%X, pHalData->RFEType = %d\n",
		         RSSIVal, TRSW_HighPwr, pHalData->RFEType ));

    if (pHalData->RFEType == 3) {	   
		
        pDM_Odm->RSSI_TRSW = RSSIVal;

        if (pDM_Odm->RSSI_TRSW >= pDM_Odm->RSSI_TRSW_H) 
		{				 
            TRSW_HighPwr = 1; // Switch to
            PHY_SetBBReg(Adapter, r_ANTSEL_SW_Jaguar, BIT1|BIT0, 0x1);  // Set ANTSW=1/ANTSWB=0  for SW control
            PHY_SetBBReg(Adapter, r_ANTSEL_SW_Jaguar, BIT9|BIT8, 0x3);  // Set ANTSW=1/ANTSWB=0  for SW control
            
        } 
		else if (pDM_Odm->RSSI_TRSW <= pDM_Odm->RSSI_TRSW_L) 
        {	  
            TRSW_HighPwr = 0; // Switched back
            PHY_SetBBReg(Adapter, r_ANTSEL_SW_Jaguar, BIT1|BIT0, 0x1);  // Set ANTSW=1/ANTSWB=0  for SW control
            PHY_SetBBReg(Adapter, r_ANTSEL_SW_Jaguar, BIT9|BIT8, 0x0);  // Set ANTSW=1/ANTSWB=0  for SW control

        }
    }  

	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("(pDM_Odm->RSSI_TRSW_H, pDM_Odm->RSSI_TRSW_L) = (%d, %d)\n", pDM_Odm->RSSI_TRSW_H, pDM_Odm->RSSI_TRSW_L));		
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("(RSSIVal, RSSIVal, pDM_Odm->RSSI_TRSW_iso) = (%d, %d, %d)\n", 
				 RSSIVal, pDM_Odm->RSSI_TRSW_iso, pDM_Odm->RSSI_TRSW));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("<=== odm_RFEControl, RSSI = %d, TRSW_HighPwr = 0x%X\n", RSSIVal, TRSW_HighPwr));	
}

VOID
odm_MPT_DIGWorkItemCallback(
    IN PVOID            pContext
    )
{
	PADAPTER	Adapter = (PADAPTER)pContext;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;

	ODM_MPT_DIG(pDM_Odm);
}

VOID
odm_MPT_DIGCallback(
	PRT_TIMER		pTimer
)
{
	PADAPTER		Adapter = (PADAPTER)pTimer->Adapter;
       HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	  PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;


	#if DEV_BUS_TYPE==RT_PCI_INTERFACE
		#if USE_WORKITEM
			PlatformScheduleWorkItem(&pDM_Odm->MPT_DIGWorkitem);
		#else
			ODM_MPT_DIG(pDM_Odm);
		#endif
	#else
		PlatformScheduleWorkItem(&pDM_Odm->MPT_DIGWorkitem);
	#endif

}

#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
VOID
odm_MPT_DIGCallback(
	IN		PVOID					pDM_VOID
)
{
	PDM_ODM_T			pDM_Odm = (PDM_ODM_T)pDM_VOID;
#if USE_WORKITEM
	PlatformScheduleWorkItem(&pDM_Odm->MPT_DIGWorkitem);
#else
	ODM_MPT_DIG(pDM_Odm);
#endif
}
#endif

#if (DM_ODM_SUPPORT_TYPE != ODM_CE)
VOID
odm_MPT_Write_DIG(
	IN		PVOID					pDM_VOID,
	IN		u1Byte					CurIGValue
)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;

	ODM_Write1Byte( pDM_Odm, ODM_REG(IGI_A,pDM_Odm), CurIGValue);

#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
	/* Set IGI value of CCK for new CCK AGC */
		if (pDM_Odm->cck_new_agc) {
			if (pDM_Odm->SupportICType & ODM_IC_PHY_STATUE_NEW_TYPE)
				ODM_SetBBReg(pDM_Odm, 0xa0c, 0x00003f00, (CurIGValue>>1));
		}

#endif
			
	if(pDM_Odm->RFType > ODM_1T1R)
		ODM_Write1Byte( pDM_Odm, ODM_REG(IGI_B,pDM_Odm), CurIGValue);

	if((pDM_Odm->SupportICType & ODM_IC_11AC_SERIES) && (pDM_Odm->RFType > ODM_2T2R))
	{
		ODM_Write1Byte( pDM_Odm, ODM_REG(IGI_C,pDM_Odm), CurIGValue);
		ODM_Write1Byte( pDM_Odm, ODM_REG(IGI_D,pDM_Odm), CurIGValue);	
	}

	pDM_DigTable->CurIGValue = CurIGValue;
	
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("CurIGValue = 0x%x\n", CurIGValue));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("pDM_Odm->RFType = 0x%x\n", pDM_Odm->RFType));
}

VOID
ODM_MPT_DIG(
	IN		PVOID					pDM_VOID
	)
{
	PDM_ODM_T					pDM_Odm = (PDM_ODM_T)pDM_VOID;
	pDIG_T						pDM_DigTable = &pDM_Odm->DM_DigTable;
	PFALSE_ALARM_STATISTICS		pFalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);
	u1Byte						CurrentIGI = pDM_DigTable->CurIGValue;
	u1Byte						DIG_Upper = 0x40, DIG_Lower = 0x20;
	u4Byte						RXOK_cal;
	u4Byte						RxPWDBAve_final;
	u1Byte						IGI_A = 0x20, IGI_B = 0x20;
	
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)

	#if ODM_FIX_2G_DIG
	IGI_A = 0x22;
	IGI_B = 0x24;		
	#endif
	
#else
	if (!(pDM_Odm->priv->pshare->rf_ft_var.mp_specific && pDM_Odm->priv->pshare->mp_dig_on))
		return;

	if (*pDM_Odm->pBandType == ODM_BAND_5G)
		DIG_Lower = 0x22;
#endif

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("===> ODM_MPT_DIG, pBandType = %d\n", *pDM_Odm->pBandType));
	
#if (ODM_FIX_2G_DIG || (DM_ODM_SUPPORT_TYPE & ODM_AP))
	if (*pDM_Odm->pBandType == ODM_BAND_5G || (pDM_Odm->SupportICType & (ODM_RTL8814A|ODM_RTL8822B))) // for 5G or 8814
#else
	if (1) // for both 2G/5G
#endif
		{
		odm_FalseAlarmCounterStatistics(pDM_Odm);

		RXOK_cal = pDM_Odm->PhyDbgInfo.NumQryPhyStatusCCK + pDM_Odm->PhyDbgInfo.NumQryPhyStatusOFDM;
		RxPWDBAve_final = (RXOK_cal != 0)?pDM_Odm->RxPWDBAve/RXOK_cal:0;

		pDM_Odm->PhyDbgInfo.NumQryPhyStatusCCK = 0;
		pDM_Odm->PhyDbgInfo.NumQryPhyStatusOFDM = 0;
		pDM_Odm->RxPWDBAve = 0;
		pDM_Odm->MPDIG_2G = FALSE;

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		pDM_Odm->Times_2G = 0;
#endif

		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("RX OK = %d\n", RXOK_cal));
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("RSSI = %d\n", RxPWDBAve_final));
	
		if (RXOK_cal >= 70 && RxPWDBAve_final <= 40)
		{
			if (CurrentIGI > 0x24)
				odm_MPT_Write_DIG(pDM_Odm, 0x24);
		}
		else
		{
			if(pFalseAlmCnt->Cnt_all > 1000){
				CurrentIGI = CurrentIGI + 8;
			}
			else if(pFalseAlmCnt->Cnt_all > 200){
				CurrentIGI = CurrentIGI + 4;
			}
			else if (pFalseAlmCnt->Cnt_all > 50){
				CurrentIGI = CurrentIGI + 2;
			}
			else if (pFalseAlmCnt->Cnt_all < 2){
				CurrentIGI = CurrentIGI - 2;
			}
			
			if (CurrentIGI < DIG_Lower ){
				CurrentIGI = DIG_Lower;
			}

			if(CurrentIGI > DIG_Upper){
				CurrentIGI = DIG_Upper;
			}

			odm_MPT_Write_DIG(pDM_Odm, CurrentIGI);
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("DIG = 0x%x, Cnt_all = %d, Cnt_Ofdm_fail = %d, Cnt_Cck_fail = %d\n", 
				CurrentIGI, pFalseAlmCnt->Cnt_all, pFalseAlmCnt->Cnt_Ofdm_fail, pFalseAlmCnt->Cnt_Cck_fail));
		}
	}
	else
	{
		if(pDM_Odm->MPDIG_2G == FALSE)
		{
			if((pDM_Odm->SupportPlatform & ODM_WIN) && !(pDM_Odm->SupportICType & (ODM_RTL8814A|ODM_RTL8822B)))
			{
				ODM_RT_TRACE(pDM_Odm,ODM_COMP_DIG, ODM_DBG_LOUD, ("===> Fix IGI\n"));
				ODM_Write1Byte( pDM_Odm, ODM_REG(IGI_A,pDM_Odm), IGI_A);
				ODM_Write1Byte( pDM_Odm, ODM_REG(IGI_B,pDM_Odm), IGI_B);
				pDM_DigTable->CurIGValue = IGI_B;
			}
			else
				odm_MPT_Write_DIG(pDM_Odm, IGI_A);
		}

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
		pDM_Odm->Times_2G++;

		if (pDM_Odm->Times_2G == 3)
#endif
		{
			pDM_Odm->MPDIG_2G = TRUE;
		}
	}

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	if (pDM_Odm->SupportICType == ODM_RTL8812)
		odm_RFEControl(pDM_Odm, RxPWDBAve_final);
#endif

	ODM_SetTimer(pDM_Odm, &pDM_Odm->MPT_DIGTimer, 700);
}
#endif


