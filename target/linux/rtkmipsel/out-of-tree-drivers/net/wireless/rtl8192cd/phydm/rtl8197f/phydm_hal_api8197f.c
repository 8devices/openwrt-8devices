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

/* ======================================================================== */
/* These following functions can be used for PHY DM only*/

VOID
phydm_CcaParByBw_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_BW_E				bandwidth
	)
{
	pDIG_T		pDM_DigTable = &pDM_Odm->DM_DigTable;
	u4Byte		regc3c;
	u4Byte		regc3c_8_6; /*regc3c[8:6]*/
	u4Byte		reg_aaa;

	regc3c = ODM_GetBBReg(pDM_Odm, 0xc3c, bMaskDWord);
	regc3c_8_6 = ((regc3c & 0x1c0)>>6);
	regc3c &= (~(0x000001f8));
	
	reg_aaa = ODM_Read1Byte(pDM_Odm, 0xaaa) & 0x1f;
	pDM_DigTable->aaa_default = reg_aaa;
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("%s ==> RFEType=%d, PackageType=%d, CutVersion=%d, bandwidth=%d, RXAntStatus=%d\n",
		__func__,
		pDM_Odm->RFEType,
		pDM_Odm->PackageType,
		pDM_Odm->CutVersion,
		bandwidth,
		pDM_Odm->RXAntStatus));	
	
	if ((pDM_Odm->RFEType == 1) && (pDM_Odm->PackageType == 1)) {	// 97FS type1
		if (pDM_Odm->CutVersion == ODM_CUT_A) {
			if ((bandwidth == ODM_BW40M) && (pDM_Odm->RXAntStatus != (ODM_RF_A|ODM_RF_B))) {
				regc3c |= ((0x1) << 3);
				regc3c |= ((0x0) << 6);
			} else if ((bandwidth == ODM_BW40M) && (pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B))){
				regc3c |= ((0x1) << 3);
				regc3c |= ((0x3) << 6);
			} else {
				regc3c |= ((0x0) << 3);
				regc3c |= ((0x0) << 6);
			}
		} else {
			regc3c |= ((0x2) << 3);
			regc3c |= ((0x6) << 6);
		}
		if ((bandwidth == ODM_BW20M)) {
			reg_aaa = 0x10;
		} else {
			reg_aaa = 0x12;
		}
		ODM_SetBBReg(pDM_Odm, 0xaa8, 0x1f0000, reg_aaa);
		pDM_DigTable->aaa_default = reg_aaa;
	} else if ((pDM_Odm->RFEType == 1) && (pDM_Odm->PackageType == 2)) { // 97FN type1
		if ((bandwidth == ODM_BW20M)) {
			if (pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) {
				regc3c |= ((0x3) << 3);
				regc3c |= (regc3c_8_6 << 6);
			} else {
				regc3c |= ((0x3) << 3);
				regc3c |= (regc3c_8_6 << 6);
			}
			reg_aaa = 0x10;
		} else {
			if (pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) {
				regc3c |= ((0x2) << 3);
				regc3c |= (regc3c_8_6 << 6);
			} else { 
				regc3c |= ((0x2) << 3);
				regc3c |= (regc3c_8_6 << 6);
			}
			reg_aaa = 0x12;
		}
		ODM_SetBBReg(pDM_Odm, 0xaa8, 0x1f0000, reg_aaa);
		pDM_DigTable->aaa_default = reg_aaa;
	
	}	
	else if ((pDM_Odm->RFEType == 2) && (pDM_Odm->PackageType == 2)) {
		if ((bandwidth == ODM_BW20M)) {
			if (pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) {
				regc3c |= ((0x1) << 3);
				regc3c |= (regc3c_8_6 << 6);
			} else {
				regc3c |= ((0x2) << 3);
				regc3c |= (regc3c_8_6 << 6);
			}
			reg_aaa = 0xf;
		}
		else {
			if (pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) {
				regc3c |= ((0x0) << 3);
				regc3c |= (regc3c_8_6 << 6);
			} else {
				regc3c |= ((0x2) << 3);
				regc3c |= (regc3c_8_6 << 6);
			}
			reg_aaa = 0x12;
		}
		ODM_SetBBReg(pDM_Odm, 0xaa8, 0x1f0000, reg_aaa);
		pDM_DigTable->aaa_default = reg_aaa;
	} else if ((pDM_Odm->RFEType == 3) && (pDM_Odm->PackageType == 2)) { // 97FN type3
		if ((bandwidth == ODM_BW20M)) {
			if (pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) {
				regc3c |= ((0x2) << 3);
				regc3c |= (regc3c_8_6 << 6);
			} else {
				regc3c |= ((0x2) << 3);
				regc3c |= (regc3c_8_6 << 6);
			}
			reg_aaa = 0x10;
		} else {
			if (pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) {
				regc3c |= ((0x1) << 3);
				regc3c |= (regc3c_8_6 << 6);
			} else { 
				regc3c |= ((0x1) << 3);
				regc3c |= (regc3c_8_6 << 6);
			}
			reg_aaa = 0x12;
		}
		ODM_SetBBReg(pDM_Odm, 0xaa8, 0x1f0000, reg_aaa);
		pDM_DigTable->aaa_default = reg_aaa;
	}
	else { // 97FN/97FS type0
		if (pDM_Odm->CutVersion == ODM_CUT_A) {
			if ((bandwidth == ODM_BW20M) && (pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B))) {
				regc3c |= ((0x0) << 3);
				regc3c |= ((0x0) << 6);
			} else {
				regc3c |= ((0x1) << 3);
				regc3c |= ((0x4) << 6);
			}
		} else {
			if ((bandwidth == ODM_BW20M)) {
				if (pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) {
					regc3c |= ((0x2) << 3);
					regc3c |= (regc3c_8_6 << 6);
				} else {
					regc3c |= ((0x2) << 3);
					regc3c |= (regc3c_8_6 << 6);
				}
				reg_aaa = 0xf;
			} else {
				if (pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) {
					regc3c |= ((0x2) << 3);
					regc3c |= (regc3c_8_6 << 6);
				} else { /*ODM_BW40M*/
					regc3c |= ((0x2) << 3);
					regc3c |= (regc3c_8_6 << 6);
				}
				reg_aaa = 0x12;
			}
			ODM_SetBBReg(pDM_Odm, 0xaa8, 0x1f0000, reg_aaa);
			pDM_DigTable->aaa_default = reg_aaa;
		}
		
	}
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("%s ==> regc3c=0x%x, reg_aaa=0x%x\n", __func__, regc3c, reg_aaa));

	ODM_SetBBReg(pDM_Odm, 0xc3c, bMaskDWord, regc3c);
}

VOID
phydm_CcaParByRxPath_8197f(
	IN	PDM_ODM_T				pDM_Odm
	)
{
	ODM_BW_E	bandwidth = *pDM_Odm->pBandWidth;

	phydm_CcaParByBw_8197f(pDM_Odm, bandwidth);
}

VOID
phydm_RxDfirParByBw_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_BW_E				bandwidth
	)
{
	ODM_SetBBReg(pDM_Odm, ODM_REG_TAP_UPD_97F, (BIT21|BIT20), 0x2);
	ODM_SetBBReg(pDM_Odm, ODM_REG_DOWNSAM_FACTOR_11N, (BIT29|BIT28), 0x2);

	if (bandwidth == ODM_BW40M) {
		/* RX DFIR for BW40 */
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DFIR_MOD_97F, BIT8, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DFIR_MOD_97F, bMaskByte0, 0x3);
	} else {
		/* RX DFIR for BW20, BW10 and BW5*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DFIR_MOD_97F, BIT8, 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DFIR_MOD_97F, bMaskByte0, 0xa3);
	}
}

VOID
phydm_init_hw_info_by_rfe_type_8197f(
	IN	PDM_ODM_T				pDM_Odm
	)
{
	u2Byte	mask_path_a = 0x0303;
	u2Byte	mask_path_b = 0x0c0c;
	/*u2Byte	mask_path_c = 0x3030;*/
	/*u2Byte	mask_path_d = 0xc0c0;*/
	
	pDM_Odm->bInitHwInfoByRfe = FALSE;

	if (pDM_Odm->RFEType == 0) {
		panic_printk("[97F] RFE type 0 PHY paratemters: DEFAULT\n");
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_DEFAULT);
		ODM_SetBBReg(pDM_Odm, 0x4c, BIT24, 0x0);		/*GPIO setting*/
		ODM_SetBBReg(pDM_Odm, 0x64, BIT28, 0x0);		/*GPIO setting*/
		ODM_SetBBReg(pDM_Odm, 0x40, 0xf000000, 0x0);	/*GPIO setting*/
		
	} else if (pDM_Odm->RFEType == 1) {
		panic_printk("[97F] RFE type 1 PHY paratemters: GPA0+GLNA0\n");
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_EXT_PA|ODM_BOARD_EXT_LNA);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GPA, (TYPE_GPA0 & (mask_path_a|mask_path_b)));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GLNA, (TYPE_GLNA0 & (mask_path_a|mask_path_b)));
		ODM_SetBBReg(pDM_Odm, 0x4c, BIT24, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x64, BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x40, 0xf000000, 0x5);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, TRUE);
	} else if (pDM_Odm->RFEType == 2) {
		panic_printk("[97F] RFE type 2 PHY paratemters: 2L internal\n");
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_EXT_PA_5G);	/*Special setting for 2L internal*/
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_APA, (TYPE_APA0 & (mask_path_a|mask_path_b)));
		ODM_SetBBReg(pDM_Odm, 0x4c, BIT24, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x64, BIT28, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x40, 0xf000000, 0x0);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, FALSE);
		
	} else if (pDM_Odm->RFEType == 3) {
		panic_printk("[97F] RFE type 3 PHY paratemters: internal with TRSW\n");
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_EXT_PA_5G);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_APA, (TYPE_APA1 & (mask_path_a|mask_path_b)));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_TRSW, 1);
		ODM_SetBBReg(pDM_Odm, 0x4c, BIT24, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x64, BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x40, 0xf000000, 0x5);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, FALSE);
		
	} else if (pDM_Odm->RFEType == 4) {
		panic_printk("[97F] RFE type 4 PHY paratemters: GLNA0\n");
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_EXT_LNA);
		ODM_SetBBReg(pDM_Odm, 0x4c, BIT24, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x64, BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x40, 0xf000000, 0x5);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, TRUE);
		
	} else if (pDM_Odm->RFEType == 5) {
		panic_printk("[97F] RFE type 5 PHY paratemters: GPA1+GLNA1\n");
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_EXT_PA|ODM_BOARD_EXT_LNA);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GPA, (TYPE_GPA1 & (mask_path_a|mask_path_b)));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GLNA, (TYPE_GLNA1 & (mask_path_a|mask_path_b)));
		ODM_SetBBReg(pDM_Odm, 0x4c, BIT24, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x64, BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x40, 0xf000000, 0x5);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, TRUE);
		
	} else if (pDM_Odm->RFEType == 6) {
		panic_printk("[97F] RFE type 6 PHY paratemters: GLNA1\n");
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_EXT_LNA);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GLNA, (TYPE_GLNA1 & (mask_path_a|mask_path_b)));
		ODM_SetBBReg(pDM_Odm, 0x4c, BIT24, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x64, BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x40, 0xf000000, 0x5);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, TRUE);
		
	} else {
		panic_printk("[97F] RFE type Unknown PHY paratemters:\n");
		ODM_SetBBReg(pDM_Odm, 0x4c, BIT24, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x64, BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x40, 0xf000000, 0x5);
	}

	pDM_Odm->bInitHwInfoByRfe = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("%s: RFE type (%d), Board type (0x%x), Package type (%d)\n", __func__, pDM_Odm->RFEType, pDM_Odm->BoardType, pDM_Odm->PackageType));
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("%s: 5G ePA (%d), 5G eLNA (%d), 2G ePA (%d), 2G eLNA (%d)\n", __func__, pDM_Odm->ExtPA5G, pDM_Odm->ExtLNA5G, pDM_Odm->ExtPA, pDM_Odm->ExtLNA));
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("%s: 5G PA type (%d), 5G LNA type (%d), 2G PA type (%d), 2G LNA type (%d)\n", __func__, pDM_Odm->TypeAPA, pDM_Odm->TypeALNA, pDM_Odm->TypeGPA, pDM_Odm->TypeGLNA));

}

/* ======================================================================== */

/* ======================================================================== */
/* These following functions can be used by driver*/

u4Byte
config_phydm_read_rf_reg_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		RFPath,
	IN	u4Byte					RegAddr,
	IN	u4Byte					BitMask
	)
{
	u4Byte	Readback_Value, Direct_Addr;
	u4Byte	offset_readRF[2] = {0x2800, 0x2c00};
	u4Byte	power_RF[2] = {0x1c, 0x78};

	/* Error handling.*/
	if (RFPath > ODM_RF_PATH_B) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8197f(): unsupported path (%d)\n", RFPath));
		return INVALID_RF_DATA;
	}

	/*  Error handling. Check if RF power is enable or not */
	/*  0xffffffff means RF power is disable */
	if (ODM_GetMACReg(pDM_Odm, power_RF[RFPath], bMaskByte3) != 0x7) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8197f(): Read fail, RF is disabled\n"));
		return INVALID_RF_DATA;
	}

	/* Calculate offset */
	RegAddr &= 0xff;
	Direct_Addr = offset_readRF[RFPath] + (RegAddr << 2);

	/* RF register only has 20bits */
	BitMask &= bRFRegOffsetMask;

	/* Read RF register directly */
	Readback_Value = ODM_GetBBReg(pDM_Odm, Direct_Addr, BitMask);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8197f(): RF-%d 0x%x = 0x%x, bit mask = 0x%x\n", 
		RFPath, RegAddr, Readback_Value, BitMask));
	return Readback_Value;
}

BOOLEAN
config_phydm_write_rf_reg_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		RFPath,
	IN	u4Byte					RegAddr,
	IN	u4Byte					BitMask,
	IN	u4Byte					Data
	)
{
	u4Byte	DataAndAddr = 0, Data_original = 0;
	u4Byte	offset_writeRF[2] = {0x840, 0x844};
	u4Byte	power_RF[2] = {0x1c, 0x78};
	u1Byte	BitShift;

	/* Error handling.*/
	if (RFPath > ODM_RF_PATH_B) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8197f(): unsupported path (%d)\n", RFPath));
		return FALSE;
	}

	/* Read RF register content first */
	RegAddr &= 0xff;
	BitMask = BitMask & bRFRegOffsetMask;

	if (BitMask != bRFRegOffsetMask) {
		Data_original = config_phydm_read_rf_reg_8197f(pDM_Odm, RFPath, RegAddr, bRFRegOffsetMask);

		/* Error handling. RF is disabled */
		if (config_phydm_read_rf_check_8197f(Data_original) == FALSE) {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8197f(): Write fail, RF is disable\n"));
			return FALSE;
		}

		/* check bit mask */
		if (BitMask != 0xfffff) {
			for (BitShift = 0; BitShift <= 19; BitShift++) {
				if (((BitMask >> BitShift) & 0x1) == 1)
					break;
			}
			Data = ((Data_original) & (~BitMask)) | ((Data << BitShift) & (BitMask));
		}
	} 
	else if (ODM_GetMACReg(pDM_Odm, power_RF[RFPath], bMaskByte3) != 0x7) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8197f(): Write fail, RF is disabled\n"));
		return FALSE;
	}

	/* Put write addr in [27:20]  and write data in [19:00] */
	DataAndAddr = ((RegAddr<<20) | (Data&0x000fffff)) & 0x0fffffff;	

	/* Write Operation */
	ODM_SetBBReg(pDM_Odm, offset_writeRF[RFPath], bMaskDWord, DataAndAddr);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8197f(): RF-%d 0x%x = 0x%x (original: 0x%x), bit mask = 0x%x\n", 
		RFPath, RegAddr, Data, Data_original, BitMask));
	return TRUE;
}

BOOLEAN
config_phydm_write_txagc_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u4Byte					PowerIndex,
	IN	ODM_RF_RADIO_PATH_E	Path,	
	IN	u1Byte					HwRate
	)
{
	u1Byte	readBack_data;	/*for 97F workaroud*/
	/* Input need to be HW rate index, not driver rate index!!!! */

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8197f(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	/* Error handling  */
	if ((Path > ODM_RF_PATH_B) || (HwRate > ODM_RATEMCS15)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8197f(): unsupported path (%d)\n", Path));
		return FALSE;
	}

	if (Path == ODM_RF_PATH_A) {
		switch (HwRate) {
		case ODM_RATE1M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_CCK1_Mcs32, 0x00007f00, PowerIndex);
			break;
		case ODM_RATE2M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, 0x00007f00, PowerIndex);
			break;
		case ODM_RATE5_5M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, 0x007f0000, PowerIndex);
			break;
		case ODM_RATE11M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, 0x7f000000, PowerIndex);
			break;

		case ODM_RATE6M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, 0x0000007f, PowerIndex);
			break;
		case ODM_RATE9M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, 0x00007f00, PowerIndex);
			break;
		case ODM_RATE12M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, 0x007f0000, PowerIndex);
			break;
		case ODM_RATE18M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, 0x7f000000, PowerIndex);
			break;
		case ODM_RATE24M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, 0x0000007f, PowerIndex);
			break;
		case ODM_RATE36M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, 0x00007f00, PowerIndex);
			break;
		case ODM_RATE48M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, 0x007f0000, PowerIndex);
			break;
		case ODM_RATE54M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, 0x7f000000, PowerIndex);
			break;

		case ODM_RATEMCS0:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, 0x0000007f, PowerIndex);
			break;
		case ODM_RATEMCS1:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, 0x00007f00, PowerIndex);
			break;
		case ODM_RATEMCS2:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, 0x007f0000, PowerIndex);
			break;
		case ODM_RATEMCS3:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, 0x7f000000, PowerIndex);
			break;
		case ODM_RATEMCS4:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, 0x0000007f, PowerIndex);
			break;
		case ODM_RATEMCS5:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, 0x00007f00, PowerIndex);
			break;
		case ODM_RATEMCS6:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, 0x007f0000, PowerIndex);
			break;
		case ODM_RATEMCS7:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, 0x7f000000, PowerIndex);
			break;

		case ODM_RATEMCS8:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, 0x0000007f, PowerIndex);
			break;
		case ODM_RATEMCS9:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, 0x00007f00, PowerIndex);
			break;
		case ODM_RATEMCS10:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, 0x007f0000, PowerIndex);
			break;
		case ODM_RATEMCS11:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, 0x7f000000, PowerIndex);
			break;
		case ODM_RATEMCS12:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, 0x0000007f, PowerIndex);
			break;
		case ODM_RATEMCS13:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, 0x00007f00, PowerIndex);
			break;
		case ODM_RATEMCS14:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, 0x007f0000, PowerIndex);
			break;
		case ODM_RATEMCS15:
			ODM_SetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, 0x7f000000, PowerIndex);
			break;

		default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid HWrate!\n"));
		break;
		}
	} else if (Path == ODM_RF_PATH_B) {
    		switch (HwRate) {
		case ODM_RATE1M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, 0x00007f00, PowerIndex);
			break;
		case ODM_RATE2M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, 0x007f0000, PowerIndex);
			break;
		case ODM_RATE5_5M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, 0x7f000000, PowerIndex);
			break;
		case ODM_RATE11M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, 0x0000007f, PowerIndex);
			break;

		case ODM_RATE6M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, 0x0000007f, PowerIndex);
			break;
		case ODM_RATE9M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, 0x00007f00, PowerIndex);
			break;
		case ODM_RATE12M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, 0x007f0000, PowerIndex);
			break;
		case ODM_RATE18M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, 0x7f000000, PowerIndex);
			break;
		case ODM_RATE24M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, 0x0000007f, PowerIndex);
			break;
		case ODM_RATE36M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, 0x00007f00, PowerIndex);
			break;
		case ODM_RATE48M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, 0x007f0000, PowerIndex);
			break;
		case ODM_RATE54M:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, 0x7f000000, PowerIndex);
			break;

		case ODM_RATEMCS0:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, 0x0000007f, PowerIndex);
			break;
		case ODM_RATEMCS1:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, 0x00007f00, PowerIndex);
			break;
		case ODM_RATEMCS2:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, 0x007f0000, PowerIndex);
			break;
		case ODM_RATEMCS3:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, 0x7f000000, PowerIndex);
			break;
		case ODM_RATEMCS4:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, 0x0000007f, PowerIndex);
			break;
		case ODM_RATEMCS5:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, 0x00007f00, PowerIndex);
			break;
		case ODM_RATEMCS6:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, 0x007f0000, PowerIndex);
			break;
		case ODM_RATEMCS7:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, 0x7f000000, PowerIndex);
			break;

		case ODM_RATEMCS8:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, 0x0000007f, PowerIndex);
			break;
		case ODM_RATEMCS9:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, 0x00007f00, PowerIndex);
			break;
		case ODM_RATEMCS10:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, 0x007f0000, PowerIndex);
			break;
		case ODM_RATEMCS11:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, 0x7f000000, PowerIndex);
			break;
		case ODM_RATEMCS12:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, 0x0000007f, PowerIndex);
			break;
		case ODM_RATEMCS13:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, 0x00007f00, PowerIndex);
			break;
		case ODM_RATEMCS14:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, 0x007f0000, PowerIndex);
			break;
		case ODM_RATEMCS15:
			ODM_SetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, 0x7f000000, PowerIndex);
			break;

		default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid HWrate!\n"));
		break;
		}
	} else
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid RF path!!\n"));

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8197f(): Path-%d Rate index 0x%x = 0x%x\n", 
		Path, HwRate, PowerIndex));
	return TRUE;
}

u1Byte
config_phydm_read_txagc_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E	Path,
	IN	u1Byte					HwRate
	)
{
	u1Byte	readBack_data;

	/* Input need to be HW rate index, not driver rate index!!!! */

	/* Error handling  */
	if ((Path > ODM_RF_PATH_B) || (HwRate > ODM_RATEMCS15)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_txagc_8197f(): unsupported path (%d)\n", Path));
		return INVALID_TXAGC_DATA;
	}

	if (Path == ODM_RF_PATH_A) {
		switch (HwRate) {
		case ODM_RATE1M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_CCK1_Mcs32, 0x00007f00);
			break;
		case ODM_RATE2M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, 0x00007f00);
			break;
		case ODM_RATE5_5M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, 0x007f0000);
			break;
		case ODM_RATE11M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, 0x7f000000);
			break;

		case ODM_RATE6M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, 0x0000007f);
			break;
		case ODM_RATE9M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, 0x00007f00);
			break;
		case ODM_RATE12M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, 0x007f0000);
			break;
		case ODM_RATE18M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate18_06, 0x7f000000);
			break;
		case ODM_RATE24M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, 0x0000007f);
			break;
		case ODM_RATE36M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, 0x00007f00);
			break;
		case ODM_RATE48M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, 0x007f0000);
			break;
		case ODM_RATE54M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Rate54_24, 0x7f000000);
			break;

		case ODM_RATEMCS0:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, 0x0000007f);
			break;
		case ODM_RATEMCS1:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, 0x00007f00);
			break;
		case ODM_RATEMCS2:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, 0x007f0000);
			break;
		case ODM_RATEMCS3:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs03_Mcs00, 0x7f000000);
			break;
		case ODM_RATEMCS4:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, 0x0000007f);
			break;
		case ODM_RATEMCS5:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, 0x00007f00);
			break;
		case ODM_RATEMCS6:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, 0x007f0000);
			break;
		case ODM_RATEMCS7:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs07_Mcs04, 0x7f000000);
			break;

		case ODM_RATEMCS8:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, 0x0000007f);
			break;
		case ODM_RATEMCS9:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, 0x00007f00);
			break;
		case ODM_RATEMCS10:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, 0x007f0000);
			break;
		case ODM_RATEMCS11:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs11_Mcs08, 0x7f000000);
			break;
		case ODM_RATEMCS12:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, 0x0000007f);
			break;
		case ODM_RATEMCS13:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, 0x00007f00);
			break;
		case ODM_RATEMCS14:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, 0x007f0000);
			break;
		case ODM_RATEMCS15:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_A_Mcs15_Mcs12, 0x7f000000);
			break;

		default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid HWrate!\n"));
		break;
		}
	} else if (Path == ODM_RF_PATH_B) {
    		switch (HwRate) {
		case ODM_RATE1M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, 0x00007f00);
			break;
		case ODM_RATE2M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, 0x007f0000);
			break;
		case ODM_RATE5_5M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK1_55_Mcs32, 0x7f000000);
			break;
		case ODM_RATE11M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_CCK11_A_CCK2_11, 0x0000007f);
			break;

		case ODM_RATE6M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, 0x0000007f);
			break;
		case ODM_RATE9M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, 0x00007f00);
			break;
		case ODM_RATE12M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, 0x007f0000);
			break;
		case ODM_RATE18M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate18_06, 0x7f000000);
			break;
		case ODM_RATE24M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, 0x0000007f);
			break;
		case ODM_RATE36M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, 0x00007f00);
			break;
		case ODM_RATE48M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, 0x007f0000);
			break;
		case ODM_RATE54M:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Rate54_24, 0x7f000000);
			break;

		case ODM_RATEMCS0:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, 0x0000007f);
			break;
		case ODM_RATEMCS1:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, 0x00007f00);
			break;
		case ODM_RATEMCS2:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, 0x007f0000);
			break;
		case ODM_RATEMCS3:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs03_Mcs00, 0x7f000000);
			break;
		case ODM_RATEMCS4:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, 0x0000007f);
			break;
		case ODM_RATEMCS5:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, 0x00007f00);
				break;
		case ODM_RATEMCS6:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, 0x007f0000);
			break;
		case ODM_RATEMCS7:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs07_Mcs04, 0x7f000000);
			break;

		case ODM_RATEMCS8:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, 0x0000007f);
			break;
		case ODM_RATEMCS9:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, 0x00007f00);
			break;
		case ODM_RATEMCS10:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, 0x007f0000);
			break;
		case ODM_RATEMCS11:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs11_Mcs08, 0x7f000000);
			break;
		case ODM_RATEMCS12:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, 0x0000007f);
			break;
		case ODM_RATEMCS13:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, 0x00007f00);
			break;
		case ODM_RATEMCS14:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, 0x007f0000);
			break;
		case ODM_RATEMCS15:
			readBack_data = ODM_GetBBReg(pDM_Odm, rTxAGC_B_Mcs15_Mcs12, 0x7f000000);
			break;

		default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid HWrate!\n"));
		break;
		}
	} else
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("Invalid RF path!!\n"));

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_txagc_8197f(): Path-%d rate index 0x%x = 0x%x\n", Path, HwRate, readBack_data));
	return readBack_data;
}

BOOLEAN
config_phydm_switch_channel_8197f(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch
	)
{
	pDIG_T		pDM_DigTable = &pDM_Odm->DM_DigTable;
	u4Byte		rf_reg18;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8197f()====================>\n"));

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8197f(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	rf_reg18 = config_phydm_read_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_A, ODM_REG_CHNBW_11N, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8197f(rf_reg18);

	/* Switch band and channel */
	if (central_ch <= 14) {
		/* 2.4G */
		/*table selection*/
		pDM_DigTable->agcTableIdx = ODM_GetBBReg(pDM_Odm, ODM_REG_BB_PWR_SAV2_11N, BIT12|BIT11|BIT10|BIT9);
		/* 1. RF band and channel*/
		rf_reg18 = (rf_reg18 & (~(BIT18|BIT17|bMaskByte0)));
		rf_reg18 = (rf_reg18|central_ch);

	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8197f(): Fail to switch band (ch: %d)\n", central_ch));
		return FALSE;
	}

	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_A, ODM_REG_CHNBW_11N, bRFRegOffsetMask, rf_reg18);
	if (pDM_Odm->RFType > ODM_1T1R)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_B, ODM_REG_CHNBW_11N, bRFRegOffsetMask, rf_reg18);

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8197f(): Fail to switch channel (ch: %d), because writing RF register is fail\n", central_ch));
		return FALSE;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8197f(): Success to switch channel (ch: %d)\n", central_ch));
	return TRUE;
}

BOOLEAN
config_phydm_switch_bandwidth_8197f(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	)
{
	u4Byte		rf_reg18;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f()===================>\n"));

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	/* Error handling  */
	if ((bandwidth >= ODM_BW_MAX) || ((bandwidth == ODM_BW40M) && (primary_ch_idx > 2)) || ((bandwidth == ODM_BW80M) && (primary_ch_idx > 4))) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f(): Fail to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
		return FALSE;
	}

	rf_reg18 = config_phydm_read_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_A, ODM_REG_CHNBW_11N, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8197f(rf_reg18);

	/* Switch bandwidth */
	switch (bandwidth) {
	case ODM_BW20M:
	{
		/* Small BW([31:30]) = 0, rf mode(800[0], 900[0]) = 0 for 20M */
		ODM_SetBBReg(pDM_Odm, ODM_REG_SMALL_BANDWIDTH_11N, (BIT31|BIT30), 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, BIT0, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_PAGE9_11N, BIT0, 0x0);

		/* ADC clock = 160M clock for BW20*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_RXCK_RFMOD, (BIT18|BIT17|BIT16), 0x4);

		/* DAC clock = 160M clock for BW20 = 3'b101*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT14|BIT13|BIT12), 0x5);

		/* ADC buffer clock 0xca4[27:26] = 2'b10*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N, (BIT27|BIT26), 0x2);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18 | BIT11 | BIT10);
		break;
	}
	case ODM_BW40M:
	{
		/* Small BW([31:30]) = 0, rf mode(800[0], 900[0]) = 1 for 40M */
		ODM_SetBBReg(pDM_Odm, ODM_REG_SMALL_BANDWIDTH_11N, (BIT31|BIT30), 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, BIT0, 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_PAGE9_11N, BIT0, 0x1);

		/* ADC clock = 160M clock for BW40 no need to setting, it will be setting in PHY_REG */

		/* DAC clock = 160M clock for BW20 = 3'b101*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT14|BIT13|BIT12), 0x5);

		/* ADC buffer clock 0xca4[27:26] = 2'b10*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N, (BIT27|BIT26), 0x2);

		/* CCK primary channel */
		if (primary_ch_idx == 1)
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N, BIT4, primary_ch_idx);
		else
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N, BIT4, 0);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18 & (~(BIT11|BIT10)));
		rf_reg18 = (rf_reg18|BIT11);
		break;
	}
	case ODM_BW5M:
	{
		/* Small BW([31:30]) = 0, rf mode(800[0], 900[0]) = 0 for 5M */
		ODM_SetBBReg(pDM_Odm, ODM_REG_SMALL_BANDWIDTH_11N, (BIT31|BIT30), 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, BIT0, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_PAGE9_11N, BIT0, 0x0);

		/* ADC clock = 40M clock for BW5 = 3'b010*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_RXCK_RFMOD, (BIT18|BIT17|BIT16), 0x2);

		/* DAC clock = 20M clock for BW20 = 3'b110*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT14|BIT13|BIT12), 0x6);

		/* ADC buffer clock 0xca4[27:26] = 2'b10*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N, (BIT27|BIT26), 0x2);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18|BIT11|BIT10);

		break;
	}
	case ODM_BW10M:
	{
		/* Small BW([31:30]) = 0, rf mode(800[0], 900[0]) = 0 for 10M */
		ODM_SetBBReg(pDM_Odm, ODM_REG_SMALL_BANDWIDTH_11N, (BIT31|BIT30), 0x2);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, BIT0, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_PAGE9_11N, BIT0, 0x0);

		/* ADC clock = 80M clock for BW5 = 3'b011*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_RXCK_RFMOD, (BIT18|BIT17|BIT16), 0x3);

		/* DAC clock = 160M clock for BW20 = 3'b110*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT14|BIT13|BIT12), 0x4);

		/* ADC buffer clock 0xca4[27:26] = 2'b10*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N, (BIT27|BIT26), 0x2);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18|BIT11|BIT10);

		break;
	}
	default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f(): Fail to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
	}

	/* Write RF register */
	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_A, ODM_REG_CHNBW_11N, bRFRegOffsetMask, rf_reg18);

	if (pDM_Odm->RFType > ODM_1T1R)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8197f(pDM_Odm, ODM_RF_PATH_B, ODM_REG_CHNBW_11N, bRFRegOffsetMask, rf_reg18);

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f(): Fail to switch bandwidth (bw: %d, primary ch: %d), because writing RF register is fail\n", bandwidth, primary_ch_idx));
		return FALSE;
	}

	/* Modify RX DFIR parameters */
	phydm_RxDfirParByBw_8197f(pDM_Odm, bandwidth);

	/* Modify CCA parameters */
	phydm_CcaParByBw_8197f(pDM_Odm, bandwidth);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8197f(): Success to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
	return TRUE;
}

BOOLEAN
config_phydm_switch_channel_bw_8197f(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	)
{
	u1Byte			eRFPath = 0;
	u4Byte			RFValToWR , RFTmpVal, BitShift, BitMask;

	/* Switch band */
	/*97F no need*/

	/* Switch channel */
	if (config_phydm_switch_channel_8197f(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	/* Switch bandwidth */
	if (config_phydm_switch_bandwidth_8197f(pDM_Odm, primary_ch_idx, bandwidth) == FALSE)
		return FALSE;

	return TRUE;
}

BOOLEAN
config_phydm_trx_mode_8197f(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_PATH_E			TxPath,
	IN	ODM_RF_PATH_E			RxPath,
	IN	BOOLEAN					bTx2Path
	)
{
	BOOLEAN		rf_reg_status = TRUE;
	u1Byte		IGI;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8197f()=====================>\n"));	

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8197f(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	if ((TxPath & (~(ODM_RF_A|ODM_RF_B))) != 0) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8197f(): Wrong TX setting (TX: 0x%x)\n", TxPath));
		return FALSE;
	}

	if ((RxPath & (~(ODM_RF_A|ODM_RF_B))) != 0) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8197f(): Wrong RX setting (RX: 0x%x)\n", RxPath));
		return FALSE;
	}

	/* RF mode of path-A and path-B */
	/* OFDM Tx and Rx path setting */
	if (TxPath == (ODM_RF_A|ODM_RF_B))
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_TX_PATH_11N, BIT27|BIT26|BIT25|BIT24|bMaskL3Byte, 0x81121313);
	else if (TxPath & ODM_RF_A)
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_TX_PATH_11N, BIT27|BIT26|BIT25|BIT24|bMaskL3Byte, 0x81121311);
	else
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_TX_PATH_11N, BIT27|BIT26|BIT25|BIT24|bMaskL3Byte, 0x82221322);

	if (RxPath == (ODM_RF_A|ODM_RF_B)) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_PATH_11N, bMaskByte0, 0x33);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_ANT_11N, BIT3|BIT2|BIT1|BIT0, 0x3);
	} else if (RxPath & ODM_RF_A) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_PATH_11N, bMaskByte0, 0x11);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_ANT_11N, BIT3|BIT2|BIT1|BIT0, 0x1);
	} else {
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_PATH_11N, bMaskByte0, 0x22);
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_RX_ANT_11N, BIT3|BIT2|BIT1|BIT0, 0x2);
	}

    /* CCK Tx and Rx path setting*/
	if (TxPath & ODM_RF_A)
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, bMaskH4Bits, 0x8);
	else
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, bMaskH4Bits, 0x4);

	if (RxPath == (ODM_RF_A|ODM_RF_B)) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, BIT27|BIT26|BIT25|BIT24, 0x4);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT18, 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT22, 0x1);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N, BIT15, 0x0);
	} else if (RxPath & ODM_RF_A) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, BIT27|BIT26|BIT25|BIT24, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT18, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT22, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N, BIT15, 0x0);
	} else {
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, BIT27|BIT26|BIT25|BIT24, 0x5);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT18, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_FA_RST_11N, BIT22, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N, BIT15, 0x0);
	}

	if(bTx2Path) {
		/*OFDM tx setting*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_TX_PATH_11N, BIT27|BIT26|BIT25|BIT24|bMaskL3Byte, 0x83321333);
		/*CCK tx setting*/
		ODM_SetBBReg(pDM_Odm, ODM_REG_TX_ANT_CTRL_11N, BIT31, 0x0);
		ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANT_SEL_11N, bMaskH4Bits, 0xc);

		/*Paath_A*/
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_RCK_OS, 0xfffff, 0x08000);	/*Select Standby mode	0x30=0x08000*/
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G1, 0xfffff, 0x0005f);	/*Set Table data*/
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G2, 0xfffff, 0x01042);	/*Enable TXIQGEN in standby mode*/
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_RCK_OS, 0xfffff, 0x18000);	/*Select RX mode	0x30=0x18000*/
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G1, 0xfffff, 0x0004f);	/*Set Table data*/
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, RF_TXPA_G2, 0xfffff, 0x71fc2);	/*Enable TXIQGEN in RX mode*/
		/*Path_B*/
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_RCK_OS, 0xfffff, 0x08000);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_TXPA_G1, 0xfffff, 0x00050);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_TXPA_G2, 0xfffff, 0x01042);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_RCK_OS, 0xfffff, 0x18000);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_TXPA_G1, 0xfffff, 0x00040);
		ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_B, RF_TXPA_G2, 0xfffff, 0x71fc2);
	}

	/* Update TXRX antenna status for PHYDM */
	pDM_Odm->TXAntStatus =  (TxPath & 0x3);
	pDM_Odm->RXAntStatus =  (RxPath & 0x3);

	/* Modify CCA parameters */
	phydm_CcaParByRxPath_8197f(pDM_Odm);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8197f(): Success to set TRx mode setting (TX: 0x%x, RX: 0x%x)\n", TxPath, RxPath));
	return TRUE;
}

BOOLEAN
config_phydm_parameter_8197f_init(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_PARAMETER_INIT_E	type
	)
{
	if (type == ODM_PRE_SETTING) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT25|BIT24), 0x0);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_8197f_init(): Pre setting: disable OFDM and CCK block\n"));
	} else if (type == ODM_POST_SETTING) {
		ODM_SetBBReg(pDM_Odm, ODM_REG_BB_CTRL_11N, (BIT25|BIT24), 0x3);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_8197f_init(): Post setting: enable OFDM and CCK block\n"));
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_8197f_init(): Wrong type!!\n"));
		return FALSE;
	}

	return TRUE;
}

/* ======================================================================== */
#endif	/* RTL8197F_SUPPORT == 1 */

