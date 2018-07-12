
#include "mp_precomp.h"
#include "../phydm_precomp.h"

#if (RTL8821C_SUPPORT == 1)


/*---------------------------Define Local Constant---------------------------*/


/*---------------------------Define Local Constant---------------------------*/


#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
void DoIQK_8821C(
	PVOID		pDM_VOID,
	u1Byte		DeltaThermalIndex,
	u1Byte		ThermalValue,	
	u1Byte		Threshold
	)
{
	PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;

	PADAPTER		Adapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	ODM_ResetIQKResult(pDM_Odm);		
	pDM_Odm->RFCalibrateInfo.ThermalValue_IQK = ThermalValue;
	PHY_IQCalibrate_8821C(pDM_Odm, TRUE);
}
#else
/*Originally pConfig->DoIQK is hooked PHY_IQCalibrate_8821C, but DoIQK_8821C and PHY_IQCalibrate_8821C have different arguments*/
void DoIQK_8821C(
	PVOID		pDM_VOID,
	u1Byte	DeltaThermalIndex,
	u1Byte	ThermalValue,	
	u1Byte	Threshold
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	BOOLEAN		bReCovery = (BOOLEAN) DeltaThermalIndex;

	PHY_IQCalibrate_8821C(pDM_Odm, TRUE);
}
#endif


VOID 
_IQK_BackupMacBB_8821C(
	IN PDM_ODM_T	pDM_Odm,
	IN pu4Byte		MAC_backup,
	IN pu4Byte		BB_backup,
	IN pu4Byte		Backup_MAC_REG,
	IN pu4Byte		Backup_BB_REG
	)
{
	u4Byte i;

	for (i = 0; i < MAC_REG_NUM_8821C; i++)
		MAC_backup[i] = ODM_Read4Byte(pDM_Odm, Backup_MAC_REG[i]);

	for (i = 0; i < BB_REG_NUM_8821C; i++)
		BB_backup[i] = ODM_Read4Byte(pDM_Odm, Backup_BB_REG[i]);
	
/*	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]BackupMacBB Success!!!!\n")); */
}


VOID
_IQK_BackupRF_8821C(
	IN PDM_ODM_T	pDM_Odm,
	IN u4Byte		RF_backup[][NUM_8821C],
	IN pu4Byte		Backup_RF_REG
	)	
{
	u4Byte i, j;

	for (i = 0; i < RF_REG_NUM_8821C; i++)
				for (j = 0; j < NUM_8821C; j++)
					RF_backup[i][j] = ODM_GetRFReg(pDM_Odm, j, Backup_RF_REG[i], bRFRegOffsetMask);
/*	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]BackupRF Success!!!!\n")); */

}

VOID
_IQK_AGCbnd_int_8821C(
	IN PDM_ODM_T	pDM_Odm
	)
{
	/*initialize RX AGC bnd, it must do after bbreset*/
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008);
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf80a7008);
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8015008);
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008);
	/*ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, ("[IQK]init. rx agc bnd\n"));*/
}


VOID
_IQK_BB_Reset_8821C(
	IN PDM_ODM_T	pDM_Odm
	)
{
	BOOLEAN		CCAing = FALSE;
	u4Byte		count = 0;		

	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x0, bRFRegOffsetMask, 0x10000);

	while (1) {
		CCAing = (BOOLEAN) ODM_GetBBReg(pDM_Odm, 0x660, BIT(16));

		if (count > 30)
			CCAing = FALSE;

		if (CCAing) {
			ODM_delay_ms(1);
			count++;
		} else {
			ODM_Write1Byte(pDM_Odm, 0x808, 0x0);	/*RX ant off*/
			ODM_SetBBReg(pDM_Odm, 0xa04, BIT(27)|BIT(26)|BIT(25)|BIT(24), 0x0);		/*CCK RX Path off*/

			/*BBreset*/
			ODM_SetBBReg(pDM_Odm, 0x0, BIT(16), 0x0);
			ODM_SetBBReg(pDM_Odm, 0x0, BIT(16), 0x1);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]BBreset!!!!\n"));
			break;
		}
	}
}

VOID
_IQK_AFESetting_8821C(
	IN PDM_ODM_T	pDM_Odm,
	IN BOOLEAN		Do_IQK
	)
{
	if (Do_IQK) {
		/*IQK AFE Setting RX_WAIT_CCA mode */ 
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x50000000); 
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x700F0040);

		 
		/*AFE setting*/
		ODM_Write4Byte(pDM_Odm, 0xc58, 0xd8000402);
		ODM_Write4Byte(pDM_Odm, 0xc5c, 0xd1000120);
		ODM_Write4Byte(pDM_Odm, 0xc6c, 0x00000a15);
		_IQK_BB_Reset_8821C(pDM_Odm);

/*		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]AFE setting for IQK mode!!!!\n")); */
	} else {
		/*IQK AFE Setting RX_WAIT_CCA mode */
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x50000000); 
		ODM_Write4Byte(pDM_Odm, 0xc60, 0x700B8040);

		/*AFE setting*/
		ODM_Write4Byte(pDM_Odm, 0xc58, 0xd8020402);
		ODM_Write4Byte(pDM_Odm, 0xc5c, 0xde000120);
		ODM_Write4Byte(pDM_Odm, 0xc6c, 0x0000122a);
	
/*		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]AFE setting for Normal mode!!!!\n")); */
	}
}

VOID
_IQK_RestoreMacBB_8821C(
	IN PDM_ODM_T		pDM_Odm,
	IN pu4Byte		MAC_backup,
	IN pu4Byte		BB_backup,
	IN pu4Byte		Backup_MAC_REG, 
	IN pu4Byte		Backup_BB_REG
	)	
{
	u4Byte i;

	for (i = 0; i < MAC_REG_NUM_8821C; i++)
		ODM_Write4Byte(pDM_Odm, Backup_MAC_REG[i], MAC_backup[i]);
	for (i = 0; i < BB_REG_NUM_8821C; i++)
		ODM_Write4Byte(pDM_Odm, Backup_BB_REG[i], BB_backup[i]);

/*	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]RestoreMacBB Success!!!!\n")); */
}

VOID
_IQK_RestoreRF_8821C(
	IN PDM_ODM_T			pDM_Odm,
	IN pu4Byte			Backup_RF_REG,
	IN u4Byte			RF_backup[][NUM_8821C]
	)
{	
	u4Byte i;

	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x0);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask, RF_backup[0][ODM_RF_PATH_A]&(~BIT4));
	for (i = 1; i < RF_REG_NUM_8821C; i++)
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, Backup_RF_REG[i], bRFRegOffsetMask, RF_backup[i][ODM_RF_PATH_A]);
/*	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]RestoreRF Success!!!!\n")); */
	
}


void
_IQK_backupIQK_8821C(
	IN PDM_ODM_T			pDM_Odm
	)
{
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte		i, j, k, path, idx;
	u4Byte		tmp;


	pIQK_info->IQK_Channel[1] = pIQK_info->IQK_Channel[0];

	for (i = 0; i < NUM_8821C; i++) {
		pIQK_info->LOK_IDAC[1][i] = pIQK_info->LOK_IDAC[0][i];
		
		for (j = 0; j < 2; j++) {
			for (k = 0; k < 8; k++) {
				pIQK_info->IQK_CFIR_real[1][i][j][k] = pIQK_info->IQK_CFIR_real[0][i][j][k];
				pIQK_info->IQK_CFIR_imag[1][i][j][k] = pIQK_info->IQK_CFIR_imag[0][i][j][k];
			}
		}
	}

	pIQK_info->IQK_Channel[0] = pIQK_info->RFReg18;

	for (path = 0; path < NUM_8821C; path++)
		pIQK_info->LOK_IDAC[0][path] = ODM_GetRFReg(pDM_Odm, path, 0x58, bRFRegOffsetMask);


	for (path = 0; path < NUM_8821C; path++) {
		for (idx = 0; idx < 2; idx++) {
			ODM_SetBBReg(pDM_Odm, 0x1b00, bMaskDWord, 0xf8000008 | path << 1);
			
			if (idx == 0)
				ODM_SetBBReg(pDM_Odm, 0x1b0c, BIT13|BIT12, 0x3);
			else
				ODM_SetBBReg(pDM_Odm, 0x1b0c, BIT13|BIT12, 0x1);

			ODM_SetBBReg(pDM_Odm, 0x1bd4, BIT20|BIT19|BIT18|BIT17|BIT16, 0x10); 
			
			for (i = 0; i < 8; i++) {
				ODM_SetBBReg(pDM_Odm, 0x1bd8, bMaskDWord, 0xe0000001+(i*4));
				tmp = ODM_GetBBReg(pDM_Odm, 0x1bfc, bMaskDWord);
				/*ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]path=%x, idx=%x, i=0x%x, tmp =0x%x\n", path, idx, i, tmp));*/
				pIQK_info->IQK_CFIR_real[0][path][idx][i] = (tmp & 0x0fff0000)>>16;
				pIQK_info->IQK_CFIR_imag[0][path][idx][i] = tmp & 0xfff;
			}
		}
		ODM_SetBBReg(pDM_Odm, 0x1bd8, bMaskDWord, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x1b0c, BIT13|BIT12, 0x0);
	}	
}

VOID
_IQK_ReloadTXIQKsettingforLPS_8821C(
	IN PDM_ODM_T			pDM_Odm
	)
{	
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte i, path;

	for (path = 0; path < 2; path++) {
		ODM_SetBBReg(pDM_Odm, 0x1b00, bMaskDWord, 0xf8000008 | path << 1);
		ODM_SetBBReg(pDM_Odm, 0x1b0c, BIT13|BIT12, 0x3);
		ODM_SetBBReg(pDM_Odm, 0x1bd4, BIT20|BIT19|BIT18|BIT17|BIT16, 0x10); 
		for (i = 0; i < 8; i++) {
			ODM_Write4Byte(pDM_Odm, 0x1bd8,	((0xc0000000 >> TX_IQK) + 0x3)+(i*4)+(pIQK_info->IQK_CFIR_real[0][path][TX_IQK][i]<<9));
			ODM_Write4Byte(pDM_Odm, 0x1bd8, ((0xc0000000 >> TX_IQK) + 0x1)+(i*4)+(pIQK_info->IQK_CFIR_imag[0][path][TX_IQK][i]<<9));
		}
		ODM_SetBBReg(pDM_Odm, 0x1bd8, bMaskDWord, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x1b0c, BIT13|BIT12, 0x0);
	}
}



VOID
_IQK_ReloadIQKsetting_8821C(
	IN PDM_ODM_T			pDM_Odm,
	IN u1Byte				channel
	)
{	
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte i, path, idx;

	for (path = 0; path < NUM_8821C; path++) {
		ODM_SetRFReg(pDM_Odm, path, 0xdf, BIT4, 0x1);
		ODM_SetRFReg(pDM_Odm, path, 0x58, bRFRegOffsetMask, pIQK_info->LOK_IDAC[channel][path]);
		
		
		for (idx = 0; idx < 2; idx++) {
			ODM_SetBBReg(pDM_Odm, 0x1b00, bMaskDWord, 0xf8000008 | path << 1);
			
			if (idx == 0)
				ODM_SetBBReg(pDM_Odm, 0x1b0c, BIT13|BIT12, 0x3);
			else
				ODM_SetBBReg(pDM_Odm, 0x1b0c, BIT13|BIT12, 0x1);

			ODM_SetBBReg(pDM_Odm, 0x1bd4, BIT20|BIT19|BIT18|BIT17|BIT16, 0x10); 


			for (i = 0; i < 8; i++) {
				ODM_Write4Byte(pDM_Odm, 0x1bd8,	((0xc0000000 >> idx) + 0x3)+(i*4)+(pIQK_info->IQK_CFIR_real[channel][path][idx][i]<<9));
				ODM_Write4Byte(pDM_Odm, 0x1bd8, ((0xc0000000 >> idx) + 0x1)+(i*4)+(pIQK_info->IQK_CFIR_imag[channel][path][idx][i]<<9));
			}
		}
		ODM_SetBBReg(pDM_Odm, 0x1bd8, bMaskDWord, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x1b0c, BIT13|BIT12, 0x0);
	}
}


BOOLEAN
_IQK_ReloadIQK_8821C(
	IN	PDM_ODM_T			pDM_Odm,
	IN	BOOLEAN			reset
	)
{	
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte i;
	BOOLEAN reload = FALSE;

	if (reset) {
		for (i = 0; i < 2; i++)
			pIQK_info->IQK_Channel[i] = 0x0;
	} else {
		pIQK_info->RFReg18 = ODM_GetRFReg(pDM_Odm, 0, 0x18, bRFRegOffsetMask);

		for (i = 0; i < 2; i++) {
			if (pIQK_info->RFReg18 == pIQK_info->IQK_Channel[i]) {
				_IQK_ReloadIQKsetting_8821C(pDM_Odm, i);
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]reload IQK result before!!!!\n"));
				reload = TRUE;
			}
		}
	}
	return reload;
}


VOID 
_IQK_RFESetting_8821C(
	IN PDM_ODM_T	pDM_Odm,
	IN BOOLEAN		extPAon
	)
{
	if (extPAon) {
		/*RFE setting*/
		ODM_Write4Byte(pDM_Odm, 0xcb0, 0x77777777);
		ODM_Write4Byte(pDM_Odm, 0xcb4, 0x00007777);
		ODM_Write4Byte(pDM_Odm, 0xcbc, 0x0000083B);
		/*ODM_Write4Byte(pDM_Odm, 0x1990, 0x00000c30);*/
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]external PA on!!!!\n"));
	} else {
		/*RFE setting*/
		ODM_Write4Byte(pDM_Odm, 0xcb0, 0x77171117);
		ODM_Write4Byte(pDM_Odm, 0xcb4, 0x00001177);
		ODM_Write4Byte(pDM_Odm, 0xcbc, 0x00000404);
		/*ODM_Write4Byte(pDM_Odm, 0x1990, 0x00000c30);*/
/*		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]external PA off!!!!\n"));*/
	}
}



VOID 
_IQK_ConfigureMACBB_8821C(
	IN PDM_ODM_T		pDM_Odm
	)
{
	/*MACBB register setting*/
	ODM_Write1Byte(pDM_Odm, 0x522, 0x7f);
	ODM_SetBBReg(pDM_Odm, 0x550, BIT(11)|BIT(3), 0x0);

	ODM_SetBBReg(pDM_Odm, 0x90c, BIT(15), 0x1);			/*0x90c[15]=1: dac_buf reset selection*/
	ODM_SetBBReg(pDM_Odm, 0x9a4, BIT(31), 0x0);         /*0x9a4[31]=0: Select da clock*/
	/*0xc94[0]=1, 0xe94[0]=1: 讓tx從iqk打出來*/
	ODM_SetBBReg(pDM_Odm, 0xc94, BIT(0), 0x1);

	/* 3-wire off*/
	ODM_Write4Byte(pDM_Odm, 0xc00, 0x00000004);
/*	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]Set MACBB setting for IQK!!!!\n"));*/

}

VOID
_IQK_LOKSetting_8821C(
	IN PDM_ODM_T	pDM_Odm,
	IN u1Byte Path
	)
{
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);
	ODM_Write4Byte(pDM_Odm, 0x1bcc, 0x9);
	ODM_Write1Byte(pDM_Odm, 0x1b23, 0x00);

	switch (*pDM_Odm->pBandType) {
		
	case ODM_BAND_2_4G:
		ODM_Write1Byte(pDM_Odm, 0x1b2b, 0x00);
		ODM_SetRFReg(pDM_Odm, Path, 0x56, bRFRegOffsetMask, 0x50EF3);
		ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0xadc00);
		break;
	case ODM_BAND_5G:
		ODM_Write1Byte(pDM_Odm, 0x1b2b, 0x00);
		ODM_SetRFReg(pDM_Odm, Path, 0x56, bRFRegOffsetMask, 0x50ef3);
		ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0xa9c00);
		break;
}
	
/*	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]Set LOK setting!!!!\n"));*/
}


VOID
_IQK_TXKSetting_8821C(
	IN PDM_ODM_T	pDM_Odm,
	IN u1Byte Path
	)
{

	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);
	ODM_Write4Byte(pDM_Odm, 0x1bcc, 0x9);
	ODM_Write4Byte(pDM_Odm, 0x1b20, 0x01440008);

	if (Path == 0x0) 
		ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf800000a);
	else
		ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008);

	ODM_Write4Byte(pDM_Odm, 0x1bcc, 0x3f);


	switch (*pDM_Odm->pBandType) {
	case ODM_BAND_2_4G:
		ODM_SetRFReg(pDM_Odm, Path, 0x56, bRFRegOffsetMask, 0x50EF3);
		ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0xadc00);
		ODM_Write1Byte(pDM_Odm, 0x1b2b, 0x00);
		break;
	case ODM_BAND_5G:
		ODM_SetRFReg(pDM_Odm, Path, 0x56, bRFRegOffsetMask, 0x50EF3);
		ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0xa9c00);
		ODM_Write1Byte(pDM_Odm, 0x1b2b, 0x00);
		break;

	}
/*	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]Set TXK setting!!!!\n"));*/

}

VOID
_IQK_RXKSetting_8821C(
	IN PDM_ODM_T	pDM_Odm,
	IN u1Byte Path
	)
{
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);

	switch (*pDM_Odm->pBandType) {
	case ODM_BAND_2_4G:
		ODM_Write1Byte(pDM_Odm, 0x1bcc, 0x12);
		ODM_Write1Byte(pDM_Odm, 0x1b2b, 0x00);
		ODM_Write4Byte(pDM_Odm, 0x1b20, 0x01450008);
		ODM_Write4Byte(pDM_Odm, 0x1b24, 0x01461068);	

		ODM_SetRFReg(pDM_Odm, Path, 0x56, bRFRegOffsetMask, 0x510e0);
		ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0xa9c00);
			break;
	case ODM_BAND_5G:
		ODM_Write1Byte(pDM_Odm, 0x1bcc, 0x9);
		ODM_Write1Byte(pDM_Odm, 0x1b2b, 0x00);
		ODM_Write4Byte(pDM_Odm, 0x1b20, 0x00450008);
		ODM_Write4Byte(pDM_Odm, 0x1b24, 0x00461468);	

		ODM_SetRFReg(pDM_Odm, Path, 0x56, bRFRegOffsetMask, 0x510e0);
		ODM_SetRFReg(pDM_Odm, Path, 0x8f, bRFRegOffsetMask, 0xa9c00);
		break;
	}
/*	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]Set RXK setting!!!!\n"));*/

}

BOOLEAN
_IQK_CheckCal_8821C(
	IN PDM_ODM_T			pDM_Odm,
	IN u4Byte				IQK_CMD
	)
{	
	BOOLEAN		notready = TRUE, fail = TRUE;	
	u4Byte		delay_count = 0x0;

	while (notready) {
		if (ODM_Read4Byte(pDM_Odm, 0x1b00) == (IQK_CMD & 0xffffff0f)) {
			fail = (BOOLEAN) ODM_GetBBReg(pDM_Odm, 0x1b08, BIT26);
			notready = FALSE;
		} else {
			ODM_delay_ms(1);
			delay_count++;
		}
		
		if (delay_count >= 50) {
			fail = TRUE;
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
				("[IQK]IQK timeout!!!\n"));
			break;
		}
	}
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
		("[IQK]delay count = 0x%x!!!\n", delay_count));

	
	return fail;
}


BOOLEAN
_IQK_RXIQK_GainSearchFail_8821C(
	IN PDM_ODM_T			pDM_Odm,
	IN u1Byte		Path
	)
{	
	u1Byte	count = 0x0;
	BOOLEAN		fail = TRUE;		
	u4Byte	IQK_CMD = 0x0, RFReg0, Reg1bcc, tmp, bb_idx, lna_idx;

	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);
	Reg1bcc = ODM_GetBBReg(pDM_Odm, 0x1bcc, bMaskDWord);

	while (1) {
		IQK_CMD = 0xf8000208 | (1 << (Path + 4));

		ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD);
		ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD+0x1);

		fail = _IQK_CheckCal_8821C(pDM_Odm, IQK_CMD);

		ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);			
		ODM_Write4Byte(pDM_Odm, 0x1bcc, Reg1bcc);

		RFReg0 = ODM_GetRFReg(pDM_Odm, Path, 0x0, bRFRegOffsetMask);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
			("[IQK]S%d ==> RF0x0 = 0x%x\n", Path, RFReg0));

		if ((RFReg0 >> 16) == 0x6) {
			tmp = (RFReg0 & 0x1fe0) >> 5;
			lna_idx = tmp >> 5;
			bb_idx = tmp & 0x1f;

			if (bb_idx == 0x1)
				lna_idx--;
			else if (bb_idx == 0xa)
				lna_idx++;
			else {
				fail = FALSE;
				break;
			}
			ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);			
			ODM_Write4Byte(pDM_Odm, 0x1b24, (ODM_Read4Byte(pDM_Odm, 0x1b24) & 0xffffe3ff) | (lna_idx << 10));
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
				("[IQK]S%d RXIQK gain search fail!!!\n", Path));
			fail = TRUE;
			break;
		}

		count++;
		if (count > 2) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
				("[IQK]S%d RXIQK gain search count out!!!\n", Path));
			fail = TRUE;
			break;
			}
	}
	return fail;
}



BOOLEAN
_LOK_One_Shot_8821C(
	IN	PVOID		pDM_VOID,
	u1Byte			Path
)
{	
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte		delay_count = 0, ii;
	BOOLEAN		LOK_notready = FALSE;
	u4Byte		LOK_temp1 = 0, LOK_temp2 = 0, LOK_temp3 = 0;
	u4Byte		IQK_CMD = 0x0;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
		("[IQK]==========S%d LOK ==========\n", Path));
		
	IQK_CMD = 0xf8000008|(1<<(4+Path));
			
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, ("[IQK]LOK_Trigger = 0x%x\n", IQK_CMD));

	ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD);
	ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD+1);

	/*LOK: CMD ID = 0	{0xf8000018, 0xf8000028}*/
	/*LOK: CMD ID = 0	{0xf8000019, 0xf8000029}*/


	ODM_delay_ms(LOK_delay_8821C);

	delay_count = 0;
	LOK_notready = TRUE;
		
	while (LOK_notready) {

		if (ODM_Read4Byte(pDM_Odm, 0x1b00) == (IQK_CMD & 0xffffff0f))
			LOK_notready = FALSE;
		else
			LOK_notready = TRUE;
		
		if (LOK_notready) {
			ODM_delay_ms(1);
			delay_count++;
		}

		if (delay_count >= 50) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
				("[IQK]S%d LOK timeout!!!\n", Path));
			break;
		}
	}

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
		("[IQK]S%d ==> delay_count = 0x%d\n", Path, delay_count));
		
		
	if (!LOK_notready) {
		LOK_temp2 = ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)Path, 0x8, bRFRegOffsetMask);
		LOK_temp3 = ODM_GetRFReg(pDM_Odm, (ODM_RF_RADIO_PATH_E)Path, 0x58, bRFRegOffsetMask);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
			("[IQK]0x8 = 0x%x, 0x58 = 0x%x\n", LOK_temp2, LOK_temp3));
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
			("[IQK]==>S%d LOK Fail!!!\n", Path));
	}
	pIQK_info->LOK_fail[Path] = LOK_notready;
	return LOK_notready;
}

BOOLEAN
_IQK_One_Shot_8821C(
	IN	PVOID		pDM_VOID,
	u1Byte		Path,
	u1Byte		idx
)
{	
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte		delay_count = 0;
	BOOLEAN		notready = TRUE, fail = TRUE, search_fail = TRUE;
	u4Byte		IQK_CMD = 0x0, tmp, lna_idx, bb_idx;
	u2Byte		IQK_Apply[2]	= {0xc94, 0xe94};

	if (idx == TX_IQK)
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]============ S%d WBTXIQK ============\n", Path));
	else
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]============ S%d WBRXIQK ============\n", Path));
	
	if (idx == TX_IQK) {
		IQK_CMD = 0xf8000008 | ((*pDM_Odm->pBandWidth + 3) << 8) | (1 << (Path + 4));
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
						("[IQK]TXK_Trigger = 0x%x\n", IQK_CMD));
						/*{0xf8000318, 0xf800032a} ==> 20 WBTXK (CMD = 3)*/
						/*{0xf8000418, 0xf800042a} ==> 40 WBTXK (CMD = 4)*/
						/*{0xf8000518, 0xf800052a} ==> 80 WBTXK (CMD = 5)*/
	} else if (idx == RX_IQK) {
#if 1
		if (!_IQK_RXIQK_GainSearchFail_8821C(pDM_Odm, Path)) {
#endif
#if 0
		if (1) {
#endif
			search_fail = FALSE;
		IQK_CMD = 0xf8000008 | ((*pDM_Odm->pBandWidth + 6) << 8) | (1 << (Path+4));
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
			("[IQK]RXK_Trigger = 0x%x\n", IQK_CMD));
						/*{0xf8000618, 0xf800062a} ==> 20 WBRXK (CMD = 6)*/
						/*{0xf8000718, 0xf800072a} ==> 40 WBRXK (CMD = 7)*/
						/*{0xf8000818, 0xf800082a} ==> 80 WBRXK (CMD = 8)*/
	}
	}

	if ((idx == TX_IQK) || ((idx == RX_IQK) && (!search_fail))) {
	ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD);
	ODM_Write4Byte(pDM_Odm, 0x1b00, IQK_CMD+0x1);

	ODM_delay_ms(WBIQK_delay_8821C);
			
	while (notready) {

		if (ODM_Read4Byte(pDM_Odm, 0x1b00) == (IQK_CMD & 0xffffff0f))
			notready = FALSE;
		else
			notready = TRUE;
		
		if (notready) {
			ODM_delay_ms(1);
			delay_count++;
		} else {
				fail = (BOOLEAN) ODM_GetBBReg(pDM_Odm, 0x1b08, BIT26);
			break;
		}

		if (delay_count >= 50) {
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
				("[IQK]S%d IQK timeout!!!\n", Path));
			break;
		}
	}

	if (pDM_Odm->DebugComponents && ODM_COMP_CALIBRATION) {
		ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | Path << 1);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
			("[IQK]S%d ==> 0x1b00 = 0x%x, 0x1b08 = 0x%x\n", Path, ODM_Read4Byte(pDM_Odm, 0x1b00), ODM_Read4Byte(pDM_Odm, 0x1b08)));
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
				("[IQK]S%d ==> delay_count = 0x%d\n", Path, delay_count));
		if (idx == RX_IQK)
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
				("[IQK]S%d ==> RF0x0 = 0x%x, RF0x56 = 0x%x\n", Path, ODM_GetRFReg(pDM_Odm, Path, 0x0, bRFRegOffsetMask), ODM_GetRFReg(pDM_Odm, Path, 0x56, bRFRegOffsetMask)));
		}
	} 
			
	if (!fail) {
		if (idx == TX_IQK)
			pIQK_info->IQC_Matrix[idx][Path] = ODM_Read4Byte(pDM_Odm, 0x1b38);	
		else if (idx == RX_IQK)
			pIQK_info->IQC_Matrix[idx][Path] = ODM_Read4Byte(pDM_Odm, 0x1b3c);

/*ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE,*/ 
/*					("[IQK]S%d_IQC = 0x%x\n", Path, pIQK_info->IQC_Matrix[idx][Path]));*/
	}

	if (idx == RX_IQK) {
		if (pIQK_info->IQK_fail[TX_IQK][Path] == FALSE)			/*TXIQK success in RXIQK*/
			ODM_Write4Byte(pDM_Odm, 0x1b38, pIQK_info->IQC_Matrix[TX_IQK][Path]);
		else
			ODM_Write4Byte(pDM_Odm, 0x1b38, 0x20000000);

		if (!fail)												/*RXIQK success*/
			ODM_SetBBReg(pDM_Odm, IQK_Apply[Path], (BIT11|BIT10), 0x1);
		else
			ODM_SetBBReg(pDM_Odm, IQK_Apply[Path], (BIT11|BIT10), 0x0);
			
	}	
	pIQK_info->IQK_fail[idx][Path] = fail;
	return fail;
}


VOID
_IQK_IQKbyPath_8821C(
	IN	PVOID		pDM_VOID
)
{	
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	BOOLEAN		KFAIL = TRUE;
	u1Byte		i;

/*	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, ("[IQK]IQKstep = 0x%x\n", pDM_Odm->RFCalibrateInfo.IQKstep)); */
#if 1
	switch (pDM_Odm->RFCalibrateInfo.IQKstep) {
	case 1:		/*S0 LOK*/
#if 1
		_IQK_LOKSetting_8821C(pDM_Odm, ODM_RF_PATH_A);
		_LOK_One_Shot_8821C(pDM_Odm, ODM_RF_PATH_A);
#endif
		pDM_Odm->RFCalibrateInfo.IQKstep++;
		break;		
	case 2:		/*S0 TXIQK*/
#if 1
		_IQK_TXKSetting_8821C(pDM_Odm, ODM_RF_PATH_A);
		KFAIL = _IQK_One_Shot_8821C(pDM_Odm, ODM_RF_PATH_A, TXIQK);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, ("[IQK]KFail = 0x%x\n", KFAIL));

		if (KFAIL && (pDM_Odm->RFCalibrateInfo.retry_count[ODM_RF_PATH_A][TXIQK] < 3))
			pDM_Odm->RFCalibrateInfo.retry_count[ODM_RF_PATH_A][TXIQK]++;
		else
#endif
			pDM_Odm->RFCalibrateInfo.IQKstep++;
		break;	
	case 3:		/*S0 RXIQK*/
#if 1
		_IQK_RXKSetting_8821C(pDM_Odm, ODM_RF_PATH_A);

		KFAIL = _IQK_One_Shot_8821C(pDM_Odm, ODM_RF_PATH_A, RXIQK);
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, ("[IQK]KFail = 0x%x\n", KFAIL));
					
		if (KFAIL && pDM_Odm->RFCalibrateInfo.retry_count[ODM_RF_PATH_A][RXIQK] < 3)
			pDM_Odm->RFCalibrateInfo.retry_count[ODM_RF_PATH_A][RXIQK]++;
		else
#endif
				pDM_Odm->RFCalibrateInfo.IQKstep++;
		break;
	case 4:
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE,
		("[IQK]==========LOK summary ==========\n"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,
		("[IQK]LOK0_notready = %d\n", 
		pIQK_info->LOK_fail[ODM_RF_PATH_A]));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE,
		("[IQK]==========IQK summary ==========\n"));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,
		("[IQK]TXIQK0_fail = %d\n", 
		pIQK_info->IQK_fail[TXIQK][ODM_RF_PATH_A]));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,
		("[IQK]TXIQK0_retry = %d\n", 
		pDM_Odm->RFCalibrateInfo.retry_count[ODM_RF_PATH_A][TXIQK]));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,
		("[IQK]RXIQK0_fail = %d\n", 
		pIQK_info->IQK_fail[RXIQK][ODM_RF_PATH_A]));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,
		("[IQK]RXIQK0_retry = %d\n", 
		pDM_Odm->RFCalibrateInfo.retry_count[ODM_RF_PATH_A][RXIQK]));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE,
		("[IQK]================================\n"));

		/*reload IQK default setting, It will be placed in NCTL*/
		for (i = 0; i < NUM_8821C; i++) {
			ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008 | i << 1);		
			ODM_Write4Byte(pDM_Odm, 0x1b2c, 0x7);	
		}

		pDM_Odm->RFCalibrateInfo.IQKstep++;
		break;
	}

#endif


}

VOID
_IQK_StartIQK_8821C(
	IN PDM_ODM_T		pDM_Odm
	)
{	

	u4Byte tmp;
	
	ODM_Write4Byte(pDM_Odm, 0x1b00, 0xf8000008);
	ODM_Write4Byte(pDM_Odm, 0x1bb8, 0x00000000);

	/*0xdf:B11 = 1,B4 = 0, B1 = 1*/
#if 1	
	tmp = ODM_GetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask);
	tmp = (tmp&(~BIT4))|BIT1|BIT11;
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xdf, bRFRegOffsetMask, tmp);	
	
#endif


	/* WE_LUT_TX_LOK*/
	if (*pDM_Odm->pBandType == ODM_BAND_5G) {
		/*[1:0]: AMODE=1; GMODE=0*/
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, BIT4, 0x1);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x33, BIT1|BIT0, 0x1);
	} else {
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xef, BIT4, 0x1);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0x33, BIT1|BIT0, 0x0);
	}


	_IQK_IQKbyPath_8821C(pDM_Odm);


}

VOID
_IQCalibrate_8821C_Init(
	IN	PVOID		pDM_VOID
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;
	PIQK_INFO	pIQK_info = &pDM_Odm->IQK_info;
	u1Byte	ii, jj;

	if (pIQK_info->IQKtimes == 0x0) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]=====>PHY_IQCalibrate_8821C_Init\n"));
		for (jj = 0; jj < 2; jj++) {
			for (ii = 0; ii < NUM; ii++) {
				pIQK_info->LOK_fail[ii] = TRUE;
				pIQK_info->IQK_fail[jj][ii] = TRUE;
				pIQK_info->IQC_Matrix[jj][ii] = 0x20000000;
			}
		}
	}
	pIQK_info->IQKtimes++;
}


VOID	
phy_IQCalibrate_8821C(
	IN PDM_ODM_T		pDM_Odm,
	IN	BOOLEAN			reset
	)
{

	u4Byte	MAC_backup[MAC_REG_NUM_8821C], BB_backup[BB_REG_NUM_8821C], RF_backup[RF_REG_NUM_8821C][1];
	u4Byte	Backup_MAC_REG[MAC_REG_NUM_8821C] = {0x520, 0x550}; 
	u4Byte	Backup_BB_REG[BB_REG_NUM_8821C] = {0x808, 0x90c, 0xc00, 0xcb0, 0xcb4, 0xcbc, 0x1990, 0x9a4, 0xa04}; 
	u4Byte	Backup_RF_REG[RF_REG_NUM_8821C] = {0xdf, 0x8f, 0x65, 0x0, 0x1}; 
	u1Byte	i, j;

	if (_IQK_ReloadIQK_8821C(pDM_Odm, reset))
		return;

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
		("[IQK]==========IQK strat!!!!!==========\n"));

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
		("[IQK]pBandType = %s, BandWidth = %d, ExtPA2G = %d, ExtPA5G = %d\n", (*pDM_Odm->pBandType == ODM_BAND_5G) ? "5G" : "2G", *pDM_Odm->pBandWidth, pDM_Odm->ExtPA, pDM_Odm->ExtPA5G));
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, 
		("[IQK]Interface = %d, CutVersion = %x\n", pDM_Odm->SupportInterface, pDM_Odm->CutVersion));
	
	pDM_Odm->RFCalibrateInfo.Kcount = 0;
	pDM_Odm->RFCalibrateInfo.IQK_TotalProgressingTime = 0;
	pDM_Odm->RFCalibrateInfo.IQKstep = 1;

	for (i = 0; i < 4; i++)
		for (j = 0; j < 2; j++)
			pDM_Odm->RFCalibrateInfo.retry_count[i][j] = 0;
#if 1
	_IQK_BackupMacBB_8821C(pDM_Odm, MAC_backup, BB_backup, Backup_MAC_REG, Backup_BB_REG);
	_IQK_BackupRF_8821C(pDM_Odm, RF_backup, Backup_RF_REG);

	_IQK_ConfigureMACBB_8821C(pDM_Odm);
	_IQK_AFESetting_8821C(pDM_Odm, TRUE);
	_IQK_RFESetting_8821C(pDM_Odm, FALSE);
	_IQK_AGCbnd_int_8821C(pDM_Odm);

	while (1) {

		pDM_Odm->RFCalibrateInfo.Kcount++;

		if (!pDM_Odm->mp_mode)	
			pDM_Odm->RFCalibrateInfo.IQK_StartTime = ODM_GetCurrentTime(pDM_Odm);

		_IQK_StartIQK_8821C(pDM_Odm);

		if (!pDM_Odm->mp_mode) {
			pDM_Odm->RFCalibrateInfo.IQK_ProgressingTime = ODM_GetProgressingTime(pDM_Odm, pDM_Odm->RFCalibrateInfo.IQK_StartTime);
			pDM_Odm->RFCalibrateInfo.IQK_TotalProgressingTime += ODM_GetProgressingTime(pDM_Odm, pDM_Odm->RFCalibrateInfo.IQK_StartTime);
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,  
				("[IQK]IQK ProgressingTime = %lld ms\n", pDM_Odm->RFCalibrateInfo.IQK_ProgressingTime));
		}
	
		if (pDM_Odm->RFCalibrateInfo.IQKstep == 5)
			break;

		if (pDM_Odm->RFCalibrateInfo.Kcount > 20)
			break;

	};

	_IQK_backupIQK_8821C(pDM_Odm);
	_IQK_AFESetting_8821C(pDM_Odm, FALSE);
	_IQK_RestoreMacBB_8821C(pDM_Odm, MAC_backup, BB_backup, Backup_MAC_REG, Backup_BB_REG);
	_IQK_RestoreRF_8821C(pDM_Odm, Backup_RF_REG, RF_backup);

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,  
			("[IQK]Total LOK/TXK/RXK count = %d\n", pDM_Odm->RFCalibrateInfo.Kcount - 1));

	pDM_Odm->RFCalibrateInfo.IQK_TotalProgressingTime += ODM_GetProgressingTime(pDM_Odm, pDM_Odm->RFCalibrateInfo.IQK_StartTime);	
	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,  
		("[IQK]Total IQK ProgressingTime = %lld ms\n", pDM_Odm->RFCalibrateInfo.IQK_TotalProgressingTime));
#endif

	ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_TRACE, 
	("[IQK]==========IQK end!!!!!==========\n"));
}


VOID
phy_IQCalibrate_By_FW_8821C(
	IN	PDM_ODM_T	pDM_Odm
	)
{

	u1Byte			IQKcmd[3] = {*pDM_Odm->pChannel, 0x0, 0x0};
	u1Byte			Buf1 = 0x0;
	u1Byte			Buf2 = 0x0;
	
}


/*IQK version:v0.1 , NCTL v0.1*/
/*1. 8821C IQK first release*/
VOID
PHY_IQCalibrate_8821C(
	IN	PVOID		pDM_VOID,
	IN	BOOLEAN		reset
	)
{
	PDM_ODM_T	pDM_Odm = (PDM_ODM_T)pDM_VOID;

	u4Byte counter;
	
#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
	PADAPTER		pAdapter = pDM_Odm->Adapter;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);	
	
	#if (MP_DRIVER == 1)
		#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)	
			PMPT_CONTEXT	pMptCtx = &(pAdapter->MptCtx);	
		#else
			PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.MptCtx);		
		#endif	
	#endif
	#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
		if (ODM_CheckPowerStatus(pAdapter) == FALSE)
			return;
	#endif

	#if MP_DRIVER == 1	
		if (pMptCtx->bSingleTone || pMptCtx->bCarrierSuppression)
			return;
	#endif
	
#endif

	pDM_Odm->IQKFWOffload = 0;

/*FW IQK*/
	if (pDM_Odm->IQKFWOffload) {
		
	if (!pDM_Odm->RFCalibrateInfo.bIQKInProgress) {
		
		ODM_AcquireSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
		pDM_Odm->RFCalibrateInfo.bIQKInProgress = TRUE;
		ODM_ReleaseSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
	
			phy_IQCalibrate_By_FW_8821C(pDM_Odm);
			
/*#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)*/
			for (counter = 0; counter < 10; counter++) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== FW IQK IN PROGRESS == #%d\n", counter));
				ODM_delay_ms(1);
				if (!pDM_Odm->RFCalibrateInfo.bIQKInProgress) {
					ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== FW IQK RETURN FROM WAITING ==\n"));
					break;
				}
			}
			
/*#elif (DM_ODM_SUPPORT_TYPE == ODM_CE) */
/*	rtl8812_iqk_wait(pAdapter, 500);*/
/*#endif */
			if (pDM_Odm->RFCalibrateInfo.bIQKInProgress) {
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== FW IQK TIMEOUT (Still in progress after 500ms) ==\n"));
				ODM_AcquireSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
				pDM_Odm->RFCalibrateInfo.bIQKInProgress = FALSE;
				ODM_ReleaseSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
			}
		}	else
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("== Return the IQK CMD, because the IQK in Progress ==\n"));

	}
	/* Driver IQK */
	else {	

		_IQCalibrate_8821C_Init(pDM_VOID);

		if (!pDM_Odm->RFCalibrateInfo.bIQKInProgress) {
			ODM_AcquireSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
			pDM_Odm->RFCalibrateInfo.bIQKInProgress = TRUE;
			ODM_ReleaseSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
			if (pDM_Odm->mp_mode)	
				pDM_Odm->RFCalibrateInfo.IQK_StartTime = ODM_GetCurrentTime(pDM_Odm);
		
#if (DM_ODM_SUPPORT_TYPE & (ODM_CE))
			phy_IQCalibrate_8821C(pDM_Odm, reset);
		/*DBG_871X("%s,%d, do IQK %u ms\n", __func__, __LINE__, rtw_get_passing_time_ms(time_iqk));*/
#else
			phy_IQCalibrate_8821C(pDM_Odm, reset);
#endif
			if (pDM_Odm->mp_mode) {
				pDM_Odm->RFCalibrateInfo.IQK_ProgressingTime = ODM_GetProgressingTime(pDM_Odm, pDM_Odm->RFCalibrateInfo.IQK_StartTime);
				ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD,  ("[IQK]IQK ProgressingTime = %lld ms\n", pDM_Odm->RFCalibrateInfo.IQK_ProgressingTime));
			}
			ODM_AcquireSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
			pDM_Odm->RFCalibrateInfo.bIQKInProgress = FALSE;
			ODM_ReleaseSpinLock(pDM_Odm, RT_IQK_SPINLOCK);
			} else
			ODM_RT_TRACE(pDM_Odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("[IQK]== Return the IQK CMD, because the IQK in Progress ==\n"));
	}
}
#endif


