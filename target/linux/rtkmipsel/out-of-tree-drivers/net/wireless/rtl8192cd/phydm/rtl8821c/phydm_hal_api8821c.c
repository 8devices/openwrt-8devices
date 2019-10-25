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

#if (RTL8821C_SUPPORT == 1)  

/* ======================================================================== */
/* These following functions can be used for PHY DM only*/

u4Byte	reg82c_8821c;
u4Byte	reg838_8821c;
u4Byte	reg830_8821c;
ODM_BW_E	bw_8821c;
u1Byte	central_ch_8821c;


BOOLEAN
phydm_rfe_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					channel
	)
{
#if 0
	/* Efuse is not wrote now */
	/* Need to check RFE type finally */
	/*if (pDM_Odm->RFEType == 1) {*/
		if (channel <= 14) {
			/* signal source */
			ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x704570);
			ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x704570);
			ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x45);
			ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x45);
		} else if (channel > 35) {
			ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x174517);
			ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x174517);
			ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x45);
			ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x45);
		} else
			return FALSE;
	
		/* chip top mux */
		ODM_SetBBReg(pDM_Odm, 0x64, BIT29|BIT28, 0x3);
		ODM_SetBBReg(pDM_Odm, 0x4c, BIT26|BIT25, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x40, BIT2, 0x1);

		/* from s0 or s1 */
		ODM_SetBBReg(pDM_Odm, 0x1990, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x30);
		ODM_SetBBReg(pDM_Odm, 0x1990, (BIT11|BIT10), 0x3);

		/* input or output */
		ODM_SetBBReg(pDM_Odm, 0x974, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x3f);
		ODM_SetBBReg(pDM_Odm, 0x974, (BIT11|BIT10), 0x3);

		/* delay 400ns for PAPE */
		ODM_SetBBReg(pDM_Odm, 0x810, bMaskByte3|BIT20|BIT21|BIT22|BIT23, 0x211);

		/* antenna switch table */
		ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa555);
		ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa555);

		/* inverse or not */
		ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x0);
		ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT11|BIT10), 0x0);
		ODM_SetBBReg(pDM_Odm, 0xebc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x0);
		ODM_SetBBReg(pDM_Odm, 0xebc, (BIT11|BIT10), 0x0);
	/*}*/
#endif
	return TRUE;
}

VOID
phydm_ccapar_8821c(
	IN	PDM_ODM_T				pDM_Odm
	)
{
#if 0
	u4Byte	cca_ifem[9][4] = {
		/*20M*/
		{0x75D97010, 0x75D97010, 0x75D97010, 0x75D97010}, /*Reg82C*/
		{0x00000000, 0x00000000, 0x00000000, 0x00000000}, /*Reg830*/
		{0x00000000, 0x00000000, 0x00000000, 0x00000000}, /*Reg838*/
		/*40M*/					
		{0x75D97010, 0x75D97010, 0x75D97010, 0x75D97010}, /*Reg82C*/
		{0x00000000, 0x79a0ea28, 0x00000000, 0x79a0ea28}, /*Reg830*/
		{0x87765541, 0x87766341, 0x87765541, 0x87766341}, /*Reg838*/
		/*80M*/
		{0x75D97010, 0x75D97010, 0x75D97010, 0x75D97010}, /*Reg82C*/
		{0x00000000, 0x00000000, 0x00000000, 0x00000000}, /*Reg830*/
		{0x00000000, 0x87746641, 0x00000000, 0x87746641} }; /*Reg838*/

	u4Byte	cca_efem[9][4] = {
		/*20M*/
		{0x75A76010, 0x75A76010, 0x75A76010, 0x75A75010}, /*Reg82C*/
		{0x00000000, 0x00000000, 0x00000000, 0x00000000}, /*Reg830*/
		{0x87766651, 0x87766431, 0x87766451, 0x87766431}, /*Reg838*/
		/*40M*/
		{0x75A75010, 0x75A75010, 0x75A75010, 0x75A75010}, /*Reg82C*/
		{0x00000000, 0x00000000, 0x00000000, 0x00000000}, /*Reg830*/
		{0x87766431, 0x87766431, 0x87766431, 0x87766431}, /*Reg838*/
		/*80M*/
		{0x75BA7010, 0x75BA7010, 0x75BA7010, 0x75BA7010}, /*Reg82C*/
		{0x00000000, 0x00000000, 0x00000000, 0x00000000}, /*Reg830*/
		{0x87766431, 0x87766431, 0x87766431, 0x87766431} }; /*Reg838*/
		
	u1Byte	row, col;
	u4Byte	reg82c, reg830, reg838;

	if (pDM_Odm->CutVersion != ODM_CUT_B)
		return;

	if (bw_8821c == ODM_BW20M)
		row = 0;
	else if (bw_8821c == ODM_BW40M)
		row = 3;
	else
		row = 6;

	if (central_ch_8821c <= 14) {
		if ((pDM_Odm->RXAntStatus == ODM_RF_A) || (pDM_Odm->RXAntStatus == ODM_RF_B))
			col = 0;
		else
			col = 1;
	} else {
		if ((pDM_Odm->RXAntStatus == ODM_RF_A) || (pDM_Odm->RXAntStatus == ODM_RF_B))
			col = 2;
		else
			col = 3;
	}

	if (pDM_Odm->RFEType == 0) {/*iFEM*/
		reg82c = (cca_ifem[row][col] != 0)?cca_ifem[row][col]:reg82c_8821c;
		reg830 = (cca_ifem[row + 1][col] != 0)?cca_ifem[row + 1][col]:reg830_8821c;
		reg838 = (cca_ifem[row + 2][col] != 0)?cca_ifem[row + 2][col]:reg838_8821c;
	} else {/*eFEM*/
		reg82c = (cca_efem[row][col] != 0)?cca_efem[row][col]:reg82c_8821c;
		reg830 = (cca_efem[row + 1][col] != 0)?cca_efem[row + 1][col]:reg830_8821c;
		reg838 = (cca_efem[row + 2][col] != 0)?cca_efem[row + 2][col]:reg838_8821c;
	}
	
	ODM_SetBBReg(pDM_Odm, 0x82c, bMaskDWord, reg82c);
	ODM_SetBBReg(pDM_Odm, 0x830, bMaskDWord, reg830);
	ODM_SetBBReg(pDM_Odm, 0x838, bMaskDWord, reg838);
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Update CCA parameters for Bcut (Pkt%d, Intf%d, RFE%d), row = %d, col = %d\n", 
		__func__, pDM_Odm->PackageType, pDM_Odm->SupportInterface, pDM_Odm->RFEType, row, col));
#endif	
}

VOID
phydm_ccapar_by_bw_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_BW_E				bandwidth
	)
{
#if 0
	u4Byte		reg82c;


	if (pDM_Odm->CutVersion != ODM_CUT_A)
		return;

	/* A-cut */
	reg82c = ODM_GetBBReg(pDM_Odm, 0x82c, bMaskDWord);

	if (bandwidth == ODM_BW20M) {
		/* 82c[15:12] = 4 */
		/* 82c[27:24] = 6 */
		
		reg82c &= (~(0x0f00f000));
		reg82c |= ((0x4) << 12);
		reg82c |= ((0x6) << 24);
	} else if (bandwidth == ODM_BW40M) {
		/* 82c[19:16] = 9 */
		/* 82c[27:24] = 6 */
	
		reg82c &= (~(0x0f0f0000));
		reg82c |= ((0x9) << 16);
		reg82c |= ((0x6) << 24);
	} else if (bandwidth == ODM_BW80M) {
		/* 82c[15:12] 7 */
		/* 82c[19:16] b */
		/* 82c[23:20] d */
		/* 82c[27:24] 3 */
	
		reg82c &= (~(0x0ffff000));
		reg82c |= ((0xdb7) << 12);
		reg82c |= ((0x3) << 24);
	}

	ODM_SetBBReg(pDM_Odm, 0x82c, bMaskDWord, reg82c);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Update CCA parameters for Acut\n", __func__));
#endif	
}

VOID
phydm_ccapar_by_rxpath_8821c(
	IN	PDM_ODM_T				pDM_Odm
	)
{
#if 0
	if (pDM_Odm->CutVersion != ODM_CUT_A)
		return;

	if ((pDM_Odm->RXAntStatus == ODM_RF_A) || (pDM_Odm->RXAntStatus == ODM_RF_B)) {
		/* 838[7:4] = 8 */
		/* 838[11:8] = 7 */
		/* 838[15:12] = 6 */
		/* 838[19:16] = 7 */
		/* 838[23:20] = 7 */
		/* 838[27:24] = 7 */
		ODM_SetBBReg(pDM_Odm, 0x838, 0x0ffffff0, 0x777678);
	} else {
		/* 838[7:4] = 3 */
		/* 838[11:8] = 3 */
		/* 838[15:12] = 6 */
		/* 838[19:16] = 6 */
		/* 838[23:20] = 7 */
		/* 838[27:24] = 7 */
		ODM_SetBBReg(pDM_Odm, 0x838, 0x0ffffff0, 0x776633);
	}
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Update CCA parameters for Acut\n", __func__));
#endif
}

VOID
phydm_rxdfirpar_by_bw_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_BW_E				bandwidth
	)
{
	if (bandwidth == ODM_BW40M) {
		/* RX DFIR for BW40 */
		ODM_SetBBReg(pDM_Odm, 0x948, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0x94c, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0xc20, BIT31, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8f0, BIT31, 0x0);
	} else if (bandwidth == ODM_BW80M) {
		/* RX DFIR for BW80 */
		ODM_SetBBReg(pDM_Odm, 0x948, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0x94c, BIT29|BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0xc20, BIT31, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8f0, BIT31, 0x1);
	} else {
		/* RX DFIR for BW20, BW10 and BW5*/
		ODM_SetBBReg(pDM_Odm, 0x948, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0x94c, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0xc20, BIT31, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x8f0, BIT31, 0x0);
	}
}

BOOLEAN
phydm_write_txagc_1byte_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u4Byte					PowerIndex,
	IN	ODM_RF_RADIO_PATH_E		Path,	
	IN	u1Byte					HwRate
	)
{
	u4Byte	offset_txagc[2] = {0x1d00, 0x1d80};
	u1Byte	rate_idx = (HwRate & 0xfc), i;
	u1Byte	rate_offset = (HwRate & 0x3);
	u4Byte	rate_mask = (0xff << (rate_offset << 3));
	u4Byte	txagc_content = 0x0;

	/* For debug command only!!!! */

	/* Error handling  */
	if ((Path > ODM_RF_PATH_A) || (HwRate > 0x53)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: unsupported path (%d)\n", __func__, Path));
		return FALSE;
	}

#if 1
	/* For HW limitation, We can't write TXAGC once a byte. */
	for (i = 0; i < 4; i++) {
		if (i != rate_offset)
			txagc_content = txagc_content|(config_phydm_read_txagc_8821c(pDM_Odm, Path, rate_idx + i) << (i << 3));
		else
			txagc_content = txagc_content|((PowerIndex & 0x3f) << (i << 3));
	}
	ODM_SetBBReg(pDM_Odm, (offset_txagc[Path] + rate_idx), bMaskDWord, txagc_content);
#else
	ODM_Write1Byte(pDM_Odm, (offset_txagc[Path] + HwRate), (PowerIndex & 0x3f));
#endif

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Path-%d Rate index 0x%x (0x%x) = 0x%x\n", 
		__func__, Path, HwRate, (offset_txagc[Path] + HwRate), PowerIndex));
	return TRUE;
}

/* ======================================================================== */

/* ======================================================================== */
/* These following functions can be used by driver*/

u4Byte
config_phydm_read_rf_reg_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		RFPath,
	IN	u4Byte					RegAddr,
	IN	u4Byte					BitMask
	)
{
	u4Byte	Readback_Value, Direct_Addr;
	u4Byte	offset_readRF[2] = {0x2800, 0x2c00};
	u4Byte	power_RF[2] = {0x1c, 0xec};

	/* Error handling.*/
	if (RFPath > ODM_RF_PATH_A) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: unsupported path (%d)\n", __func__, RFPath));
		return INVALID_RF_DATA;
	}

	/*  Error handling. Check if RF power is enable or not */
	/*  0xffffffff means RF power is disable */
	if (ODM_GetMACReg(pDM_Odm, power_RF[RFPath], bMaskByte3) != 0x7) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Read fail, RF is disabled\n", __func__));
		return INVALID_RF_DATA;
	}

	/* Calculate offset */
	RegAddr &= 0xff;
	Direct_Addr = offset_readRF[RFPath] + (RegAddr << 2);

	/* RF register only has 20bits */
	BitMask &= bRFRegOffsetMask;

	/* Read RF register directly */
	Readback_Value = ODM_GetBBReg(pDM_Odm, Direct_Addr, BitMask);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: RF-%d 0x%x = 0x%x, bit mask = 0x%x\n", 
		__func__, RFPath, RegAddr, Readback_Value, BitMask));
	return Readback_Value;
}

BOOLEAN
config_phydm_write_rf_reg_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		RFPath,
	IN	u4Byte					RegAddr,
	IN	u4Byte					BitMask,
	IN	u4Byte					Data
	)
{
	u4Byte	DataAndAddr = 0, Data_original = 0;
	u4Byte	offset_writeRF[2] = {0xc90, 0xe90};
	u4Byte	power_RF[2] = {0x1c, 0xec};
	u1Byte	BitShift;

	/* Error handling.*/
	if (RFPath > ODM_RF_PATH_A) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: unsupported path (%d)\n", __func__, RFPath));
		return FALSE;
	}

	/* Read RF register content first */
	RegAddr &= 0xff;
	BitMask = BitMask & bRFRegOffsetMask;

	if (BitMask != bRFRegOffsetMask) {
		Data_original = config_phydm_read_rf_reg_8821c(pDM_Odm, RFPath, RegAddr, bRFRegOffsetMask);

		/* Error handling. RF is disabled */
		if (config_phydm_read_rf_check_8821c(Data_original) == FALSE) {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Write fail, RF is disable\n", __func__));
			return FALSE;
		}

		/* check bit mask */
		if (BitMask != 0xfffff) {
			for (BitShift = 0; BitShift <= 19; BitShift++) {
				if (((BitMask >> BitShift) & 0x1) == 1)
					break;
			}
			Data = ((Data_original) & (~BitMask)) | (Data << BitShift);
		}
	} else if (ODM_GetMACReg(pDM_Odm, power_RF[RFPath], bMaskByte3) != 0x7) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Write fail, RF is disabled\n", __func__));
		return FALSE;
	}

	/* Put write addr in [27:20]  and write data in [19:00] */
	DataAndAddr = ((RegAddr<<20) | (Data&0x000fffff)) & 0x0fffffff;	

	/* Write Operation */
	ODM_SetBBReg(pDM_Odm, offset_writeRF[RFPath], bMaskDWord, DataAndAddr);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: RF-%d 0x%x = 0x%x (original: 0x%x), bit mask = 0x%x\n", 
		__func__, RFPath, RegAddr, Data, Data_original, BitMask));
	return TRUE;
}

BOOLEAN
config_phydm_write_txagc_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u4Byte					PowerIndex,
	IN	ODM_RF_RADIO_PATH_E		Path,	
	IN	u1Byte					HwRate
	)
{
	u4Byte	offset_txagc[2] = {0x1d00, 0x1d80};
	u1Byte	rate_idx = (HwRate & 0xfc), i;
	u4Byte	txagc_content = 0x0;

	/* Input need to be HW rate index, not driver rate index!!!! */

        if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: disable PHY API for debug!!\n", __func__));
		return TRUE;
	}

	/* Error handling  */
	if ((Path > ODM_RF_PATH_A) || (HwRate > 0x53)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: unsupported path (%d)\n", __func__, Path));
		return FALSE;
	}

	/* driver need to construct a 4-byte power index */
	ODM_SetBBReg(pDM_Odm, (offset_txagc[Path] + rate_idx), bMaskDWord, PowerIndex);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Path-%d Rate index 0x%x (0x%x) = 0x%x\n", 
		__func__, Path, HwRate, (offset_txagc[Path] + HwRate), PowerIndex));
	return TRUE;
}

u1Byte
config_phydm_read_txagc_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		Path,
	IN	u1Byte					HwRate
	)
{
	u1Byte	readBack_data;

	/* Input need to be HW rate index, not driver rate index!!!! */

	/* Error handling  */
	if ((Path > ODM_RF_PATH_A) || (HwRate > 0x53)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: unsupported path (%d)\n", __func__, Path));
		return INVALID_TXAGC_DATA;
	}

	/* Disable TX AGC report */
	ODM_SetBBReg(pDM_Odm, 0x1998, BIT16, 0x0);							/* need to check */

	/* Set data rate index (bit0~6) and path index (bit7) */
	ODM_SetBBReg(pDM_Odm, 0x1998, bMaskByte0, (HwRate|(Path << 7)));

	/* Enable TXAGC report */
	ODM_SetBBReg(pDM_Odm, 0x1998, BIT16, 0x1);

	/* Read TX AGC report */
	readBack_data = (u1Byte)ODM_GetBBReg(pDM_Odm, 0xd30, 0x7f0000);

	/* Driver have to disable TXAGC report after reading TXAGC (ref. user guide v11) */
	ODM_SetBBReg(pDM_Odm, 0x1998, BIT16, 0x0);	
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Path-%d rate index 0x%x = 0x%x\n", __func__, Path, HwRate, readBack_data));
	return readBack_data;
}

BOOLEAN
config_phydm_switch_band_8821c(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch
	)
{
	u4Byte		rf_reg18;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]======================>\n", __func__));	

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: disable PHY API for debug!!\n", __func__));
		return TRUE;
	}

	rf_reg18 = config_phydm_read_rf_reg_8821c(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8821c(rf_reg18);

	if (central_ch <= 14) {
		/* 2.4G */

		/* Enable CCK block */
		ODM_SetBBReg(pDM_Odm, 0x808, BIT28, 0x1);

		/* Disable MAC CCK check */
		ODM_SetBBReg(pDM_Odm, 0x454, BIT7, 0x0);

		/* Disable BB CCK check */
		ODM_SetBBReg(pDM_Odm, 0xa80, BIT18, 0x0);

		/*CCA Mask*/
		ODM_SetBBReg(pDM_Odm, 0x814, 0x0000FC00, 15); /*default value*/

		/* RF band */
		rf_reg18 = (rf_reg18 & (~(BIT16|BIT9|BIT8)));
	} else if (central_ch > 35) {
		/* 5G */

		/* Enable BB CCK check */
		ODM_SetBBReg(pDM_Odm, 0xa80, BIT18, 0x1);
		
		/* Enable CCK check */
		ODM_SetBBReg(pDM_Odm, 0x454, BIT7, 0x1);

		/* Disable CCK block */
		ODM_SetBBReg(pDM_Odm, 0x808, BIT28, 0x0);

		/*CCA Mask*/
		ODM_SetBBReg(pDM_Odm, 0x814, 0x0000FC00, 34); /*CCA mask = 13.6us*/

		/* RF band */
		rf_reg18 = (rf_reg18 & (~(BIT16|BIT9|BIT8)));
		rf_reg18 = (rf_reg18|BIT8|BIT16);
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Fail to switch band (ch: %d)\n", __func__, central_ch));
		return FALSE;
	}

	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8821c(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask, rf_reg18);

	if (phydm_rfe_8821c(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Fail to switch band (ch: %d), because writing RF register is fail\n", __func__, central_ch));	
		return FALSE;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Success to switch band (ch: %d)\n", __func__, central_ch));	
	return TRUE;
}

BOOLEAN
config_phydm_switch_channel_8821c(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch
	)
{
	pDIG_T		pDM_DigTable = &pDM_Odm->DM_DigTable;
	u4Byte		rf_reg18, rf_regB8 = 0;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]====================>\n", __func__));

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: disable PHY API for debug!!\n", __func__));
		return TRUE;
	}

	central_ch_8821c = central_ch;
	rf_reg18 = config_phydm_read_rf_reg_8821c(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8821c(rf_reg18);

	if (pDM_Odm->CutVersion == ODM_CUT_A) {
		rf_regB8 = config_phydm_read_rf_reg_8821c(pDM_Odm, ODM_RF_PATH_A, 0xb8, bRFRegOffsetMask);
		rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8821c(rf_regB8);
	}

	/* Switch band and channel */
	if (central_ch <= 14) {
		/* 2.4G */

		/* 1. RF band and channel*/
		rf_reg18 = (rf_reg18 & (~(BIT18|BIT17|bMaskByte0)));
		rf_reg18 = (rf_reg18|central_ch);

		/* 2. AGC table selection */
		ODM_SetBBReg(pDM_Odm, 0xc1c, 0x00000F00, 0x0);
		pDM_DigTable->agcTableIdx = 0x0;

		/* 3. Set central frequency for clock offset tracking */
		ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x96a);

		/* Fix A-cut LCK fail issue @ 5285MHz~5375MHz, 0xb8[19]=0x0 */
		if (pDM_Odm->CutVersion == ODM_CUT_A)
			rf_regB8 = rf_regB8 | BIT19;

	} else if (central_ch > 35) {
		/* 5G */

		/* 1. RF band and channel*/
		rf_reg18 = (rf_reg18 & (~(BIT18|BIT17|bMaskByte0)));
		rf_reg18 = (rf_reg18 | central_ch);

		if (central_ch >= 36 && central_ch <= 64)
			;
		else if ((central_ch >= 100) && (central_ch <= 140))
			rf_reg18 = (rf_reg18 | BIT17);
		else if (central_ch > 140)
			rf_reg18 = (rf_reg18 | BIT18);
		else {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Fail to switch channel (RF18) (ch: %d)\n", __func__, central_ch));
			return FALSE;
		 }

		/* 2. AGC table selection */
		if ((central_ch >= 36) && (central_ch <= 64)) {
			ODM_SetBBReg(pDM_Odm, 0xc1c, 0x00000F00, 0x1);
			pDM_DigTable->agcTableIdx = 0x1;
		} else if ((central_ch >= 100) && (central_ch <= 144)) {
			ODM_SetBBReg(pDM_Odm, 0xc1c, 0x00000F00, 0x2);
			pDM_DigTable->agcTableIdx = 0x2;
		} else if (central_ch >= 149) {
			ODM_SetBBReg(pDM_Odm, 0xc1c, 0x00000F00, 0x3);
			pDM_DigTable->agcTableIdx = 0x3;
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Fail to switch channel (AGC) (ch: %d)\n", __func__, central_ch));
			return FALSE;
		}

		/* 3. Set central frequency for clock offset tracking */
		if ((central_ch >= 36) && (central_ch <= 48))
			ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x494);
		else if ((central_ch >= 52) && (central_ch <= 64))
			ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x453);
		else if ((central_ch >= 100) && (central_ch <= 116))
			ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x452);
		else if ((central_ch >= 118) && (central_ch <= 177))
			ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x412);
		else {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Fail to switch channel (fc_area) (ch: %d)\n", __func__, central_ch));
			return FALSE;
		}

		/* Fix A-cut LCK fail issue @ 5285MHz~5375MHz, 0xb8[19]=0x0 */
		if (pDM_Odm->CutVersion == ODM_CUT_A) {
			if ((central_ch >= 57) && (central_ch <= 75))
				rf_regB8 = rf_regB8 & (~BIT19);
			else
				rf_regB8 = rf_regB8 | BIT19;
		}

	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Fail to switch band (ch: %d)\n", __func__, central_ch));
		return FALSE;
	}

	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8821c(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask, rf_reg18);

	if (pDM_Odm->CutVersion == ODM_CUT_A)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8821c(pDM_Odm, ODM_RF_PATH_A, 0xb8, bRFRegOffsetMask, rf_regB8);

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Fail to switch channel (ch: %d), because writing RF register is fail\n", __func__, central_ch));
		return FALSE;
	}

	phydm_ccapar_8821c(pDM_Odm);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Success to switch channel (ch: %d)\n", __func__, central_ch));
	return TRUE;
}

BOOLEAN
config_phydm_switch_bandwidth_8821c(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	)
{
	u4Byte		rf_reg18;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]===================>\n", __func__));

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: disable PHY API for debug!!\n", __func__));
		return TRUE;
	}

	/* Error handling  */
	if ((bandwidth >= ODM_BW_MAX) || ((bandwidth == ODM_BW40M) && (primary_ch_idx > 2)) || ((bandwidth == ODM_BW80M) && (primary_ch_idx > 4))) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Fail to switch bandwidth (bw: %d, primary ch: %d)\n", __func__, bandwidth, primary_ch_idx));
		return FALSE;
	}

	bw_8821c = bandwidth;
	rf_reg18 = config_phydm_read_rf_reg_8821c(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8821c(rf_reg18);

	/* Switch bandwidth */
	switch (bandwidth) {
	case ODM_BW20M:
	{
		/* Small BW([7:6]) = 0, primary channel ([5:2]) = 0, rf mode([1:0]) = 20M */
		ODM_SetBBReg(pDM_Odm, 0x8ac, bMaskByte0, ODM_BW20M);

		/* ADC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT9|BIT8), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT16, 0x1);

		/* DAC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT21|BIT20), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT28, 0x1);

		/* ADC buffer clock */
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT30, 0x1);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18 | BIT11 | BIT10);

		break;
	}
	case ODM_BW40M:
	{
		/* Small BW([7:6]) = 0, primary channel ([5:2]) = sub-channel, rf mode([1:0]) = 40M */
		ODM_SetBBReg(pDM_Odm, 0x8ac, bMaskByte0, (((primary_ch_idx & 0xf) << 2)|ODM_BW40M));

		/* CCK primary channel */
		if (primary_ch_idx == 1)
			ODM_SetBBReg(pDM_Odm, 0xa00, BIT4, primary_ch_idx);
		else
			ODM_SetBBReg(pDM_Odm, 0xa00, BIT4, 0);

		/* ADC clock = 160M clock for BW40 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT11|BIT10), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT17, 0x1);

		/* DAC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT23|BIT22), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT29, 0x1);

		/* ADC buffer clock */
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT30, 0x1);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18 & (~(BIT11|BIT10)));
		rf_reg18 = (rf_reg18|BIT11);		

		break;
	}
	case ODM_BW80M:
	{
		/* Small BW([7:6]) = 0, primary channel ([5:2]) = sub-channel, rf mode([1:0]) = 80M */
		ODM_SetBBReg(pDM_Odm, 0x8ac, bMaskByte0, (((primary_ch_idx & 0xf) << 2)|ODM_BW80M));

		/* ADC clock = 160M clock for BW80 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT13|BIT12), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT18, 0x1);

		/* DAC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT25|BIT24), 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT30, 0x1);

		/* ADC buffer clock */
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT30, 0x1);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18 & (~(BIT11|BIT10)));
		rf_reg18 = (rf_reg18|BIT10);

		break;
	}
	case ODM_BW5M:
	{
		/* Small BW([7:6]) = 1, primary channel ([5:2]) = 0, rf mode([1:0]) = 20M */
		ODM_SetBBReg(pDM_Odm, 0x8ac, bMaskByte0, (BIT6|ODM_BW20M));

		/* ADC clock = 40M clock */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT9|BIT8), 0x2);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT16, 0x0);

		/* DAC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT21|BIT20), 0x2);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT28, 0x0);

		/* ADC buffer clock */
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT30, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8c8, BIT31, 0x1);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18|BIT11|BIT10);

		break;
	}
	case ODM_BW10M:
	{
		/* Small BW([7:6]) = 1, primary channel ([5:2]) = 0, rf mode([1:0]) = 20M */
		ODM_SetBBReg(pDM_Odm, 0x8ac, bMaskByte0, (BIT7|ODM_BW20M));

		/* ADC clock = 80M clock */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT9|BIT8), 0x3);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT16, 0x0);

		/* DAC clock = 160M clock for BW20 */
		ODM_SetBBReg(pDM_Odm, 0x8ac, (BIT21|BIT20), 0x3);
		ODM_SetBBReg(pDM_Odm, 0x8ac, BIT28, 0x0);

		/* ADC buffer clock */
		ODM_SetBBReg(pDM_Odm, 0x8c4, BIT30, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x8c8, BIT31, 0x1);

		/* RF bandwidth */
		rf_reg18 = (rf_reg18|BIT11|BIT10);

		break;
	}
	default:
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Fail to switch bandwidth (bw: %d, primary ch: %d)\n", __func__, bandwidth, primary_ch_idx));
	}

	/* Write RF register */
	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8821c(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask, rf_reg18);

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Fail to switch bandwidth (bw: %d, primary ch: %d), because writing RF register is fail\n", __func__, bandwidth, primary_ch_idx));
		return FALSE;
	}

	/* Modify RX DFIR parameters */
	phydm_rxdfirpar_by_bw_8821c(pDM_Odm, bandwidth);

	/* Modify CCA parameters */
	phydm_ccapar_by_bw_8821c(pDM_Odm, bandwidth);
	phydm_ccapar_8821c(pDM_Odm);

	/* Toggle RX path to avoid RX dead zone issue */
	/*ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, 0x0);*/
	/*ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, 0x11);*/

	

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Success to switch bandwidth (bw: %d, primary ch: %d)\n", __func__, bandwidth, primary_ch_idx));
	return TRUE;
}

BOOLEAN
config_phydm_switch_channel_bw_8821c(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	)
{
	u1Byte			eRFPath = 0;
	u4Byte			RFValToWR , RFTmpVal, BitShift, BitMask;

	/* Switch band */
	if (config_phydm_switch_band_8821c(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	/* Switch channel */
	if (config_phydm_switch_channel_8821c(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	/* Switch bandwidth */
	if (config_phydm_switch_bandwidth_8821c(pDM_Odm, primary_ch_idx, bandwidth) == FALSE)
		return FALSE;

	return TRUE;
}

BOOLEAN
config_phydm_trx_mode_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_PATH_E			TxPath,
	IN	ODM_RF_PATH_E			RxPath,
	IN	BOOLEAN					bTx2Path
	)
{
	return TRUE;
}

BOOLEAN
config_phydm_parameter_init_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_PARAMETER_INIT_E	type
	)
{
	if (type == ODM_PRE_SETTING) {
		ODM_SetBBReg(pDM_Odm, 0x808, (BIT28|BIT29), 0x0);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Pre setting: disable OFDM and CCK block\n", __func__));
	} else if (type == ODM_POST_SETTING) {
		ODM_SetBBReg(pDM_Odm, 0x808, (BIT28|BIT29), 0x3);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Post setting: enable OFDM and CCK block\n", __func__));
		reg82c_8821c = ODM_GetBBReg(pDM_Odm, 0x82c, bMaskDWord);
		reg838_8821c = ODM_GetBBReg(pDM_Odm, 0x838, bMaskDWord);
		reg830_8821c = ODM_GetBBReg(pDM_Odm, 0x830, bMaskDWord);	
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: Wrong type!!\n", __func__));
		return FALSE;
	}

	return TRUE;
}

u4Byte
query_phydm_trx_capability_8821c(
	IN	PDM_ODM_T				pDM_Odm
){
	u4Byte value32 = 0x00000000;
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: trx_capability = 0x%x\n", __func__, value32));
	return value32;
}

u4Byte
query_phydm_stbc_capability_8821c(
	IN	PDM_ODM_T				pDM_Odm
){
	u4Byte value32 = 0x00010001;
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: stbc_capability = 0x%x\n", __func__, value32));
	return value32;
}

u4Byte
query_phydm_ldpc_capability_8821c(
	IN	PDM_ODM_T				pDM_Odm
){
	u4Byte value32 = 0x01000100;
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: ldpc_capability = 0x%x\n", __func__, value32));
	return value32;
}

u4Byte
query_phydm_txbf_parameters_8821c(
	IN	PDM_ODM_T				pDM_Odm
){
	u4Byte value32 = 0x00030003;
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: txbf_parameters = 0x%x\n", __func__, value32));
	return value32;
}

u4Byte
query_phydm_txbf_capability_8821c(
	IN	PDM_ODM_T				pDM_Odm
){
	u4Byte value32 = 0x01010001;
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("[%s]: txbf_capability = 0x%x\n", __func__, value32));
	return value32;
}

/* ======================================================================== */
#endif	/* RTL8821C_SUPPORT == 1 */

