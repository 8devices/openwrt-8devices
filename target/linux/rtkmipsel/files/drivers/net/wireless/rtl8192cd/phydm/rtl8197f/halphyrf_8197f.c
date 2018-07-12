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

#include "mp_precomp.h"
#include "../phydm_precomp.h"

#if (RTL8197F_SUPPORT == 1)

void setIqkMatrix_8197F(
	PVOID		pDM_VOID,
	u1Byte		OFDM_index,
	u1Byte		RFPath,
	s4Byte		IqkResult_X,
	s4Byte		IqkResult_Y
)
{
	PDM_ODM_T	pDM_Odm	= (PDM_ODM_T)pDM_VOID;
	
	s4Byte		ele_A = 0, ele_D, ele_C = 0, value32;

	ele_D = (OFDMSwingTable_New[OFDM_index] & 0xFFC00000) >> 22;

	/*new element A = element D x X*/
	if (IqkResult_X != 0) {
		if ((IqkResult_X & 0x00000200) != 0)	/*consider minus*/
			IqkResult_X = IqkResult_X | 0xFFFFFC00;
		ele_A = ((IqkResult_X * ele_D) >> 8) & 0x000003FF;

		/*new element C = element D x Y*/
		if ((IqkResult_Y & 0x00000200) != 0)
			IqkResult_Y = IqkResult_Y | 0xFFFFFC00;
		ele_C = ((IqkResult_Y * ele_D) >> 8) & 0x000003FF;

		/*if (RFPath == ODM_RF_PATH_A)// Remove this to Fix Path B PowerTracking */
		switch (RFPath) {
		case ODM_RF_PATH_A:
			/*write new elements A, C, D to regC80 and regC94, element B is always 0*/
			value32 = (ele_D << 22) | ((ele_C & 0x3F) << 16) | ele_A;
			ODM_SetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, bMaskDWord, value32);

			value32 = (ele_C & 0x000003C0) >> 6;
			ODM_SetBBReg(pDM_Odm, rOFDM0_XCTxAFE, bMaskH4Bits, value32);

			value32 = ((IqkResult_X * ele_D) >> 7) & 0x01;
			ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT24, value32);
			break;
		case ODM_RF_PATH_B:
			/*write new elements A, C, D to regC88 and regC9C, element B is always 0*/
			value32 = (ele_D << 22) | ((ele_C & 0x3F) << 16) | ele_A;
			ODM_SetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, bMaskDWord, value32);

			value32 = (ele_C & 0x000003C0) >> 6;
			ODM_SetBBReg(pDM_Odm, rOFDM0_XDTxAFE, bMaskH4Bits, value32);

			value32 = ((IqkResult_X * ele_D) >> 7) & 0x01;
			ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT28, value32);

			break;
		default:
			break;
		}
	} else {
		switch (RFPath) {
		case ODM_RF_PATH_A:
			ODM_SetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, bMaskDWord, OFDMSwingTable_New[OFDM_index]);
			ODM_SetBBReg(pDM_Odm, rOFDM0_XCTxAFE, bMaskH4Bits, 0x00);
			ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT24, 0x00);
			break;

		case ODM_RF_PATH_B:
			ODM_SetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, bMaskDWord, OFDMSwingTable_New[OFDM_index]);
			ODM_SetBBReg(pDM_Odm, rOFDM0_XDTxAFE, bMaskH4Bits, 0x00);
			ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT28, 0x00);
			break;

		default:
			break;
		}
	}
	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPwrTracking path %c: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x 0xeb4 = 0x%x 0xebc = 0x%x\n",
				 (RFPath == ODM_RF_PATH_A ? 'A' : 'B'), (u4Byte)IqkResult_X, (u4Byte)IqkResult_Y, (u4Byte)ele_A, (u4Byte)ele_C, (u4Byte)ele_D, (u4Byte)IqkResult_X, (u4Byte)IqkResult_Y));

}


#if 0
VOID
setCCKFilterCoefficient_8197F(
	PVOID		pDM_VOID,
	u1Byte		CCKSwingIndex
)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	prtl8192cd_priv priv = pDM_Odm->priv;

	unsigned char channel;

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific)
		channel = priv->pshare->working_channel;
	else
#endif
		channel = (priv->pmib->dot11RFEntry.dot11channel);
		
	if (channel != 14) {
		ODM_Write1Byte(pDM_Odm, 0xa22, CCKSwingTable_Ch1_Ch13_New[CCKSwingIndex][0]);
		ODM_Write1Byte(pDM_Odm, 0xa23, CCKSwingTable_Ch1_Ch13_New[CCKSwingIndex][1]);
		ODM_Write1Byte(pDM_Odm, 0xa24, CCKSwingTable_Ch1_Ch13_New[CCKSwingIndex][2]);
		ODM_Write1Byte(pDM_Odm, 0xa25, CCKSwingTable_Ch1_Ch13_New[CCKSwingIndex][3]);
		ODM_Write1Byte(pDM_Odm, 0xa26, CCKSwingTable_Ch1_Ch13_New[CCKSwingIndex][4]);
		ODM_Write1Byte(pDM_Odm, 0xa27, CCKSwingTable_Ch1_Ch13_New[CCKSwingIndex][5]);
		ODM_Write1Byte(pDM_Odm, 0xa28, CCKSwingTable_Ch1_Ch13_New[CCKSwingIndex][6]);
		ODM_Write1Byte(pDM_Odm, 0xa29, CCKSwingTable_Ch1_Ch13_New[CCKSwingIndex][7]);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("CCK Channel=%d 0xa20=0x%x 0xa24=0x%x 0xa28=0x%x\n", channel, ODM_GetBBReg(pDM_Odm, 0xa20, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xa24, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xa28, bMaskDWord)));
	} else {
		ODM_Write1Byte(pDM_Odm, 0xa22, CCKSwingTable_Ch14_New[CCKSwingIndex][0]);
		ODM_Write1Byte(pDM_Odm, 0xa23, CCKSwingTable_Ch14_New[CCKSwingIndex][1]);
		ODM_Write1Byte(pDM_Odm, 0xa24, CCKSwingTable_Ch14_New[CCKSwingIndex][2]);
		ODM_Write1Byte(pDM_Odm, 0xa25, CCKSwingTable_Ch14_New[CCKSwingIndex][3]);
		ODM_Write1Byte(pDM_Odm, 0xa26, CCKSwingTable_Ch14_New[CCKSwingIndex][4]);
		ODM_Write1Byte(pDM_Odm, 0xa27, CCKSwingTable_Ch14_New[CCKSwingIndex][5]);
		ODM_Write1Byte(pDM_Odm, 0xa28, CCKSwingTable_Ch14_New[CCKSwingIndex][6]);
		ODM_Write1Byte(pDM_Odm, 0xa29, CCKSwingTable_Ch14_New[CCKSwingIndex][7]);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("CCK Channel=%d 0xa20=0x%x 0xa24=0x%x 0xa28=0x%x\n", channel, ODM_GetBBReg(pDM_Odm, 0xa20, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xa24, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xa28, bMaskDWord)));
	}
}
#endif



VOID
ODM_TxPwrTrackSetPwr8197F(
	PVOID				pDM_VOID,
	PWRTRACK_METHOD		Method,
	u1Byte				RFPath,
	u1Byte				ChannelMappedIndex
	)
{
	PDM_ODM_T		pDM_Odm		= (PDM_ODM_T)pDM_VOID;
	PODM_RF_CAL_T	pRFCalibrateInfo	= &(pDM_Odm->RFCalibrateInfo);
	prtl8192cd_priv priv = pDM_Odm->priv;
	s1Byte			Final_OFDM_Swing_Index = 0;
	s1Byte			Final_CCK_Swing_Index = 0;
	u4Byte			BitMask_10_0 = (BIT10 | BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
	u4Byte			BitMask_21_11 = (BIT21 | BIT20 | BIT19 | BIT18 | BIT17 | BIT16 | BIT15 | BIT14 | BIT13 | BIT12 | BIT11);
		
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, 
		("pRF->DefaultOfdmIndex=%d   pRF->DefaultCckIndex=%d\n", pRFCalibrateInfo->DefaultOfdmIndex, pRFCalibrateInfo->DefaultCckIndex));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, 
		("pRF->Absolute_OFDMSwingIdx=%d   pRF->Remnant_OFDMSwingIdx=%d   pRF->Absolute_CCKSwingIdx=%d   pRF->Remnant_CCKSwingIdx=%d   RFPath=%d\n",
		pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->Remnant_OFDMSwingIdx[RFPath], pRFCalibrateInfo->Absolute_CCKSwingIdx[RFPath], pRFCalibrateInfo->Remnant_CCKSwingIdx, RFPath));

	Final_OFDM_Swing_Index = pRFCalibrateInfo->DefaultOfdmIndex + pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath];
	Final_CCK_Swing_Index = pRFCalibrateInfo->DefaultCckIndex + pRFCalibrateInfo->Absolute_CCKSwingIdx[RFPath];

	/*OFDM BB-Swing Index Limit*/
	if (Final_OFDM_Swing_Index < 0)
		Final_OFDM_Swing_Index = 0;
	else if (Final_OFDM_Swing_Index > OFDM_TABLE_SIZE_92D - 1)
		Final_OFDM_Swing_Index = OFDM_TABLE_SIZE_92D - 1;

	/*CCK BB-Swing Index Limit*/
	if (Final_CCK_Swing_Index < 0)
		Final_CCK_Swing_Index = 0;
	else if (Final_CCK_Swing_Index > CCK_TABLE_SIZE_8723D - 1)
		Final_CCK_Swing_Index = CCK_TABLE_SIZE_8723D - 1;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, 
		("Final_OFDM_Swing_Index=%d	 Final_CCK_Swing_Index=%d RFPath=%d\n", Final_OFDM_Swing_Index, Final_CCK_Swing_Index, RFPath));



	if (Method == BBSWING) {
		switch (RFPath) {
			
		case ODM_RF_PATH_A:
			//setIqkMatrix_8197F(pDM_VOID, Final_OFDM_Swing_Index, RFPath, priv->pshare->RegE94, priv->pshare->RegE9C);
			ODM_SetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, bMaskDWord, OFDMSwingTable_New[Final_OFDM_Swing_Index]);
/*			setCCKFilterCoefficient_8197F(pDM_VOID, Final_CCK_Swing_Index);*/
			ODM_SetBBReg(pDM_Odm, 0xAb4, BitMask_10_0, CCKSwingTable_Ch1_Ch14_8723D[Final_CCK_Swing_Index]);

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("BBSwing=0x%x CCKBBSwing=0x%x RFPath=%d\n", 
				ODM_GetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, bMaskDWord), 
				ODM_GetBBReg(pDM_Odm, 0xAb4, BitMask_10_0), RFPath));
		break;
		
		case ODM_RF_PATH_B:
			//setIqkMatrix_8197F(pDM_VOID, Final_OFDM_Swing_Index, RFPath, priv->pshare->RegEB4, priv->pshare->RegEBC);
			ODM_SetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, bMaskDWord, OFDMSwingTable_New[Final_OFDM_Swing_Index]);
/*			setCCKFilterCoefficient_8197F(pDM_VOID, Final_CCK_Swing_Index);*/
			ODM_SetBBReg(pDM_Odm, 0xAb4, BitMask_21_11, CCKSwingTable_Ch1_Ch14_8723D[Final_CCK_Swing_Index]);

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("BBSwing=0x%x CCKBBSwing=0x%x RFPath=%d\n", 
				ODM_GetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, bMaskDWord), 
				ODM_GetBBReg(pDM_Odm, 0xAb4, BitMask_21_11), RFPath));
		break;
		
		default:
			break;
		}
		
		
	}

}

VOID
GetDeltaSwingTable_8197F(
	PVOID		pDM_VOID,
	pu1Byte		*TemperatureUP_A,
	pu1Byte		*TemperatureDOWN_A,
	pu1Byte		*TemperatureUP_B,
	pu1Byte		*TemperatureDOWN_B,
	pu1Byte		*TemperatureUP_CCK_A,
	pu1Byte		*TemperatureDOWN_CCK_A,
	pu1Byte		*TemperatureUP_CCK_B,
	pu1Byte		*TemperatureDOWN_CCK_B
	)
{
	PDM_ODM_T		pDM_Odm		= (PDM_ODM_T)pDM_VOID;
	PODM_RF_CAL_T	pRFCalibrateInfo	= &(pDM_Odm->RFCalibrateInfo);
	u1Byte			channel			= *(pDM_Odm->pChannel);

	if (channel >= 1 && channel <= 14) {
		*TemperatureUP_CCK_A   = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_P;
		*TemperatureDOWN_CCK_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_N;
		*TemperatureUP_CCK_B   = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_P;
		*TemperatureDOWN_CCK_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_N;
		
		*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_2GA_P;
		*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GA_N;
		*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_2GB_P;
		*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GB_N;
	} else {
		*TemperatureUP_CCK_A	= (pu1Byte)DeltaSwingTableIdx_2GA_P_DEFAULT;
		*TemperatureDOWN_CCK_A	= (pu1Byte)DeltaSwingTableIdx_2GA_N_DEFAULT;
		*TemperatureUP_CCK_B	= (pu1Byte)DeltaSwingTableIdx_2GA_P_DEFAULT;
		*TemperatureDOWN_CCK_B	= (pu1Byte)DeltaSwingTableIdx_2GA_N_DEFAULT;
		*TemperatureUP_A	= (pu1Byte)DeltaSwingTableIdx_2GA_P_DEFAULT;
		*TemperatureDOWN_A	= (pu1Byte)DeltaSwingTableIdx_2GA_N_DEFAULT;
		*TemperatureUP_B	= (pu1Byte)DeltaSwingTableIdx_2GA_P_DEFAULT;
		*TemperatureDOWN_B	= (pu1Byte)DeltaSwingTableIdx_2GA_N_DEFAULT;
	}
	
	return;
	
}



void ConfigureTxpowerTrack_8197F(
	PTXPWRTRACK_CFG	pConfig
	)
{
	pConfig->SwingTableSize_CCK = TXSCALE_TABLE_SIZE;
	pConfig->SwingTableSize_OFDM = TXSCALE_TABLE_SIZE;
	pConfig->Threshold_IQK = IQK_THRESHOLD;
	pConfig->Threshold_DPK = DPK_THRESHOLD;	
	pConfig->AverageThermalNum = AVG_THERMAL_NUM_8197F;
	pConfig->RfPathCount = MAX_PATH_NUM_8197F;
	pConfig->ThermalRegAddr = RF_T_METER_8197F;
		
	pConfig->ODM_TxPwrTrackSetPwr = ODM_TxPwrTrackSetPwr8197F;
	pConfig->DoIQK = DoIQK_8197F;
	pConfig->PHY_LCCalibrate = PHY_LCCalibrate_8197F;
	pConfig->GetDeltaAllSwingTable = GetDeltaSwingTable_8197F;
}

#if 0

VOID PHY_SetRFPathSwitch_8197F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	BOOLEAN		bMain
	)
{
}

BOOLEAN 
phy_QueryRFPathSwitch_8197F(
	IN	PADAPTER	pAdapter
	)
{
	return TRUE;
}


BOOLEAN PHY_QueryRFPathSwitch_8197F(	
	IN	PADAPTER	pAdapter
	)
{

#if DISABLE_BB_RF
	return TRUE;
#endif

	return phy_QueryRFPathSwitch_8197F(pAdapter);
}
#endif

#endif	/* (RTL8197F_SUPPORT == 0)*/
