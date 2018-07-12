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


/*---------------------------Define Local Constant---------------------------*/


/*---------------------------Define Local Constant---------------------------*/


#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
void DoIQK_8197F(
	PVOID		pDM_VOID,
	u1Byte		DeltaThermalIndex,
	u1Byte		ThermalValue,	
	u1Byte		Threshold
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;

	PADAPTER	Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	ODM_ResetIQKResult(pDM_Odm);		

	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = ThermalValue;
	
	PHY_IQCalibrate_8197F(pDM_Odm, FALSE);
	
}
#else
/*Originally pConfig->DoIQK is hooked PHY_IQCalibrate_8197F, but DoIQK_8197F and PHY_IQCalibrate_8197F have different arguments*/
void DoIQK_8197F(
	PVOID		pDM_VOID,
	u1Byte		DeltaThermalIndex,
	u1Byte		ThermalValue,	
	u1Byte		Threshold
	)
{
#if 1
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	BOOLEAN		bReCovery = (BOOLEAN) DeltaThermalIndex;

	PHY_IQCalibrate_8197F(pDM_Odm, bReCovery);
#endif
}
#endif

#define MAX_TOLERANCE		5
#define IQK_DELAY_TIME		1		/*ms*/

VOID
do_dpk_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	BOOLEAN		is2T,
	u1Byte		k
	
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	u1Byte			i = 0;
	u4Byte			tmp_rf_a_18, tmp_rf_b_18;
	
	tmp_rf_a_18 = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x18, 0x00fff);
	tmp_rf_b_18 = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x18, 0x00fff);

#if 1
	switch (k)
	{
		case 0: /*channel 3*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x18, 0x00fff, 0xc01);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x18, 0x00fff, 0xc01);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] switch to 20M-CH3!!!!\n"));
			break;
	
		case 1: /*channel 7*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x18, 0x00fff, 0xc07);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x18, 0x00fff, 0xc07);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] switch to 20M-CH7!!!!\n"));
			break;
	
		case 2: /*channel 11*/
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x18, 0x00fff, 0xc0b);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x18, 0x00fff, 0xc0b);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] switch to 20M-CH11!!!!\n"));
			break;
	}
#endif	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 0x18@ path_A = 0x%x, path_B = 0x%x\n", 
	ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask), ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x18, bRFRegOffsetMask)));

	for (i = 0; i < 6; i++)
	{
		//printk("[DPK] Retry DPK i=%d\n", i);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] Retry DPK i=%d\n", i)); 
		if (phy_dpcalibrate_8197f(pDM_Odm, TRUE, k) == 1)
			break;
	}
	
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x18, 0x00fff, tmp_rf_a_18);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x18, 0x00fff, tmp_rf_b_18);


	if ((pDM_Odm->RFCalibrateInfo.bDPPathAOK == 1) && (pDM_Odm->RFCalibrateInfo.bDPPathBOK == 1) && k == 2)
		phy_lut_sram_write_8197f(pDM_Odm);
	else
		phy_dpk_enable_disable_8197f(pDM_Odm, false);
}


VOID
phy_dpk_track_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	prtl8192cd_priv		priv = pDM_Odm->priv;	
	s1Byte		pwsf_a, pwsf_b;
	u1Byte		offset, delta_DPK, is_increase, ThermalValue = 0, ThermalValue_DPK_AVG_count = 0, i = 0, k = 0;
	u4Byte		ThermalValue_DPK_AVG = 0;

#ifdef MP_TEST
	if ((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific) {
		if (priv->pshare->mp_tx_dpk_tracking == FALSE)
			return;
	}
#endif

	if (!pDM_Odm->RFCalibrateInfo.ThermalValue_DPK)
		pDM_Odm->RFCalibrateInfo.ThermalValue_DPK = priv->pmib->dot11RFEntry.ther;

	/* calculate average thermal meter */

	ODM_SetRFReg(pDM_Odm, RF_PATH_A, RF_T_METER_8197F, BIT17 | BIT16, 0x3);		/*thermal meter trigger*/
	ODM_delay_ms(1);
	ThermalValue = (u1Byte)ODM_GetRFReg(pDM_Odm, RF_PATH_A, RF_T_METER_8197F, 0xfc00);		/*get thermal meter*/
	
	pDM_Odm->RFCalibrateInfo.ThermalValue_DPK_AVG[pDM_Odm->RFCalibrateInfo.ThermalValue_DPK_AVG_index] = ThermalValue;
	pDM_Odm->RFCalibrateInfo.ThermalValue_DPK_AVG_index++;
	
	if (pDM_Odm->RFCalibrateInfo.ThermalValue_DPK_AVG_index == Thermal_DPK_AVG_num)   /*Average times */
		pDM_Odm->RFCalibrateInfo.ThermalValue_DPK_AVG_index = 0;

	for (i = 0; i < Thermal_DPK_AVG_num; i++) {
		if (pDM_Odm->RFCalibrateInfo.ThermalValue_DPK_AVG[i]) {
			ThermalValue_DPK_AVG += pDM_Odm->RFCalibrateInfo.ThermalValue_DPK_AVG[i];
			ThermalValue_DPK_AVG_count++;
		}
	}

	if (ThermalValue_DPK_AVG_count) {/*Calculate Average ThermalValue after average enough times*/
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,
			("[DPK_track] ThermalValue_DPK_AVG=0x%x(%d)  ThermalValue_DPK_AVG_count = %d\n"
			, ThermalValue_DPK_AVG, ThermalValue_DPK_AVG, ThermalValue_DPK_AVG_count));
		
		ThermalValue = (u1Byte)(ThermalValue_DPK_AVG / ThermalValue_DPK_AVG_count);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,
			("[DPK_track] AVG Thermal Meter = 0x%X(%d), PG Thermal Meter = 0x%X(%d)\n", 
			ThermalValue, ThermalValue, priv->pmib->dot11RFEntry.ther, priv->pmib->dot11RFEntry.ther));
	}


	delta_DPK = RTL_ABS(ThermalValue, priv->pmib->dot11RFEntry.ther);
	is_increase = ((ThermalValue < priv->pmib->dot11RFEntry.ther) ? 0 : 1);

	offset = delta_DPK / DPK_THRESHOLD_8197F;

	k = phy_dpk_channel_transfer_8197f(pDM_Odm);

	pwsf_a = pDM_Odm->RFCalibrateInfo.pwsf_2g_a[k];
	pwsf_b = pDM_Odm->RFCalibrateInfo.pwsf_2g_b[k];

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK track] delta_DPK = %d, offset = %d, track direction is %s\n", delta_DPK, offset, (is_increase ? "Plus" : "Minus")));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK track] pwsf_a before tracking is 0x%x, pwsf_b before tracking is 0x%x\n", pwsf_a, pwsf_b));

	if ((pwsf_a>>4) != 0)
	pwsf_a = (pwsf_a | 0xe0);

	if ((pwsf_b>>4) != 0)
	pwsf_b = (pwsf_b | 0xe0);
	
	if (is_increase) {
		
		pwsf_a = pwsf_a + offset;
		pwsf_b = pwsf_b + offset;
			
	} else {

		pwsf_a = pwsf_a - offset;
		pwsf_b = pwsf_b - offset;
		
	}		

	ODM_SetBBReg(pDM_Odm, 0xb68, 0x00007C00, pwsf_a);
	ODM_SetBBReg(pDM_Odm, 0xb6c, 0x00007C00, pwsf_b);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK track] pwsf_a after tracking is %d (0x%x), 0xb68 = 0x%x\n", pwsf_a, (pwsf_a & 0x1f), ODM_GetBBReg(pDM_Odm, 0xb68, bMaskDWord)));	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK track] pwsf_b after tracking is %d (0x%x), 0xb6c = 0x%x\n", pwsf_b, (pwsf_b & 0x1f), ODM_GetBBReg(pDM_Odm, 0xb6c, bMaskDWord)));

}



VOID
phy_dpkoff_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	/*Path A*/
	ODM_Write1Byte(pDM_Odm, 0xb00, 0x18);
	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x80);
	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x00);
	ODM_delay_ms(10);

	/*Path B*/
	ODM_Write1Byte(pDM_Odm, 0xb70, 0x18);
	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x80);
	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x00);
	ODM_delay_ms(10);
	
}

VOID
phy_dpkon_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	/*Path A*/
	ODM_Write1Byte(pDM_Odm, 0xb00, 0x98);
	ODM_Write1Byte(pDM_Odm, 0xb07, 0xF7);
	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x00);

	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x80);
	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x00);
	ODM_delay_ms(10);

	ODM_Write1Byte(pDM_Odm, 0xb07, 0x77);
	ODM_SetBBReg(pDM_Odm, 0xb08, bMaskDWord, 0x4c104c10);  /*1.5dB*/

	/*Path B*/
	ODM_Write1Byte(pDM_Odm, 0xb70, 0x98);
	ODM_Write1Byte(pDM_Odm, 0xb77, 0xF7);
	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x00);

	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x80);
	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x00);
	ODM_delay_ms(10);

	ODM_Write1Byte(pDM_Odm, 0xb77, 0x77);
	ODM_SetBBReg(pDM_Odm, 0xb78, bMaskDWord, 0x4c104c10);  /*1.5dB*/
	
}


u1Byte			/*bit0 = 1 => Tx OK, bit1 = 1 => Rx OK*/
phy_PathA_IQK_8197F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	BOOLEAN		configPathB
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte regEAC, regE94, regE9C, regEA4, tmp;
	u1Byte result = 0x00;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] ====================Path A TXIQK start!====================\n"));

	/*=============================TXIQK setting=============================*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);


	if (pDM_Odm->ExtPA) {
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, 0x00800, 0x1);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, 0x003ff, 0x71);	
	} else if (pDM_Odm->ExtPA == 0 && pDM_Odm->CutVersion != ODM_CUT_A) {	
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, 0x00800, 0x1);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, 0x003ff, 0xe8);
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0x56 at Path A TXIQK = 0x%x\n",
				 ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, bRFRegOffsetMask)));


	/*enter IQK mode*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x808000);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path-A IQK setting!\n"));*/
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x18008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_B, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_B, bMaskDWord, 0x38008c0c);

	if (pDM_Odm->CutVersion == ODM_CUT_A) { 
		if (pDM_Odm->ExtPA)
			ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x8214400f);
		else
			ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x82140002);
	} else
		ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x8214000f);
	
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_A, bMaskDWord, 0x28140000);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] LO calibration setting!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x00e62911);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] One shot, path A LOK & IQK!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf9005800);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8005800);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_97F));*/
	ODM_delay_ms(IQK_DELAY_TIME_97F);

	/*Check failed*/
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regE94 = ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord);
	regE9C = ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xe94 = 0x%x, 0xe9c = 0x%x\n", regE94, regE9C));
	/*monitor image power before & after IQK*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xe90(before IQK)= 0x%x, 0xe98(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xe90, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xe98, bMaskDWord)));


	/*reload 0xdf and CCK_IND off  */
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x0, BIT14, 0x0);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, 0x00800, 0x0);

	if (!(regEAC & BIT28) &&
		(((regE94 & 0x03FF0000) >> 16) != 0x142) &&
		(((regE9C & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] path A TXIQK is not success\n"));	
	return result;

}

u1Byte			/*bit0 = 1 => Tx OK, bit1 = 1 => Rx OK*/
phy_PathA_RxIQK_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	BOOLEAN		configPathB
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte regEAC, regE94, regE9C, regEA4, u4tmp, RXPGA;
	u1Byte result = 0x00;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path A RxIQK start!\n"));

	/* =============================Get TXIMR setting=============================*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] ====================Path A RXIQK step1!====================\n"));


	/*modify RXIQK mode table*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);


	if (pDM_Odm->CutVersion == ODM_CUT_A) {
		RXPGA = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask);
		
		if (pDM_Odm->ExtPA)
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xa8000);
		else
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xae000);
	}
	/*enter IQK mode*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x808000);

	/*path-A IQK setting*/
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x18008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_B, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_B, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x82160000);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_A, bMaskDWord, 0x28160000);
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, 0x01007c00);
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] LO calibration setting!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x0046a911);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] One shot, path A LOK & IQK!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf9005800);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8005800);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_92E));*/
	ODM_delay_ms(IQK_DELAY_TIME_97F);


	/*Check failed*/
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regE94 = ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord);
	regE9C = ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xe94 = 0x%x, 0xe9c = 0x%x\n", regE94, regE9C));
	/*monitor image power before & after IQK*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xe90(before IQK)= 0x%x, 0xe98(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xe90, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xe98, bMaskDWord)));

	if (pDM_Odm->CutVersion == ODM_CUT_A) {
	/*Restore RXPGA*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, RXPGA);
	}

	if (!(regEAC & BIT28) &&
		(((regE94 & 0x03FF0000) >> 16) != 0x142) &&
		(((regE9C & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else {						/*if Tx not OK, ignore Rx*/
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path A RXIQK step1 is not success\n"));


	return result;
	
	}

	u4tmp = 0x80007C00 | (regE94 & 0x3FF0000)  | ((regE9C & 0x3FF0000) >> 16);
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, u4tmp);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xe40 = 0x%x u4tmp = 0x%x\n", ODM_GetBBReg(pDM_Odm, rTx_IQK, bMaskDWord), u4tmp));


	/* =============================RXIQK setting=============================*/
	/*modify RXIQK mode table*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]====================Path A RXIQK step2!====================\n"));
	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path A RXIQK modify RXIQK mode table 2!\n"));*/
	/*leave IQK mode*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);


	/*PA/PAD control by 0x56, and set = 0x0*/	
	if (pDM_Odm->CutVersion == ODM_CUT_A) {
		
		RXPGA = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask);
		
		if (pDM_Odm->ExtPA) {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, 0x00800, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, 0x003e0, 0x3);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0x28000);
		} else {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, 0x00800, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, 0x003e0, 0x0);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xae000);
		}
	} else {
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, 0x00800, 0x1);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x56, 0x003e0, 0x3);
	}
	/*enter IQK mode*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x808000);

	/*IQK setting*/
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);

	/*path-A IQK setting*/
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x18008c0c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_B, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_B, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_A, bMaskDWord, 0x82170000);
	
	if (pDM_Odm->ExtPA == 1 && pDM_Odm->CutVersion == ODM_CUT_A)
		ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_A, bMaskDWord, 0x28170c00);
	else
		ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_A, bMaskDWord, 0x28170000);

	/*ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xe3c(RX_PI_Data)= 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xe3c, bMaskDWord)));*/


	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] LO calibration setting!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x0046a8d1);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] One shot, path A LOK & IQK!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf9005800);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8005800);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_92E));*/
	ODM_delay_ms(IQK_DELAY_TIME_97F);

	/*Check failed*/
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regEA4 = ODM_GetBBReg(pDM_Odm, rRx_Power_Before_IQK_A_2, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xea4 = 0x%x, 0xeac = 0x%x\n", regEA4, regEAC));
	/*monitor image power before & after IQK*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xea0(before IQK)= 0x%x, 0xea8(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xea0, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xea8, bMaskDWord)));

	/*PA/PAD controlled by 0x0 & Restore RXPGA*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, 0x00800, 0x0);
	if (pDM_Odm->CutVersion == ODM_CUT_A)
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, RXPGA);

	if (!(regEAC & BIT27) &&	/*if Tx is OK, check whether Rx is OK*/
		(((regEA4 & 0x03FF0000) >> 16) != 0x132) &&
		(((regEAC & 0x03FF0000) >> 16) != 0x36))
		result |= 0x02;
	else
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path A RxIQK step2 is not success!!\n"));
	return result;



}

u1Byte				/*bit0 = 1 => Tx OK, bit1 = 1 => Rx OK*/
phy_PathB_IQK_8197F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID
#else
	PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte regEAC, regEB4, regEBC;
	u1Byte	result = 0x00;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] ====================Path B TXIQK start!====================\n"));

	/* =============================TXIQK setting=============================*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);

	/*path A to SI mode to avoid RF go to shot-down mode*/
	ODM_SetBBReg(pDM_Odm, 0x820, BIT8, 0x0);

	if (pDM_Odm->ExtPA) {
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, 0x00800, 0x1);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, 0x003ff, 0x71);	
	} else if (pDM_Odm->ExtPA == 0 && pDM_Odm->CutVersion != ODM_CUT_A) {	
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, 0x00800, 0x1);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, 0x003ff, 0xe8);
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0x56 at Path B TXIQK = 0x%x\n",
				 ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, bRFRegOffsetMask)));

	/*enter IQK mode*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x808000);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_B, bMaskDWord, 0x18008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_B, bMaskDWord, 0x38008c0c);

	if (pDM_Odm->CutVersion == ODM_CUT_A) { 
		if (pDM_Odm->ExtPA)
			ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_B, bMaskDWord, 0x8214400f);
		else
			ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_B, bMaskDWord, 0x82140002);
	} else
		ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_B, bMaskDWord, 0x8214000f);

	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_B, bMaskDWord, 0x28140000);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] LO calibration setting!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x00e62911);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] One shot, path B LOK & IQK!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xfa005800);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8005800);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Delay %d ms for One shot, path B LOK & IQK.\n", IQK_DELAY_TIME_97F));*/
	ODM_delay_ms(IQK_DELAY_TIME_97F);

	/*Check failed*/
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regEB4 = ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_B, bMaskDWord);
	regEBC = ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_B, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xeb4 = 0x%x, 0xebc = 0x%x\n", regEB4, regEBC));
	/*monitor image power before & after IQK*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xeb0(before IQK)= 0x%x, 0xeb8(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xeb0, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xeb8, bMaskDWord)));

	/*Path A back to PI mode*/
	ODM_SetBBReg(pDM_Odm, 0x820, BIT8, 0x1);

	/*reload 0xdf and CCK_IND off  */
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x0, BIT14, 0x0);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, 0x00800, 0x0);

	if (!(regEAC & BIT31) &&
		(((regEB4 & 0x03FF0000) >> 16) != 0x142) &&
		(((regEBC & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else 
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path B TXIQK is not success\n"));
	return result;

}



u1Byte			/*bit0 = 1 => Tx OK, bit1 = 1 => Rx OK*/
phy_PathB_RxIQK_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	BOOLEAN		configPathB
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte regEAC, regEB4, regEBC, regECC, regEC4, u4tmp, RXPGA;
	u1Byte result = 0x00;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path B RxIQK start!\n"));

	/* =============================Get TXIMR setting=============================*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] ====================Path B RXIQK step1!====================\n"));
	/*modify RXIQK mode table*/
	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path B RXIQK modify RXIQK mode table!\n"));*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);

	
	if (pDM_Odm->CutVersion == ODM_CUT_A) {
		RXPGA = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask);
		
		if (pDM_Odm->ExtPA)
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xa8000);
		else
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xae000);
	}	

	/*path A to SI mode to avoid RF go to shot-down mode*/
	ODM_SetBBReg(pDM_Odm, 0x820, BIT8, 0x0);
	
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x808000);

	/*path-B IQK setting*/
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_B, bMaskDWord, 0x18008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_B, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_B, bMaskDWord, 0x82160000);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_B, bMaskDWord, 0x28160000);
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, 0x01007c00);
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] LO calibration setting!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x0046a911);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] One shot, Path B LOK & IQK!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xfa005800);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8005800);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]Delay %d ms for One shot, path B LOK & IQK.\n", IQK_DELAY_TIME_97F));*/
	ODM_delay_ms(IQK_DELAY_TIME_97F);

	/*Check failed*/
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regEB4 = ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_B, bMaskDWord);
	regEBC = ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_B, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xeb4 = 0x%x, 0xebc = 0x%x\n", regEB4, regEBC));
	/*monitor image power before & after IQK*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xeb0(before IQK)= 0x%x, 0xeb8(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xeb0, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xeb8, bMaskDWord)));

	if (pDM_Odm->CutVersion == ODM_CUT_A) {
	/*Restore RXPGA*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, RXPGA);
	}
	
	/*Path A back to PI mode*/
	ODM_SetBBReg(pDM_Odm, 0x820, BIT8, 0x1);


	if (!(regEAC & BIT31) &&
		(((regEB4 & 0x03FF0000) >> 16) != 0x142) &&
		(((regEBC & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else {						/*if Tx not OK, ignore Rx*/
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path B RXIQK step1 is not success\n"));
		return result;
		
	}

	u4tmp = 0x80007C00 | (regEB4 & 0x3FF0000)  | ((regEBC & 0x3FF0000) >> 16);
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, u4tmp);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe40 = 0x%x u4tmp = 0x%x\n", ODM_GetBBReg(pDM_Odm, rTx_IQK, bMaskDWord), u4tmp));


	/* =============================RXIQK setting=============================*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] ====================Path B RXIQK step2!====================\n"));

	/*modify RXIQK mode table*/
	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path-B RXIQK modify RXIQK mode table 2!\n"));*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);


	/*PA/PAD control by 0x56, and set = 0x0*/	
	if (pDM_Odm->CutVersion == ODM_CUT_A) {
		
		RXPGA = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask);
		
		if (pDM_Odm->ExtPA) {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, 0x00800, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, 0x003e0, 0x3);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0x28000);
		} else {
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, 0x00800, 0x1);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, 0x003e0, 0x0);
			ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xae000);
		}
	} else {
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, 0x00800, 0x1);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x56, 0x003e0, 0x3);
	}

	/*path A to SI mode to avoid RF go to shot-down mode*/
	ODM_SetBBReg(pDM_Odm, 0x820, BIT8, 0x0);

	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x808000);

	/*IQK setting*/
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);

	/*path-B IQK setting*/
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_B, bMaskDWord, 0x38008c0c);
	ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_B, bMaskDWord, 0x18008c0c);
	ODM_SetBBReg(pDM_Odm, rTx_IQK_PI_B, bMaskDWord, 0x82170000);

	if (pDM_Odm->ExtPA == 1 && pDM_Odm->CutVersion == ODM_CUT_A)
		ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_B, bMaskDWord, 0x28170c00);
	else
		ODM_SetBBReg(pDM_Odm, rRx_IQK_PI_B, bMaskDWord, 0x28170000);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xe5c(RX_PI_Data)= 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xe5c, bMaskDWord)));

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LO calibration setting!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Rsp, bMaskDWord, 0x0046a8d1);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path B LOK & IQK!\n"));*/
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xfa005800);
	ODM_SetBBReg(pDM_Odm, rIQK_AGC_Pts, bMaskDWord, 0xf8005800);

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Delay %d ms for One shot, Path B LOK & IQK.\n", IQK_DELAY_TIME_97F));*/
	ODM_delay_ms(IQK_DELAY_TIME_97F);


	/*Check failed*/
	regEAC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord);
	regEC4 = ODM_GetBBReg(pDM_Odm, rRx_Power_Before_IQK_B_2, bMaskDWord);
	regECC = ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_B_2, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xeac = 0x%x\n", regEAC));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xec4 = 0x%x, 0xecc = 0x%x\n", regEC4, regECC));
	/*monitor image power before & after IQK*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xec0(before IQK)= 0x%x, 0xec8(afer IQK) = 0x%x\n",
				 ODM_GetBBReg(pDM_Odm, 0xec0, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xec8, bMaskDWord)));

	/*PA/PAD controlled by 0x0 & Restore RXPGA*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, 0x00800, 0x0);
	if (pDM_Odm->CutVersion == ODM_CUT_A)
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, RXPGA);	

	/*Path A back to PI mode*/
	ODM_SetBBReg(pDM_Odm, 0x820, BIT8, 0x1);

	if (!(regEAC & BIT30) &&/*if Tx is OK, check whether Rx is OK*/
		(((regEC4 & 0x03FF0000) >> 16) != 0x132) &&
		(((regECC & 0x03FF0000) >> 16) != 0x36))
		result |= 0x02;
	else
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path B RXIQK step2 is not success!!\n"));
	return result;


}

VOID
_PHY_PathAFillIQKMatrix_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	BOOLEAN		bIQKOK,
	s4Byte		result[][8],
	u1Byte		final_candidate,
	BOOLEAN		bTxOnly
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte	Oldval_0, X, TX0_A, reg;
	s4Byte	Y, TX0_C;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path A IQ Calibration %s !\n", (bIQKOK) ? "Success" : "Failed"));

	if (final_candidate == 0xFF)
		return;

	else if (bIQKOK) {
#if 0		
		Oldval_0 = (ODM_GetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, bMaskDWord) >> 22) & 0x3FF;

		X = result[final_candidate][0];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX0_A = (X * Oldval_0) >> 8;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] X = 0x%x, TX0_A = 0x%x, Oldval_0 0x%x\n", X, TX0_A, Oldval_0));
		ODM_SetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, 0x3FF, TX0_A);

		ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT(31), ((X * Oldval_0 >> 7) & 0x1));

		Y = result[final_candidate][1];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;


		TX0_C = (Y * Oldval_0) >> 8;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Y = 0x%x, TX0_C = 0x%x\n", Y, TX0_C));
		ODM_SetBBReg(pDM_Odm, rOFDM0_XCTxAFE, 0xF0000000, ((TX0_C & 0x3C0) >> 6));
		ODM_SetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, 0x003F0000, (TX0_C & 0x3F));

		ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT(29), ((Y * Oldval_0 >> 7) & 0x1));
#endif
				Oldval_0 = (ODM_GetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, bMaskDWord) >> 22) & 0x3FF;
		
				ODM_SetBBReg(pDM_Odm, rOFDM0_XATxIQImbalance, 0x3FF, Oldval_0);
				
				X = result[final_candidate][0];
				if ((X & 0x00000200) != 0)
					X = X | 0xFFFFFC00;
				
				TX0_A = (X * 0x100) >> 8;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] X = 0x%x, TX0_A = 0x%x\n", X, TX0_A));
		
				ODM_SetBBReg(pDM_Odm, 0xe30, 0x3FF00000, TX0_A);
		
				Y = result[final_candidate][1];
				if ((Y & 0x00000200) != 0)
					Y = Y | 0xFFFFFC00;
		
				TX0_C = (Y * 0x100) >> 8;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Y = 0x%x, TX0_C = 0x%x\n", Y, TX0_C));
				ODM_SetBBReg(pDM_Odm, 0xe20, 0x000003C0, ((TX0_C & 0x3C0) >> 6));
				ODM_SetBBReg(pDM_Odm, 0xe20, 0x0000003F, (TX0_C & 0x3F));

		if (bTxOnly) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] _PHY_PathAFillIQKMatrix_97F only Tx OK\n"));
			return;
		}

		reg = result[final_candidate][2];
		#if (DM_ODM_SUPPORT_TYPE == ODM_AP)
		if (RTL_ABS(reg , 0x100) >= 16)
			reg = 0x100;
		#endif
		ODM_SetBBReg(pDM_Odm, rOFDM0_XARxIQImbalance, 0x3FF, reg);

		reg = result[final_candidate][3] & 0x3F;
		ODM_SetBBReg(pDM_Odm, rOFDM0_XARxIQImbalance, 0xFC00, reg);

		reg = (result[final_candidate][3] >> 6) & 0xF;
		ODM_SetBBReg(pDM_Odm, rOFDM0_RxIQExtAnta, 0xF0000000, reg);
	}
}

VOID
_PHY_PathBFillIQKMatrix_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	BOOLEAN		bIQKOK,
	s4Byte		result[][8],
	u1Byte		final_candidate,
	BOOLEAN		bTxOnly			/*do Tx only*/
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte	Oldval_1, X, TX1_A, reg;
	s4Byte	Y, TX1_C;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path B IQ Calibration %s !\n", (bIQKOK) ? "Success" : "Failed"));

	if (final_candidate == 0xFF)
		return;

	else if (bIQKOK) {

#if 0		
		Oldval_1 = (ODM_GetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, bMaskDWord) >> 22) & 0x3FF;

		X = result[final_candidate][4];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX1_A = (X * Oldval_1) >> 8;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] X = 0x%x, TX1_A = 0x%x\n", X, TX1_A));
		ODM_SetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, 0x3FF, TX1_A);

		ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT(27), ((X * Oldval_1 >> 7) & 0x1));

		Y = result[final_candidate][5];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;

		TX1_C = (Y * Oldval_1) >> 8;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Y = 0x%x, TX1_C = 0x%x\n", Y, TX1_C));
		ODM_SetBBReg(pDM_Odm, rOFDM0_XDTxAFE, 0xF0000000, ((TX1_C & 0x3C0) >> 6));
		ODM_SetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, 0x003F0000, (TX1_C & 0x3F));

		ODM_SetBBReg(pDM_Odm, rOFDM0_ECCAThreshold, BIT(25), ((Y * Oldval_1 >> 7) & 0x1));

#endif
				Oldval_1 = (ODM_GetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, bMaskDWord) >> 22) & 0x3FF;
		
				ODM_SetBBReg(pDM_Odm, rOFDM0_XBTxIQImbalance, 0x3FF, Oldval_1);
		
				X = result[final_candidate][4];
				if ((X & 0x00000200) != 0)
					X = X | 0xFFFFFC00;
				
				TX1_A = (X * 0x100) >> 8;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] X = 0x%x, TX1_A = 0x%x\n", X, TX1_A));
		
				ODM_SetBBReg(pDM_Odm, 0xe50, 0x3FF00000, TX1_A);
		
				Y = result[final_candidate][5];
				if ((Y & 0x00000200) != 0)
					Y = Y | 0xFFFFFC00;
			
				TX1_C = (Y * 0x100) >> 8;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Y = 0x%x, TX1_C = 0x%x\n", Y, TX1_C));
				ODM_SetBBReg(pDM_Odm, 0xe24, 0x000003C0, ((TX1_C & 0x3C0) >> 6));
				ODM_SetBBReg(pDM_Odm, 0xe24, 0x0000003F, (TX1_C & 0x3F));

		if (bTxOnly) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] _PHY_PathBFillIQKMatrix_97F only Tx OK\n"));
			return;
		}

		reg = result[final_candidate][6];
		ODM_SetBBReg(pDM_Odm, rOFDM0_XBRxIQImbalance, 0x3FF, reg);

		reg = result[final_candidate][7] & 0x3F;
		ODM_SetBBReg(pDM_Odm, rOFDM0_XBRxIQImbalance, 0xFC00, reg);

		reg = (result[final_candidate][7] >> 6) & 0xF;
		ODM_SetBBReg(pDM_Odm, 0xca8, 0x000000F0, reg);
	}
}

#if (DM_ODM_SUPPORT_TYPE & ODM_CE)
BOOLEAN
ODM_CheckPowerStatus(
	IN	PADAPTER		Adapter)
{
#if 0
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	PDM_ODM_T			pDM_Odm = &pHalData->DM_OutSrc;
	RT_RF_POWER_STATE	rtState;
	PMGNT_INFO			pMgntInfo	= &(Adapter->MgntInfo);

	/* 2011/07/27 MH We are not testing ready~~!! We may fail to get correct value when init sequence.*/
	if (pMgntInfo->init_adpt_in_progress == TRUE) {
		ODM_RT_TRACE(pDM_Odm, COMP_INIT, DBG_LOUD, ("ODM_CheckPowerStatus Return TRUE, due to initadapter"));
		return	TRUE;
	}


	Adapter->HalFunc.GetHwRegHandler(Adapter, HW_VAR_RF_STATE, (pu1Byte)(&rtState));
	if (Adapter->bDriverStopped || Adapter->bDriverIsGoingToPnpSetPowerSleep || rtState == eRfOff) {
		ODM_RT_TRACE(pDM_Odm, COMP_INIT, DBG_LOUD, ("ODM_CheckPowerStatus Return FALSE, due to %d/%d/%d\n",
		Adapter->bDriverStopped, Adapter->bDriverIsGoingToPnpSetPowerSleep, rtState));
		return	FALSE;
	}
#endif
	return	TRUE;
}
#endif


VOID
_PHY_SaveADDARegisters_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	pu4Byte		ADDAReg,
	pu4Byte		ADDABackup,
	u4Byte		RegisterNum
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte	i;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif

	if (ODM_CheckPowerStatus(pAdapter) == FALSE)
		return;
	#endif

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Save ADDA parameters.\n"));*/
	for (i = 0 ; i < RegisterNum ; i++)
		ADDABackup[i] = ODM_GetBBReg(pDM_Odm, ADDAReg[i], bMaskDWord);
}


VOID
_PHY_SaveMACRegisters_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	pu4Byte		MACReg,
	pu4Byte		MACBackup
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte	i;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Save MAC parameters.\n"));*/
	for (i = 0 ; i < (IQK_MAC_REG_NUM - 1); i++)
		MACBackup[i] = ODM_Read1Byte(pDM_Odm, MACReg[i]);
	MACBackup[i] = ODM_Read4Byte(pDM_Odm, MACReg[i]);

}


VOID
_PHY_ReloadADDARegisters_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	pu4Byte		ADDAReg,
	pu4Byte		ADDABackup,
	u4Byte		RegiesterNum
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte	i;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Reload ADDA power saving parameters !\n"));
	for (i = 0 ; i < RegiesterNum; i++)
		ODM_SetBBReg(pDM_Odm, ADDAReg[i], bMaskDWord, ADDABackup[i]);
}

VOID
_PHY_ReloadMACRegisters_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	pu4Byte		MACReg,
	pu4Byte		MACBackup
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte	i;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Reload MAC parameters !\n"));
	#if 0
	ODM_SetBBReg(pDM_Odm, 0x520, bMaskByte2, 0x0);
	#else
	for (i = 0 ; i < (IQK_MAC_REG_NUM - 1); i++)
		ODM_Write1Byte(pDM_Odm, MACReg[i], (u1Byte)MACBackup[i]);
	ODM_Write4Byte(pDM_Odm, MACReg[i], MACBackup[i]);
	#endif
}

VOID
_PHY_PathADDAOn_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	pu4Byte		ADDAReg,
	BOOLEAN		isPathAOn,
	BOOLEAN		is2T
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte	pathOn;
	u4Byte	i;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] ADDA ON.\n"));*/

	ODM_SetBBReg(pDM_Odm, 0xd94, 0x00ff0000, 0xff);	
	ODM_SetBBReg(pDM_Odm, 0xe70, bMaskDWord, 0x00400040);
	
#if 0
	pathOn = isPathAOn ? 0x0fc01616 : 0x0fc01616;
	if (FALSE == is2T) {
		pathOn = 0x0fc01616;
		ODM_SetBBReg(pDM_Odm, ADDAReg[0], bMaskDWord, 0x0fc01616);
	} else
		ODM_SetBBReg(pDM_Odm, ADDAReg[0], bMaskDWord, pathOn);

	for (i = 1 ; i < IQK_ADDA_REG_NUM ; i++)
		ODM_SetBBReg(pDM_Odm, ADDAReg[i], bMaskDWord, pathOn);
#endif
}

VOID
_PHY_MACSettingCalibration_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	pu4Byte		MACReg,
	pu4Byte		MACBackup
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	/*u4Byte	i = 0;*/
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
/*	ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("MAC settings for Calibration.\n"));*/

#if 0
	ODM_Write1Byte(pDM_Odm, MACReg[i], 0x3F);

	for (i = 1 ; i < (IQK_MAC_REG_NUM - 1); i++)
		ODM_Write1Byte(pDM_Odm, MACReg[i], (u1Byte)(MACBackup[i] & (~BIT3)));

	ODM_Write1Byte(pDM_Odm, MACReg[i], (u1Byte)(MACBackup[i] & (~BIT5)));
#endif

	#if 1
	ODM_SetBBReg(pDM_Odm, 0x520, bMaskByte2, 0xff);
	#else
	ODM_SetBBReg(pDM_Odm, 0x522, bMaskByte0, 0x7f);
	ODM_SetBBReg(pDM_Odm, 0x550, bMaskByte0, 0x15);
	ODM_SetBBReg(pDM_Odm, 0x551, bMaskByte0, 0x00);
	#endif


}

VOID
_PHY_PathAStandBy_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID
#else
	PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path-A standby mode!\n"));

	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x0);
	/*ODM_SetBBReg(pDM_Odm, 0x840, bMaskDWord, 0x00010000);*/
	ODM_SetRFReg(pDM_Odm, 0x0, 0x0, bRFRegOffsetMask, 0x10000);
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x808000);
}

VOID
_PHY_PathBStandBy_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID
#else
	PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path-B standby mode!\n"));

	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);
	ODM_SetRFReg(pDM_Odm, 0x1, 0x0, bRFRegOffsetMask, 0x10000);

	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x808000);
}


VOID
_PHY_PIModeSwitch_97F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	BOOLEAN		PIMode
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte	mode;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	/*DM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] BB Switch to %s mode!\n", (PIMode ? "PI" : "SI")));*/

	mode = PIMode ? 0x01000100 : 0x01000000;
	ODM_SetBBReg(pDM_Odm, rFPGA0_XA_HSSIParameter1, bMaskDWord, mode);
	ODM_SetBBReg(pDM_Odm, rFPGA0_XB_HSSIParameter1, bMaskDWord, mode);
}

BOOLEAN
phy_SimularityCompare_8197F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	s4Byte		result[][8],
	u1Byte		 c1,
	u1Byte		 c2
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u4Byte		i, j, diff, SimularityBitMap, bound = 0;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	u1Byte		final_candidate[2] = {0xFF, 0xFF};	/*for path A and path B*/
	BOOLEAN		bResult = TRUE;
/*#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)*/
/*	BOOLEAN		is2T = IS_92C_SERIAL( pHalData->VersionID);*/
/*#else*/
	BOOLEAN		is2T = TRUE;
/*#endif*/

	s4Byte tmp1 = 0, tmp2 = 0;

	if (is2T)
		bound = 8;
	else
		bound = 4;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] ===> IQK:phy_SimularityCompare_8197F c1 %d c2 %d!!!\n", c1, c2));


	SimularityBitMap = 0;

	for (i = 0; i < bound; i++) {

		if ((i == 1) || (i == 3) || (i == 5) || (i == 7)) {
			if ((result[c1][i] & 0x00000200) != 0)
				tmp1 = result[c1][i] | 0xFFFFFC00;
			else
				tmp1 = result[c1][i];

			if ((result[c2][i] & 0x00000200) != 0)
				tmp2 = result[c2][i] | 0xFFFFFC00;
			else
				tmp2 = result[c2][i];
		} else {
			tmp1 = result[c1][i];
			tmp2 = result[c2][i];
		}

		diff = (tmp1 > tmp2) ? (tmp1 - tmp2) : (tmp2 - tmp1);

		if (diff > MAX_TOLERANCE) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] differnece overflow %d index %d compare1 0x%x compare2 0x%x!!!\n",  diff, i, result[c1][i], result[c2][i]));

			if ((i == 2 || i == 6) && !SimularityBitMap) {
				if (result[c1][i] + result[c1][i + 1] == 0)
					final_candidate[(i / 4)] = c2;
				else if (result[c2][i] + result[c2][i + 1] == 0)
					final_candidate[(i / 4)] = c1;
				else
					SimularityBitMap = SimularityBitMap | (1 << i);
			} else
				SimularityBitMap = SimularityBitMap | (1 << i);
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] phy_SimularityCompare_8197F SimularityBitMap   %x !!!\n", SimularityBitMap));

	if (SimularityBitMap == 0) {
		for (i = 0; i < (bound / 4); i++) {
			if (final_candidate[i] != 0xFF) {
				for (j = i * 4; j < (i + 1) * 4 - 2; j++)
					result[3][j] = result[final_candidate[i]][j];
				bResult = FALSE;
			}
		}
		return bResult;

	} else {

		if (!(SimularityBitMap & 0x03)) {		/*path A TX OK*/
			for (i = 0; i < 2; i++)
				result[3][i] = result[c1][i];
		}

		if (!(SimularityBitMap & 0x0c)) {		/*path A RX OK*/
			for (i = 2; i < 4; i++)
				result[3][i] = result[c1][i];
		}

		if (!(SimularityBitMap & 0x30)) {	/*path B TX OK*/
			for (i = 4; i < 6; i++)
				result[3][i] = result[c1][i];

		}

		if (!(SimularityBitMap & 0xc0)) {	/*path B RX OK*/
			for (i = 6; i < 8; i++)
				result[3][i] = result[c1][i];
		}

		return FALSE;
	}

}

VOID
_phy_iqk_check_97f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif	
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	u4Byte		txa_fail, rxa_fail, txb_fail, rxb_fail;


	txa_fail = ODM_GetBBReg(pDM_Odm, 0xeac, BIT(28));
	rxa_fail = ODM_GetBBReg(pDM_Odm, 0xeac, BIT(27));
	txb_fail = ODM_GetBBReg(pDM_Odm, 0xeac, BIT(31));
	rxb_fail = ODM_GetBBReg(pDM_Odm, 0xeac, BIT(30));

	if (txa_fail == 1)
		panic_printk("[IQK] path A TXIQK load default value!!!\n");
	if (rxa_fail == 1)
		panic_printk("[IQK] path A RXIQK load default value!!!\n");
	if (txb_fail == 1)
		panic_printk("[IQK] path B TXIQK load default value!!!\n");
	if (rxb_fail == 1)
		panic_printk("[IQK] path B RXIQK load default value!!!\n");
		
}



VOID
phy_IQCalibrate_8197F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	s4Byte		result[][8],
	u1Byte		t,
	BOOLEAN		is2T
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	u4Byte			i, DAC_gain_A, DAC_gain_B;
	u1Byte			PathAOK = 0, PathBOK = 0;
	u1Byte			tmp0xc50 = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xC50, bMaskByte0);
	u1Byte			tmp0xc58 = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xC58, bMaskByte0);
	u4Byte			ADDA_REG[IQK_ADDA_REG_NUM] = {
		0xd94, rRx_Wait_CCA		
	};
	u4Byte			IQK_MAC_REG[IQK_MAC_REG_NUM] = {
		REG_TXPAUSE,	REG_BCN_CTRL,
		REG_BCN_CTRL_1,	REG_GPIO_MUXCFG
	};

	/*since 92C & 92D have the different define in IQK_BB_REG*/
	u4Byte	IQK_BB_REG_92C[IQK_BB_REG_NUM] = {
		rOFDM0_TRxPathEnable,	rOFDM0_TRMuxPar,
		rFPGA0_XCD_RFInterfaceSW,	rConfig_AntA,	rConfig_AntB,
		0x930,	0x934,
		0x93c,	rCCK0_AFESetting
	};

	#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	u4Byte	retryCount = 2;
	#else
	#if MP_DRIVER
	const u4Byte	retryCount = 9;
	#else
	const u4Byte	retryCount = 2;
	#endif
	#endif

	/*Note: IQ calibration must be performed after loading*/
	/*PHY_REG.txt,and radio_a,radio_b.txt*/

	/*u4Byte bbvalue;*/

	#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	#ifdef MP_TEST
	if (pDM_Odm->priv->pshare->rf_ft_var.mp_specific)
		retryCount = 2;
	#endif
	#endif

#if 0
		/*RF setting :RXBB leave power saving*/
		if (pDM_Odm->CutVersion != ODM_CUT_A) {
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, 0x00002, 0x1);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, 0x00002, 0x1);
		}
#endif
		/*Save and set DAC gain for IQK*/
		DAC_gain_A = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x55, bRFRegOffsetMask);
		DAC_gain_B = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x55, bRFRegOffsetMask);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Ori_0x55 at Path A = 0x%x\n", DAC_gain_A));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Ori_0x55 at Path B = 0x%x\n", DAC_gain_B));
	
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x55, 0x000e0, 0x0);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x55, 0x000e0, 0x0);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Set_0x55 at Path A = 0x%x\n",
				 ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x55, bRFRegOffsetMask)));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Set_0x55 at Path B = 0x%x\n",
				 ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x55, bRFRegOffsetMask)));

	if (t == 0) {
		/*bbvalue = ODM_GetBBReg(pDM_Odm, rFPGA0_RFMOD, bMaskDWord);*/
		/*RT_DISP(FINIT, INIT_IQK, ("phy_IQCalibrate_8188E()==>0x%08x\n",bbvalue));*/
		/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQ Calibration for %s for %d times\n", (is2T ? "2T2R" : "1T1R"), t));*/

		/*Save ADDA parameters, turn Path A ADDA on*/
		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
		_PHY_SaveADDARegisters_97F(pAdapter, ADDA_REG, pDM_Odm->RFCalibrateInfo.ADDA_backup, IQK_ADDA_REG_NUM);
		_PHY_SaveMACRegisters_97F(pAdapter, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);
		_PHY_SaveADDARegisters_97F(pAdapter, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup, IQK_BB_REG_NUM);
		#else
		_PHY_SaveADDARegisters_97F(pDM_Odm, ADDA_REG, pDM_Odm->RFCalibrateInfo.ADDA_backup, IQK_ADDA_REG_NUM);
		_PHY_SaveMACRegisters_97F(pDM_Odm, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);
		_PHY_SaveADDARegisters_97F(pDM_Odm, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup, IQK_BB_REG_NUM);
		#endif

	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] IQ Calibration for %s for %d times\n", (is2T ? "2T2R" : "1T1R"), t));

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)

	_PHY_PathADDAOn_97F(pAdapter, ADDA_REG, TRUE, is2T);
	#else
	_PHY_PathADDAOn_97F(pDM_Odm, ADDA_REG, TRUE, is2T);
	#endif


	/*BB setting*/
	/*ODM_SetBBReg(pDM_Odm, rFPGA0_RFMOD, BIT24, 0x00);*/
	ODM_SetBBReg(pDM_Odm, rCCK0_AFESetting, 0x0f000000, 0xf);
	ODM_SetBBReg(pDM_Odm, rOFDM0_TRxPathEnable, bMaskDWord, 0x6f005403);
	ODM_SetBBReg(pDM_Odm, rOFDM0_TRMuxPar, bMaskDWord, 0x000804e4);
	ODM_SetBBReg(pDM_Odm, rFPGA0_XCD_RFInterfaceSW, bMaskDWord, 0x04203400);	
	
	
#if 1
	/*FEM off when ExtPA or ExtLNA= 1*/
	if (pDM_Odm->ExtPA || pDM_Odm->ExtLNA) {
		ODM_SetBBReg(pDM_Odm, 0x930, bMaskDWord, 0xFFFF77FF);
		ODM_SetBBReg(pDM_Odm, 0x934, bMaskDWord, 0xFFFFFFF7);
		ODM_SetBBReg(pDM_Odm, 0x93c, bMaskDWord, 0xFFFF777F);
	} 	
#endif


/*	if(is2T) {*/
/*		ODM_SetBBReg(pDM_Odm, rFPGA0_XA_LSSIParameter, bMaskDWord, 0x00010000);*/
/*		ODM_SetBBReg(pDM_Odm, rFPGA0_XB_LSSIParameter, bMaskDWord, 0x00010000);*/
/*	}*/

	/*MAC settings*/
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	_PHY_MACSettingCalibration_97F(pAdapter, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);
	#else
	_PHY_MACSettingCalibration_97F(pDM_Odm, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);
	#endif

	/* IQ calibration setting*/
	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK setting!\n"));*/
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x808000);
	ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, 0x01007c00);
	ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);
	

/*path A TXIQK*/
	#if 1
	for (i = 0 ; i < retryCount ; i++) {
		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
		PathAOK = phy_PathA_IQK_8197F(pAdapter, is2T);
		#else
		PathAOK = phy_PathA_IQK_8197F(pDM_Odm, is2T);
		#endif
		if (PathAOK == 0x01) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path A TXIQK Success!!\n"));
			result[t][0] = (ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord) & 0x3FF0000) >> 16;
			result[t][1] = (ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord) & 0x3FF0000) >> 16;
			break;
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path A TXIQK Fail!!\n"));
			result[t][0] = 0x100;
			result[t][1] = 0x0;
		}
		#if 0
		else if (i == (retryCount - 1) && PathAOK == 0x01) {	/*Tx IQK OK*/
			RT_DISP(FINIT, INIT_IQK, ("Path A IQK Only  Tx Success!!\n"));

			result[t][0] = (ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_A, bMaskDWord) & 0x3FF0000) >> 16;
			result[t][1] = (ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_A, bMaskDWord) & 0x3FF0000) >> 16;
		}
		#endif
	}
	#endif

/*path A RXIQK*/
	#if 1
	for (i = 0 ; i < retryCount ; i++) {
		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
		PathAOK = phy_PathA_RxIQK_97F(pAdapter, is2T);
		#else
		PathAOK = phy_PathA_RxIQK_97F(pDM_Odm, is2T);
		#endif
		if (PathAOK == 0x03) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path A RXIQK Success!!\n"));
			result[t][2] = (ODM_GetBBReg(pDM_Odm, rRx_Power_Before_IQK_A_2, bMaskDWord) & 0x3FF0000) >> 16;
			result[t][3] = (ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_A_2, bMaskDWord) & 0x3FF0000) >> 16;
			break;
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path A RXIQK Fail!!\n"));
			result[t][2] = 0x100;
			result[t][3] = 0x0;
		}
	}

	if (0x00 == PathAOK)
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path A IQK failed!!\n"));

	#endif

	if (is2T) {

		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
		_PHY_PathAStandBy_97F(pAdapter);

		/*Turn ADDA on*/
		_PHY_PathADDAOn_97F(pAdapter, ADDA_REG, FALSE, is2T);
		#else
		_PHY_PathAStandBy_97F(pDM_Odm);

		/*Turn ADDA on*/
		_PHY_PathADDAOn_97F(pDM_Odm, ADDA_REG, FALSE, is2T);
		#endif

		/*IQ calibration setting*/
		/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK setting!\n"));*/
		ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x808000);
		ODM_SetBBReg(pDM_Odm, rTx_IQK, bMaskDWord, 0x01007c00);
		ODM_SetBBReg(pDM_Odm, rRx_IQK, bMaskDWord, 0x01004800);

/*path B Tx IQK*/
		#if 1
		for (i = 0 ; i < retryCount ; i++) {
			#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
			PathBOK = phy_PathB_IQK_8197F(pAdapter);
			#else
			PathBOK = phy_PathB_IQK_8197F(pDM_Odm);
			#endif

			if (PathBOK == 0x01) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path B TXIQK Success!!\n"));
				result[t][4] = (ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_B, bMaskDWord) & 0x3FF0000) >> 16;
				result[t][5] = (ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_B, bMaskDWord) & 0x3FF0000) >> 16;
				break;
			} else {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B TXIQK Fail!!\n"));
				result[t][4] = 0x100;
				result[t][5] = 0x0;
			}
			#if 0
			else if (i == (retryCount - 1) && PathAOK == 0x01) {	/*Tx IQK OK*/
				RT_DISP(FINIT, INIT_IQK, ("Path B IQK Only  Tx Success!!\n"));

				result[t][0] = (ODM_GetBBReg(pDM_Odm, rTx_Power_Before_IQK_B, bMaskDWord) & 0x3FF0000) >> 16;
				result[t][1] = (ODM_GetBBReg(pDM_Odm, rTx_Power_After_IQK_B, bMaskDWord) & 0x3FF0000) >> 16;
			}
			#endif
		}
		#endif

/*path B RX IQK*/
		#if 1

		for (i = 0 ; i < retryCount ; i++) {
			#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
			PathBOK = phy_PathB_RxIQK_97F(pAdapter, is2T);
			#else
			PathBOK = phy_PathB_RxIQK_97F(pDM_Odm, is2T);
			#endif
			if (PathBOK == 0x03) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Path B RXIQK Success!!\n"));
				result[t][6] = (ODM_GetBBReg(pDM_Odm, rRx_Power_Before_IQK_B_2, bMaskDWord) & 0x3FF0000) >> 16;
				result[t][7] = (ODM_GetBBReg(pDM_Odm, rRx_Power_After_IQK_B_2, bMaskDWord) & 0x3FF0000) >> 16;
				break;

			} else {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B Rx IQK Fail!!\n"));
				result[t][6] = 0x100;
				result[t][7] = 0x0;
		} 
	}

		if (0x00 == PathBOK)
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Path B IQK failed!!\n"));

		#endif
	}

	/*Back to BB mode, load original value*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Back to BB mode, load original value!\n"));
	ODM_SetBBReg(pDM_Odm, rFPGA0_IQK, 0xffffff00, 0x000000);

	if (t != 0) {


		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)

		/* Reload ADDA power saving parameters*/
		_PHY_ReloadADDARegisters_97F(pAdapter, ADDA_REG, pDM_Odm->RFCalibrateInfo.ADDA_backup, IQK_ADDA_REG_NUM);

		/* Reload MAC parameters*/
		_PHY_ReloadMACRegisters_97F(pAdapter, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);

		_PHY_ReloadADDARegisters_97F(pAdapter, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup, IQK_BB_REG_NUM);
		#else
		/* Reload ADDA power saving parameters*/
		_PHY_ReloadADDARegisters_97F(pDM_Odm, ADDA_REG, pDM_Odm->RFCalibrateInfo.ADDA_backup, IQK_ADDA_REG_NUM);

		/* Reload MAC parameters*/
		_PHY_ReloadMACRegisters_97F(pDM_Odm, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);

		_PHY_ReloadADDARegisters_97F(pDM_Odm, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup, IQK_BB_REG_NUM);
		#endif

		/*Allen initial gain 0xc50*/
		/* Restore RX initial gain*/
		ODM_SetBBReg(pDM_Odm, 0xc50, bMaskByte0, 0x50);
		ODM_SetBBReg(pDM_Odm, 0xc50, bMaskByte0, tmp0xc50);
		if (is2T) {
			ODM_SetBBReg(pDM_Odm, 0xc58, bMaskByte0, 0x50);
			ODM_SetBBReg(pDM_Odm, 0xc58, bMaskByte0, tmp0xc58);
		}
#if 0		
		/*RF setting :RXBB enter power saving*/
		if (pDM_Odm->CutVersion != ODM_CUT_A) {
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, 0x00002, 0x0);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0xdf, 0x00002, 0x0);
		}
#endif
		/*reload DAC gain for K-free*/
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x55, bRFRegOffsetMask, DAC_gain_A);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x55, bRFRegOffsetMask, DAC_gain_B);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Restore_0x55 at Path A = 0x%x\n",
				 ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x55, bRFRegOffsetMask)));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Restore_0x55 at Path B = 0x%x\n",
				 ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x55, bRFRegOffsetMask)));

		#if 0
		/*load 0xe30 IQC default value*/
		ODM_SetBBReg(pDM_Odm, rTx_IQK_Tone_A, bMaskDWord, 0x01008c00);
		ODM_SetBBReg(pDM_Odm, rRx_IQK_Tone_A, bMaskDWord, 0x01008c00);
		#endif

	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] phy_IQCalibrate_8197F() <==\n"));

}

VOID
phy_LCCalibrate_8197F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	BOOLEAN		is2T
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	u1Byte	tmpReg;
	u4Byte	RF_Amode = 0, RF_Bmode = 0, LC_Cal, cnt;
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
	/*Check continuous TX and Packet TX*/
	tmpReg = ODM_Read1Byte(pDM_Odm, 0xd03);

	if ((tmpReg & 0x70) != 0)			/*Deal with contisuous TX case*/
		ODM_Write1Byte(pDM_Odm, 0xd03, tmpReg & 0x8F);	/*disable all continuous TX*/
	else							/* Deal with Packet TX case*/
		ODM_Write1Byte(pDM_Odm, REG_TXPAUSE, 0xFF);			/* block all queues*/


	/*backup RF0x18*/
	LC_Cal = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask);
		
	/*Start LCK*/
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal|0x08000);
	
	for (cnt = 0; cnt < 100; cnt++) {
		if (ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, 0x8000) != 0x1)
			break;
		
		ODM_delay_ms(10);
	}
	
	/*Recover channel number*/
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, LC_Cal);	


	/*Restore original situation*/
	if ((tmpReg&0x70) != 0) {
		/*Deal with contisuous TX case*/
		ODM_Write1Byte(pDM_Odm, 0xd03, tmpReg);
	} else { 
		/* Deal with Packet TX case*/
		ODM_Write1Byte(pDM_Odm, REG_TXPAUSE, 0x00);
	}

}


VOID
PHY_IQCalibrate_8197F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	BOOLEAN		bReCovery
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#else  /*(DM_ODM_SUPPORT_TYPE == ODM_CE)*/
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif

	#if (MP_DRIVER == 1)
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PMPT_CONTEXT	pMptCtx = &(pAdapter->MptCtx);
	#else/* (DM_ODM_SUPPORT_TYPE == ODM_CE)*/
	PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.MptCtx);
	#endif
	#endif/*(MP_DRIVER == 1)*/
	#endif
	PODM_RF_CAL_T	pRFCalibrateInfo = &(pDM_Odm->RFCalibrateInfo);


	s4Byte			result[4][8];	/*last is final result*/
	u1Byte			i, final_candidate, Indexforchannel;
	u1Byte          channelToIQK = 7;
	BOOLEAN			bPathAOK, bPathBOK;
	s4Byte			RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC;
	BOOLEAN			is12simular, is13simular, is23simular;
	BOOLEAN			bStartContTx = FALSE, bSingleTone = FALSE, bCarrierSuppression = FALSE;
	u4Byte			IQK_BB_REG_92C[IQK_BB_REG_NUM] = {
		rOFDM0_XARxIQImbalance,		rOFDM0_XBRxIQImbalance,
		rOFDM0_ECCAThreshold,		0xca8,
		rOFDM0_XATxIQImbalance,		rOFDM0_XBTxIQImbalance,
		rOFDM0_XCTxAFE,				rOFDM0_XDTxAFE,
		rOFDM0_RxIQExtAnta
	};

	#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
	if (ODM_CheckPowerStatus(pAdapter) == FALSE)
		return;
	#else
	prtl8192cd_priv	priv = pDM_Odm->priv;

	#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific) {
		if ((OPMODE & WIFI_MP_CTX_PACKET) || (OPMODE & WIFI_MP_CTX_ST))
			return;
	}
	#endif
	/* if (priv->pshare->IQK_92E_done)*/
	/*		bReCovery= 1; */
	/*	priv->pshare->IQK_92E_done = 1;*/
	#endif

	#if 1//(DM_ODM_SUPPORT_TYPE == ODM_CE)
	if (!(pDM_Odm->SupportAbility & ODM_RF_CALIBRATION))
		return;
	#endif

	#if MP_DRIVER == 1
	bStartContTx = pMptCtx->bStartContTx;
	bSingleTone = pMptCtx->bSingleTone;
	bCarrierSuppression = pMptCtx->bCarrierSuppression;
	#endif

	/* 20120213<Kordan> Turn on when continuous Tx to pass lab testing. (required by Edlu)*/
	if (bSingleTone || bCarrierSuppression)
		return;

	#ifdef DISABLE_BB_RF
	return;
	#endif

	if (pRFCalibrateInfo->bIQKInProgress)
		return;	

	#if (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_AP))
	if (bReCovery)
	#else /*for ODM_WIN*/
	if (bReCovery && 
	(!pAdapter->bInHctTest)) /*YJ,add for PowerTest,120405*/
	#endif
	{
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("[IQK] PHY_IQCalibrate_97F: Return due to bReCovery!\n"));
#if 0
		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
		_PHY_ReloadADDARegisters_97F(pAdapter, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup_recover, 9);
		#else
		_PHY_ReloadADDARegisters_97F(pDM_Odm, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup_recover, 9);
		#endif
#endif		
		return;
	}

	/*turn off DPK*/
	phy_dpkoff_8197f(pDM_Odm);

	/*check IC cut and IQK version*/
	if (pDM_Odm->CutVersion == ODM_CUT_A)
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Cut Version is A\n"));	
	else if (pDM_Odm->CutVersion == ODM_CUT_B)
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Cut Version is B\n"));
	else if (pDM_Odm->CutVersion == ODM_CUT_C)
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Cut Version is C\n"));
	else if (pDM_Odm->CutVersion == ODM_CUT_D)
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] Cut Version is D\n"));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] IQK Version is v2.6 (20170109)\n"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] PHY Version is v%d \n", ODM_GetHWImgVersion(ODMPTR)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] ExtPA = %d, ExtLNA = %d\n", pDM_Odm->ExtPA, pDM_Odm->ExtLNA));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] IQK Start!!!\n"));
	priv->pshare->IQK_total_cnt++;

#if 0
	ODM_AcquireSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
	pRFCalibrateInfo->bIQKInProgress = TRUE;
	ODM_ReleaseSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
#endif

	for (i = 0; i < 8; i++) {
		result[0][i] = 0;
		result[1][i] = 0;
		result[2][i] = 0;

		if ((i == 0) || (i == 2) || (i == 4)  || (i == 6))
			result[3][i] = 0x100;
		else
			result[3][i] = 0;
	}

	final_candidate = 0xff;
	bPathAOK = FALSE;
	bPathBOK = FALSE;
	is12simular = FALSE;
	is23simular = FALSE;
	is13simular = FALSE;


	for (i = 0; i < 3; i++) {

		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)

		phy_IQCalibrate_8197F(pAdapter, result, i, TRUE);


		#else


		phy_IQCalibrate_8197F(pDM_Odm, result, i, TRUE);
		#endif


		if (i == 1) {
			#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
			is12simular = phy_SimularityCompare_8197F(pAdapter, result, 0, 1);
			#else
			is12simular = phy_SimularityCompare_8197F(pDM_Odm, result, 0, 1);
			#endif

			if (is12simular) {
				final_candidate = 0;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] is12simular final_candidate is %x\n", final_candidate));
				break;
			}
		}

		if (i == 2) {
			#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
			is13simular = phy_SimularityCompare_8197F(pAdapter, result, 0, 2);
			#else
			is13simular = phy_SimularityCompare_8197F(pDM_Odm, result, 0, 2);
			#endif

			if (is13simular) {
				final_candidate = 0;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] is13simular final_candidate is %x\n", final_candidate));

				break;
			}
			#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
			is23simular = phy_SimularityCompare_8197F(pAdapter, result, 1, 2);
			#else
			is23simular = phy_SimularityCompare_8197F(pDM_Odm, result, 1, 2);
			#endif

			if (is23simular) {
				final_candidate = 1;
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] is23simular final_candidate is %x\n", final_candidate));
			} else {
#if 0
								for (i = 0; i < 4; i++)
									RegTmp &= result[3][i*2];

								if (RegTmp != 0)
									final_candidate = 3;
								else
									final_candidate = 0xFF;
#endif
				final_candidate = 3;

			}
		}
	}
#if 0
	if ((result[final_candidate][0] | result[final_candidate][2] | result[final_candidate][4] | result[final_candidate][6]) == 0) {
		for (i = 0; i < 8; i++) {
			if ((i == 0) || (i == 2) || (i == 4)  || (i == 6))
				result[final_candidate][i] = 0x100;
			else
				result[final_candidate][i] = 0;
		}
	}
#endif

	for (i = 0; i < 4; i++) {
		RegE94 = result[i][0];
		RegE9C = result[i][1];
		RegEA4 = result[i][2];
		RegEAC = result[i][3];
		RegEB4 = result[i][4];
		RegEBC = result[i][5];
		RegEC4 = result[i][6];
		RegECC = result[i][7];
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] RegE94=%x RegE9C=%x RegEA4=%x RegEAC=%x RegEB4=%x RegEBC=%x RegEC4=%x RegECC=%x\n ", RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC));
	}

	if (final_candidate != 0xff) {
		priv->pshare->RegE94 = pDM_Odm->RFCalibrateInfo.RegE94 = RegE94 = result[final_candidate][0];
		priv->pshare->RegE9C = pDM_Odm->RFCalibrateInfo.RegE9C = RegE9C = result[final_candidate][1];
		RegEA4 = result[final_candidate][2];
		RegEAC = result[final_candidate][3];
		priv->pshare->RegEB4 = pDM_Odm->RFCalibrateInfo.RegEB4 = RegEB4 = result[final_candidate][4];
		priv->pshare->RegEBC = pDM_Odm->RFCalibrateInfo.RegEBC = RegEBC = result[final_candidate][5];
		RegEC4 = result[final_candidate][6];
		RegECC = result[final_candidate][7];
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] final_candidate is %x\n", final_candidate));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] TX0_X=%x TX0_Y=%x RX0_X=%x RX0_Y=%x TX1_X=%x TX1_Y=%x RX1_X=%x RX1_Y=%x\n ", RegE94, RegE9C, RegEA4, RegEAC, RegEB4, RegEBC, RegEC4, RegECC));
		bPathAOK = bPathBOK = TRUE;
	} else {
		/*ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] FAIL use default value\n"));*/

		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
		_phy_iqk_check_97f(pAdapter);
		#else
		_phy_iqk_check_97f(pDM_Odm);
		#endif
		
		pDM_Odm->RFCalibrateInfo.RegE94 = pDM_Odm->RFCalibrateInfo.RegEB4 = 0x100;	/*X default value*/
		pDM_Odm->RFCalibrateInfo.RegE9C = pDM_Odm->RFCalibrateInfo.RegEBC = 0x0;	/*Y default value*/
		priv->pshare->IQK_fail_cnt++;
	}

#if 0
	/*reset TXIQC for DPD*/
	ODM_SetBBReg(pDM_Odm, 0xe30, bMaskDWord, 0x10000000);
	ODM_SetBBReg(pDM_Odm, 0xe20, bMaskDWord, 0x00000000);
	ODM_SetBBReg(pDM_Odm, 0xe50, bMaskDWord, 0x10000000);
	ODM_SetBBReg(pDM_Odm, 0xe24, bMaskDWord, 0x00000000);
#endif

	if ((RegE94 != 0)/*&&(RegEA4 != 0)*/) {
		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
		_PHY_PathAFillIQKMatrix_97F(pAdapter, bPathAOK, result, final_candidate, (RegEA4 == 0));
		#else
		_PHY_PathAFillIQKMatrix_97F(pDM_Odm, bPathAOK, result, final_candidate, (RegEA4 == 0));
		#endif
	}

	if ((RegEB4 != 0)/*&&(RegEC4 != 0)*/) {
		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
		_PHY_PathBFillIQKMatrix_97F(pAdapter, bPathBOK, result, final_candidate, (RegEC4 == 0));
		#else
		_PHY_PathBFillIQKMatrix_97F(pDM_Odm, bPathBOK, result, final_candidate, (RegEC4 == 0));
		#endif
	}

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	Indexforchannel = ODM_GetRightChnlPlaceforIQK(pHalData->CurrentChannel);
	#else
	Indexforchannel = 0;
	#endif

/*To Fix BSOD when final_candidate is 0xff*/
/*by sherry 20120321*/
	if (final_candidate < 4) {
		for (i = 0; i < IQK_Matrix_REG_NUM; i++)
			pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[Indexforchannel].Value[0][i] = result[final_candidate][i];
		pDM_Odm->RFCalibrateInfo.IQKMatrixRegSetting[Indexforchannel].bIQKDone = TRUE;
	}
	/*RT_DISP(FINIT, INIT_IQK, ("\nIQK OK Indexforchannel %d.\n", Indexforchannel));*/
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] IQK OK Indexforchannel %d.\n", Indexforchannel));
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)

	_PHY_SaveADDARegisters_97F(pAdapter, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup_recover, 9);
	#else
	_PHY_SaveADDARegisters_97F(pDM_Odm, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup_recover, IQK_BB_REG_NUM);
	#endif

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xc80 = 0x%x, 0xc94 = 0x%x\n", ODM_GetBBReg(pDM_Odm, 0xc80, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xc94, bMaskDWord)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xe30 = 0x%x, 0xe20 = 0x%x\n", ODM_GetBBReg(pDM_Odm, 0xe30, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xe20, bMaskDWord)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xc14 = 0x%x, 0xca0 = 0x%x\n", ODM_GetBBReg(pDM_Odm, 0xc14, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xca0, bMaskDWord)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xc88 = 0x%x, 0xc9c = 0x%x\n", ODM_GetBBReg(pDM_Odm, 0xc88, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xc9c, bMaskDWord)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xe50 = 0x%x, 0xe24 = 0x%x\n", ODM_GetBBReg(pDM_Odm, 0xe50, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xe24, bMaskDWord)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0xc1c = 0x%x, 0xca8 = 0x%x\n", ODM_GetBBReg(pDM_Odm, 0xc1c, bMaskDWord), ODM_GetBBReg(pDM_Odm, 0xca8, bMaskDWord)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0x58 at Path A = 0x%x\n",
				 ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x58, bRFRegOffsetMask)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] 0x58 at Path B = 0x%x\n",
				 ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x58, bRFRegOffsetMask)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK] IQK finished\n"));

	/*trun on DPK*/
	phy_dpkon_8197f(pDM_Odm);

}


VOID
PHY_LCCalibrate_8197F(
	PVOID		pDM_VOID
)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	BOOLEAN			bStartContTx = FALSE, bSingleTone = FALSE, bCarrierSuppression = FALSE;
	u4Byte			timeout = 2000, timecount = 0;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	PADAPTER		pAdapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	#if (MP_DRIVER == 1)
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PMPT_CONTEXT	pMptCtx = &(pAdapter->MptCtx);
	#else/* (DM_ODM_SUPPORT_TYPE == ODM_CE)*/
	PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.MptCtx);
	#endif
	#endif/*(MP_DRIVER == 1)*/
	#endif




	#if MP_DRIVER == 1
	bStartContTx = pMptCtx->bStartContTx;
	bSingleTone = pMptCtx->bSingleTone;
	bCarrierSuppression = pMptCtx->bCarrierSuppression;
	#endif


	#ifdef DISABLE_BB_RF
	return;
	#endif

	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	if (!(pDM_Odm->SupportAbility & ODM_RF_CALIBRATION))
		return;
	#endif
	/* 20120213<Kordan> Turn on when continuous Tx to pass lab testing. (required by Edlu) */
	if (bSingleTone || bCarrierSuppression)
		return;

	while (*(pDM_Odm->pbScanInProcess) && timecount < timeout) {
		ODM_delay_ms(50);
		timecount += 50;
	}

	pDM_Odm->RFCalibrateInfo.bLCKInProgress = TRUE;

	/*ODM_RT_TRACE(pDM_Odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LCK:Start!!!interface %d currentband %x delay %d ms\n", pDM_Odm->interfaceIndex, pHalData->CurrentBandType92D, timecount));*/
	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)

	if (IS_2T2R(pHalData->VersionID))
		phy_LCCalibrate_8197F(pAdapter, TRUE);
	else
	#endif
	{
		/* For 88C 1T1R*/
		#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
		phy_LCCalibrate_8197F(pAdapter, FALSE);
		#else
		phy_LCCalibrate_8197F(pDM_Odm, FALSE);
		#endif
	}

	pDM_Odm->RFCalibrateInfo.bLCKInProgress = FALSE;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LCK:Finish!!!interface %d\n", pDM_Odm->InterfaceIndex));

}

VOID
phy_path_a_dpk_init_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	/**/
	ODM_SetBBReg(pDM_Odm, 0xb00, bMaskDWord, 0x0001e018);
	ODM_SetBBReg(pDM_Odm, 0xb04, bMaskDWord, 0xf76d9f84);
	ODM_SetBBReg(pDM_Odm, 0xb28, bMaskDWord, 0x000844aa);
	ODM_SetBBReg(pDM_Odm, 0xb68, bMaskDWord, 0x11160200);
														 
	ODM_SetBBReg(pDM_Odm, 0xb30, bMaskDWord, 0x0007bdef);
	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x40000000);
														 
	ODM_SetBBReg(pDM_Odm, 0xbc0, bMaskDWord, 0x00021d5f);
														 
	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x00000000);
														 													 
	ODM_SetBBReg(pDM_Odm, 0xb08, bMaskDWord, 0x41382e21);
	ODM_SetBBReg(pDM_Odm, 0xb0c, bMaskDWord, 0x5b554f48);
	ODM_SetBBReg(pDM_Odm, 0xb10, bMaskDWord, 0x6f6b6661);
	ODM_SetBBReg(pDM_Odm, 0xb14, bMaskDWord, 0x817d7874);
	ODM_SetBBReg(pDM_Odm, 0xb18, bMaskDWord, 0x908c8884);
	ODM_SetBBReg(pDM_Odm, 0xb1c, bMaskDWord, 0x9d9a9793);
	ODM_SetBBReg(pDM_Odm, 0xb20, bMaskDWord, 0xaaa7a4a1);
	ODM_SetBBReg(pDM_Odm, 0xb24, bMaskDWord, 0xb6b3b0ad);
														 													 
	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x40000000);														 
														 
	ODM_SetBBReg(pDM_Odm, 0xb00, bMaskDWord, 0x02ce03e8);
	ODM_SetBBReg(pDM_Odm, 0xb04, bMaskDWord, 0x01fd024c);
	ODM_SetBBReg(pDM_Odm, 0xb08, bMaskDWord, 0x01a101c9);
	ODM_SetBBReg(pDM_Odm, 0xb0C, bMaskDWord, 0x016a0183);
	ODM_SetBBReg(pDM_Odm, 0xb10, bMaskDWord, 0x01430153);
	ODM_SetBBReg(pDM_Odm, 0xb14, bMaskDWord, 0x01280134);
	ODM_SetBBReg(pDM_Odm, 0xb18, bMaskDWord, 0x0112011c);
	ODM_SetBBReg(pDM_Odm, 0xb1C, bMaskDWord, 0x01000107);
	ODM_SetBBReg(pDM_Odm, 0xb20, bMaskDWord, 0x00f200f9);
	ODM_SetBBReg(pDM_Odm, 0xb24, bMaskDWord, 0x00e500eb);
	ODM_SetBBReg(pDM_Odm, 0xb28, bMaskDWord, 0x00da00e0);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x00d200d6);
	ODM_SetBBReg(pDM_Odm, 0xb30, bMaskDWord, 0x00c900cd);
	ODM_SetBBReg(pDM_Odm, 0xb34, bMaskDWord, 0x00c200c5);
	ODM_SetBBReg(pDM_Odm, 0xb38, bMaskDWord, 0x00bb00be);
	ODM_SetBBReg(pDM_Odm, 0xb3c, bMaskDWord, 0x00b500b8);
														 
	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x00000000);
														 														 
	ODM_SetBBReg(pDM_Odm, 0xbac, bMaskDWord, 0x00000c00);
														 
	ODM_SetBBReg(pDM_Odm, 0xba8, bMaskDWord, 0x00000000);
														 
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x10000304);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x10010203);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x10020102);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x10030101);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x10040101);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x10050101);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x10060101);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x10070101);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x1008caff);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x100980a1);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x100a5165);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x100b3340);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x100c2028);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x100d1419);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x100e0810);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x100f0506);
														 
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x00000000);
														 														 
	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x00000000);
														 
	ODM_SetBBReg(pDM_Odm, 0xbac, bMaskDWord, 0x00000c00);
														 
	ODM_SetBBReg(pDM_Odm, 0xba8, bMaskDWord, 0x0000000d);
														 	
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01007800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01017800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01027800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01037800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01047800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01057800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01067800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01077800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01087800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01097800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x010A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x010B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x010C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x010D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x010E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x010F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01107800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01117800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01127800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01137800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01147800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01157800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01167800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01177800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01187800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01197800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x011A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x011B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x011C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x011D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x011E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x011F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01207800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01217800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01227800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01237800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01247800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01257800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01267800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01277800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01287800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01297800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x012A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x012B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x012C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x012D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x012E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x012F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01307800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01317800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01327800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01337800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01347800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01357800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01367800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01377800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01387800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x01397800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x013A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x013B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x013C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x013D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x013E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x013F7800);
															 
	ODM_SetBBReg(pDM_Odm, 0xba8, bMaskDWord, 0x0000000d);
														 
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02007800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02017800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02027800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02037800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02047800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02057800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02067800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02077800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02087800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02097800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x020A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x020B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x020C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x020D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x020E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x020F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02107800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02117800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02127800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02137800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02147800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02157800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02167800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02177800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02187800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02197800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x021A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x021B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x021C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x021D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x021E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x021F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02207800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02217800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02227800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02237800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02247800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02257800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02267800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02277800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02287800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02297800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x022A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x022B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x022C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x022D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x022E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x022F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02307800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02317800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02327800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02337800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02347800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02357800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02367800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02377800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02387800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x02397800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x023A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x023B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x023C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x023D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x023E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x023F7800);
															 
	ODM_SetBBReg(pDM_Odm, 0xba8, bMaskDWord, 0x00000000);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x00000000);

}

VOID
phy_path_b_dpk_init_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	/**/
	ODM_SetBBReg(pDM_Odm, 0xb70, bMaskDWord, 0x0001e018);
	ODM_SetBBReg(pDM_Odm, 0xb74, bMaskDWord, 0xf76d9f84);
	ODM_SetBBReg(pDM_Odm, 0xb98, bMaskDWord, 0x000844aa);
	ODM_SetBBReg(pDM_Odm, 0xb6c, bMaskDWord, 0x11160200);
														 
	ODM_SetBBReg(pDM_Odm, 0xba0, bMaskDWord, 0x0007bdef);
	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x40000000);
														 
	ODM_SetBBReg(pDM_Odm, 0xbc4, bMaskDWord, 0x00021d5f);
														 
	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x00000000);
														 													 
	ODM_SetBBReg(pDM_Odm, 0xb78, bMaskDWord, 0x41382e21);
	ODM_SetBBReg(pDM_Odm, 0xb7c, bMaskDWord, 0x5b554f48);
	ODM_SetBBReg(pDM_Odm, 0xb80, bMaskDWord, 0x6f6b6661);
	ODM_SetBBReg(pDM_Odm, 0xb84, bMaskDWord, 0x817d7874);
	ODM_SetBBReg(pDM_Odm, 0xb88, bMaskDWord, 0x908c8884);
	ODM_SetBBReg(pDM_Odm, 0xb8c, bMaskDWord, 0x9d9a9793);
	ODM_SetBBReg(pDM_Odm, 0xb90, bMaskDWord, 0xaaa7a4a1);
	ODM_SetBBReg(pDM_Odm, 0xb94, bMaskDWord, 0xb6b3b0ad);
														 													 
	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x40000000);														 
														 
	ODM_SetBBReg(pDM_Odm, 0xb60, bMaskDWord, 0x02ce03e8);
	ODM_SetBBReg(pDM_Odm, 0xb64, bMaskDWord, 0x01fd024c);
	ODM_SetBBReg(pDM_Odm, 0xb68, bMaskDWord, 0x01a101c9);
	ODM_SetBBReg(pDM_Odm, 0xb6C, bMaskDWord, 0x016a0183);
	ODM_SetBBReg(pDM_Odm, 0xb70, bMaskDWord, 0x01430153);
	ODM_SetBBReg(pDM_Odm, 0xb74, bMaskDWord, 0x01280134);
	ODM_SetBBReg(pDM_Odm, 0xb78, bMaskDWord, 0x0112011c);
	ODM_SetBBReg(pDM_Odm, 0xb7C, bMaskDWord, 0x01000107);
	ODM_SetBBReg(pDM_Odm, 0xb80, bMaskDWord, 0x00f200f9);
	ODM_SetBBReg(pDM_Odm, 0xb84, bMaskDWord, 0x00e500eb);
	ODM_SetBBReg(pDM_Odm, 0xb88, bMaskDWord, 0x00da00e0);
	ODM_SetBBReg(pDM_Odm, 0xb8c, bMaskDWord, 0x00d200d6);
	ODM_SetBBReg(pDM_Odm, 0xb90, bMaskDWord, 0x00c900cd);
	ODM_SetBBReg(pDM_Odm, 0xb94, bMaskDWord, 0x00c200c5);
	ODM_SetBBReg(pDM_Odm, 0xb98, bMaskDWord, 0x00bb00be);
	ODM_SetBBReg(pDM_Odm, 0xb9c, bMaskDWord, 0x00b500b8);
														 
	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x00000000);
														 														 
	ODM_SetBBReg(pDM_Odm, 0xbac, bMaskDWord, 0x00000c00);
														 
	ODM_SetBBReg(pDM_Odm, 0xba8, bMaskDWord, 0x00000000);
														 
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x20000304);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x20010203);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x20020102);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x20030101);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x20040101);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x20050101);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x20060101);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x20070101);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x2008caff);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x200980a1);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x200a5165);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x200b3340);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x200c2028);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x200d1419);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x200e0810);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x200f0506);
														 
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x00000000);
														 														 
	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x00000000);
														 
	ODM_SetBBReg(pDM_Odm, 0xbac, bMaskDWord, 0x00000c00);
														 
	ODM_SetBBReg(pDM_Odm, 0xba8, bMaskDWord, 0x0000000d);
														 	
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04007800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04017800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04027800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04037800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04047800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04057800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04067800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04077800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04087800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04097800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x040A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x040B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x040C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x040D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x040E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x040F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04107800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04117800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04127800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04137800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04147800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04157800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04167800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04177800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04187800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04197800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x041A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x041B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x041C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x041D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x041E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x041F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04207800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04217800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04227800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04237800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04247800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04257800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04267800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04277800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04287800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04297800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x042A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x042B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x042C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x042D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x042E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x042F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04307800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04317800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04327800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04337800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04347800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04357800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04367800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04377800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04387800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x04397800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x043A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x043B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x043C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x043D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x043E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x043F7800);
															 
	ODM_SetBBReg(pDM_Odm, 0xba8, bMaskDWord, 0x0000000d);
														 
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08007800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08017800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08027800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08037800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08047800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08057800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08067800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08077800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08087800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08097800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x080A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x080B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x080C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x080D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x080E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x080F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08107800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08117800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08127800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08137800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08147800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08157800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08167800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08177800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08187800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08197800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x081A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x081B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x081C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x081D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x081E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x081F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08207800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08217800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08227800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08237800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08247800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08257800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08267800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08277800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08287800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08297800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x082A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x082B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x082C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x082D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x082E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x082F7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08307800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08317800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08327800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08337800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08347800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08357800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08367800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08377800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08387800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x08397800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x083A7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x083B7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x083C7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x083D7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x083E7800);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x083F7800);
															 
	ODM_SetBBReg(pDM_Odm, 0xba8, bMaskDWord, 0x00000000);
	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x00000000);

}


VOID
phy_set_txrate_index_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	/*Path A*/
	ODM_SetBBReg(pDM_Odm, 0xe08, 0x00007F00, 0x30);
	ODM_SetBBReg(pDM_Odm, 0x86c, 0xFFFFFF00, 0x303030);
	ODM_SetBBReg(pDM_Odm, 0xe00, bMaskDWord, 0x30303030);
	ODM_SetBBReg(pDM_Odm, 0xe04, bMaskDWord, 0x30303030);
	ODM_SetBBReg(pDM_Odm, 0xe10, bMaskDWord, 0x30303030);
	ODM_SetBBReg(pDM_Odm, 0xe14, bMaskDWord, 0x30303030);

	/*Path B*/
	ODM_SetBBReg(pDM_Odm, 0x838, 0xFFFFFF00, 0x303030);
	ODM_SetBBReg(pDM_Odm, 0x86c, 0x000000FF, 0x30);
	ODM_SetBBReg(pDM_Odm, 0x830, bMaskDWord, 0x30303030);
	ODM_SetBBReg(pDM_Odm, 0x834, bMaskDWord, 0x30303030);
	ODM_SetBBReg(pDM_Odm, 0x83c, bMaskDWord, 0x30303030);
	ODM_SetBBReg(pDM_Odm, 0x848, bMaskDWord, 0x30303030);
	
}

u1Byte
phy_path_a_gainloss_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	int				k;
	u1Byte			TX_AGC_search = 0x0, result[5] = {0x0};

	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xae000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x00, bRFRegOffsetMask, 0x5801c);
	ODM_Write1Byte(pDM_Odm, 0xb00, 0x18);
	ODM_Write1Byte(pDM_Odm, 0xb07, 0xf7);
	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x00);
	ODM_SetBBReg(pDM_Odm, 0xb68, 0xffff0000, 0x159a);
	ODM_SetBBReg(pDM_Odm, 0xb08, bMaskDWord, 0x41382e21);
	ODM_Write1Byte(pDM_Odm, 0xbad, 0x2c);
	ODM_SetBBReg(pDM_Odm, 0xe34, bMaskDWord, 0x10000000);
	
	ODM_delay_ms(5);
	
	for (k = 0; k < 5; k++)
	{
		ODM_Write1Byte(pDM_Odm, 0xb2b, 0x80);
		ODM_Write1Byte(pDM_Odm, 0xb2b, 0x00);
		ODM_delay_ms(20);

		result[k] = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xbdc, 0x0000000f);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] result[%d] = 0x%x\n", k, result[k]));

		if ((k != 0) && (result[k] == result[k-1]))
			break;
	}
	
	if (k == 4)
		TX_AGC_search = ((result[0] + result[1] + result[2] + result[3] + result[4]) / 5);
	else
		TX_AGC_search = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xbdc, 0x0000000f);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] TXAGC_search_a = 0x%x\n", TX_AGC_search));
	return TX_AGC_search;
	
}

u1Byte
phy_path_b_gainloss_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	int				k;
	u1Byte			TX_AGC_search = 0x0, result[5] = {0x0};

	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xae000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x00, bRFRegOffsetMask, 0x5801c);
	ODM_Write1Byte(pDM_Odm, 0xb70, 0x18);
	ODM_Write1Byte(pDM_Odm, 0xb77, 0xf7);
	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x00);
	ODM_SetBBReg(pDM_Odm, 0xb6c, 0xffff0000, 0x159a);
	ODM_SetBBReg(pDM_Odm, 0xb78, bMaskDWord, 0x41382e21);
	ODM_Write1Byte(pDM_Odm, 0xbad, 0x0c);
	ODM_SetBBReg(pDM_Odm, 0xe54, bMaskDWord, 0x10000000);
	
	ODM_delay_ms(5);

	for (k = 0; k < 5; k++)
	{
		ODM_Write1Byte(pDM_Odm, 0xb9b, 0x80);
		ODM_Write1Byte(pDM_Odm, 0xb9b, 0x00);
		ODM_delay_ms(20);

		result[k] = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xbec, 0x0000000f);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] result[%d] = 0x%x\n", k, result[k]));

		if ((k != 0) && (result[k] == result[k-1]))
			break;
	}
	
	if (k == 4)
		TX_AGC_search = ((result[0] + result[1] + result[2] + result[3] + result[4]) / 5);
	else
		TX_AGC_search = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xbec, 0x0000000f);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] TXAGC_search_b = 0x%x\n", TX_AGC_search));
	return TX_AGC_search;
	
}


u4Byte
phy_path_a_dodpk_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID,
#else
		PADAPTER	pAdapter,
#endif
		u1Byte		TX_AGC_search,
		u1Byte		k
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	u4Byte			REG_B68, result;
	u1Byte			TX_AGC = 0x0;


	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x40);
	
	switch (TX_AGC_search)
	{
		case 0x0:
			REG_B68 = 0x11160200 | (0x19<<10);
			TX_AGC = 0x12;
			break;

		case 0x1:
			REG_B68 = 0x11160200 | (0x1a<<10);
			TX_AGC = 0x13;
			break;
			
		case 0x2:
			REG_B68 = 0x11160200 | (0x1b<<10);
			TX_AGC = 0x14;
			break;

		case 0x3:
			REG_B68 = 0x11160200 | (0x1c<<10);
			TX_AGC = 0x15;
			break;	

		case 0x4:
			REG_B68 = 0x11160200 | (0x1d<<10);
			TX_AGC = 0x16;
			break;
			
		case 0x5:
			REG_B68 = 0x11160200 | (0x1e<<10);
			TX_AGC = 0x17;
			break;	

		case 0x6:
			REG_B68 = 0x11160200 | (0x1f<<10);
			TX_AGC = 0x18;
			break;	

		case 0x7:
			REG_B68 = 0x11160200 | (0x00<<10);
			TX_AGC = 0x19;
			break;

		case 0x8:
			REG_B68 = 0x11160200 | (0x01<<10);
			TX_AGC = 0x1a;
			break;

		case 0x9:
			REG_B68 = 0x11160200 | (0x02<<10);
			TX_AGC = 0x1b;
			break;

		case 0xa:
			REG_B68 = 0x11160200 | (0x03<<10);
			TX_AGC = 0x1c;
			break;	
	
	}

	ODM_SetBBReg(pDM_Odm, 0xb68, bMaskDWord, REG_B68);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, 0xae000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x00, bRFRegOffsetMask, (0x58000 | TX_AGC));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] RF_a 0x0 = 0x%x\n", ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x00, bRFRegOffsetMask)));
	
	pDM_Odm->RFCalibrateInfo.pwsf_2g_a[k] = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xb68, 0x00007C00);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 0xb68[%d] = 0x%x, pwsf_2g_a[%d] = 0x%x\n", 
		k, ODM_GetBBReg(pDM_Odm, 0xb68, bMaskDWord), k, pDM_Odm->RFCalibrateInfo.pwsf_2g_a[k]));
	//ODM_delay_ms(5);	
	ODM_Write1Byte(pDM_Odm, 0xb2b, 0xc0);
	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x40);
	ODM_delay_ms(10);

	result = ODM_GetBBReg(pDM_Odm, 0xbd8, (BIT18));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] fail bit = %x\n", result));
	return result;
	
}

u4Byte
phy_path_b_dodpk_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID,
#else
		PADAPTER	pAdapter,
#endif
		u1Byte		TX_AGC_search,
		u1Byte		k
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	u4Byte			REG_B6C, result;
	u1Byte			TX_AGC = 0x0;


	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x40);
	
	switch (TX_AGC_search)
	{
		case 0x0:
			REG_B6C = 0x11160200 | (0x19<<10);
			TX_AGC = 0x12;
			break;

		case 0x1:
			REG_B6C = 0x11160200 | (0x1a<<10);
			TX_AGC = 0x13;
			break;
			
		case 0x2:
			REG_B6C = 0x11160200 | (0x1b<<10);
			TX_AGC = 0x14;
			break;

		case 0x3:
			REG_B6C = 0x11160200 | (0x1c<<10);
			TX_AGC = 0x15;
			break;	

		case 0x4:
			REG_B6C = 0x11160200 | (0x1d<<10);
			TX_AGC = 0x16;
			break;
			
		case 0x5:
			REG_B6C = 0x11160200 | (0x1e<<10);
			TX_AGC = 0x17;
			break;	

		case 0x6:
			REG_B6C = 0x11160200 | (0x1f<<10);
			TX_AGC = 0x18;
			break;	

		case 0x7:
			REG_B6C = 0x11160200 | (0x00<<10);
			TX_AGC = 0x19;
			break;

		case 0x8:
			REG_B6C = 0x11160200 | (0x01<<10);
			TX_AGC = 0x1a;
			break;

		case 0x9:
			REG_B6C = 0x11160200 | (0x02<<10);
			TX_AGC = 0x1b;
			break;

		case 0xa:
			REG_B6C = 0x11160200 | (0x03<<10);
			TX_AGC = 0x1c;
			break;	
	
	}

	ODM_SetBBReg(pDM_Odm, 0xb6c, bMaskDWord, REG_B6C);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, 0xae000);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x00, bRFRegOffsetMask, (0x58000 | TX_AGC));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] RF_b 0x0 = 0x%x\n", ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x00, bRFRegOffsetMask)));

	pDM_Odm->RFCalibrateInfo.pwsf_2g_b[k] = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xb6c, 0x00007C00);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 0xb6c[%d] = 0x%x, pwsf_2g_b[%d] = 0x%x\n", 
		k, ODM_GetBBReg(pDM_Odm, 0xb6c, bMaskDWord), k, pDM_Odm->RFCalibrateInfo.pwsf_2g_b[k]));
	//ODM_delay_ms(5);
	ODM_Write1Byte(pDM_Odm, 0xb9b, 0xc0);
	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x40);
	ODM_delay_ms(10);

	result = ODM_GetBBReg(pDM_Odm, 0xbd8, (BIT19));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] fail bit = %x\n", result));
	return result;
	
}

BOOLEAN
phy_path_a_iq_check_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID,
#else
		PADAPTER	pAdapter,
#endif
		u1Byte		group,
		u1Byte		addr
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
														 
	u4Byte		i_val, q_val;

	if(addr == 0) {
	
		i_val = ODM_GetBBReg(pDM_Odm, 0xbc0, 0x003FF800);
		q_val = ODM_GetBBReg(pDM_Odm, 0xbc0, 0x000007FF);

		if (((q_val & 0x400)>>10) == 1)
			q_val = 0x800 - q_val;

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] RF_a[%d] LMS check I=0x%x, Q=0x%x\n", group, i_val, q_val));

		if ((i_val*i_val + q_val*q_val) < 0x29526) {/* LMS (I^2 + Q^2) < -1.9dB happen*/
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] RF_a[%d] I^2 + Q^2 = 0x%x\n", group, (i_val*i_val + q_val*q_val)));
			return 1;
		} else {	
			return 0;
		}
	} else {
		return 0;
	}

}

BOOLEAN
phy_path_b_iq_check_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID,
#else
		PADAPTER	pAdapter,
#endif
		u1Byte		group,
		u1Byte		addr
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
														 
	u4Byte		i_val, q_val;

	if(addr == 0) {
	
		i_val = ODM_GetBBReg(pDM_Odm, 0xbc8, 0x003FF800);
		q_val = ODM_GetBBReg(pDM_Odm, 0xbc8, 0x000007FF);

		if (((q_val & 0x400)>>10) == 1)
			q_val = 0x800 - q_val;

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] RF_b[%d] LMS check I=0x%x, Q=0x%x\n", group, i_val, q_val));

		if ((i_val*i_val + q_val*q_val) < 0x29526) {/* LMS (I^2 + Q^2) < -2dB happen*/
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] RF_b[%d] I^2 + Q^2 = 0x%x\n", group, (i_val*i_val + q_val*q_val)));
			return 1;
		} else {	
			return 0;
		}
	} else {
		return 0;
	}

}


VOID
phy_path_a_pas_read_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID,
#else
		PADAPTER	pAdapter,
#endif
		BOOLEAN 	is_gainloss
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
														 
	int			k;
	u4Byte		reg_b00, reg_b68, reg_bdc, reg_be0, reg_be4, reg_be8;
	
	
	reg_b00 = ODM_GetBBReg(pDM_Odm, 0xb00, bMaskDWord);
	reg_b68 = ODM_GetBBReg(pDM_Odm, 0xb68, bMaskDWord);	

	if(is_gainloss) {
		ODM_SetBBReg(pDM_Odm, 0xb68, BIT26, 0x0);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 0xb68 = 0x%x\n", ODM_GetBBReg(pDM_Odm, 0xb68, bMaskDWord)));
	}
	
	for (k = 0; k < 8; k++) 
	{
	ODM_SetBBReg(pDM_Odm, 0xb00, bMaskDWord, (0x0101f038 | k));
	//ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 0xb00[%d] = 0x%x\n", k, ODM_GetBBReg(pDM_Odm, 0xb00, bMaskDWord)));
	reg_bdc = ODM_GetBBReg(pDM_Odm, 0xbdc, bMaskDWord);
	reg_be0 = ODM_GetBBReg(pDM_Odm, 0xbe0, bMaskDWord);
	reg_be4 = ODM_GetBBReg(pDM_Odm, 0xbe4, bMaskDWord);
	reg_be8 = ODM_GetBBReg(pDM_Odm, 0xbe8, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] PA scan_A[%d] = 0x%x\n", k, reg_bdc));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] PA scan_A[%d] = 0x%x\n", k, reg_be0));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] PA scan_A[%d] = 0x%x\n", k, reg_be4));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] PA scan_A[%d] = 0x%x\n", k, reg_be8));
	}

	ODM_SetBBReg(pDM_Odm, 0xb00, bMaskDWord, reg_b00);
	ODM_SetBBReg(pDM_Odm, 0xb68, bMaskDWord, reg_b68);
			
}

VOID
phy_path_b_pas_read_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID,
#else
		PADAPTER	pAdapter,
#endif
		BOOLEAN 	is_gainloss
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
														 
	int			k;
	u4Byte		reg_b70, reg_b6c, reg_bec, reg_bf0, reg_bf4, reg_bf8;
	
	reg_b70 = ODM_GetBBReg(pDM_Odm, 0xb70, bMaskDWord);
	reg_b6c = ODM_GetBBReg(pDM_Odm, 0xb6c, bMaskDWord);	


	if(is_gainloss) {
		ODM_SetBBReg(pDM_Odm, 0xb6c, BIT26, 0x0);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 0xb6c = 0x%x\n", ODM_GetBBReg(pDM_Odm, 0xb6c, bMaskDWord)));
	}	
	
	for (k = 0; k < 8; k++) 
	{
	ODM_SetBBReg(pDM_Odm, 0xb70, bMaskDWord, (0x0101f038 | k));
	//ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 0xb70[%d] = 0x%x\n", k, ODM_GetBBReg(pDM_Odm, 0xb70, bMaskDWord)));
	reg_bec = ODM_GetBBReg(pDM_Odm, 0xbec, bMaskDWord);
	reg_bf0 = ODM_GetBBReg(pDM_Odm, 0xbf0, bMaskDWord);
	reg_bf4 = ODM_GetBBReg(pDM_Odm, 0xbf4, bMaskDWord);
	reg_bf8 = ODM_GetBBReg(pDM_Odm, 0xbf8, bMaskDWord);
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] PA scan_B[%d] = 0x%x\n", k, reg_bec));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] PA scan_B[%d] = 0x%x\n", k, reg_bf0));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] PA scan_B[%d] = 0x%x\n", k, reg_bf4));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] PA scan_B[%d] = 0x%x\n", k, reg_bf8));
	}

	ODM_SetBBReg(pDM_Odm, 0xb70, bMaskDWord, reg_b70);
	ODM_SetBBReg(pDM_Odm, 0xb6c, bMaskDWord, reg_b6c);
			
}

VOID
phy_path_a_dpk_enable_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	ODM_Write1Byte(pDM_Odm, 0xb00, 0x98);
	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x00);
	ODM_Write1Byte(pDM_Odm, 0xb6a, 0x26);

	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x80);
	ODM_Write1Byte(pDM_Odm, 0xb2b, 0x00);
	ODM_delay_ms(10);

	ODM_Write1Byte(pDM_Odm, 0xb07, 0x77);
	ODM_SetBBReg(pDM_Odm, 0xb08, bMaskDWord, 0x4c104c10);  /*1.5dB*/

	/*ODM_SetBBReg(pDM_Odm, 0xe30, bMaskDWord, 0x10000000);*/
	/*ODM_SetBBReg(pDM_Odm, 0xe20, bMaskDWord, 0x00000000);*/
	
}

VOID
phy_path_b_dpk_enable_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	ODM_Write1Byte(pDM_Odm, 0xb70, 0x98);
	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x00);
	ODM_Write1Byte(pDM_Odm, 0xb6e, 0x26);

	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x80);
	ODM_Write1Byte(pDM_Odm, 0xb9b, 0x00);
	ODM_delay_ms(10);

	ODM_Write1Byte(pDM_Odm, 0xb77, 0x77);
	ODM_SetBBReg(pDM_Odm, 0xb78, bMaskDWord, 0x4c104c10);  /*1.5dB*/

	/*ODM_SetBBReg(pDM_Odm, 0xe30, bMaskDWord, 0x10000000);*/
	/*ODM_SetBBReg(pDM_Odm, 0xe20, bMaskDWord, 0x00000000);*/
	
}

u1Byte
phy_dpk_channel_transfer_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	u1Byte		channel, bandwidth, i;

	channel = *pDM_Odm->pChannel;
	bandwidth = *pDM_Odm->pBandWidth;

	if (channel <= 4)
		i = 0;
	else if (channel >= 5 && channel <= 8)
		i = 1;
	else if (channel >= 9)
		i = 2;


	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] channel = %d, bandwidth = %d, transfer idx = %d\n", 
		channel, bandwidth, i));
	
	return i;

}


u1Byte
phy_lut_sram_read_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID,
#else
		PADAPTER	pAdapter,
#endif
		u1Byte		k
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
														 
	u1Byte		addr, i;
	u4Byte		regb2c;

	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x00000000);
	ODM_SetBBReg(pDM_Odm, 0xbac, bMaskDWord, 0x00000c00);

	for(addr = 0; addr < 64; addr++)
	{
		regb2c = (0x00801234 | (addr << 16));
		ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, regb2c);
		//ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_read 0xb2c = 0x%x\n", regb2c));

		/*A even*/
		pDM_Odm->RFCalibrateInfo.lut_2g_even_a[k][addr] = ODM_GetBBReg(pDM_Odm, 0xbc0, 0x003FFFFF);
		if (pDM_Odm->RFCalibrateInfo.lut_2g_even_a[k][addr] > 0x100000) {
			//printk("[DPK] 8197F LUT value > 0x100000, please check path A[%d]\n", k);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 8197F LUT value > 0x100000, please check path A[%d]\n", k));
			return 0;
		}
		if (phy_path_a_iq_check_8197f(pDM_Odm, k, addr)) {
			//printk("[DPK] 8197F LUT IQ value < -1.9dB, please check path A[%d]\n", k);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 8197F LUT IQ value < -1.9dB, please check path A[%d]\n", k));
			return 0;
		}

		/*A odd*/
		pDM_Odm->RFCalibrateInfo.lut_2g_odd_a[k][addr] = ODM_GetBBReg(pDM_Odm, 0xbc4, 0x003FFFFF);
		if (pDM_Odm->RFCalibrateInfo.lut_2g_odd_a[k][addr] > 0x100000) {
			//printk("[DPK] 8197F LUT value > 0x100000, please check path A[%d]\n", k);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 8197F LUT value > 0x100000, please check path A[%d]\n", k));
			return 0;
		}

		/*B even*/
		pDM_Odm->RFCalibrateInfo.lut_2g_even_b[k][addr] = ODM_GetBBReg(pDM_Odm, 0xbc8, 0x003FFFFF);
		if (pDM_Odm->RFCalibrateInfo.lut_2g_even_b[k][addr] > 0x100000) {
			//printk("[DPK] 8197F LUT value > 0x100000, please check path B[%d]\n", k);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 8197F LUT value > 0x100000, please check path B[%d]\n", k));
			return 0;
		}

		if (phy_path_b_iq_check_8197f(pDM_Odm, k, addr)) {
			//printk("[DPK] 8197F LUT IQ value < -1.9dB, please check path B[%d]\n", k);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 8197F LUT IQ value < -1.9dB, please check path B[%d]\n", k));
			return 0;
		}

		/*B odd*/
		pDM_Odm->RFCalibrateInfo.lut_2g_odd_b[k][addr] = ODM_GetBBReg(pDM_Odm, 0xbcc, 0x003FFFFF);
		if (pDM_Odm->RFCalibrateInfo.lut_2g_odd_b[k][addr] > 0x100000) {
			//printk("[DPK] 8197F LUT value > 0x100000, please check path B[%d]\n", k);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] 8197F LUT value > 0x100000, please check path B[%d]\n", k));
			return 0;
		}
	}

	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x00000000);

#if (DPK_SRAM_read_DBG_8197F)

	if (k == 2)
	{
		for (i = 0; i < 3; i++)
		{
			for(addr = 0; addr < 16; addr++)		
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_read_2G_A even 0xbc0[%d][%2d] = 0x%x\n", i, addr, pDM_Odm->RFCalibrateInfo.lut_2g_even_a[i][addr]));

			for(addr = 0; addr < 16; addr++)
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_read_2G_A  odd 0xbc4[%d][%2d] = 0x%x\n", i, addr, pDM_Odm->RFCalibrateInfo.lut_2g_odd_a[i][addr]));

			for(addr = 0; addr < 16; addr++)
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_read_2G_B even 0xbc8[%d][%2d] = 0x%x\n", i, addr, pDM_Odm->RFCalibrateInfo.lut_2g_even_b[i][addr]));

			for(addr = 0; addr < 16; addr++)
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_read_2G_B  odd 0xbcc[%d][%2d] = 0x%x\n", i, addr, pDM_Odm->RFCalibrateInfo.lut_2g_odd_b[i][addr]));
		}
	}
#endif

#if 0
	if (k == 2)
	{
		printk("bDPPathAOK=%d   bDPPathBOK=%d\n", pDM_Odm->RFCalibrateInfo.bDPPathAOK, pDM_Odm->RFCalibrateInfo.bDPPathBOK);
		u4Byte j;
		
		//for (i = 0; i < 3; i++)
		{
			printk("sram_read pwsf_2g_a[%d]=0x%x   pwsf_2g_b[%d]=0x%x\n", k, pDM_Odm->RFCalibrateInfo.pwsf_2g_a[k], k, pDM_Odm->RFCalibrateInfo.pwsf_2g_b[k]);	
		}

		//for (i = 0; i < 3; i++)
		{	
			for (j = 0; j < 64; j++)
				printk("sram_read lut_2g_even_a[%2d][%2d]=0x%x\n", k, j, pDM_Odm->RFCalibrateInfo.lut_2g_even_a[k][j]);

			for (j = 0; j < 64; j++)
				printk("sram_read  lut_2g_odd_a[%2d][%2d]=0x%x\n", k, j, pDM_Odm->RFCalibrateInfo.lut_2g_odd_a[k][j]);

			for (j = 0; j < 64; j++)
				printk("sram_read lut_2g_even_b[%2d][%2d]=0x%x\n", k, j, pDM_Odm->RFCalibrateInfo.lut_2g_even_b[k][j]);
				
			for (j = 0; j < 64; j++)
				printk("sram_read  lut_2g_odd_b[%2d][%2d]=0x%x\n", k, j, pDM_Odm->RFCalibrateInfo.lut_2g_odd_b[k][j]);	
		}
	}
#endif

return 1;

}

VOID
phy_lut_sram_write_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID
#else
		PADAPTER	pAdapter
#endif
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif
														 
	u1Byte		addr, k;
	u4Byte		regba8_a_even[64] = {0}, regb2c_a_even[64] = {0};
	u4Byte		regba8_a_odd[64] = {0}, regb2c_a_odd[64] = {0};
	u4Byte		regba8_b_even[64] = {0}, regb2c_b_even[64] = {0};
	u4Byte		regba8_b_odd[64] = {0}, regb2c_b_odd[64] = {0};


	k = phy_dpk_channel_transfer_8197f(pDM_Odm);

#if 0
{
		printk("bDPPathAOK=%d   bDPPathBOK=%d\n", pDM_Odm->RFCalibrateInfo.bDPPathAOK, pDM_Odm->RFCalibrateInfo.bDPPathBOK);
		u4Byte j;
		
		//for (i = 0; i < 3; i++)
		{
			printk("sram_write pwsf_2g_a[%d]=0x%x   pwsf_2g_b[%d]=0x%x\n", k, pDM_Odm->RFCalibrateInfo.pwsf_2g_a[k], k, pDM_Odm->RFCalibrateInfo.pwsf_2g_b[k]);	
		}

		//for (i = 0; i < 3; i++)
		{	
			for (j = 0; j < 64; j++)
				printk("sram_write lut_2g_even_a[%2d][%2d]=0x%x\n", k, j, pDM_Odm->RFCalibrateInfo.lut_2g_even_a[k][j]);

			for (j = 0; j < 64; j++)
				printk("sram_write  lut_2g_odd_a[%2d][%2d]=0x%x\n", k, j, pDM_Odm->RFCalibrateInfo.lut_2g_odd_a[k][j]);

			for (j = 0; j < 64; j++)
				printk("sram_write lut_2g_even_b[%2d][%2d]=0x%x\n", k, j, pDM_Odm->RFCalibrateInfo.lut_2g_even_b[k][j]);
				
			for (j = 0; j < 64; j++)
				printk("sram_write  lut_2g_odd_b[%2d][%2d]=0x%x\n", k, j, pDM_Odm->RFCalibrateInfo.lut_2g_odd_b[k][j]);	
		}
}
#endif


	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x00000000);
	ODM_SetBBReg(pDM_Odm, 0xbac, bMaskDWord, 0x00000c00);

	for(addr = 0; addr < 64; addr++)
	{
		/*A even*/
		ODM_SetBBReg(pDM_Odm, 0xb2c, 0x0F000000, 0x1);
		regba8_a_even[addr] = (pDM_Odm->RFCalibrateInfo.lut_2g_even_a[k][addr] & 0x003F0000)>>16;
		regb2c_a_even[addr] = pDM_Odm->RFCalibrateInfo.lut_2g_even_a[k][addr] & 0x0000FFFF;

		ODM_SetBBReg(pDM_Odm, 0xba8, 0x0000003F, regba8_a_even[addr]);
		ODM_SetBBReg(pDM_Odm, 0xb2c, 0x003FFFFF, (regb2c_a_even[addr] | (addr<<16)));
		
#if (DPK_SRAM_write_DBG_8197F)
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_write_2G_A even 0xba8[%2d] = 0x%x\n", addr, ODM_GetBBReg(pDM_Odm, 0xba8, bMaskDWord)));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_write_2G_A even 0xb2c[%2d] = 0x%x\n", addr, ODM_GetBBReg(pDM_Odm, 0xb2c, bMaskDWord)));
#endif
	}

	for(addr = 0; addr < 64; addr++)
	{
		/*A odd*/
		ODM_SetBBReg(pDM_Odm, 0xb2c, 0x0F000000, 0x2);
		regba8_a_odd[addr] = (pDM_Odm->RFCalibrateInfo.lut_2g_odd_a[k][addr] & 0x003F0000)>>16;
		regb2c_a_odd[addr] = pDM_Odm->RFCalibrateInfo.lut_2g_odd_a[k][addr] & 0x0000FFFF;

		ODM_SetBBReg(pDM_Odm, 0xba8, 0x0000003F, regba8_a_odd[addr]);
		ODM_SetBBReg(pDM_Odm, 0xb2c, 0x003FFFFF, (regb2c_a_odd[addr] | (addr<<16)));

#if (DPK_SRAM_write_DBG_8197F)
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_write_2G_A  odd 0xba8[%2d] = 0x%x\n", addr, ODM_GetBBReg(pDM_Odm, 0xba8, bMaskDWord)));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_write_2G_A  odd 0xb2c[%2d] = 0x%x\n", addr, ODM_GetBBReg(pDM_Odm, 0xb2c, bMaskDWord)));
#endif
	}

	for(addr = 0; addr < 64; addr++)
	{
		/*B even*/
		ODM_SetBBReg(pDM_Odm, 0xb2c, 0x0F000000, 0x4);
		regba8_b_even[addr] = (pDM_Odm->RFCalibrateInfo.lut_2g_even_b[k][addr] & 0x003F0000)>>16;
		regb2c_b_even[addr] = pDM_Odm->RFCalibrateInfo.lut_2g_even_b[k][addr] & 0x0000FFFF;

		ODM_SetBBReg(pDM_Odm, 0xba8, 0x0000003F, regba8_b_even[addr]);
		ODM_SetBBReg(pDM_Odm, 0xb2c, 0x003FFFFF, (regb2c_b_even[addr] | (addr<<16)));

#if (DPK_SRAM_write_DBG_8197F)
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_write_2G_B even 0xba8[%2d] = 0x%x\n", addr, ODM_GetBBReg(pDM_Odm, 0xba8, bMaskDWord)));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_write_2G_B even 0xb2c[%2d] = 0x%x\n", addr, ODM_GetBBReg(pDM_Odm, 0xb2c, bMaskDWord)));
#endif
	}

	for(addr = 0; addr < 64; addr++)
	{
		/*B odd*/
		ODM_SetBBReg(pDM_Odm, 0xb2c, 0x0F000000, 0x8);
		regba8_b_odd[addr] = (pDM_Odm->RFCalibrateInfo.lut_2g_odd_b[k][addr] & 0x003F0000)>>16;
		regb2c_b_odd[addr] = pDM_Odm->RFCalibrateInfo.lut_2g_odd_b[k][addr] & 0x0000FFFF;

		ODM_SetBBReg(pDM_Odm, 0xba8, 0x0000003F, regba8_b_odd[addr]);
		ODM_SetBBReg(pDM_Odm, 0xb2c, 0x003FFFFF, (regb2c_b_odd[addr] | (addr<<16)));

#if (DPK_SRAM_write_DBG_8197F)
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_write_2G_B  odd 0xba8[%2d] = 0x%x\n", addr, ODM_GetBBReg(pDM_Odm, 0xba8, bMaskDWord)));
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] LUT_SRAM_write_2G_B  odd 0xb2c[%2d] = 0x%x\n", addr, ODM_GetBBReg(pDM_Odm, 0xb2c, bMaskDWord)));
#endif
	}

	ODM_SetBBReg(pDM_Odm, 0xb2c, bMaskDWord, 0x00000000);

	ODM_SetBBReg(pDM_Odm, 0xb68, bMaskDWord, (0x11260200 | (pDM_Odm->RFCalibrateInfo.pwsf_2g_a[k]<<10)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] init pwsf_a[%d] = 0x%x\n", k, ODM_GetBBReg(pDM_Odm, 0xb68, 0x00007c00)));
	
	ODM_SetBBReg(pDM_Odm, 0xb6c, bMaskDWord, (0x11260200 | (pDM_Odm->RFCalibrateInfo.pwsf_2g_b[k]<<10)));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] init pwsf_b[%d] = 0x%x\n", k, ODM_GetBBReg(pDM_Odm, 0xb6c, 0x00007c00)));

	
}

VOID
phy_dpk_enable_disable_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
		PVOID		pDM_VOID,
#else
		PADAPTER	pAdapter,
#endif
		BOOLEAN		enable
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	if (enable) { /*use dpk result*/
		ODM_SetBBReg(pDM_Odm, 0xb68, BIT29, 0x0);
		ODM_SetBBReg(pDM_Odm, 0xb6c, BIT29, 0x0);
		pDM_Odm->RFCalibrateInfo.bDPPathAOK = 1;
		pDM_Odm->RFCalibrateInfo.bDPPathBOK = 1;
	} else { /*bypass dpk result*/
		ODM_SetBBReg(pDM_Odm, 0xb68, BIT29, 0x1);
		ODM_SetBBReg(pDM_Odm, 0xb6c, BIT29, 0x1);
		pDM_Odm->RFCalibrateInfo.bDPPathAOK = 0;
		pDM_Odm->RFCalibrateInfo.bDPPathBOK = 0;
	}
}

u1Byte
phy_dpcalibrate_8197f(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	PVOID		pDM_VOID,
#else
	PADAPTER	pAdapter,
#endif
	BOOLEAN		is2T,
	u1Byte		k
	
)
{
	PDM_ODM_T		pDM_Odm	= (PDM_ODM_T)pDM_VOID;

	#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
	PDM_ODM_T		pDM_Odm = &pHalData->odmpriv;
	#endif
	#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	PDM_ODM_T		pDM_Odm = &pHalData->DM_OutSrc;
	#endif
	#endif

	u4Byte			ADDA_REG[2] = {
		0xd94, rRx_Wait_CCA		
	};
	u4Byte			IQK_MAC_REG[IQK_MAC_REG_NUM] = {
		REG_TXPAUSE,	REG_BCN_CTRL,
		REG_BCN_CTRL_1,	REG_GPIO_MUXCFG
	};

	u4Byte			IQK_BB_REG_92C[9] = {
		rOFDM0_TRxPathEnable,	rOFDM0_TRMuxPar,
		rFPGA0_XCD_RFInterfaceSW, 0x88c, 0xc5c,
		0xc14, 0xc1c, 0x880, 0x884
	};

	u4Byte			Txrate_REG[11] = {
		0xe08, 0x86c, 0xe00, 0xe04, 0xe10, 0xe14,
		0x838, 0x830, 0x834, 0x83c, 0x848
	};	

	u4Byte			result_a = 0x1, result_b = 0x1, tmp_rf_a_00, tmp_rf_a_8f, tmp_rf_b_00, tmp_rf_b_8f;
	u1Byte			TXAGC_a_search = 0x0, TXAGC_b_search = 0x0;

	if (!((pDM_Odm->RFEType == 0) || (pDM_Odm->RFEType == 2)))
	{
		printk("[DPK] Skip DPK due to RFE type != 0 or 2\n"); 
		return 3;
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] DPK Start!!!!\n"));

	_PHY_SaveADDARegisters_97F(pDM_Odm, ADDA_REG, pDM_Odm->RFCalibrateInfo.ADDA_backup, 2);
	_PHY_SaveMACRegisters_97F(pDM_Odm, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);
	_PHY_SaveADDARegisters_97F(pDM_Odm, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup, 9);
	_PHY_SaveADDARegisters_97F(pDM_Odm, Txrate_REG, pDM_Odm->RFCalibrateInfo.Txrate, 11);

	tmp_rf_a_00 = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x00, bRFRegOffsetMask);
	tmp_rf_a_8f = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask);
	tmp_rf_b_00 = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x00, bRFRegOffsetMask);
	tmp_rf_b_8f = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask);
	
	ODM_SetBBReg(pDM_Odm, 0x520, bMaskByte2, 0xff); /*Tx Pause*/

	phy_path_a_dpk_init_8197f(pDM_Odm);
	phy_path_b_dpk_init_8197f(pDM_Odm);

	phy_set_txrate_index_8197f(pDM_Odm);

	ODM_SetBBReg(pDM_Odm, 0xe28, bMaskDWord, 0x00000000);
	ODM_SetBBReg(pDM_Odm, 0x88c, bMaskDWord, 0xccf008c0);
	ODM_SetBBReg(pDM_Odm, 0xc5c, bMaskDWord, 0x80708492);
	ODM_SetBBReg(pDM_Odm, 0xd94, bMaskDWord, 0x44FFBB44);
	ODM_SetBBReg(pDM_Odm, 0xe70, bMaskDWord, 0x00400040);
	ODM_SetBBReg(pDM_Odm, 0x87c, bMaskDWord, 0x004f0201);
	ODM_SetBBReg(pDM_Odm, 0x884, bMaskDWord, 0xc0000120);
	ODM_SetBBReg(pDM_Odm, 0x880, bMaskDWord, 0xd8001402);	
	ODM_SetBBReg(pDM_Odm, 0xc04, bMaskDWord, 0x6f005433);
	ODM_SetBBReg(pDM_Odm, 0xc08, bMaskDWord, 0x000804e4); 
	ODM_SetBBReg(pDM_Odm, 0x874, bMaskDWord, 0x25204000);	 
	ODM_SetBBReg(pDM_Odm, 0xc14, bMaskDWord, 0x00000000);
	ODM_SetBBReg(pDM_Odm, 0xc1c, bMaskDWord, 0x00000000);

#if 1
	/*path A DPK*/	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] ============ Path A[%d] DPK ============\n", k));
	
	TXAGC_a_search = phy_path_a_gainloss_8197f(pDM_Odm);

	if (DPK_PAS_DBG_8197F)
		phy_path_a_pas_read_8197f(pDM_Odm, TRUE);

	ODM_delay_ms(5);

	result_a = phy_path_a_dodpk_8197f(pDM_Odm, TXAGC_a_search, k);

	if (DPK_PAS_DBG_8197F)
		phy_path_a_pas_read_8197f(pDM_Odm, FALSE);
	
#endif
	
#if 1
	/*path B DPK*/	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] ============ Path B[%d] DPK ============\n", k));
	
	TXAGC_b_search = phy_path_b_gainloss_8197f(pDM_Odm);

	if (DPK_PAS_DBG_8197F)
		phy_path_b_pas_read_8197f(pDM_Odm, TRUE);

	ODM_delay_ms(5);
	
	result_b = phy_path_b_dodpk_8197f(pDM_Odm, TXAGC_b_search, k);

	if (DPK_PAS_DBG_8197F)
		phy_path_b_pas_read_8197f(pDM_Odm, FALSE);
	
#endif	

	phy_path_a_dpk_enable_8197f(pDM_Odm);
	phy_path_b_dpk_enable_8197f(pDM_Odm);

	if (result_a != 0)
	{
		ODM_SetBBReg(pDM_Odm, 0xb68, BIT29, 0x1);	/*bypass path_A DPD*/
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] Bypass Path_A[%d] DPD!!!!\n", k));
		pDM_Odm->RFCalibrateInfo.DPPathAResult[k] = 0;
		pDM_Odm->RFCalibrateInfo.bDPPathAOK = 0;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] DPK PathA need check k=%d\n", k));
		//printk("[DPK] DPK PathA need check k=%d\n", k);
	} else {
		pDM_Odm->RFCalibrateInfo.DPPathAResult[k] = 1;

		if (k == 2
			&& pDM_Odm->RFCalibrateInfo.DPPathAResult[0] == 1
			&& pDM_Odm->RFCalibrateInfo.DPPathAResult[1] == 1
			&& pDM_Odm->RFCalibrateInfo.DPPathAResult[2] == 1)
			pDM_Odm->RFCalibrateInfo.bDPPathAOK = 0x1;
	}

	if (result_b != 0)
	{
		ODM_SetBBReg(pDM_Odm, 0xb6c, BIT29, 0x1);	/*bypass path_B DPD*/
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] Bypass Path_B DPD!!!!\n"));
		pDM_Odm->RFCalibrateInfo.DPPathBResult[k] = 0;
		pDM_Odm->RFCalibrateInfo.bDPPathBOK = 0;
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] DPK PathB need check k=%d\n", k));
		//printk("[DPK] DPK PathB need check k=%d\n", k);

	} else {
		pDM_Odm->RFCalibrateInfo.DPPathBResult[k] = 1;

		if (k == 2
			&& pDM_Odm->RFCalibrateInfo.DPPathBResult[0] == 1
			&& pDM_Odm->RFCalibrateInfo.DPPathBResult[1] == 1
			&& pDM_Odm->RFCalibrateInfo.DPPathBResult[2] == 1)
			pDM_Odm->RFCalibrateInfo.bDPPathBOK = 0x1;
	}

	_PHY_ReloadADDARegisters_97F(pDM_Odm, ADDA_REG, pDM_Odm->RFCalibrateInfo.ADDA_backup, 2);
	_PHY_ReloadMACRegisters_97F(pDM_Odm, IQK_MAC_REG, pDM_Odm->RFCalibrateInfo.IQK_MAC_backup);
	_PHY_ReloadADDARegisters_97F(pDM_Odm, IQK_BB_REG_92C, pDM_Odm->RFCalibrateInfo.IQK_BB_backup, 9);
	_PHY_ReloadADDARegisters_97F(pDM_Odm, Txrate_REG, pDM_Odm->RFCalibrateInfo.Txrate, 11);

	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x00, bRFRegOffsetMask, tmp_rf_a_00);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x8f, bRFRegOffsetMask, tmp_rf_a_8f);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x00, bRFRegOffsetMask, tmp_rf_b_00);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, 0x8f, bRFRegOffsetMask, tmp_rf_b_8f);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[DPK] DPK Finish!!!!\n"));
	
#if 1

	if((result_a == 0) && (result_b == 0))
	{
		if (phy_lut_sram_read_8197f(pDM_Odm, k))
			return 1;
		else {
			pDM_Odm->RFCalibrateInfo.DPPathAResult[k] = 0;
			pDM_Odm->RFCalibrateInfo.DPPathBResult[k] = 0;
			return 0;
		}
	} else {
		pDM_Odm->RFCalibrateInfo.DPPathAResult[k] = 0;
		pDM_Odm->RFCalibrateInfo.DPPathBResult[k] = 0;
		return 0;
	}

#endif
}


