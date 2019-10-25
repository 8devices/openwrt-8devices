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

#if (RTL8814A_SUPPORT == 1)


/*---------------------------Define Local Constant---------------------------*/
// 2010/04/25 MH Define the max tx power tracking tx agc power.
#define		ODM_TXPWRTRACK_MAX_IDX_8814A		6

/*---------------------------Define Local Constant---------------------------*/

/*3============================================================*/
//3 Tx Power Tracking
/*3============================================================*/
#define	rA_TXAGC	0xC94
#define	rB_TXAGC	0xE94
#define	rC_TXAGC	0x1894
#define	rD_TXAGC	0x1A94
#define	TXAGC_BITMASK	(BIT29 | BIT28 | BIT27 | BIT26 | BIT25)
#define	rA_BBSWING	0xC1C
#define	rB_BBSWING	0xE1C
#define	rC_BBSWING	0x181C
#define	rD_BBSWING	0x1A1C
#define	BBSWING_BITMASK	0xFFE00000


#if 0

u1Byte CheckRFGainOffset(
	PDM_ODM_T			pDM_Odm,
	PWRTRACK_METHOD			Method,
	u1Byte				RFPath
)
{
	s1Byte	UpperBound = 10, LowerBound = -5;	/*4'b1010 = 10*/
	s1Byte	Final_RF_Index = 0;
	BOOLEAN	bPositive = FALSE;
	u4Byte	bitMask = 0;
	u1Byte	Final_OFDM_Swing_Index = 0, TxScalingUpperBound = 28, TxScalingLowerBound = 4;	/*upper bound +2dB, lower bound -10dB*/
	PODM_RF_CAL_T	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

	if (Method == MIX_MODE) {	/*normal Tx power tracking*/
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("is 8814 MP chip\n"));
		bitMask = BIT19;
		pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] = pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] + pRFCalibrateInfo->KfreeOffset[RFPath];

		if (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] >= 0)				/*check if RF_Index is positive or not*/
			bPositive = TRUE;
		else
			bPositive = FALSE;

		ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, rRF_TxGainOffset, bitMask, bPositive);

		bitMask = BIT18 | BIT17 | BIT16 | BIT15;
		Final_RF_Index = pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] / 2;		/*TxBB 1 step equal 1dB, BB swing 1step equal 0.5dB*/

	}

	if (Final_RF_Index > UpperBound) {		/*Upper bound = 10dB, if more htan upper bound, then move to bb swing max = +2dB*/
		ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, rRF_TxGainOffset, bitMask, UpperBound);	/*set RF Reg0x55 per path*/

		Final_OFDM_Swing_Index = pRFCalibrateInfo->DefaultOfdmIndex + (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] - (UpperBound << 1));

		if (Final_OFDM_Swing_Index > TxScalingUpperBound)	/*bb swing upper bound = +2dB*/
			Final_OFDM_Swing_Index = TxScalingUpperBound;

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("******Path-%d Compensate with TXBB = %d\n", RFPath, UpperBound));

		return Final_OFDM_Swing_Index;
	} else if (Final_RF_Index < LowerBound) {	/*lower bound = -5dB*/
		ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, rRF_TxGainOffset, bitMask, (-1) * (LowerBound));	/*set RF Reg0x55 per path*/

		Final_OFDM_Swing_Index = pRFCalibrateInfo->DefaultOfdmIndex - ((LowerBound << 1) - pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);

		if (Final_OFDM_Swing_Index < TxScalingLowerBound)	/*bb swing lower bound = -10dB*/
			Final_OFDM_Swing_Index = TxScalingLowerBound;

		return Final_OFDM_Swing_Index;
		
	} else {	/*normal case*/

		if (bPositive == TRUE)
			ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, rRF_TxGainOffset, bitMask, Final_RF_Index);	/*set RF Reg0x55 per path*/
		else
			ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, rRF_TxGainOffset, bitMask, (-1)*Final_RF_Index);	/*set RF Reg0x55 per path*/

		Final_OFDM_Swing_Index = pRFCalibrateInfo->DefaultOfdmIndex + (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]) % 2;
		return Final_OFDM_Swing_Index;
	}

	return FALSE;
}

#endif

u1Byte GetTSSIVALUE(
	PDM_ODM_T			pDM_Odm,
	PWRTRACK_METHOD	Method,
	u1Byte				RFPath
)
{

	PADAPTER		Adapter = pDM_Odm->Adapter;
	PODM_RF_CAL_T		pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	
	s1Byte				PowerByRateValue = 0;
	u1Byte				Tx_num, TSSIValue = 0;
	u1Byte				Channel  = pHalData->CurrentChannel;
	u1Byte				BandWidth  = pHalData->CurrentChannelBW;
	u1Byte				TxRate = 0xFF;
	u1Byte				TxLimit = 0;
	u1Byte				RegPwrTblSel = 0;

#ifdef CONFIG_PHYDM_POWERTRACK_BY_TSSI


#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PMGNT_INFO			pMgntInfo = &(Adapter->MgntInfo);

	RegPwrTblSel = pMgntInfo->RegPwrTblSel;
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	RegPwrTblSel = Adapter->registrypriv.RegPwrTblSel;
#endif
#endif

	if (pDM_Odm->mp_mode == TRUE) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#if (MP_DRIVER == 1)
		PMPT_CONTEXT pMptCtx = &(Adapter->MptCtx);

		TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
#endif
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
		PMPT_CONTEXT pMptCtx = &(Adapter->mppriv.MptCtx);

		TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
#endif
#endif
	} else {
		u2Byte	rate	 = *(pDM_Odm->pForcedDataRate);

		if (!rate) { /*auto rate*/
			if (rate != 0xFF) {
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
				TxRate = Adapter->HalFunc.GetHwRateFromMRateHandler(pDM_Odm->TxRate);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
				if (pDM_Odm->number_linked_client != 0)
					TxRate = HwRateToMRate(pDM_Odm->TxRate);
#endif
			}
		} else { /*force rate*/
			TxRate = (u1Byte) rate;
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s TxRate=0x%X\n", __func__, TxRate));
	Tx_num = MgntQuery_NssTxRate(TxRate);


	if (Channel >= 1 && Channel <= 14) {
		PowerByRateValue = PHY_GetTxPowerByRateOriginal(Adapter, BAND_ON_2_4G, (ODM_RF_RADIO_PATH_E) RFPath, Tx_num, TxRate);
		TxLimit = PHY_GetTxPowerLimitOriginal(Adapter, RegPwrTblSel, BAND_ON_2_4G, BandWidth, ODM_RF_PATH_A, TxRate, Channel);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s PowerByRateValue=%d   TxRate=0x%X RFPath=%d   Tx_num=%d\n", __func__, PowerByRateValue, TxRate, RFPath, Tx_num));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s TxLimit=%d   RegPwrTblSel=0x%X BandWidth=%d   Channel=%d\n", __func__, TxLimit, RegPwrTblSel, BandWidth, Channel));

		PowerByRateValue = PowerByRateValue > TxLimit ? TxLimit : PowerByRateValue;

		if (IS_CCK_RATE(TxRate)) {
			switch (RFPath) {
			case ODM_RF_PATH_A:
				TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_2GCCKA[PowerByRateValue];
				break;

			case ODM_RF_PATH_B:
				TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_2GCCKB[PowerByRateValue];
				break;

			case ODM_RF_PATH_C:
				TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_2GCCKC[PowerByRateValue];
				break;

			case ODM_RF_PATH_D:
				TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_2GCCKD[PowerByRateValue];
				break;

			default:
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							("Call:%s Wrong Path name!!!\n", __func__));			
			break;
			}
		} else {
			switch (RFPath) {
			case ODM_RF_PATH_A:
				TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_2GA[PowerByRateValue];
				break;

			case ODM_RF_PATH_B:
				TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_2GB[PowerByRateValue];
				break;

			case ODM_RF_PATH_C:
				TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_2GC[PowerByRateValue];
				break;

			case ODM_RF_PATH_D:
				TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_2GD[PowerByRateValue];
				break;

			default:
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							  ("Call:%s Wrong Path name!!!!\n", __func__));
			break;
			}
		}
	} else if (Channel >= 36 && Channel <= 64) {
		PowerByRateValue = PHY_GetTxPowerByRateOriginal(Adapter, BAND_ON_5G, (ODM_RF_RADIO_PATH_E) RFPath, Tx_num, TxRate);

		TxLimit = PHY_GetTxPowerLimitOriginal(Adapter, RegPwrTblSel, BAND_ON_5G, BandWidth, ODM_RF_PATH_A, TxRate, Channel);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s PowerByRateValue=%d   TxRate=0x%X RFPath=%d   Tx_num=%d\n", __func__, PowerByRateValue, TxRate, RFPath, Tx_num));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s TxLimit=%d   RegPwrTblSel=0x%X BandWidth=%d   Channel=%d\n", __func__, TxLimit, RegPwrTblSel, BandWidth, Channel));

		PowerByRateValue = PowerByRateValue > TxLimit ? TxLimit : PowerByRateValue;

		switch (RFPath) {
		case ODM_RF_PATH_A:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GA[0][PowerByRateValue];
			break;

		case ODM_RF_PATH_B:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GB[0][PowerByRateValue];
			break;

		case ODM_RF_PATH_C:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GC[0][PowerByRateValue];
			break;

		case ODM_RF_PATH_D:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GD[0][PowerByRateValue];
			break;

		default:
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						  ("Call:%s Wrong Path name!!!!\n", __func__));
			break;
		}
	} else if (Channel >= 100 && Channel <= 144) {
		PowerByRateValue = PHY_GetTxPowerByRateOriginal(Adapter, BAND_ON_5G, (ODM_RF_RADIO_PATH_E) RFPath, Tx_num, TxRate);

		TxLimit = PHY_GetTxPowerLimitOriginal(Adapter, RegPwrTblSel, BAND_ON_5G, BandWidth, ODM_RF_PATH_A, TxRate, Channel);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s PowerByRateValue=%d   TxRate=0x%X RFPath=%d   Tx_num=%d\n", __func__, PowerByRateValue, TxRate, RFPath, Tx_num));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s TxLimit=%d   RegPwrTblSel=0x%X BandWidth=%d   Channel=%d\n", __func__, TxLimit, RegPwrTblSel, BandWidth, Channel));

		PowerByRateValue = PowerByRateValue > TxLimit ? TxLimit : PowerByRateValue;

		switch (RFPath) {
		case ODM_RF_PATH_A:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GA[1][PowerByRateValue];
			break;

		case ODM_RF_PATH_B:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GB[1][PowerByRateValue];
			break;

		case ODM_RF_PATH_C:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GC[1][PowerByRateValue];
			break;

		case ODM_RF_PATH_D:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GD[1][PowerByRateValue];
			break;

		default:
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						  ("Call:%s Wrong Path name!!!!\n", __func__));
			break;
		}
	} else if (Channel >= 149 && Channel <= 173) {
		PowerByRateValue = PHY_GetTxPowerByRateOriginal(Adapter, BAND_ON_5G, (ODM_RF_RADIO_PATH_E) RFPath, Tx_num, TxRate);

		TxLimit = PHY_GetTxPowerLimitOriginal(Adapter, RegPwrTblSel, BAND_ON_5G, BandWidth, ODM_RF_PATH_A, TxRate, Channel);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s PowerByRateValue=%d   TxRate=0x%X RFPath=%d   Tx_num=%d\n", __func__, PowerByRateValue, TxRate, RFPath, Tx_num));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s TxLimit=%d   RegPwrTblSel=0x%X BandWidth=%d   Channel=%d\n", __func__, TxLimit, RegPwrTblSel, BandWidth, Channel));

		PowerByRateValue = PowerByRateValue > TxLimit ? TxLimit : PowerByRateValue;

		switch (RFPath) {
		case ODM_RF_PATH_A:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GA[2][PowerByRateValue];
			break;

		case ODM_RF_PATH_B:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GB[2][PowerByRateValue];
			break;

		case ODM_RF_PATH_C:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GC[2][PowerByRateValue];
			break;

		case ODM_RF_PATH_D:
			TSSIValue = pRFCalibrateInfo->DeltaSwingTSSITable_5GD[2][PowerByRateValue];
			break;

		default:
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						  ("Call:%s Wrong Path name!!!!\n", __func__));
			break;
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s Index=%d   TSSIValue=%d\n", __func__, PowerByRateValue, TSSIValue));

#endif	/*CONFIG_PHYDM_POWERTRACK_BY_TSSI*/

	return TSSIValue;
}

BOOLEAN GetTSSIModeTxAGCBBSWingOffset(
	IN PDM_ODM_T			pDM_Odm,
	IN PWRTRACK_METHOD	Method,
	IN u1Byte					RFPath,
	IN u4Byte					OffsetVaule,
	IN u1Byte					TxPowerIndexOffest
)
{
	PODM_RF_CAL_T	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

	u1Byte		BBSwingUpperBound = pRFCalibrateInfo->DefaultOfdmIndex + 10;
	u1Byte		BBSwingLowerBound = 0;
	u1Byte		TX_AGC_Index = (u1Byte) pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath];
	u1Byte		TX_BBSwing_Index = (u1Byte) pRFCalibrateInfo->BbSwingIdxOfdm[RFPath];

	if (TxPowerIndexOffest > 0XF)
		TxPowerIndexOffest = 0XF;

	if (TX_AGC_Index == 0 && TX_BBSwing_Index == pRFCalibrateInfo->DefaultOfdmIndex) {
		if ((OffsetVaule & 0X20) >> 5 == 0) {
			OffsetVaule = OffsetVaule & 0X1F;

			if (OffsetVaule > TxPowerIndexOffest) {
				TX_AGC_Index = TxPowerIndexOffest;
				TX_BBSwing_Index = TX_BBSwing_Index + (u1Byte) OffsetVaule - TxPowerIndexOffest;

				if (TX_BBSwing_Index > BBSwingUpperBound)
					TX_BBSwing_Index = BBSwingUpperBound;

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							("TX_AGC_Index(0) TX_BBSwing_Index(18) +++ ( OffsetVaule > 0XF) OffsetVaule = 0X%X	TXAGCIndex = 0X%X	TX_BBSwing_Index = %d\n", OffsetVaule, TX_AGC_Index, TX_BBSwing_Index));
			} else {
				TX_AGC_Index = (u1Byte) OffsetVaule;
				TX_BBSwing_Index = pRFCalibrateInfo->DefaultOfdmIndex;

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							("TX_AGC_Index(0) TX_BBSwing_Index(18) +++ ( OffsetVaule <= 0XF) OffsetVaule = 0X%X	TXAGCIndex = 0X%X	TX_BBSwing_Index = %d\n", OffsetVaule, TX_AGC_Index, TX_BBSwing_Index));
			}
		} else {
			TX_AGC_Index = 0;
			OffsetVaule = ((~OffsetVaule) + 1) & 0X1F;

			if (TX_BBSwing_Index >= (u1Byte) OffsetVaule)
				TX_BBSwing_Index = TX_BBSwing_Index - (u1Byte) OffsetVaule;
			else
				TX_BBSwing_Index = BBSwingLowerBound;

			if (TX_BBSwing_Index <= BBSwingLowerBound)
				TX_BBSwing_Index = BBSwingLowerBound;

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("TX_AGC_Index(0) TX_BBSwing_Index(18) --- OffsetVaule = 0X%X   TXAGCIndex = 0X%X   TX_BBSwing_Index = %d\n", OffsetVaule, TX_AGC_Index, TX_BBSwing_Index));
		}

	} else if (TX_AGC_Index > 0 && TX_BBSwing_Index == pRFCalibrateInfo->DefaultOfdmIndex) {
		if ((OffsetVaule & 0X20) >> 5 == 0) {
			if (OffsetVaule > TxPowerIndexOffest) {
				TX_AGC_Index = TxPowerIndexOffest;
				TX_BBSwing_Index = TX_BBSwing_Index + (u1Byte) OffsetVaule - TxPowerIndexOffest;

				if (TX_BBSwing_Index > BBSwingUpperBound)
					TX_BBSwing_Index = BBSwingUpperBound;
			} else
				TX_AGC_Index = (u1Byte) OffsetVaule;

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("TX_AGC_Index > 0 TX_BBSwing_Index(18) +++ OffsetVaule = 0X%X   TXAGCIndex = 0X%X   TX_BBSwing_Index = %d\n", OffsetVaule, TX_AGC_Index, TX_BBSwing_Index));
		} else {
			TX_AGC_Index = 0;
			OffsetVaule = ((~OffsetVaule) + 1) & 0X1F;

			if (TX_BBSwing_Index >= (u1Byte) OffsetVaule)
				TX_BBSwing_Index = TX_BBSwing_Index - (u1Byte) OffsetVaule;
			else
				TX_BBSwing_Index = BBSwingLowerBound;

			if (TX_BBSwing_Index <= BBSwingLowerBound)
				TX_BBSwing_Index = BBSwingLowerBound;

			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("TX_AGC_Index > 0 TX_BBSwing_Index(18)  --- OffsetVaule = 0X%X	TXAGCIndex = 0X%X	TX_BBSwing_Index = %d\n", OffsetVaule, TX_AGC_Index, TX_BBSwing_Index));
		}


	} else if (TX_AGC_Index > 0 && TX_BBSwing_Index > pRFCalibrateInfo->DefaultOfdmIndex) {
		if ((OffsetVaule & 0X20) >> 5 == 0) {
			TX_AGC_Index = TxPowerIndexOffest;
			TX_BBSwing_Index = TX_BBSwing_Index + (u1Byte) OffsetVaule - TxPowerIndexOffest;

			if (TX_BBSwing_Index > BBSwingUpperBound)
				TX_BBSwing_Index = BBSwingUpperBound;

			if (TX_BBSwing_Index < pRFCalibrateInfo->DefaultOfdmIndex) {
				TX_AGC_Index = TxPowerIndexOffest - (pRFCalibrateInfo->DefaultOfdmIndex - TX_BBSwing_Index);
				TX_BBSwing_Index = pRFCalibrateInfo->DefaultOfdmIndex;
			}
		} else {
			TX_AGC_Index = 0;
			OffsetVaule = ((~OffsetVaule) + 1) & 0X1F;

			if (TX_BBSwing_Index >= (u1Byte) OffsetVaule)
				TX_BBSwing_Index = pRFCalibrateInfo->DefaultOfdmIndex - (u1Byte) OffsetVaule;
			else
				TX_BBSwing_Index = BBSwingLowerBound;

			if (TX_BBSwing_Index <= BBSwingLowerBound)
				TX_BBSwing_Index = BBSwingLowerBound;
		}

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
					("TX_AGC_Index>0 TX_BBSwing_Index>18 --- OffsetVaule = 0X%X	 TXAGCIndex = 0X%X	 TX_BBSwing_Index = %d	 OffsetVaule=%d\n", OffsetVaule, TX_AGC_Index, TX_BBSwing_Index, OffsetVaule - TxPowerIndexOffest));

	} else if (TX_AGC_Index == 0 && TX_BBSwing_Index < pRFCalibrateInfo->DefaultOfdmIndex) {
		if ((OffsetVaule & 0X20) >> 5 == 1) {
			OffsetVaule = ((~OffsetVaule) + 1) & 0X1F;

			if (TX_BBSwing_Index >= (u1Byte) OffsetVaule)
				TX_BBSwing_Index = TX_BBSwing_Index - (u1Byte) OffsetVaule;
			else
				TX_BBSwing_Index = BBSwingLowerBound;
		} else {
			OffsetVaule = (OffsetVaule & 0x1F);
			TX_BBSwing_Index = TX_BBSwing_Index + (u1Byte) OffsetVaule;

			if (TX_BBSwing_Index > pRFCalibrateInfo->DefaultOfdmIndex) {
				TX_AGC_Index = TX_BBSwing_Index - pRFCalibrateInfo->DefaultOfdmIndex;
				TX_BBSwing_Index = pRFCalibrateInfo->DefaultOfdmIndex;

				if (TX_AGC_Index > TxPowerIndexOffest) {
					TX_BBSwing_Index = pRFCalibrateInfo->DefaultOfdmIndex + (u1Byte) (TX_AGC_Index) - TxPowerIndexOffest;
					TX_AGC_Index = TxPowerIndexOffest;

					if (TX_BBSwing_Index  > BBSwingUpperBound)
						TX_BBSwing_Index = BBSwingUpperBound;
				}
			}
		}

		if (TX_BBSwing_Index <= BBSwingLowerBound) {
			TX_BBSwing_Index = BBSwingLowerBound;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						("Call %s  Path_%d BBSwing Lower Bound\n", __func__, RFPath));
		}

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
					("TX_AGC_Index(0) TX_BBSwing_Index < 18 OffsetVaule = 0X%X	 TXAGCIndex = 0X%X	 TX_BBSwing_Index = %d\n", OffsetVaule, TX_AGC_Index, TX_BBSwing_Index));

	}

	if ((TX_AGC_Index == pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]) && (TX_BBSwing_Index == pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]))
		return FALSE;
	
	else {
		pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] = TX_AGC_Index;
		pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] = TX_BBSwing_Index;
		return TRUE;
	}

}


VOID SetTxAGCBBSWingOffset(
	PDM_ODM_T			pDM_Odm,
	PWRTRACK_METHOD 	Method,
	u1Byte				RFPath
)
{

	PADAPTER		Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	u1Byte				TxRate = 0xFF;
	u1Byte				Channel  = pHalData->CurrentChannel;
	u1Byte				BandWidth  = pHalData->CurrentChannelBW;
	u4Byte				TxPath  = pHalData->AntennaTxPath;

	u1Byte				TSSIValue = 0;
	u1Byte				TxPowerIndex = 0;
	u1Byte				TxPowerIndexOffest = 0;
	u4Byte				OffsetVaule = 0;
	u4Byte				TSSIFunction = 0;
	u4Byte				TXBBSwing = 0;
	u1Byte				TX_BBSwing_Index = 0;
	u4Byte				TX_AGC_Index = 0;
	u4Byte				WaitTXAGCOffsetTimer = 0;
	u1Byte				i = 0;
	BOOLEAN			rtn = FALSE;

	PODM_RF_CAL_T	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

	if (pDM_Odm->mp_mode == TRUE) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#if (MP_DRIVER == 1)
		PMPT_CONTEXT pMptCtx = &(Adapter->MptCtx);

		TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
#endif
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
		PMPT_CONTEXT pMptCtx = &(Adapter->mppriv.MptCtx);

		TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
#endif
#endif
	} else {
		u2Byte	rate	 = *(pDM_Odm->pForcedDataRate);

		if (!rate) { /*auto rate*/
			if (rate != 0xFF) {
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
				TxRate = Adapter->HalFunc.GetHwRateFromMRateHandler(pDM_Odm->TxRate);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
				if (pDM_Odm->number_linked_client != 0)
					TxRate = HwRateToMRate(pDM_Odm->TxRate);
#endif
			}
		} else { /*force rate*/
			TxRate = (u1Byte) rate;
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s TxRate=0x%X\n", __func__, TxRate));

	if (Method == TSSI_MODE) {
		switch (RFPath) {
		case ODM_RF_PATH_A:
				/*Disable Path B TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x65, BIT10, 0);
				/*Enable Path A TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x65, BIT10, 1);

				/*Read power by rate table to set TSSI value by power and set rf reg 0x65[19:15]*/
				TSSIValue = GetTSSIVALUE(pDM_Odm, TSSI_MODE, (ODM_RF_RADIO_PATH_E) RFPath);

				ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT19 | BIT18 | BIT17 | BIT16 | BIT15, TSSIValue);

				/*Write BB 0xC8C for Setting Max. packet (30) of tracking power and the initial Value of TXAGC*/
				ODM_SetBBReg(pDM_Odm, 0xC8C, BIT18 | BIT17 | BIT16 | BIT15 | BIT14, 30);

				ODM_SetBBReg(pDM_Odm, 0xC8C, BIT13 | BIT12 | BIT11 | BIT10 | BIT9 | BIT8, 0);

				/*Write BB TXAGC Initial Power Index for EEPROM*/
				TxPowerIndex = PHY_GetTxPowerIndex_8814A(Adapter, (ODM_RF_RADIO_PATH_E) RFPath, TxRate, BandWidth, Channel);
				ODM_SetBBReg(pDM_Odm, 0xC8C, BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0, TxPowerIndex);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							("Call %s   TxPowerIndex=%d    RFPath=%d   TxRate=%d   BandWidth=%d   Channel=%d   0x65[19:15]=0X%X   0x65[11:10]=0X%X\n",
							   __func__,
							   ODM_GetBBReg(pDM_Odm, 0xC8C, BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0),
							   RFPath, TxRate, BandWidth, Channel,
							   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT19 | BIT18 | BIT17 | BIT16 | BIT15),
							   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT11 | BIT10)
							  ));

				/*Disable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0xC8C, BIT7, 0);
				/*Enable Path A TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x65, BIT10, 1);
				/*Enable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0xC8C, BIT7, 1);

/*			delay_us(500);*/
				WaitTXAGCOffsetTimer = 0;

				while ((ODM_GetBBReg(pDM_Odm, 0xD2C, BIT30) != 1) && ((TxPath & 8) == 8)) {
					WaitTXAGCOffsetTimer++;

					if (WaitTXAGCOffsetTimer >= 1000)
						break;
				}

				/*Read the offset value at BB Reg.*/
				OffsetVaule = ODM_GetBBReg(pDM_Odm, 0xD2C, BIT29 | BIT28 | BIT27 | BIT26 | BIT25 | BIT24);

				TSSIFunction = ODM_GetBBReg(pDM_Odm, 0xD2C, BIT30);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							("Call %s  0XD2C	 TSSIFunction[30]=0X%X OffsetVaule[29:24]=0X%X	RFPath=%d\n", __func__, TSSIFunction, OffsetVaule, RFPath));

				/*Disable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0xC8C, BIT7, 0);
				/*Disable Path A TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x65, BIT10, 0);

				if (TSSIFunction == 1) {
					TXBBSwing = ODM_GetBBReg(pDM_Odm, rA_BBSWING, BBSWING_BITMASK);

					for (i = 0; i <= 36; i++) {
						if (TXBBSwing == TxScalingTable_Jaguar[i]) {
							pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] = i;
							ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
										(" PathA TXBBSwing = %d TXBBSwing=0X%X\n", TX_BBSwing_Index, TXBBSwing));
							break;
							
						} else {
							pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] = pRFCalibrateInfo->DefaultOfdmIndex;
						}
					}

					pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] = (u1Byte) ODM_GetBBReg(pDM_Odm, rA_TXAGC, TXAGC_BITMASK);


					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								("TXAGCIndex = 0X%X  TX_BBSwing_Index = %d\n", pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]));

					TxPowerIndexOffest = 63 - TxPowerIndex;

					rtn = GetTSSIModeTxAGCBBSWingOffset(pDM_Odm, Method, (ODM_RF_RADIO_PATH_E) RFPath, OffsetVaule, TxPowerIndexOffest);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								(" TX_AGC_Index = %d   TX_BBSwing_Index = %d rtn=%d\n", pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->BbSwingIdxOfdm[RFPath], rtn));

					if (rtn == TRUE) {
						ODM_SetBBReg(pDM_Odm, rA_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
						ODM_SetBBReg(pDM_Odm, rA_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
					} else
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
									("TXAGC And BB Swing are the same Path=%d\n", RFPath));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								(" ========================================================\n"));


					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								("  OffsetValue(0XD2C)=0X%X   TXAGC(rA_TXAGC)=0X%X   0XC1C(PathC BBSwing)(%d)=0X%X\n",
								ODM_GetBBReg(pDM_Odm, 0xD2C, BIT29 | BIT28 | BIT27 | BIT26 | BIT25 | BIT24),
								ODM_GetBBReg(pDM_Odm, rA_TXAGC, TXAGC_BITMASK),
								pRFCalibrateInfo->BbSwingIdxOfdm[RFPath],
								ODM_GetBBReg(pDM_Odm, rA_BBSWING, BBSWING_BITMASK)
								));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" 0X55[13:9]=0X%X	 0X56=0X%X\n",
								   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0X55, BIT13 | BIT12 | BIT11 | BIT10 | BIT9),
								   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0X56, 0XFFFFFFFF)
								  ));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" ========================================================\n"));

				} else
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" TSSI does not Calculate Finish\n"));

			break;

		case ODM_RF_PATH_B:
				/*Disable Path A TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x65, BIT10, 0);
				/*Enable Path B TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x65, BIT10, 1);

				/*Read power by rate table to set TSSI value by power and set rf reg 0x65[19:15]*/
				TSSIValue = GetTSSIVALUE(pDM_Odm, TSSI_MODE, (ODM_RF_RADIO_PATH_E) RFPath);

				ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT19 | BIT18 | BIT17 | BIT16 | BIT15, TSSIValue);

				/*Write BB 0xE8C for Setting Max. packet (30) of tracking power and the initial Value of TXAGC*/
				ODM_SetBBReg(pDM_Odm, 0xE8C, BIT18 | BIT17 | BIT16 | BIT15 | BIT14, 30);

				ODM_SetBBReg(pDM_Odm, 0xE8C, BIT13 | BIT12 | BIT11 | BIT10 | BIT9 | BIT8, 0);

				/*Write BB TXAGC Initial Power Index for EEPROM*/
				TxPowerIndex = PHY_GetTxPowerIndex_8814A(Adapter, (ODM_RF_RADIO_PATH_E) RFPath, TxRate, BandWidth, Channel);
				ODM_SetBBReg(pDM_Odm, 0xE8C, BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0, TxPowerIndex);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							  ("Call %s   TxPowerIndex=%d    RFPath=%d   TxRate=%d   BandWidth=%d   Channel=%d   0x65[19:15]=0X%X   0x65[11:10]=0X%X\n",
							   __func__,
							   ODM_GetBBReg(pDM_Odm, 0xE8C, BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0),
							   RFPath, TxRate, BandWidth, Channel,
							   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT19 | BIT18 | BIT17 | BIT16 | BIT15),
							   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT11 | BIT10)
							  ));

				/*Disable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0xE8C, BIT7, 0);
				/*Enable Path B TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x65, BIT10, 1);
				/*Enable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0xE8C, BIT7, 1);

/*				delay_us(500);*/
				WaitTXAGCOffsetTimer = 0;

				while ((ODM_GetBBReg(pDM_Odm, 0xD6C, BIT30) != 1) && ((TxPath & 4) == 4)) {
					WaitTXAGCOffsetTimer++;

					if (WaitTXAGCOffsetTimer >= 1000)
						break;
				}

				/*Read the offset value at BB Reg.*/
				OffsetVaule = ODM_GetBBReg(pDM_Odm, 0xD6C, BIT29 | BIT28 | BIT27 | BIT26 | BIT25 | BIT24);

				TSSIFunction = ODM_GetBBReg(pDM_Odm, 0xD6C, BIT30);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							  ("Call %s  0XD6C	 TSSIFunction[30]=0X%X OffsetVaule[29:24]=0X%X	RFPath=%d\n", __func__, TSSIFunction, OffsetVaule, RFPath));

				/*Disable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0xE8C, BIT7, 0);
				/*Disable Path B TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x65, BIT10, 0);

				if (TSSIFunction == 1) {
					TXBBSwing = ODM_GetBBReg(pDM_Odm, rB_BBSWING, BBSWING_BITMASK);

					for (i = 0; i <= 36; i++) {
						if (TXBBSwing == TxScalingTable_Jaguar[i]) {
							pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] = i;
							ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
										  (" PathB TXBBSwing = %d TXBBSwing=0X%X\n", TX_BBSwing_Index, TXBBSwing));
							break;
							
						} else {
							pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] = pRFCalibrateInfo->DefaultOfdmIndex;
						}
					}

					pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] = (u1Byte) ODM_GetBBReg(pDM_Odm, rB_TXAGC, TXAGC_BITMASK);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  ("TXAGCIndex = 0X%X  TX_BBSwing_Index = %d\n", pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]));

					TxPowerIndexOffest = 63 - TxPowerIndex;

					rtn = GetTSSIModeTxAGCBBSWingOffset(pDM_Odm, Method, (ODM_RF_RADIO_PATH_E) RFPath, OffsetVaule, TxPowerIndexOffest);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" TX_AGC_Index = %d   TX_BBSwing_Index = %d rtn=%d\n", pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->BbSwingIdxOfdm[RFPath], rtn));

					if (rtn == TRUE) {
						ODM_SetBBReg(pDM_Odm, rB_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
						ODM_SetBBReg(pDM_Odm, rB_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
					} else
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
									  ("TXAGC And BB Swing are the same Path=%d\n", RFPath));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" ========================================================\n"));


					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" OffsetValue(0XD6C)=0X%X   TXAGC(rB_TXAGC)=0X%X   0XE1C(PathB BBSwing)(%d)=0X%X\n",
								   ODM_GetBBReg(pDM_Odm, 0xD6C, BIT29 | BIT28 | BIT27 | BIT26 | BIT25 | BIT24),
								   ODM_GetBBReg(pDM_Odm, rB_TXAGC, TXAGC_BITMASK),
								   pRFCalibrateInfo->BbSwingIdxOfdm[RFPath],
								   ODM_GetBBReg(pDM_Odm, rB_BBSWING, BBSWING_BITMASK)
								  ));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" 0X55[13:9]=0X%X	 0X56=0X%X\n",
								   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0X55, BIT13 | BIT12 | BIT11 | BIT10 | BIT9),
								   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0X56, 0XFFFFFFFF)
								  ));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" ========================================================\n"));

				} else
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" TSSI does not Calculate Finish\n"));

			break;

		case ODM_RF_PATH_C:
				/*Disable Path D TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_D, 0x65, BIT10, 0);
				/*Enable Path C TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_C, 0x65, BIT10, 1);

				/*Read power by rate table to set TSSI value by power and set rf reg 0x65[19:15]*/
				TSSIValue = GetTSSIVALUE(pDM_Odm, TSSI_MODE, (ODM_RF_RADIO_PATH_E) RFPath);

				ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT19 | BIT18 | BIT17 | BIT16 | BIT15, TSSIValue);

				/*Write BB 0x188C for Setting Max. packet (30) of tracking power and the initial Value of TXAGC*/
				ODM_SetBBReg(pDM_Odm, 0x188C, BIT18 | BIT17 | BIT16 | BIT15 | BIT14, 30);

				ODM_SetBBReg(pDM_Odm, 0x188C, BIT13 | BIT12 | BIT11 | BIT10 | BIT9 | BIT8, 0);

				/*Write BB TXAGC Initial Power Index for EEPROM*/
				TxPowerIndex = PHY_GetTxPowerIndex_8814A(Adapter, (ODM_RF_RADIO_PATH_E) RFPath, TxRate, BandWidth, Channel);
				ODM_SetBBReg(pDM_Odm, 0x188C, BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0, TxPowerIndex);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							  ("Call %s   TxPowerIndex=%d    RFPath=%d   TxRate=%d   BandWidth=%d   Channel=%d   0x65[19:15]=0X%X   0x65[11:10]=0X%X\n",
							   __func__,
							   ODM_GetBBReg(pDM_Odm, 0x188C, BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0),
							   RFPath, TxRate, BandWidth, Channel,
							   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT19 | BIT18 | BIT17 | BIT16 | BIT15),
							   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT11 | BIT10)
							  ));

				/*Disable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0x188C, BIT7, 0);
				/*Enable Path C TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_C, 0x65, BIT10, 1);
				/*Enable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0x188C, BIT7, 1);

/*				delay_us(500);*/
				WaitTXAGCOffsetTimer = 0;

				while ((ODM_GetBBReg(pDM_Odm, 0xDAC, BIT30) != 1) && ((TxPath & 2) == 2)) {
					WaitTXAGCOffsetTimer++;

					if (WaitTXAGCOffsetTimer >= 1000)
						break;
				}

				/*Read the offset value at BB Reg.*/
				OffsetVaule = ODM_GetBBReg(pDM_Odm, 0xDAC, BIT29 | BIT28 | BIT27 | BIT26 | BIT25 | BIT24);

				TSSIFunction = ODM_GetBBReg(pDM_Odm, 0xDAC, BIT30);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							  ("Call %s  0XDAC	 TSSIFunction[30]=0X%X OffsetVaule[29:24]=0X%X	RFPath=%d\n", __func__, TSSIFunction, OffsetVaule, RFPath));

				/*Disable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0x188C, BIT7, 0);
				/*Disable Path C TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_C, 0x65, BIT10, 0);

				if (TSSIFunction == 1) {
					TXBBSwing = ODM_GetBBReg(pDM_Odm, rC_BBSWING, BBSWING_BITMASK);

					for (i = 0; i <= 36; i++) {
						if (TXBBSwing == TxScalingTable_Jaguar[i]) {
							pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] = i;
							ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
										  (" PathC TXBBSwing = %d TXBBSwing=0X%X\n", TX_BBSwing_Index, TXBBSwing));
							break;
							
						} else {
							pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] = pRFCalibrateInfo->DefaultOfdmIndex;
						}
					}

					pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] = (u1Byte) ODM_GetBBReg(pDM_Odm, rC_TXAGC, TXAGC_BITMASK);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  ("TXAGCIndex = 0X%X  TX_BBSwing_Index = %d\n", pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]));

					TxPowerIndexOffest = 63 - TxPowerIndex;

					rtn = GetTSSIModeTxAGCBBSWingOffset(pDM_Odm, Method, (ODM_RF_RADIO_PATH_E) RFPath, OffsetVaule, TxPowerIndexOffest);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" TX_AGC_Index = %d   TX_BBSwing_Index = %d rtn=%d\n", pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->BbSwingIdxOfdm[RFPath], rtn));

					if (rtn == TRUE) {
						ODM_SetBBReg(pDM_Odm, rC_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
						ODM_SetBBReg(pDM_Odm, rC_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
					} else
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
									  ("TXAGC And BB Swing are the same Path=%d\n", RFPath));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" ========================================================\n"));


					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" OffsetValue(0XDAC)=0X%X   TXAGC(rC_TXAGC)=0X%X   0X181C(PathC BBSwing)(%d)=0X%X\n",
								   ODM_GetBBReg(pDM_Odm, 0xDAC, BIT29 | BIT28 | BIT27 | BIT26 | BIT25 | BIT24),
								   ODM_GetBBReg(pDM_Odm, rC_TXAGC, TXAGC_BITMASK),
								   pRFCalibrateInfo->BbSwingIdxOfdm[RFPath],
								   ODM_GetBBReg(pDM_Odm, rC_BBSWING, BBSWING_BITMASK)
								  ));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" 0X55[13:9]=0X%X	 0X56=0X%X\n",
								   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0X55, BIT13 | BIT12 | BIT11 | BIT10 | BIT9),
								   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0X56, 0XFFFFFFFF)
								  ));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" ========================================================\n"));

				} else
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" TSSI does not Calculate Finish\n"));

			break;


		case ODM_RF_PATH_D:
				/*Disable Path C TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_C, 0x65, BIT10, 0);
				/*Enable Path D TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_D, 0x65, BIT10, 1);

				/*Read power by rate table to set TSSI value by power and set rf reg 0x65[19:15]*/
				TSSIValue = GetTSSIVALUE(pDM_Odm, TSSI_MODE, (ODM_RF_RADIO_PATH_E) RFPath);

				ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT19 | BIT18 | BIT17 | BIT16 | BIT15, TSSIValue);

				/*Write BB 0x1A8C for Setting Max. packet (30) of tracking power and the initial Value of TXAGC*/
				ODM_SetBBReg(pDM_Odm, 0x1A8C, BIT18 | BIT17 | BIT16 | BIT15 | BIT14, 30);

				ODM_SetBBReg(pDM_Odm, 0x1A8C, BIT13 | BIT12 | BIT11 | BIT10 | BIT9 | BIT8, 0);

				/*Write BB TXAGC Initial Power Index for EEPROM*/
				TxPowerIndex = PHY_GetTxPowerIndex_8814A(Adapter, (ODM_RF_RADIO_PATH_E) RFPath, TxRate, BandWidth, Channel);
				ODM_SetBBReg(pDM_Odm, 0x1A8C, BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0, TxPowerIndex);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							  ("Call %s   TxPowerIndex=%d	  RFPath=%d   TxRate=%d   BandWidth=%d	 Channel=%d   0x65[19:15]=0X%X	 0x65[11:10]=0X%X\n",
							   __func__,
							   ODM_GetBBReg(pDM_Odm, 0x1A8C, BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0),
							   RFPath, TxRate, BandWidth, Channel,
							   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT19 | BIT18 | BIT17 | BIT16 | BIT15),
							   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0x65, BIT11 | BIT10)
							  ));

				/*Disable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0x1A8C, BIT7, 0);
				/*Enable Path D TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_D, 0x65, BIT10, 1);
				/*Enable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0x1A8C, BIT7, 1);

				/*				delay_us(500);*/
				WaitTXAGCOffsetTimer = 0;

				while ((ODM_GetBBReg(pDM_Odm, 0xDEC, BIT30) != 1) && ((TxPath & 1) == 1)) {
					WaitTXAGCOffsetTimer++;

					if (WaitTXAGCOffsetTimer >= 1000)
						break;
				}

				/*Read the offset value at BB Reg.*/
				OffsetVaule = ODM_GetBBReg(pDM_Odm, 0xDEC, BIT29 | BIT28 | BIT27 | BIT26 | BIT25 | BIT24);

				TSSIFunction = ODM_GetBBReg(pDM_Odm, 0xDEC, BIT30);

				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							  ("Call %s  0XDEC  TSSIFunction[30]=0X%X OffsetVaule[29:24]=0X%X	RFPath=%d\n", __func__, TSSIFunction, OffsetVaule, RFPath));

				/*Disable BB TSSI Power Tracking*/
				ODM_SetBBReg(pDM_Odm, 0x1A8C, BIT7, 0);
				/*Disable Path D TSSI Circuit*/
				ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_D, 0x65, BIT10, 0);

				if (TSSIFunction == 1) {
					TXBBSwing = ODM_GetBBReg(pDM_Odm, rD_BBSWING, BBSWING_BITMASK);

					for (i = 0; i <= 36; i++) {
						if (TXBBSwing == TxScalingTable_Jaguar[i]) {
							pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] = i;
							ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
										  (" PathD TXBBSwing = %d TXBBSwing=0X%X\n", TX_BBSwing_Index, TXBBSwing));
							break;
							
						} else {
							pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] = pRFCalibrateInfo->DefaultOfdmIndex;
						}
					}

					pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] = (u1Byte) ODM_GetBBReg(pDM_Odm, rD_TXAGC, TXAGC_BITMASK);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  ("TXAGCIndex = 0X%X	TX_BBSwing_Index = %d\n", pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]));

					TxPowerIndexOffest = 63 - TxPowerIndex;

					rtn = GetTSSIModeTxAGCBBSWingOffset(pDM_Odm, Method, (ODM_RF_RADIO_PATH_E) RFPath, OffsetVaule, TxPowerIndexOffest);

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" TX_AGC_Index = %d   TX_BBSwing_Index = %d rtn=%d\n", pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->BbSwingIdxOfdm[RFPath], rtn));

					if (rtn == TRUE) {
						ODM_SetBBReg(pDM_Odm, rD_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
						ODM_SetBBReg(pDM_Odm, rD_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
					} else
						ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
									  ("TXAGC And BB Swing are the same Path=%d\n", RFPath));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" ========================================================\n"));


					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" OffsetValue(0XDEC)=0X%X	TXAGC(rD_TXAGC)=0X%X	 0X1A1C(PathD BBSwing)(%d)=0X%X\n",
								   ODM_GetBBReg(pDM_Odm, 0xDEC, BIT29 | BIT28 | BIT27 | BIT26 | BIT25 | BIT24),
								   ODM_GetBBReg(pDM_Odm, rD_TXAGC, TXAGC_BITMASK),
								   pRFCalibrateInfo->BbSwingIdxOfdm[RFPath],
								   ODM_GetBBReg(pDM_Odm, rD_BBSWING, BBSWING_BITMASK)
								  ));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" 0X55[13:9]=0X%X	 0X56=0X%X\n",
								   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0X55, BIT13 | BIT12 | BIT11 | BIT10 | BIT9),
								   ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, 0X56, 0XFFFFFFFF)
								  ));

					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" ========================================================\n"));

				} else
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
								  (" TSSI does not Calculate Finish\n"));

			break;


		default:
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							  ("Wrong Path name!!!!\n"));

			break;
		}
	}

}

BOOLEAN GetMixModeTXAGCBBSWingOffset(
	IN PDM_ODM_T			pDM_Odm,
	IN PWRTRACK_METHOD	Method,
	IN u1Byte					RFPath,
	IN u1Byte					TxPowerIndexOffest
)
{
	PODM_RF_CAL_T		pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

	u1Byte	BBSwingUpperBound = pRFCalibrateInfo->DefaultOfdmIndex + 10;
	u1Byte	BBSwingLowerBound = 0;

	s1Byte	TX_AGC_Index = 0;
	u1Byte	TX_BBSwing_Index = pRFCalibrateInfo->DefaultOfdmIndex;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
				  ("Path_%d pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]=%d, TxPowerIndexOffest=%d\n",
				   RFPath, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], TxPowerIndexOffest));

	if (TxPowerIndexOffest > 0XF)
		TxPowerIndexOffest = 0XF;

	if (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] >= 0 && pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] <= TxPowerIndexOffest) {
		TX_AGC_Index = pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath];
		TX_BBSwing_Index = pRFCalibrateInfo->DefaultOfdmIndex;
	} else if (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] > TxPowerIndexOffest) {
		TX_AGC_Index = TxPowerIndexOffest;
		pRFCalibrateInfo->Remnant_OFDMSwingIdx[RFPath] = pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] - TxPowerIndexOffest;
		TX_BBSwing_Index = pRFCalibrateInfo->DefaultOfdmIndex + pRFCalibrateInfo->Remnant_OFDMSwingIdx[RFPath];

		if (TX_BBSwing_Index > BBSwingUpperBound)
			TX_BBSwing_Index = BBSwingUpperBound;
	} else {
		TX_AGC_Index = 0;

		if (pRFCalibrateInfo->DefaultOfdmIndex > (pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] * (-1)))
			TX_BBSwing_Index = pRFCalibrateInfo->DefaultOfdmIndex + pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath];
		else
			TX_BBSwing_Index = BBSwingLowerBound;

		if (TX_BBSwing_Index <  BBSwingLowerBound)
			TX_BBSwing_Index = BBSwingLowerBound;
	}

	pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath] = TX_AGC_Index;
	pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] = TX_BBSwing_Index;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
				  ("MixMode Offset Path_%d   pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]=%d   pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]=%d   TxPowerIndexOffest=%d\n",
				   RFPath, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->BbSwingIdxOfdm[RFPath] , TxPowerIndexOffest));

	return TRUE;
}

VOID PowerTrackingByMixMode(
	PDM_ODM_T			pDM_Odm,
	PWRTRACK_METHOD	Method,
	u1Byte				RFPath
)
{
	PADAPTER			Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	PODM_RF_CAL_T		pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);

	u1Byte				TxRate = 0xFF;
	u1Byte				Channel  = pHalData->CurrentChannel;
	u1Byte				BandWidth  = pHalData->CurrentChannelBW;
	u1Byte				TxPowerIndexOffest = 0;
	u1Byte				TxPowerIndex = 0;

	if (pDM_Odm->mp_mode == TRUE) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#if (MP_DRIVER == 1)
		PMPT_CONTEXT pMptCtx = &(Adapter->MptCtx);

		TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
#endif
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
		PMPT_CONTEXT pMptCtx = &(Adapter->mppriv.MptCtx);

		TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
#endif
#endif
	} else {
		u2Byte	rate	 = *(pDM_Odm->pForcedDataRate);

		if (!rate) { /*auto rate*/
			if (rate != 0xFF) {
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
				TxRate = Adapter->HalFunc.GetHwRateFromMRateHandler(pDM_Odm->TxRate);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
				if (pDM_Odm->number_linked_client != 0)
					TxRate = HwRateToMRate(pDM_Odm->TxRate);
#endif
			}
		} else { /*force rate*/
			TxRate = (u1Byte) rate;
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s TxRate=0x%X\n", __func__, TxRate));

	if ((pRFCalibrateInfo->PowerIndexOffset[ODM_RF_PATH_A] != 0 ||
		  pRFCalibrateInfo->PowerIndexOffset[ODM_RF_PATH_B] != 0 ||
		  pRFCalibrateInfo->PowerIndexOffset[ODM_RF_PATH_C] != 0 ||
		  pRFCalibrateInfo->PowerIndexOffset[ODM_RF_PATH_D] != 0) &&
		 pRFCalibrateInfo->TxPowerTrackControl && (pHalData->EEPROMThermalMeter != 0xff)) {

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
					  ("****************Path_%d POWER Tracking MIX MODE**********\n", RFPath));

		TxPowerIndex = PHY_GetTxPowerIndex_8814A(Adapter, (ODM_RF_RADIO_PATH_E) RFPath, TxRate, BandWidth, Channel);
		TxPowerIndexOffest = 63 - TxPowerIndex;

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
					  ("pRFCalibrateInfo->Absolute_OFDMSwingIdx[%d] =%d   TxPowerIndex=%d\n", RFPath, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], TxPowerIndex));

		switch (RFPath) {
		case ODM_RF_PATH_A:
			GetMixModeTXAGCBBSWingOffset(pDM_Odm, Method, (ODM_RF_RADIO_PATH_E) RFPath, TxPowerIndexOffest);
			ODM_SetBBReg(pDM_Odm, rA_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
			ODM_SetBBReg(pDM_Odm, rA_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
			break;

		case ODM_RF_PATH_B:
			GetMixModeTXAGCBBSWingOffset(pDM_Odm, Method, (ODM_RF_RADIO_PATH_E) RFPath, TxPowerIndexOffest);
			ODM_SetBBReg(pDM_Odm, rB_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
			ODM_SetBBReg(pDM_Odm, rB_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
			break;

		case ODM_RF_PATH_C:
			GetMixModeTXAGCBBSWingOffset(pDM_Odm, Method, (ODM_RF_RADIO_PATH_E) RFPath, TxPowerIndexOffest);
			ODM_SetBBReg(pDM_Odm, rC_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
			ODM_SetBBReg(pDM_Odm, rC_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
			break;

		case ODM_RF_PATH_D:
			GetMixModeTXAGCBBSWingOffset(pDM_Odm, Method, (ODM_RF_RADIO_PATH_E) RFPath, TxPowerIndexOffest);
			ODM_SetBBReg(pDM_Odm, rD_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
			ODM_SetBBReg(pDM_Odm, rD_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
			break;

		default:
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
							  ("Wrong Path name!!!!\n"));
			break;
		}
	} else
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
					  ("Power Index is the same, EEPROMThermalMeter = 0XFF or TxPowerTrackControl is Disable !!!!\n"));
}

VOID PowerTrackingByTSSIMode(
	PDM_ODM_T			pDM_Odm,
	PWRTRACK_METHOD	Method,
	u1Byte				RFPath
)
{
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
				  ("****************Path_%d POWER Tracking TSSI_MODE**********\n", RFPath));

	SetTxAGCBBSWingOffset(pDM_Odm, TSSI_MODE, (ODM_RF_RADIO_PATH_E) RFPath);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
				  ("****************Path_%d End POWER Tracking TSSI_MODE**********\n", RFPath));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("\n"));
}



VOID ODM_TxPwrTrackSetPwr8814A(
	IN	PVOID		pDM_VOID,
	PWRTRACK_METHOD	Method,
	u1Byte				RFPath,
	u1Byte				ChannelMappedIndex
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);

	u1Byte			Final_OFDM_Swing_Index = 0;
	u1Byte			Channel  = pHalData->CurrentChannel;
	PODM_RF_CAL_T		pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);


/*K-Free*/
#if 0
	s1Byte			TXBBIndex = 0;
	s1Byte			TXBBUpperBound = 10, TXBBLowerBound = -5;

	TXBBIndex = pRFCalibrateInfo->KfreeOffset[RFPath] / 2;

	if (TXBBIndex > TXBBUpperBound)
		TXBBIndex = TXBBUpperBound;
	else if (TXBBIndex < TXBBLowerBound)
		TXBBIndex = TXBBLowerBound;

	if (TXBBIndex >= 0) {
		ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, rRF_TxGainOffset, BIT19, 1);
		ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, rRF_TxGainOffset, (BIT18 | BIT17 | BIT16 | BIT15), TXBBIndex);	/*set RF Reg0x55 per path*/
	} else {
		ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, rRF_TxGainOffset, BIT19, 0);
		ODM_SetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E) RFPath, rRF_TxGainOffset, (BIT18 | BIT17 | BIT16 | BIT15), (-1)*TXBBIndex);
	}

#endif

	if (Method == TXAGC) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
					  ("****************Path_%d POWER Tracking No TXAGC MODE**********\n", RFPath));

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("pRFCalibrateInfo->Absolute_OFDMSwingIdx[%d] =%d\n",
					  RFPath, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]));
	} else if (Method == TSSI_MODE) {
		PowerTrackingByTSSIMode(pDM_Odm, TSSI_MODE, (ODM_RF_RADIO_PATH_E) RFPath);
	} else if (Method == BBSWING) {	/*use for mp driver clean power tracking status*/
		switch (RFPath) {
		case ODM_RF_PATH_A:
			ODM_SetBBReg(pDM_Odm, rA_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
			ODM_SetBBReg(pDM_Odm, rA_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
			break;

		case ODM_RF_PATH_B:
			ODM_SetBBReg(pDM_Odm, rB_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
			ODM_SetBBReg(pDM_Odm, rB_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
			break;

		case ODM_RF_PATH_C:
			ODM_SetBBReg(pDM_Odm, rC_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
			ODM_SetBBReg(pDM_Odm, rC_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
			break;

		case ODM_RF_PATH_D:
			ODM_SetBBReg(pDM_Odm, rD_TXAGC, TXAGC_BITMASK, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath]);
			ODM_SetBBReg(pDM_Odm, rD_BBSWING, BBSWING_BITMASK, TxScalingTable_Jaguar[pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]]);	/*set BBswing*/
			break;

		default:
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD,
						  ("Wrong Path name!!!!\n"));

			break;
		}

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("RFPath=%d   Clear 8814 Power tracking TXAGC=%d  BBSwing=%d\n",
					  RFPath, pRFCalibrateInfo->Absolute_OFDMSwingIdx[RFPath], pRFCalibrateInfo->BbSwingIdxOfdm[RFPath]));

	} else if (Method == MIX_MODE) {
		PowerTrackingByMixMode(pDM_Odm, MIX_MODE, (ODM_RF_RADIO_PATH_E) RFPath);
	} else if (Method == MIX_2G_TSSI_5G_MODE) {
		if (Channel <= 14)
			PowerTrackingByMixMode(pDM_Odm, MIX_MODE, (ODM_RF_RADIO_PATH_E) RFPath);
		else
			PowerTrackingByTSSIMode(pDM_Odm, TSSI_MODE, (ODM_RF_RADIO_PATH_E) RFPath);
	} else if (Method == MIX_5G_TSSI_2G_MODE) {
		if (Channel <= 14)
			PowerTrackingByTSSIMode(pDM_Odm, TSSI_MODE, (ODM_RF_RADIO_PATH_E) RFPath);
		else
			PowerTrackingByMixMode(pDM_Odm, MIX_MODE, (ODM_RF_RADIO_PATH_E) RFPath);
	}
}	/*ODM_TxPwrTrackSetPwr8814A*/


VOID GetDeltaSwingTable_8814A(
	PVOID	pDM_VOID,
	pu1Byte	*TemperatureUP_A,
	pu1Byte	*TemperatureDOWN_A,
	pu1Byte	*TemperatureUP_B,
	pu1Byte	*TemperatureDOWN_B
)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PADAPTER	Adapter = pDM_Odm->Adapter;
	PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	HAL_DATA_TYPE  	*pHalData  		 = GET_HAL_DATA(Adapter);
	u1Byte		TxRate			= 0xFF;
	u1Byte         	channel   		 = pHalData->CurrentChannel;

	if (pDM_Odm->mp_mode == TRUE) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#if (MP_DRIVER == 1)
		PMPT_CONTEXT pMptCtx = &(Adapter->MptCtx);

		TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
#endif
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
		PMPT_CONTEXT pMptCtx = &(Adapter->mppriv.MptCtx);

		TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
#endif
#endif
	} else {
		u2Byte	rate	 = *(pDM_Odm->pForcedDataRate);

		if (!rate) { /*auto rate*/
			if (rate != 0xFF) {
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
				TxRate = Adapter->HalFunc.GetHwRateFromMRateHandler(pDM_Odm->TxRate);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
				if (pDM_Odm->number_linked_client != 0)
					TxRate = HwRateToMRate(pDM_Odm->TxRate);
#endif
			}
		} else { /*force rate*/
			TxRate = (u1Byte) rate;
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s TxRate=0x%X\n", __func__, TxRate));

	if (1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(TxRate)) {
			*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_P;
			*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKA_N;
			*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_P;
			*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKB_N;
		} else {
			*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_2GA_P;
			*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_2GA_N;
			*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_2GB_P;
			*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_2GB_N;
		}
	} else if (36 <= channel && channel <= 64) {
		*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[0];
		*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[0];
		*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[0];
		*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[0];
	} else if (100 <= channel && channel <= 144) {
		*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[1];
		*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[1];
		*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[1];
		*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[1];
	} else if (149 <= channel && channel <= 177) {
		*TemperatureUP_A   = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_P[2]; 
		*TemperatureDOWN_A = pRFCalibrateInfo->DeltaSwingTableIdx_5GA_N[2]; 
		*TemperatureUP_B   = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_P[2]; 
		*TemperatureDOWN_B = pRFCalibrateInfo->DeltaSwingTableIdx_5GB_N[2]; 
	} else {
		*TemperatureUP_A   = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_A = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;
		*TemperatureUP_B   = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_B = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;
	}

	return;
}


VOID GetDeltaSwingTable_8814A_PathCD(
	IN	PVOID		pDM_VOID,
	OUT pu1Byte 			*TemperatureUP_C,
	OUT pu1Byte 			*TemperatureDOWN_C,
	OUT pu1Byte 			*TemperatureUP_D,
	OUT pu1Byte			*TemperatureDOWN_D
)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PADAPTER	Adapter = pDM_Odm->Adapter;
	PODM_RF_CAL_T  	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);
	HAL_DATA_TYPE  	*pHalData  		 = GET_HAL_DATA(Adapter);
	u1Byte		TxRate			= 0xFF;
	u1Byte         	channel   		 = pHalData->CurrentChannel;

	if (pDM_Odm->mp_mode == TRUE) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#if (MP_DRIVER == 1)
		PMPT_CONTEXT pMptCtx = &(Adapter->MptCtx);

		TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
#endif
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
		PMPT_CONTEXT pMptCtx = &(Adapter->mppriv.MptCtx);

		TxRate = MptToMgntRate(pMptCtx->MptRateIndex);
#endif
#endif
	} else {
		u2Byte	rate	 = *(pDM_Odm->pForcedDataRate);

		if (!rate) { /*auto rate*/
			if (rate != 0xFF) {
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
				TxRate = Adapter->HalFunc.GetHwRateFromMRateHandler(pDM_Odm->TxRate);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
				if (pDM_Odm->number_linked_client != 0)
					TxRate = HwRateToMRate(pDM_Odm->TxRate);
#endif
			}
		} else { /*force rate*/
			TxRate = (u1Byte) rate;
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Call:%s TxRate=0x%X\n", __func__, TxRate));

	if (1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(TxRate)) {
			*TemperatureUP_C  = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKC_P;
			*TemperatureDOWN_C = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKC_N;
			*TemperatureUP_D   = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKD_P;
			*TemperatureDOWN_D = pRFCalibrateInfo->DeltaSwingTableIdx_2GCCKD_N;
		} else {
			*TemperatureUP_C   = pRFCalibrateInfo->DeltaSwingTableIdx_2GC_P;
			*TemperatureDOWN_C = pRFCalibrateInfo->DeltaSwingTableIdx_2GC_N;
			*TemperatureUP_D   = pRFCalibrateInfo->DeltaSwingTableIdx_2GD_P;
			*TemperatureDOWN_D = pRFCalibrateInfo->DeltaSwingTableIdx_2GD_N;
		}
	} else if (36 <= channel && channel <= 64) {
		*TemperatureUP_C   = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_P[0];
		*TemperatureDOWN_C = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_N[0];
		*TemperatureUP_D   = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_P[0];
		*TemperatureDOWN_D = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_N[0];
	} else if (100 <= channel && channel <= 144) {
		*TemperatureUP_C   = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_P[1];
		*TemperatureDOWN_C = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_N[1];
		*TemperatureUP_D   = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_P[1];
		*TemperatureDOWN_D = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_N[1];
	} else if (149 <= channel && channel <= 177) {
		*TemperatureUP_C   = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_P[2]; 
		*TemperatureDOWN_C = pRFCalibrateInfo->DeltaSwingTableIdx_5GC_N[2]; 
		*TemperatureUP_D   = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_P[2]; 
		*TemperatureDOWN_D = pRFCalibrateInfo->DeltaSwingTableIdx_5GD_N[2]; 
	} else {
		*TemperatureUP_C   = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_C = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;
		*TemperatureUP_D   = (pu1Byte)DeltaSwingTableIdx_2GA_P_8188E;
		*TemperatureDOWN_D = (pu1Byte)DeltaSwingTableIdx_2GA_N_8188E;
	}

	return;
}

void ConfigureTxpowerTrack_8814A(
	PTXPWRTRACK_CFG	pConfig
)
{
	pConfig->SwingTableSize_CCK = CCK_TABLE_SIZE;
	pConfig->SwingTableSize_OFDM = OFDM_TABLE_SIZE;
	pConfig->Threshold_IQK = 8;
	pConfig->AverageThermalNum = AVG_THERMAL_NUM_8814A;
	pConfig->RfPathCount = MAX_PATH_NUM_8814A;
	pConfig->ThermalRegAddr = RF_T_METER_88E;

	pConfig->ODM_TxPwrTrackSetPwr = ODM_TxPwrTrackSetPwr8814A;
	pConfig->DoIQK = DoIQK_8814A;
	pConfig->PHY_LCCalibrate = PHY_LCCalibrate_8814A;
	pConfig->GetDeltaSwingTable = GetDeltaSwingTable_8814A;
	pConfig->GetDeltaSwingTable8814only = GetDeltaSwingTable_8814A_PathCD;
}

VOID	
phy_LCCalibrate_8814A(
	IN PDM_ODM_T		pDM_Odm,
	IN	BOOLEAN		is2T
	)
{
	u4Byte	/*RF_Amode=0, RF_Bmode=0,*/ LC_Cal = 0, tmp = 0, cnt;
	
	//Check continuous TX and Packet TX
	u4Byte	reg0x914 = ODM_Read4Byte(pDM_Odm, rSingleTone_ContTx_Jaguar);;

	// Backup RF reg18.

	if((reg0x914 & 0x70000) == 0)
		ODM_Write1Byte(pDM_Odm, REG_TXPAUSE, 0xFF);			

	//3 3. Read RF reg18
	LC_Cal = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask);

	//3 4. Set LC calibration begin bit15
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, 0x1b126);

	ODM_delay_ms(100);		

	for (cnt = 0; cnt < 100; cnt++) {
		if (ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, 0x8000) != 0x1)
			break;
		ODM_delay_ms(10);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("retry cnt = %d\n", cnt));

	ODM_SetRFReg( pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, 0x13126);
	ODM_SetRFReg( pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, 0x13124);
	//3 Restore original situation
	if((reg0x914 & 70000) == 0)
		ODM_Write1Byte(pDM_Odm, REG_TXPAUSE, 0x00);	

	// Recover channel number
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal);

	DbgPrint("Call %s\n", __FUNCTION__);
}


VOID	
phy_APCalibrate_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	s1Byte 		delta,
	IN	BOOLEAN		is2T
	)
{
}


VOID
PHY_LCCalibrate_8814A(
	IN	PVOID		pDM_VOID
	)
{
	BOOLEAN 		bStartContTx = FALSE, bSingleTone = FALSE, bCarrierSuppression = FALSE;
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	PADAPTER 		pAdapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);	
	
#if (MP_DRIVER == 1)
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)	
	PMPT_CONTEXT	pMptCtx = &(pAdapter->MptCtx);
	bStartContTx = pMptCtx->bStartContTx;
	bSingleTone = pMptCtx->bSingleTone;
	bCarrierSuppression = pMptCtx->bCarrierSuppression;
#else
	PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.MptCtx);		
#endif	
#endif
#endif	

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("===> PHY_LCCalibrate_8814A\n"));

//#if (MP_DRIVER == 1)	
	phy_LCCalibrate_8814A(pDM_Odm, TRUE);
//#endif 

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("<=== PHY_LCCalibrate_8814A\n"));

}

VOID
PHY_APCalibrate_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	s1Byte 		delta	
	)
{

}


VOID	                                                 
PHY_DPCalibrate_8814A(                                   
	IN 	PDM_ODM_T	pDM_Odm                             
	)
{
}


BOOLEAN 
phy_QueryRFPathSwitch_8814A(
	IN	PADAPTER	pAdapter
	)
{
	return TRUE;
}


BOOLEAN PHY_QueryRFPathSwitch_8814A(	
	IN	PADAPTER	pAdapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

#if DISABLE_BB_RF
	return TRUE;
#endif

	return phy_QueryRFPathSwitch_8814A(pAdapter);
}


VOID phy_SetRFPathSwitch_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	BOOLEAN		bMain,
	IN	BOOLEAN		is2T
	)
{
}
VOID PHY_SetRFPathSwitch_8814A(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	BOOLEAN		bMain
	)
{
}




#endif	/* (RTL8814A_SUPPORT == 0)*/
