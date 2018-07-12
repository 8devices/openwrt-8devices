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

#if (RTL8822B_SUPPORT == 1)  

/* ======================================================================== */
/* These following functions can be used for PHY DM only*/

ODM_BW_E	bw_8822b;
u1Byte	central_ch_8822b;

#if !(DM_ODM_SUPPORT_TYPE == ODM_CE)
	u32	cca_ifem_bcut[3][4] = {
		{0x75D97010, 0x75D97010, 0x75D97010, 0x75D97010}, /*Reg82C*/
		{0x79a0ea2a, 0x79a0ea2a, 0x79a0ea2a, 0x79a0ea2a}, /*Reg830*/
		{0x87766441, 0x87746341, 0x87765541, 0x87746341} /*Reg838*/
	};
	u32	cca_efem_bcut[3][4] = {
		{0x75B76010, 0x75B76010, 0x75B76010, 0x75B75010}, /*Reg82C*/
		{0x79a0ea2a, 0x79a0ea2a, 0x79a0ea2a, 0x79a0ea2a}, /*Reg830*/
		{0x87766451, 0x87766431, 0x87766451, 0x87766431} /*Reg838*/
	};
#endif

u32 cca_ifem_ccut[3][4] = {
	{0x75C97010, 0x75C97010, 0x75C97010, 0x75C97010}, /*Reg82C*/
	{0x79a0ea2a, 0x79A0EA2C, 0x79a0ea2a, 0x79a0ea2a}, /*Reg830*/
	{0x87765541, 0x87746341, 0x87765541, 0x87746341} /*Reg838*/
};
u32 cca_efem_ccut[3][4] = {
	{0x75B86010, 0x75B76010, 0x75B86010, 0x75B76010}, /*Reg82C*/
	{0x79A0EA28, 0x79A0EA2C, 0x79A0EA28, 0x79a0ea2a}, /*Reg830*/
	{0x87766451, 0x87766431, 0x87766451, 0x87766431} /*Reg838*/
};
u32 cca_ifem_ccut_rfetype[3][4] = {
	{0x75C97010, 0x75C97010, 0x75C97010, 0x75C97010}, /*Reg82C*/
	{0x79a0ea2a, 0x97A0EA2C, 0x79a0ea2a, 0x79a0ea2a}, /*Reg830*/
	{0x87765541, 0x86666341, 0x87765561, 0x86666361} /*Reg838*/
};

void
phydm_igi_toggle_8822b(
	IN	PDM_ODM_T				pDM_Odm
)
{
	u32 igi = 0x20;

	igi = ODM_GetBBReg(pDM_Odm, 0xc50, 0x7f);
	ODM_SetBBReg(pDM_Odm, 0xc50, 0x7f, (igi - 2));
	ODM_SetBBReg(pDM_Odm, 0xc50, 0x7f, igi);
	ODM_SetBBReg(pDM_Odm, 0xe50, 0x7f, (igi - 2));
	ODM_SetBBReg(pDM_Odm, 0xe50, 0x7f, igi);
}

BOOLEAN
phydm_rfe_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					channel
	)
{
	if ((pDM_Odm->RFEType == 4) || (pDM_Odm->RFEType == 11)) {

		/*TRSW  = trsw_forced_BT ? 0x804[0] : (0xCB8[2] ? 0xCB8[0] : trsw_lut);	trsw_lut = TXON*/
		/*TRSWB = trsw_forced_BT ? (~0x804[0]) : (0xCB8[2] ? 0xCB8[1] : trswb_lut);	trswb_lut = TXON*/
		/*trsw_forced_BT = 0x804[1] ? 0 : (~GNT_WL); */
		/*ODM_SetBBReg(pDM_Odm, 0x804, (BIT1|BIT0), 0x0);*/
		/* Default setting is in PHY parameters */
	
		if (channel <= 14) {
			/* signal source */
			ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x745774);
			ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x745774);
			ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x57);
			ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x57);

			/* inverse or not */
			ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x8);
			ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT11|BIT10), 0x2);
			ODM_SetBBReg(pDM_Odm, 0xebc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x8);
			ODM_SetBBReg(pDM_Odm, 0xebc, (BIT11|BIT10), 0x2);

			/* antenna switch table */
			if ((pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) || (pDM_Odm->TXAntStatus == (ODM_RF_A|ODM_RF_B))) {
				/* 2TX or 2RX */
				ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xf050);
				ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xf050);
			} else if (pDM_Odm->RXAntStatus == pDM_Odm->TXAntStatus) {
				/* TXA+RXA or TXB+RXB */
				ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xf055);
				ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xf055);
			} else {
				/* TXB+RXA or TXA+RXB */
				ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xf550);
				ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xf550);
			}
				
		} else if (channel > 35) {
			/* signal source */
			ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x477547);
			ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x477547);
			ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x75);
			ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x75);

			/* inverse or not */
			ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT11|BIT10), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xebc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xebc, (BIT11|BIT10), 0x0);

			/* antenna switch table */
			if ((pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) || (pDM_Odm->TXAntStatus == (ODM_RF_A|ODM_RF_B))) {
				/* 2TX or 2RX */
				ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa501);
				ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa501);
			} else if (pDM_Odm->RXAntStatus == pDM_Odm->TXAntStatus) {
				/* TXA+RXA or TXB+RXB */
				ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa500);
				ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa500);
			} else {
				/* TXB+RXA or TXA+RXB */
				ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa005);
				ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa005);
			}
		} else
			return FALSE;

		
	} 
	else if ((pDM_Odm->RFEType == 1) || (pDM_Odm->RFEType == 2) || (pDM_Odm->RFEType == 6) || (pDM_Odm->RFEType == 7) || (pDM_Odm->RFEType == 9)) {
		/* efem */
		if (((pDM_Odm->CutVersion == ODM_CUT_A) || (pDM_Odm->CutVersion == ODM_CUT_B)) && (pDM_Odm->RFEType < 2)) {
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

			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("%s: Using old RFE control pin setting for A-cut and B-cut\n", __func__));
		} else {
			/*After B-Cut*/
			if (channel <= 14) {
				/* signal source */
				ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x705770);
				ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x705770);
				ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x57);
				ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x57);
				ODM_SetBBReg(pDM_Odm, 0xcb8, BIT4, 0);
				ODM_SetBBReg(pDM_Odm, 0xeb8, BIT4, 0);
			} else if (channel > 35) {
				/* signal source */
				ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x177517);
				ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x177517);
				ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x75);
				ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x75);
				ODM_SetBBReg(pDM_Odm, 0xcb8, BIT5, 0);
				ODM_SetBBReg(pDM_Odm, 0xeb8, BIT5, 0);
			} else
				return FALSE;

			/* inverse or not */
			ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT11|BIT10), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xebc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xebc, (BIT11|BIT10), 0x0);

			/* delay 400ns for PAPE */
			/* ODM_SetBBReg(pDM_Odm, 0x810, bMaskByte3|BIT20|BIT21|BIT22|BIT23, 0x211); */

			/* antenna switch table */
			if ((pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) || (pDM_Odm->TXAntStatus == (ODM_RF_A|ODM_RF_B))) {
				/* 2TX or 2RX */
				ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa501);
				ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa501);
			} else if (pDM_Odm->RXAntStatus == pDM_Odm->TXAntStatus) {
				/* TXA+RXA or TXB+RXB */
				ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa500);
				ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa500);
			} else {
				/* TXB+RXA or TXA+RXB */
				ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa005);
				ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa005);
			}
		}
	} 
	else if ((pDM_Odm->RFEType == 0) || (pDM_Odm->RFEType == 3) || (pDM_Odm->RFEType == 5) || (pDM_Odm->RFEType == 8) || (pDM_Odm->RFEType == 10) || (pDM_Odm->RFEType == 12) || (pDM_Odm->RFEType == 13) || (pDM_Odm->RFEType == 14)) {
			/* iFEM */
			if (channel <= 14) {
				/* signal source */
				/*ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x705770);
				ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x705770);
				ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x57);
				ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x57);
				ODM_SetBBReg(pDM_Odm, 0xcb8, BIT4, 0);
				ODM_SetBBReg(pDM_Odm, 0xeb8, BIT4, 0);*/

				ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x745774);
				ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x745774);
				ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x57);
				ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x57);

			} else if (channel > 35) {
				/* signal source */
				/*ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x177517);
				ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x177517);
				ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x75);
				ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x75);
				ODM_SetBBReg(pDM_Odm, 0xcb8, BIT5, 1);
				ODM_SetBBReg(pDM_Odm, 0xeb8, BIT5, 1);*/

				ODM_SetBBReg(pDM_Odm, 0xcb0, (bMaskByte2|bMaskLWord), 0x477547);
				ODM_SetBBReg(pDM_Odm, 0xeb0, (bMaskByte2|bMaskLWord), 0x477547);
				ODM_SetBBReg(pDM_Odm, 0xcb4, bMaskByte1, 0x75);
				ODM_SetBBReg(pDM_Odm, 0xeb4, bMaskByte1, 0x75);
			} else
				return FALSE;

			/* inverse or not */
			ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xcbc, (BIT11|BIT10), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xebc, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xebc, (BIT11|BIT10), 0x0);

			/* antenna switch table */
			if (channel <= 14) {
				if ((pDM_Odm->RXAntStatus == (ODM_RF_A|ODM_RF_B)) || (pDM_Odm->TXAntStatus == (ODM_RF_A|ODM_RF_B))) {
					/* 2TX or 2RX */
					ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa501);
					ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa501);
				} else if (pDM_Odm->RXAntStatus == pDM_Odm->TXAntStatus) {
					/* TXA+RXA or TXB+RXB */
					ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa500);
					ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa500);
				} else {
					/* TXB+RXA or TXA+RXB */
					ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa005);
					ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa005);
				}
			} else if (channel > 35) {
				ODM_SetBBReg(pDM_Odm, 0xca0, bMaskLWord, 0xa5a5);
				ODM_SetBBReg(pDM_Odm, 0xea0, bMaskLWord, 0xa5a5);
			}
	}
	
	/* chip top mux */
	ODM_SetBBReg(pDM_Odm, 0x64, BIT29|BIT28, 0x3);
	ODM_SetBBReg(pDM_Odm, 0x4c, BIT26|BIT25, 0x0);
	ODM_SetBBReg(pDM_Odm, 0x40, BIT3, 0x1); 

	/* from s0 or s1 */
	ODM_SetBBReg(pDM_Odm, 0x1990, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x30);
	ODM_SetBBReg(pDM_Odm, 0x1990, (BIT11|BIT10), 0x3);

	/* input or output */
	ODM_SetBBReg(pDM_Odm, 0x974, (BIT5|BIT4|BIT3|BIT2|BIT1|BIT0), 0x3f);
	ODM_SetBBReg(pDM_Odm, 0x974, (BIT11|BIT10), 0x3);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("%s: Update RFE control pin setting (ch%d, TxPath 0x%x, RxPath 0x%x)\n", __func__, channel, pDM_Odm->TXAntStatus, pDM_Odm->RXAntStatus));

	return TRUE;
}

u8
phydm_is_dfs_channel(u8 channel_num)
{
	if(channel_num >= 52 && channel_num <= 140)
		return 1;
	else
		return 0;
}

VOID
phydm_ccapar_by_rfe_8822b(
	IN	PDM_ODM_T				pDM_Odm
	)
{
	u32	cca_ifem[3][4], cca_efem[3][4];
	u8	col;
	u32	reg82c, reg830, reg838;
	BOOLEAN	is_efem_cca = false, is_ifem_cca = false, is_rfe_type = false;

#if !(DM_ODM_SUPPORT_TYPE == ODM_CE)
	if (pDM_Odm->CutVersion == ODM_CUT_B) {
		ODM_MoveMemory(pDM_Odm, cca_efem, cca_efem_bcut, 12 * 4);
		ODM_MoveMemory(pDM_Odm, cca_ifem, cca_ifem_bcut, 12 * 4);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("%s: Update CCA parameters for Bcut\n", __func__));
	} else
#endif
	{
		ODM_MoveMemory(pDM_Odm, cca_efem, cca_efem_ccut, 12 * 4);
	if ((pDM_Odm->RFEType == 3) || (pDM_Odm->RFEType == 5)) {
		ODM_MoveMemory(pDM_Odm, cca_ifem, cca_ifem_ccut_rfetype, 12 * 4);
		is_rfe_type = true;
	} else
		ODM_MoveMemory(pDM_Odm, cca_ifem, cca_ifem_ccut, 12 * 4);
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("%s: Update CCA parameters for Ccut\n", __func__));
	}

	if (central_ch_8822b <= 14) {
		if ((pDM_Odm->RXAntStatus == ODM_RF_A) || (pDM_Odm->RXAntStatus == ODM_RF_B))
			col = 0;	/*1R 2G*/
		else
			col = 1;	/*2R 2G*/
	} else {
		if ((pDM_Odm->RXAntStatus == ODM_RF_A) || (pDM_Odm->RXAntStatus == ODM_RF_B))
			col = 2;	/*1R 5G*/
		else
			col = 3;	/*2R 5G*/
	}

	if ((pDM_Odm->RFEType == 1) || (pDM_Odm->RFEType == 4) || (pDM_Odm->RFEType == 6) || (pDM_Odm->RFEType == 7) || (pDM_Odm->RFEType == 11)) {
		/*eFEM => RFE type 1 & RFE type 4 & RFE type 6 & RFE type 7 & RFE type 11*/
		reg82c = cca_efem[0][col];
		reg830 = cca_efem[1][col];
		reg838 = cca_efem[2][col];
		is_efem_cca = true;
	} else if ((pDM_Odm->RFEType == 2) || (pDM_Odm->RFEType == 9)) {
		/*5G eFEM, 2G iFEM => RFE type 2, 5G eFEM => RFE type 9 */
		if (central_ch_8822b <= 14) {
			reg82c = cca_ifem[0][col];
			reg830 = cca_ifem[1][col];
			reg838 = cca_ifem[2][col];
			is_ifem_cca = true;
		} else {
			reg82c = cca_efem[0][col];
			reg830 = cca_efem[1][col];
			reg838 = cca_efem[2][col];
			is_efem_cca = true;
		}
	} else {
		/* iFEM =>RFEtype 3 & RFE type 5 & RFE type 0 & RFE type 8 & RFE type 10 & RFE type 12 & RFE type 13*/
		reg82c = cca_ifem[0][col];
		reg830 = cca_ifem[1][col];
		reg838 = cca_ifem[2][col];
		is_ifem_cca = true;
	}

	ODM_SetBBReg(pDM_Odm, 0x82c, bMaskDWord, reg82c);

	if (is_ifem_cca == true)
		if (((pDM_Odm->CutVersion == ODM_CUT_B) && (col == 1 || col == 3) && (bw_8822b == ODM_BW40M)) ||
			((is_rfe_type == false) && (col == 3) && (bw_8822b == ODM_BW40M)) ||
			((pDM_Odm->RFEType == 5) && (col == 3)))
			ODM_SetBBReg(pDM_Odm, 0x830, bMaskDWord, 0x79a0ea28);
		else
			ODM_SetBBReg(pDM_Odm, 0x830, bMaskDWord, reg830);
	else
		ODM_SetBBReg(pDM_Odm, 0x830, bMaskDWord, reg830);

	ODM_SetBBReg(pDM_Odm, 0x838, bMaskDWord, reg838);

	if (!(pDM_Odm->CutVersion == ODM_CUT_B))
		ODM_SetBBReg(pDM_Odm, 0x83c, bMaskDWord, 0x9194b2b9);

	if (phydm_is_dfs_channel(central_ch_8822b) && (bw_8822b == ODM_BW80M))
		ODM_SetBBReg(pDM_Odm, 0x838, 0x1, 0);
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("%s: (Pkt%d, Intf%d, RFE%d), col = %d\n",
		__func__, pDM_Odm->PackageType, pDM_Odm->SupportInterface, pDM_Odm->RFEType, col));
}

VOID
phydm_ccapar_by_bw_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_BW_E				bandwidth
	)
{
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
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("phydm_CcaParByBw_8822b(): Update CCA parameters for Acut\n"));
	
}

VOID
phydm_ccapar_by_rxpath_8822b(
	IN	PDM_ODM_T				pDM_Odm
	)
{

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
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("phydm_CcaParByRxPath_8822b(): Update CCA parameters for Acut\n"));

}

VOID
phydm_rxdfirpar_by_bw_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_BW_E				bandwidth
	)
{
	if (bandwidth == ODM_BW40M) {
		/* RX DFIR for BW40 */
		ODM_SetBBReg(pDM_Odm, 0x948, BIT29|BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x94c, BIT29|BIT28, 0x0);
		ODM_SetBBReg(pDM_Odm, 0xc20, BIT31, 0x0);
		ODM_SetBBReg(pDM_Odm, 0xe20, BIT31, 0x0);
	} else if (bandwidth == ODM_BW80M) {
		/* RX DFIR for BW80 */
		ODM_SetBBReg(pDM_Odm, 0x948, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0x94c, BIT29|BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0xc20, BIT31, 0x0);
		ODM_SetBBReg(pDM_Odm, 0xe20, BIT31, 0x0);
	} else {
		/* RX DFIR for BW20, BW10 and BW5*/
		ODM_SetBBReg(pDM_Odm, 0x948, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0x94c, BIT29|BIT28, 0x2);
		ODM_SetBBReg(pDM_Odm, 0xc20, BIT31, 0x1);
		ODM_SetBBReg(pDM_Odm, 0xe20, BIT31, 0x1);
	}
}

BOOLEAN
phydm_write_txagc_1byte_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u4Byte					PowerIndex,
	IN	ODM_RF_RADIO_PATH_E		Path,	
	IN	u1Byte					HwRate
	)
{
	u4Byte	offset_txagc[2] = {0x1d00, 0x1d80};
	u1Byte	rate_idx = (HwRate & 0xfc), i;
	u1Byte	rate_offset = (HwRate & 0x3);
	u4Byte	txagc_content = 0x0;

	/* For debug command only!!!! */

	/* Error handling  */
	if ((Path > ODM_RF_PATH_B) || (HwRate > 0x53)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("phydm_write_txagc_1byte_8822b(): unsupported path (%d)\n", Path));
		return FALSE;
	}

	/* For HW limitation, We can't write TXAGC once a byte. */
	for (i = 0; i < 4; i++) {
		if (i != rate_offset)
			txagc_content = txagc_content|(config_phydm_read_txagc_8822b(pDM_Odm, Path, rate_idx + i) << (i << 3));
		else
			txagc_content = txagc_content|((PowerIndex & 0x3f) << (i << 3));
	}
	ODM_SetBBReg(pDM_Odm, (offset_txagc[Path] + rate_idx), bMaskDWord, txagc_content);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("phydm_write_txagc_1byte_8822b(): Path-%d Rate index 0x%x (0x%x) = 0x%x\n", 
		Path, HwRate, (offset_txagc[Path] + HwRate), PowerIndex));
	return TRUE;
}

VOID
phydm_init_hw_info_by_rfe_type_8822b(
	IN	PDM_ODM_T				pDM_Odm
)
{
	u2Byte	mask_path_a = 0x0303;
	u2Byte	mask_path_b = 0x0c0c;
	/*u2Byte	mask_path_c = 0x3030;*/
	/*u2Byte	mask_path_d = 0xc0c0;*/

	pDM_Odm->bInitHwInfoByRfe = FALSE;
	/* 1R for path-b always used in AP platform */
	#if (DM_ODM_SUPPORT_TYPE == ODM_AP) 
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_NORMAL_RX_PATH_CHANGE, TRUE);
	#endif

	if ((pDM_Odm->RFEType == 1) || (pDM_Odm->RFEType == 6) || (pDM_Odm->RFEType == 7)) {
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, (ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_LNA_5G|ODM_BOARD_EXT_PA|ODM_BOARD_EXT_PA_5G));

		if (pDM_Odm->RFEType == 6) {
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GPA, (TYPE_GPA1 & (mask_path_a|mask_path_b)));
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_APA, (TYPE_APA1 & (mask_path_a|mask_path_b)));
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GLNA, (TYPE_GLNA1 & (mask_path_a|mask_path_b)));
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_ALNA, (TYPE_ALNA1 & (mask_path_a|mask_path_b)));
		} else if (pDM_Odm->RFEType == 7) {
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GPA, (TYPE_GPA2 & (mask_path_a|mask_path_b)));
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_APA, (TYPE_APA2 & (mask_path_a|mask_path_b)));
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GLNA, (TYPE_GLNA2 & (mask_path_a|mask_path_b)));
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_ALNA, (TYPE_ALNA2 & (mask_path_a|mask_path_b)));
		} else {
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GPA, (TYPE_GPA0 & (mask_path_a|mask_path_b)));
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_APA, (TYPE_APA0 & (mask_path_a|mask_path_b)));
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GLNA, (TYPE_GLNA0 & (mask_path_a|mask_path_b)));
			ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_ALNA, (TYPE_ALNA0 & (mask_path_a|mask_path_b)));
		}

		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 1);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, TRUE);
	} else if (pDM_Odm->RFEType == 2) {
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, (ODM_BOARD_EXT_LNA_5G|ODM_BOARD_EXT_PA_5G));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_APA, (TYPE_APA0 & (mask_path_a|mask_path_b)));	
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_ALNA, (TYPE_ALNA0 & (mask_path_a|mask_path_b)));	
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 2);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, TRUE);
	} else if (pDM_Odm->RFEType == 9) {
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, (ODM_BOARD_EXT_LNA_5G));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_ALNA, (TYPE_ALNA0 & (mask_path_a|mask_path_b)));	
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 1);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, FALSE);
	} else if (pDM_Odm->RFEType == 3) {
		/* RFE type 3: 8822BS\8822BU TFBGA iFEM */
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, 0);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 2);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, FALSE);
	} else if (pDM_Odm->RFEType == 5) {
		/* RFE type 5: 8822BE TFBGA iFEM */
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_SLIM);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 2);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, FALSE);
	} else if (pDM_Odm->RFEType == 12) {
		/* RFE type 12: QFN iFEM */
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, 0);

		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 1);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, false);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, false);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, false);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, false);
	} else if (pDM_Odm->RFEType == 4) {
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, (ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_LNA_5G|ODM_BOARD_EXT_PA|ODM_BOARD_EXT_PA_5G));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GPA, (TYPE_GPA0 & (mask_path_a|mask_path_b)));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_APA, (TYPE_APA0 & (mask_path_a|mask_path_b)));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GLNA, (TYPE_GLNA0 & (mask_path_a|mask_path_b)));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_ALNA, (TYPE_ALNA0 & (mask_path_a|mask_path_b)));	

		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 2);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, TRUE);
	} else if (pDM_Odm->RFEType == 11) {
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, (ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_LNA_5G|ODM_BOARD_EXT_PA|ODM_BOARD_EXT_PA_5G));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GPA, (TYPE_GPA1 & (mask_path_a|mask_path_b)));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_APA, (TYPE_APA1 & (mask_path_a|mask_path_b)));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_GLNA, (TYPE_GLNA1 & (mask_path_a|mask_path_b)));
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_ALNA, (TYPE_ALNA1 & (mask_path_a|mask_path_b)));	

		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 2);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, TRUE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, TRUE);
	} else if (pDM_Odm->RFEType == 8) {
		/* RFE Type 8: TFBGA iFEM AP */
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, 0);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 2);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, FALSE);
	} else if (pDM_Odm->RFEType == 10) {
		/* RFE Type 10: QFN iFEM AP PCIE */
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, ODM_BOARD_EXT_TRSW);
			
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 1);
			
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, FALSE);

	} else {
		/* RFE Type 0 & 13: QFN iFEM */
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_BOARD_TYPE, 0);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_PACKAGE_TYPE, 1);
		
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_LNA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_EXT_PA, FALSE);
		ODM_CmnInfoInit(pDM_Odm, ODM_CMNINFO_5G_EXT_PA, FALSE);
	}

	pDM_Odm->bInitHwInfoByRfe = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("phydm_init_hw_info_by_rfe_type_8822b(): RFE type (%d), Board type (0x%x), Package type (%d)\n", pDM_Odm->RFEType, pDM_Odm->BoardType, pDM_Odm->PackageType));
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("phydm_init_hw_info_by_rfe_type_8822b(): 5G ePA (%d), 5G eLNA (%d), 2G ePA (%d), 2G eLNA (%d)\n", pDM_Odm->ExtPA5G, pDM_Odm->ExtLNA5G, pDM_Odm->ExtPA, pDM_Odm->ExtLNA));
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("phydm_init_hw_info_by_rfe_type_8822b(): 5G PA type (%d), 5G LNA type (%d), 2G PA type (%d), 2G LNA type (%d)\n", pDM_Odm->TypeAPA, pDM_Odm->TypeALNA, pDM_Odm->TypeGPA, pDM_Odm->TypeGLNA));
}

s4Byte
phydm_get_condition_number_8822B(
	IN	PDM_ODM_T				pDM_Odm
)
{
	s4Byte	ret_val;

	ODM_SetBBReg( pDM_Odm, 0x1988, BIT22, 0x1);
	ret_val = (s4Byte)ODM_GetBBReg(pDM_Odm, 0xf84, (BIT17|BIT16|bMaskLWord));

	if (bw_8822b == 0) {
		ret_val = ret_val << (8 - 4);
		ret_val = ret_val / 234;
	} else if (bw_8822b == 1) {
		ret_val = ret_val << (7 - 4);
		ret_val = ret_val / 108;
	} else if (bw_8822b == 2) {
		ret_val = ret_val << (6 - 4);
		ret_val = ret_val / 52;
	}
	
	return ret_val;
}


/* ======================================================================== */

/* ======================================================================== */
/* These following functions can be used by driver*/

u4Byte
config_phydm_read_rf_reg_8822b(
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
	if (RFPath > ODM_RF_PATH_B) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8822b(): unsupported path (%d)\n", RFPath));
		return INVALID_RF_DATA;
	}

	/*  Error handling. Check if RF power is enable or not */
	/*  0xffffffff means RF power is disable */
	if (ODM_GetMACReg(pDM_Odm, power_RF[RFPath], bMaskByte3) != 0x7) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8822b(): Read fail, RF is disabled\n"));
		return INVALID_RF_DATA;
	}

	/* Calculate offset */
	RegAddr &= 0xff;
	Direct_Addr = offset_readRF[RFPath] + (RegAddr << 2);

	/* RF register only has 20bits */
	BitMask &= bRFRegOffsetMask;

	/* Read RF register directly */
	Readback_Value = ODM_GetBBReg(pDM_Odm, Direct_Addr, BitMask);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_rf_reg_8822b(): RF-%d 0x%x = 0x%x, bit mask = 0x%x\n", 
		RFPath, RegAddr, Readback_Value, BitMask));
	return Readback_Value;
}

BOOLEAN
config_phydm_write_rf_reg_8822b(
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
	if (RFPath > ODM_RF_PATH_B) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8822b(): unsupported path (%d)\n", RFPath));
		return FALSE;
	}

	/* Read RF register content first */
	RegAddr &= 0xff;
	BitMask = BitMask & bRFRegOffsetMask;

	if (BitMask != bRFRegOffsetMask) {
		Data_original = config_phydm_read_rf_reg_8822b(pDM_Odm, RFPath, RegAddr, bRFRegOffsetMask);

		/* Error handling. RF is disabled */
		if (config_phydm_read_rf_check_8822b(Data_original) == FALSE) {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8822b(): Write fail, RF is disable\n"));
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
	} else if (ODM_GetMACReg(pDM_Odm, power_RF[RFPath], bMaskByte3) != 0x7) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8822b(): Write fail, RF is disabled\n"));
		return FALSE;
	}

	/* Put write addr in [27:20]  and write data in [19:00] */
	DataAndAddr = ((RegAddr<<20) | (Data&0x000fffff)) & 0x0fffffff;	

	/* Write Operation */
	ODM_SetBBReg(pDM_Odm, offset_writeRF[RFPath], bMaskDWord, DataAndAddr);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_rf_reg_8822b(): RF-%d 0x%x = 0x%x (original: 0x%x), bit mask = 0x%x\n", 
		RFPath, RegAddr, Data, Data_original, BitMask));

	ODM_delay_us(2);
	return TRUE;
}

BOOLEAN
config_phydm_write_txagc_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u4Byte					PowerIndex,
	IN	ODM_RF_RADIO_PATH_E		Path,	
	IN	u1Byte					HwRate
	)
{
	u4Byte	offset_txagc[2] = {0x1d00, 0x1d80};
	u1Byte	rate_idx = (HwRate & 0xfc);

	/* Input need to be HW rate index, not driver rate index!!!! */

        if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8822b(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	/* Error handling  */
	if ((Path > ODM_RF_PATH_B) || (HwRate > 0x53)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8822b(): unsupported path (%d)\n", Path));
		return FALSE;
	}

	/* driver need to construct a 4-byte power index */
	ODM_SetBBReg(pDM_Odm, (offset_txagc[Path] + rate_idx), bMaskDWord, PowerIndex);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_write_txagc_8822b(): Path-%d Rate index 0x%x (0x%x) = 0x%x\n", 
		Path, HwRate, (offset_txagc[Path] + HwRate), PowerIndex));
	return TRUE;
}

u1Byte
config_phydm_read_txagc_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		Path,
	IN	u1Byte					HwRate
	)
{
	u1Byte	readBack_data;

	/* Input need to be HW rate index, not driver rate index!!!! */

	/* Error handling  */
	if ((Path > ODM_RF_PATH_B) || (HwRate > 0x53)) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_txagc_8822b(): unsupported path (%d)\n", Path));
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
	
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_read_txagc_8822b(): Path-%d rate index 0x%x = 0x%x\n", Path, HwRate, readBack_data));
	return readBack_data;
}

VOID
phydm_dynamic_spur_det_elimitor(
	IN	PDM_ODM_T				pDM_Odm
)
{
	u32		freq_2g[number_of_2g_freq_pt] = {0xFC67, 0xFC27, 0xFFE6, 0xFFA6, 0xFC67, 0xFCE7, 0xFCA7, 0xFC67, 0xFC27, 0xFFE6, 0xFFA6, 0xFF66, 0xFF26, 0xFCE7};
	u32		freq_5g[number_of_5g_freq_pt] = {0xFFC0, 0xFFC0, 0xFC81, 0xFC81, 0xFC41, 0xFC40, 0xFF80, 0xFF80, 0xFF40, 0xFD42};
	u32		freq_2g_n1[number_of_2g_freq_pt] = {0}, freq_2g_p1[number_of_2g_freq_pt] = {0};
	u32		freq_5g_n1[number_of_5g_freq_pt] = {0}, freq_5g_p1[number_of_5g_freq_pt] = {0};
	u32		freq_pt_2g_final = 0, freq_pt_5g_final = 0, freq_pt_2g_b_final = 0, freq_pt_5g_b_final = 0;
	u32		max_ret_psd_final = 0, max_ret_psd_b_final = 0;
	u32		max_ret_psd_2nd[number_of_sample] = {0}, max_ret_psd_b_2nd[number_of_sample] = {0};
	u32		psd_set[number_of_psd_value] = {0}, psd_set_B[number_of_psd_value] = {0};
	u32		rank_psd_index_in[number_of_psd_value] = {0}, rank_sample_index_in[number_of_sample] = {0};
	u32		rank_psd_index_out[number_of_psd_value] = {0};
	u32		rank_sample_index_out[number_of_sample] = {0};
	u32		reg_910_15_12 = 0;
	u8		j = 0, k = 0, threshold_nbi = 0x8D, threshold_csi = 0x8D;
	u8		idx = 0, set_result_nbi = SET_NO_NEED, set_result_csi = SET_NO_NEED;
	BOOLEAN	s_dopsd = false, s_donbi_a = false, s_docsi = false, s_donbi_b = false;

	if (!(pDM_Odm->SupportAbility & ODM_BB_DYNAMIC_PSDTOOL)) {
		ODM_RT_TRACE(pDM_Odm, ODM_BB_DYNAMIC_PSDTOOL, ODM_DBG_LOUD, ("[Return Init]   Not Support Dynamic Spur Detection and Eliminator\n"));
		return;
	}

	ODM_RT_TRACE(pDM_Odm, ODM_BB_DYNAMIC_PSDTOOL, ODM_DBG_LOUD, ("Dynamic Spur Detection and Eliminator is ON\n"));

	/* 2G Channel Setting > 20M: 5, 6, 7, 8, 13; 40M: 3~11 */
	if ((*pDM_Odm->pChannel >= 1) && (*pDM_Odm->pChannel <= 14)) {
		if (*pDM_Odm->pBandWidth == ODM_BW20M) {

			if (*pDM_Odm->pChannel >= 5 && *pDM_Odm->pChannel <= 8)
				idx = *pDM_Odm->pChannel - 5;
			else if (*pDM_Odm->pChannel == 13)
				idx = 4;
			else
				idx = 16;
		} else {

			if (*pDM_Odm->pChannel >= 3 && *pDM_Odm->pChannel <= 11)
				idx = *pDM_Odm->pChannel + 2;
			else
				idx = 16;
		}
	} else { /* 5G Channel Setting > 20M: 153, 161; 40M: 54, 118, 151, 159; 80M: 58, 122, 155, 155 */
		switch (*pDM_Odm->pChannel) {
		case 153:
			idx = 0;
			break;
		case 161:
			idx = 1;
			break;
		case 54:
			idx = 2;
			break;
		case 118:
			idx = 3;
			break;
		case 151:
			idx = 4;
			break;
		case 159:
			idx = 5;
			break;
		case 58:
			idx = 6;
			break;
		case 122:
			idx = 7;
			break;
		case 155:
			idx = 8;
			break;
		default:
			idx = 16;
			break;
		}
	}

	if (idx == 16)
		s_dopsd = false;
	else
		s_dopsd = true;

	ODM_RT_TRACE(pDM_Odm, ODM_BB_DYNAMIC_PSDTOOL, ODM_DBG_LOUD, ("[%s] idx = %d, BW = %d, Channel = %d\n", __func__, idx, *pDM_Odm->pBandWidth, *pDM_Odm->pChannel));

	for (k = 0; k < number_of_2g_freq_pt; k++) {
		freq_2g_n1[k] = freq_2g[k] - 1;
		freq_2g_p1[k] = freq_2g[k] + 1;
	}

	for (k = 0; k < number_of_5g_freq_pt; k++) {
		freq_5g_n1[k] = freq_5g[k] - 1;
		freq_5g_p1[k] = freq_5g[k] + 1;
	}

	if ((s_dopsd == TRUE) && (idx <= 13)) {
		for (k = 0; k < number_of_sample; k++) {
			if (k == 0) {
				freq_pt_2g_final = freq_2g_n1[idx];
				freq_pt_5g_final = freq_5g_n1[idx];
				freq_pt_2g_b_final = freq_2g_n1[idx] | BIT(16);
				freq_pt_5g_b_final = freq_5g_n1[idx] | BIT(16);
			} else if (k == 1) {
				freq_pt_2g_final = freq_2g[idx];
				freq_pt_5g_final = freq_5g[idx];
				freq_pt_2g_b_final = freq_2g[idx] | BIT(16);
				freq_pt_5g_b_final = freq_5g[idx] | BIT(16);
			} else if (k == 2) {
				freq_pt_2g_final = freq_2g_p1[idx];
				freq_pt_5g_final = freq_5g_p1[idx];
				freq_pt_2g_b_final = freq_2g_p1[idx] | BIT(16);
				freq_pt_5g_b_final = freq_5g_p1[idx] | BIT(16);
			}

			for (j = 0; j < number_of_psd_value; j++) {
				ODM_SetBBReg(pDM_Odm, 0xc00, bMaskByte0, 0x4);/* disable 3-wire, path-A */
				ODM_SetBBReg(pDM_Odm, 0xe00, bMaskByte0, 0x4);/* disable 3-wire, path-B */
				reg_910_15_12 = ODM_GetBBReg(pDM_Odm, 0x910, (BIT15 | BIT14 | BIT13 | BIT12));

				if (pDM_Odm->RXAntStatus & ODM_RF_A) {
					ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, (((ODM_RF_A)<<4) | ODM_RF_A));/*path-A*/

					if ((*pDM_Odm->pChannel >= 1) && (*pDM_Odm->pChannel <= 14))
						ODM_SetBBReg(pDM_Odm, 0x910, bMaskDWord, BIT(22) | freq_pt_2g_final);/* Start PSD */
					else
						ODM_SetBBReg(pDM_Odm, 0x910, bMaskDWord, BIT(22) | freq_pt_5g_final);/* Start PSD */

					ODM_delay_us(500);

					psd_set[j] = ODM_GetBBReg(pDM_Odm, 0xf44, bMaskLWord);

					ODM_SetBBReg(pDM_Odm, 0x910, BIT22, 0x0);/* turn off PSD */
				}

				if (pDM_Odm->RXAntStatus & ODM_RF_B) {
					ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, (((ODM_RF_B)<<4) | ODM_RF_B));/*path-B*/

					if ((*pDM_Odm->pChannel > 0) && (*pDM_Odm->pChannel <= 14))
						ODM_SetBBReg(pDM_Odm, 0x910, bMaskDWord, BIT(22) | freq_pt_2g_b_final);/* Start PSD */
					else
						ODM_SetBBReg(pDM_Odm, 0x910, bMaskDWord, BIT(22) | freq_pt_5g_b_final);/* Start PSD */

					ODM_delay_us(500);

					psd_set_B[j] = ODM_GetBBReg(pDM_Odm, 0xf44, bMaskLWord);

					ODM_SetBBReg(pDM_Odm, 0x910, BIT22, 0x0);/* turn off PSD */
				}

				ODM_SetBBReg(pDM_Odm, 0xc00, bMaskByte0, 0x7);/*eanble 3-wire*/
				ODM_SetBBReg(pDM_Odm, 0xe00, bMaskByte0, 0x7);
				ODM_SetBBReg(pDM_Odm, 0x910, (BIT15 | BIT14 | BIT13 | BIT12), reg_910_15_12);

				ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, (((pDM_Odm->RXAntStatus)<<4) | pDM_Odm->RXAntStatus));

				/* Toggle IGI to let RF enter RX mode, because BB doesn't send 3-wire command when RX path is enable */
				phydm_igi_toggle_8822b(pDM_Odm);

			}
			if (pDM_Odm->RXAntStatus & ODM_RF_A) {
				phydm_seq_sorting(pDM_Odm, psd_set, rank_psd_index_in, rank_psd_index_out, number_of_psd_value);
				max_ret_psd_2nd[k] = psd_set[0];
			}
			if (pDM_Odm->RXAntStatus & ODM_RF_B) {
				phydm_seq_sorting(pDM_Odm, psd_set_B, rank_psd_index_in, rank_psd_index_out, number_of_psd_value);
				max_ret_psd_b_2nd[k] = psd_set_B[0];
			}
		}

		if (pDM_Odm->RXAntStatus & ODM_RF_A) {
			phydm_seq_sorting(pDM_Odm, max_ret_psd_2nd, rank_sample_index_in, rank_sample_index_out, number_of_sample);
			max_ret_psd_final = max_ret_psd_2nd[0];

			if (max_ret_psd_final >= threshold_nbi)
				s_donbi_a = true;
			else
				s_donbi_a = false;
		}
		if (pDM_Odm->RXAntStatus & ODM_RF_B) {
			phydm_seq_sorting(pDM_Odm, max_ret_psd_b_2nd, rank_sample_index_in, rank_sample_index_out, number_of_sample);
			max_ret_psd_b_final = max_ret_psd_b_2nd[0];

			if (max_ret_psd_b_final >= threshold_nbi)
				s_donbi_b = true;
			else
				s_donbi_b = false;
		}

		ODM_RT_TRACE(pDM_Odm, ODM_BB_DYNAMIC_PSDTOOL, ODM_DBG_LOUD, ("[%s] max_ret_psd_final = %d, max_ret_psd_b_final = %d\n", __func__, max_ret_psd_final, max_ret_psd_b_final));

		if ((max_ret_psd_final >= threshold_csi) || (max_ret_psd_b_final >= threshold_csi))
			s_docsi = true;
		else
			s_docsi = false;

	}

	/* Reset NBI/CSI everytime after changing channel/BW/band  */
	ODM_SetBBReg(pDM_Odm, 0x880, bMaskDWord, 0);
	ODM_SetBBReg(pDM_Odm, 0x884, bMaskDWord, 0);
	ODM_SetBBReg(pDM_Odm, 0x888, bMaskDWord, 0);
	ODM_SetBBReg(pDM_Odm, 0x88c, bMaskDWord, 0);
	ODM_SetBBReg(pDM_Odm, 0x890, bMaskDWord, 0);
	ODM_SetBBReg(pDM_Odm, 0x894, bMaskDWord, 0);
	ODM_SetBBReg(pDM_Odm, 0x898, bMaskDWord, 0);
	ODM_SetBBReg(pDM_Odm, 0x89c, bMaskDWord, 0);
	ODM_SetBBReg(pDM_Odm, 0x874, BIT(0), 0x0);

	ODM_SetBBReg(pDM_Odm, 0x87c, BIT(13), 0x0);
	ODM_SetBBReg(pDM_Odm, 0xc20, BIT(28), 0x0);
	ODM_SetBBReg(pDM_Odm, 0xe20, BIT(28), 0x0);

	if (s_donbi_a == true || s_donbi_b == true) {
		if (*pDM_Odm->pBandWidth == ODM_BW20M) {
			if (*pDM_Odm->pChannel == 153)
				set_result_nbi = phydm_nbi_setting(pDM_Odm, NBI_ENABLE, *pDM_Odm->pChannel, 20, 5760, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel == 161)
				set_result_nbi = phydm_nbi_setting(pDM_Odm, NBI_ENABLE, *pDM_Odm->pChannel, 20, 5800, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel >= 5 && *pDM_Odm->pChannel <= 8)
				set_result_nbi = phydm_nbi_setting(pDM_Odm, NBI_ENABLE, *pDM_Odm->pChannel, 20, 2440, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel == 13)
				set_result_nbi = phydm_nbi_setting(pDM_Odm, NBI_ENABLE, *pDM_Odm->pChannel, 20, 2480, PHYDM_DONT_CARE);
			else
				set_result_nbi = SET_NO_NEED;
		} else if (*pDM_Odm->pBandWidth == ODM_BW40M) {
			if (*pDM_Odm->pChannel == 54) {
				set_result_nbi = phydm_nbi_setting(pDM_Odm, NBI_ENABLE, *pDM_Odm->pChannel, 40, 5280, PHYDM_DONT_CARE);
			} else if (*pDM_Odm->pChannel == 118) {
				set_result_nbi = phydm_nbi_setting(pDM_Odm, NBI_ENABLE, *pDM_Odm->pChannel, 40, 5600, PHYDM_DONT_CARE);
			} else if (*pDM_Odm->pChannel == 151) {
				set_result_nbi = phydm_nbi_setting(pDM_Odm, NBI_ENABLE, *pDM_Odm->pChannel, 40, 5760, PHYDM_DONT_CARE);
			} else if (*pDM_Odm->pChannel == 159) {
				set_result_nbi = phydm_nbi_setting(pDM_Odm, NBI_ENABLE, *pDM_Odm->pChannel, 40, 5800, PHYDM_DONT_CARE);
				/* 2.4G */
			} else if ((*pDM_Odm->pChannel >= 4) && (*pDM_Odm->pChannel <= 6)) {
				set_result_nbi = phydm_nbi_setting(pDM_Odm, NBI_ENABLE, *pDM_Odm->pChannel, 40, 2440, PHYDM_DONT_CARE);
			} else if (*pDM_Odm->pChannel == 11) {
				set_result_nbi = phydm_nbi_setting(pDM_Odm, NBI_ENABLE, *pDM_Odm->pChannel, 40, 2480, PHYDM_DONT_CARE);
			} else
				set_result_nbi = SET_NO_NEED;
		} else
			set_result_nbi = SET_NO_NEED;
	}

	if (s_docsi == true) {
		if (*pDM_Odm->pBandWidth == ODM_BW20M) {
			if (*pDM_Odm->pChannel == 153)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 20, 5760, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel == 161)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 20, 5800, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel >= 5 && *pDM_Odm->pChannel <= 8)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 20, 2440, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel == 13)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 20, 2480, PHYDM_DONT_CARE);
			else
				set_result_csi = SET_NO_NEED;
		} else if (*pDM_Odm->pBandWidth == ODM_BW40M) {
			if (*pDM_Odm->pChannel == 54)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 40, 5280, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel == 118)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 40, 5600, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel == 151)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 40, 5760, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel == 159)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 40, 5800, PHYDM_DONT_CARE);
			else if ((*pDM_Odm->pChannel >= 3) && (*pDM_Odm->pChannel <= 10))
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 40, 2440, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel == 11)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 40, 2480, PHYDM_DONT_CARE);
			else
				set_result_csi = SET_NO_NEED;
		} else if (*pDM_Odm->pBandWidth == ODM_BW80M) {
			if (*pDM_Odm->pChannel == 58)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 80, 5280, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel == 122)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 80, 5600, PHYDM_DONT_CARE);
			else if (*pDM_Odm->pChannel == 155)
				set_result_csi = phydm_csi_mask_setting(pDM_Odm, CSI_MASK_ENABLE, *pDM_Odm->pChannel, 80, 5760, PHYDM_DONT_CARE);
			else
				set_result_csi = SET_NO_NEED;
		} else
			set_result_csi = SET_NO_NEED;
	}

}

BOOLEAN
config_phydm_switch_band_8822b(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch
	)
{
	u4Byte		rf_reg18;
	BOOLEAN		rf_reg_status = TRUE;
	u4Byte		reg_8;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_band_8822b()======================>\n"));	

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_band_8822b(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	rf_reg18 = config_phydm_read_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8822b(rf_reg18);

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

		/* RxHP dynamic control */
		if ((pDM_Odm->RFEType == 2) || (pDM_Odm->RFEType == 3) || (pDM_Odm->RFEType == 5)) {
			ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108492);
			ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x1);
		}
		
		/* QFN eFEM RxHP are always low at 2G */
		reg_8 = ODM_GetBBReg(pDM_Odm, 0x19a8, BIT31);

		/* SoML on */
		if (reg_8 == 0x1) {
			ODM_SetBBReg(pDM_Odm, 0xc04, (BIT18|BIT21), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xe04, (BIT18|BIT21), 0x0);
			if ((pDM_Odm->RFEType == 3) || (pDM_Odm->RFEType == 5) || (pDM_Odm->RFEType == 8)) {
				ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108492);
				ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x1);
			} else {
				ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108000);
				ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x0);
			}
		}
		
		/* SoML off */
		if (reg_8 == 0x0) {
			ODM_SetBBReg(pDM_Odm, 0xc04, (BIT18|BIT21), 0x0); 
			ODM_SetBBReg(pDM_Odm, 0xe04, (BIT18|BIT21), 0x0); 
			if ((pDM_Odm->RFEType == 1) || (pDM_Odm->RFEType == 6) || (pDM_Odm->RFEType == 7) || (pDM_Odm->RFEType == 9)) {
				ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108000);
				ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x0);
			} else {
				ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108492);
				ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x1);
			}
		}
		
	} else if (central_ch > 35) {
		/* 5G */

		/* Enable BB CCK check */
		ODM_SetBBReg(pDM_Odm, 0xa80, BIT18, 0x1);
		
		/* Enable CCK check */
		ODM_SetBBReg(pDM_Odm, 0x454, BIT7, 0x1);

		/* Disable CCK block */
		ODM_SetBBReg(pDM_Odm, 0x808, BIT28, 0x0);

		/*CCA Mask*/
		if ((!pDM_Odm->WIFITest==1) || ((DM_ODM_SUPPORT_TYPE == ODM_AP) && (pDM_Odm->WIFITest)))
			ODM_SetBBReg(pDM_Odm, 0x814, 0x0000FC00, 34); /*CCA mask = 13.6us*/
		else
			ODM_SetBBReg(pDM_Odm, 0x814, 0x0000FC00, 15); /*default value*/

		/* RF band */
		rf_reg18 = (rf_reg18 & (~(BIT16|BIT9|BIT8)));
		rf_reg18 = (rf_reg18|BIT8|BIT16);

		/* RxHP dynamic control */
		reg_8 = ODM_GetBBReg(pDM_Odm, 0x19a8, BIT31);

		/* SoML on */
		if (reg_8 == 0x1) {
			ODM_SetBBReg(pDM_Odm, 0xc04, (BIT18|BIT21), 0x0);
			ODM_SetBBReg(pDM_Odm, 0xe04, (BIT18|BIT21), 0x0);
			ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108000);
			ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x0);
		}
		
		/* SoML off */
		if (reg_8 == 0x0) {
			ODM_SetBBReg(pDM_Odm, 0xc04, (BIT18|BIT21), 0x0); 
			ODM_SetBBReg(pDM_Odm, 0xe04, (BIT18|BIT21), 0x0); 
			if ((pDM_Odm->RFEType == 1) || (pDM_Odm->RFEType == 6) || (pDM_Odm->RFEType == 7) || (pDM_Odm->RFEType == 9)) {
				ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108000);
				ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x0);
			} else {
				ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108492);
				ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x1);
			}
		}

	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_band_8822b(): Fail to switch band (ch: %d)\n", central_ch));
		return FALSE;
	}

	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask, rf_reg18);

	if (pDM_Odm->RFType > ODM_1T1R)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_B, 0x18, bRFRegOffsetMask, rf_reg18);

	if (phydm_rfe_8822b(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_band_8822b(): Fail to switch band (ch: %d), because writing RF register is fail\n", central_ch));	
		return FALSE;
	}

	/* Dynamic spur detection by PSD and NBI/CSI mask */
	if (pDM_Odm->priv->pshare->rf_ft_var.mp_specific)
		phydm_dynamic_spur_det_elimitor(pDM_Odm);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_band_8822b(): Success to switch band (ch: %d)\n", central_ch));	
	return TRUE;
}

BOOLEAN
config_phydm_switch_channel_8822b(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch
	)
{
	pDIG_T		pDM_DigTable = &pDM_Odm->DM_DigTable;
	u4Byte		rf_reg18 = 0, rf_regB8 = 0, rf_regBE = 0xff;
	BOOLEAN		rf_reg_status = TRUE;
	u1Byte		low_band[15] = {0x7, 0x6, 0x6, 0x5, 0x0, 0x0, 0x7, 0xff, 0x6, 0x5, 0x0, 0x0, 0x7, 0x6, 0x6};
	u1Byte		middle_band[23] = {0x6, 0x5, 0x0, 0x0, 0x7, 0x6, 0x6, 0xff, 0x0, 0x0, 0x7, 0x6, 0x6, 0x5, 0x0, 0xff, 0x7, 0x6, 0x6, 0x5, 0x0, 0x0, 0x7};
	u1Byte		high_band[15] = {0x5, 0x5, 0x0, 0x7, 0x7, 0x6, 0x5, 0xff, 0x0, 0x7, 0x7, 0x6, 0x5, 0x5, 0x0};

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b()====================>\n"));

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	central_ch_8822b = central_ch;
	rf_reg18 = config_phydm_read_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8822b(rf_reg18);
	rf_reg18 = (rf_reg18 & (~(BIT18|BIT17|bMaskByte0)));

	if (pDM_Odm->CutVersion == ODM_CUT_A) {
		rf_regB8 = config_phydm_read_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xb8, bRFRegOffsetMask);
		rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8822b(rf_regB8);
	}

	/* Switch band and channel */
	if (central_ch <= 14) {
		/* 2.4G */

		/* 1. RF band and channel*/
		rf_reg18 = (rf_reg18|central_ch);

		/* 2. AGC table selection */
		ODM_SetBBReg(pDM_Odm, 0x958, 0x1f, 0x0);
		pDM_DigTable->agcTableIdx = 0x0;

		/* 3. Set central frequency for clock offset tracking */
		ODM_SetBBReg(pDM_Odm, 0x860, 0x1ffe0000, 0x96a);

		/* Fix A-cut LCK fail issue @ 5285MHz~5375MHz, 0xb8[19]=0x0 */
		if (pDM_Odm->CutVersion == ODM_CUT_A)
			rf_regB8 = rf_regB8 | BIT19;

		/* CCK TX filter parameters */
		if (central_ch == 14) {
			ODM_SetBBReg(pDM_Odm, 0xa24, bMaskDWord, 0x00006577);
			ODM_SetBBReg(pDM_Odm, 0xa28, bMaskLWord, 0x0000);
		} else {
			ODM_SetBBReg(pDM_Odm, 0xa24, bMaskDWord, 0x384f6577);
			ODM_SetBBReg(pDM_Odm, 0xa28, bMaskLWord, 0x1525);
		}

	} else if (central_ch > 35) {
		/* 5G */

		/* 1. RF band and channel*/
		rf_reg18 = (rf_reg18 | central_ch);

		/* 2. AGC table selection */
		if ((central_ch >= 36) && (central_ch <= 64)) {
			ODM_SetBBReg(pDM_Odm, 0x958, 0x1f, 0x1);
			pDM_DigTable->agcTableIdx = 0x1;
		} else if ((central_ch >= 100) && (central_ch <= 144)) {
			ODM_SetBBReg(pDM_Odm, 0x958, 0x1f, 0x2);
			pDM_DigTable->agcTableIdx = 0x2;
		} else if (central_ch >= 149) {
			ODM_SetBBReg(pDM_Odm, 0x958, 0x1f, 0x3);
			pDM_DigTable->agcTableIdx = 0x3;
		} else {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Fail to switch channel (AGC) (ch: %d)\n", central_ch));
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
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Fail to switch channel (fc_area) (ch: %d)\n", central_ch));
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
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Fail to switch channel (ch: %d)\n", central_ch));
		return FALSE;
	}

	/* Modify IGI for MP driver to aviod PCIE interference */
	if ((pDM_Odm->mp_mode == TRUE) && ((pDM_Odm->RFEType == 3) || (pDM_Odm->RFEType == 5))) {
		if (central_ch == 14)
			ODM_Write_DIG(pDM_Odm, 0x26);
		else
			ODM_Write_DIG(pDM_Odm, 0x20);
	}

	/* Modify the setting of register 0xBE to reduce phase noise */
	if (central_ch <= 14)
		rf_regBE = 0x0;
	else if ((central_ch >= 36) && (central_ch <= 64))
		rf_regBE = low_band[(central_ch - 36)>>1];
	else if ((central_ch >= 100) && (central_ch <= 144))
		rf_regBE = middle_band[(central_ch - 100)>>1];
	else if ((central_ch >= 149) && (central_ch <= 177))
		rf_regBE = high_band[(central_ch - 149)>>1];
	else
		rf_regBE = 0xff;

	if (rf_regBE != 0xff)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xbe, (BIT17|BIT16|BIT15), rf_regBE);
	else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Fail to switch channel (ch: %d, Phase noise)\n", central_ch));
		return FALSE;
	}

	/* Fix channel 144 issue, ask by RFSI Alvin*/
	/* 00 when freq < 5400;  01 when 5400<=freq<=5720; 10 when freq > 5720; 2G don't care*/
	/* need to set 0xdf[18]=1 before writing RF18 when channel 144 */
	if (central_ch == 144) {
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xdf, BIT18, 0x1);
		rf_reg18 = (rf_reg18 | BIT17);
	} else {
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xdf, BIT18, 0x0);

		if (central_ch > 144)
			rf_reg18 = (rf_reg18 | BIT18);
		else if (central_ch >= 80)
			rf_reg18 = (rf_reg18 | BIT17);
	}

	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask, rf_reg18);

	if (pDM_Odm->CutVersion == ODM_CUT_A)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xb8, bRFRegOffsetMask, rf_regB8);

	if (pDM_Odm->RFType > ODM_1T1R) {
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_B, 0x18, bRFRegOffsetMask, rf_reg18);

		if (pDM_Odm->CutVersion == ODM_CUT_A)
			rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_B, 0xb8, bRFRegOffsetMask, rf_regB8);
	}

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Fail to switch channel (ch: %d), because writing RF register is fail\n", central_ch));
		return FALSE;
	}

	/* Dynamic spur detection by PSD and NBI/CSI mask */
	if (pDM_Odm->priv->pshare->rf_ft_var.mp_specific)
		phydm_dynamic_spur_det_elimitor(pDM_Odm);

	/* Debug for RF resister reading error during synthesizer parameters parsing */
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xb8, BIT19, 0);
	ODM_SetRFReg(pDM_Odm, ODM_RF_PATH_A, 0xb8, BIT19, 1);
	
	phydm_ccapar_by_rfe_8822b(pDM_Odm);
	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_channel_8822b(): Success to switch channel (ch: %d)\n", central_ch));
	return TRUE;
}

BOOLEAN
config_phydm_switch_bandwidth_8822b(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	)
{
	u4Byte		rf_reg18;
	u1Byte		IGI;
	BOOLEAN		rf_reg_status = TRUE;

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b()===================>\n"));

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	/* Error handling  */
	if ((bandwidth >= ODM_BW_MAX) || ((bandwidth == ODM_BW40M) && (primary_ch_idx > 2)) || ((bandwidth == ODM_BW80M) && (primary_ch_idx > 4))) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b(): Fail to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
		return FALSE;
	}

	bw_8822b = bandwidth;
	rf_reg18 = config_phydm_read_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask);
	rf_reg_status = rf_reg_status & config_phydm_read_rf_check_8822b(rf_reg18);

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
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b(): Fail to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
	}

	/* Write RF register */
	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x18, bRFRegOffsetMask, rf_reg18);

	if (pDM_Odm->RFType > ODM_1T1R)
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_B, 0x18, bRFRegOffsetMask, rf_reg18);

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b(): Fail to switch bandwidth (bw: %d, primary ch: %d), because writing RF register is fail\n", bandwidth, primary_ch_idx));
		return FALSE;
	}

	/* Toggle IGI to let RF enter RX mode, because BB doesn't send 3-wire command when RX path is enable */
	IGI = (u1Byte)ODM_GetBBReg(pDM_Odm, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm));
	ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm), IGI - 2);
	ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B, pDM_Odm), ODM_BIT(IGI, pDM_Odm), IGI - 2);
	ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm), IGI);
	ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B, pDM_Odm), ODM_BIT(IGI, pDM_Odm), IGI);
	
	/* Modify RX DFIR parameters */
	phydm_rxdfirpar_by_bw_8822b(pDM_Odm, bandwidth);

	/* Dynamic spur detection by PSD and NBI/CSI mask */
	if (pDM_Odm->priv->pshare->rf_ft_var.mp_specific)
		phydm_dynamic_spur_det_elimitor(pDM_Odm);

	/* Modify CCA parameters */
	phydm_ccapar_by_bw_8822b(pDM_Odm, bandwidth);
	phydm_ccapar_by_rfe_8822b(pDM_Odm);

	/* Toggle RX path to avoid RX dead zone issue */
	ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, 0x0);
	ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, (pDM_Odm->RXAntStatus|(pDM_Odm->RXAntStatus<<4)));

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_switch_bandwidth_8822b(): Success to switch bandwidth (bw: %d, primary ch: %d)\n", bandwidth, primary_ch_idx));
	return TRUE;
}

BOOLEAN
config_phydm_switch_channel_bw_8822b(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	)
{

	/* Switch band */
	if (config_phydm_switch_band_8822b(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	/* Switch channel */
	if (config_phydm_switch_channel_8822b(pDM_Odm, central_ch) == FALSE)
		return FALSE;

	/* Switch bandwidth */
	if (config_phydm_switch_bandwidth_8822b(pDM_Odm, primary_ch_idx, bandwidth) == FALSE)
		return FALSE;

	return TRUE;
}

BOOLEAN
config_phydm_trx_mode_8822b(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_PATH_E			TxPath,
	IN	ODM_RF_PATH_E			RxPath,
	IN	BOOLEAN					bTx2Path
	)
{
	BOOLEAN		rf_reg_status = TRUE;
	u1Byte		IGI;
	u4Byte		rf_reg33 = 0;
	u2Byte		counter = 0;
	//PDM_ODM_T		pDM_Odm = (PDM_ODM_T)pDM_VOID;
	//PADAPTER		pAdapter	= pDM_Odm->Adapter;
	//PMGNT_INFO		pMgntInfo = &(pAdapter->MgntInfo);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b()=====================>\n"));	

	if (pDM_Odm->bDisablePhyApi) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): disable PHY API for debug!!\n"));
		return TRUE;
	}

	if ((TxPath & (~(ODM_RF_A|ODM_RF_B))) != 0) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Wrong TX setting (TX: 0x%x)\n", TxPath));
		return FALSE;
	}

	if ((RxPath & (~(ODM_RF_A|ODM_RF_B))) != 0) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Wrong RX setting (RX: 0x%x)\n", RxPath));
		return FALSE;
	}

	/* RF mode of path-A and path-B */
	/* Cannot shut down path-A, beacause synthesizer will be shut down when path-A is in shut down mode */
	if ((TxPath|RxPath) & ODM_RF_A)
		ODM_SetBBReg(pDM_Odm, 0xc08, bMaskLWord, 0x3231);
	else
		ODM_SetBBReg(pDM_Odm, 0xc08, bMaskLWord, 0x1111);

	if ((TxPath|RxPath) & ODM_RF_B)
		ODM_SetBBReg(pDM_Odm, 0xe08, bMaskLWord, 0x3231);
	else
		ODM_SetBBReg(pDM_Odm, 0xe08, bMaskLWord, 0x1111);

	/* Set TX antenna by Nsts */
	ODM_SetBBReg(pDM_Odm, 0x93c, (BIT19|BIT18), 0x3);
	ODM_SetBBReg(pDM_Odm, 0x80c, (BIT29|BIT28), 0x1);

	/* Control CCK TX path by 0xa07[7] */
	ODM_SetBBReg(pDM_Odm, 0x80c, BIT30, 0x1);

	/* TX logic map and TX path en for Nsts = 1, and CCK TX path*/	
	if (TxPath & ODM_RF_A) {
		ODM_SetBBReg(pDM_Odm, 0x93c, 0xfff00000, 0x001);
		ODM_SetBBReg(pDM_Odm, 0xa04, 0xf0000000, 0x8);
	} else if (TxPath & ODM_RF_B) {
		ODM_SetBBReg(pDM_Odm, 0x93c, 0xfff00000, 0x002);
		ODM_SetBBReg(pDM_Odm, 0xa04, 0xf0000000, 0x4);
	}
		
	/* TX logic map and TX path en for Nsts = 2*/
	if ((TxPath == ODM_RF_A) || (TxPath == ODM_RF_B))
		ODM_SetBBReg(pDM_Odm, 0x940, 0xfff0, 0x01);
	else
		ODM_SetBBReg(pDM_Odm, 0x940, 0xfff0, 0x43);

	/* TX path enable */
	ODM_SetBBReg(pDM_Odm, 0x80c, bMaskByte0, ((TxPath << 4)|TxPath));

	/* Tx2path for 1ss */
	if (!((TxPath == ODM_RF_A) || (TxPath == ODM_RF_B))) {
		if (bTx2Path || pDM_Odm->mp_mode) {
			/* 2Tx for OFDM */
			ODM_SetBBReg(pDM_Odm, 0x93c, 0xfff00000, 0x043);

			/* 2Tx for CCK */
			ODM_SetBBReg(pDM_Odm, 0xa04, 0xf0000000, 0xc);
		}
	}

	/* Always disable MRC for CCK CCA */
	ODM_SetBBReg(pDM_Odm, 0xa2c, BIT22, 0x0);

	/* Always disable MRC for CCK barker */
	ODM_SetBBReg(pDM_Odm, 0xa2c, BIT18, 0x0);

	/* CCK RX 1st and 2nd path setting*/	
	if (RxPath & ODM_RF_A)
		ODM_SetBBReg(pDM_Odm, 0xa04, 0x0f000000, 0x0);
	else if (RxPath & ODM_RF_B)
		ODM_SetBBReg(pDM_Odm, 0xa04, 0x0f000000, 0x5);

	/* RX path enable */
	ODM_SetBBReg(pDM_Odm, 0x808, bMaskByte0, ((RxPath << 4)|RxPath));

	if ((RxPath == ODM_RF_A) || (RxPath == ODM_RF_B)) {
		/* 1R */

		/* Disable MRC for CCA */
		/* ODM_SetBBReg(pDM_Odm, 0xa2c, BIT22, 0x0); */

		/* Disable MRC for barker */
		/* ODM_SetBBReg(pDM_Odm, 0xa2c, BIT18, 0x0); */
		
		/* Disable CCK antenna diversity */
		/* ODM_SetBBReg(pDM_Odm, 0xa00, BIT15, 0x0); */

		/* Disable Antenna weighting */
		ODM_SetBBReg(pDM_Odm, 0x1904, BIT16, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x800, BIT28, 0x0);
		ODM_SetBBReg(pDM_Odm, 0x850, BIT23, 0x0);
	} else {
		/* 2R */

		/* Enable MRC for CCA */
		/* ODM_SetBBReg(pDM_Odm, 0xa2c, BIT22, 0x1); */

		/* Enable MRC for barker */
		/* ODM_SetBBReg(pDM_Odm, 0xa2c, BIT18, 0x1); */

		/* Disable CCK antenna diversity */
		/* ODM_SetBBReg(pDM_Odm, 0xa00, BIT15, 0x0); */

		/* Enable Antenna weighting */
		ODM_SetBBReg(pDM_Odm, 0x1904, BIT16, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x800, BIT28, 0x1);
		ODM_SetBBReg(pDM_Odm, 0x850, BIT23, 0x1);

		/* AP only, L1 SBD change in 2R case */
		if (pDM_Odm->SupportPlatform == ODM_AP) {
			ODM_SetBBReg(pDM_Odm, 0x840, bMaskDWord, 0x171750E0);
			} 
	}

	/* Update TXRX antenna status for PHYDM */
	pDM_Odm->TXAntStatus =  (TxPath & 0x3);
	pDM_Odm->RXAntStatus =  (RxPath & 0x3);

	/* MP driver need to support path-B TX\RX */

	while(1){
		counter++;
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x80000);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x33, bRFRegOffsetMask, 0x00001);

		ODM_delay_us(2);
		rf_reg33 = config_phydm_read_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x33, bRFRegOffsetMask);
		
		if ((rf_reg33 == 0x00001) && (config_phydm_read_rf_check_8822b(rf_reg33)))
			break;
		else if (counter == 100) {
			ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Fail to set TRx mode setting, because writing RF mode table is fail\n"));
			return FALSE;
		}
	}

	if ((pDM_Odm->mp_mode) || (pDM_Odm->pAntennaTest) || (pDM_Odm->Normalrxpath)) {
		/*	0xef 0x80000  0x33 0x00001  0x3e 0x00034  0x3f 0x4080e  0xef 0x00000    suggested by Lucas*/
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x80000);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x33, bRFRegOffsetMask, 0x00001);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x3e, bRFRegOffsetMask, 0x00034);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x3f, bRFRegOffsetMask, 0x4080e);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x00000);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): MP mode or Antenna test mode!! support path-B TX and RX\n"));
	} else {
		/*	0xef 0x80000  0x33 0x00001  0x3e 0x00034  0x3f 0x4080c  0xef 0x00000 */
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x80000);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x33, bRFRegOffsetMask, 0x00001);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x3e, bRFRegOffsetMask, 0x00034);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0x3f, bRFRegOffsetMask, 0x4080c);
		rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x00000);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Normal mode!! Do not support path-B TX and RX\n"));
	}

	rf_reg_status = rf_reg_status & config_phydm_write_rf_reg_8822b(pDM_Odm, ODM_RF_PATH_A, 0xef, bRFRegOffsetMask, 0x00000);

	if (rf_reg_status == FALSE) {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Fail to set TRx mode setting (TX: 0x%x, RX: 0x%x), because writing RF register is fail\n", TxPath, RxPath));
		return FALSE;
	}

	/* Toggle IGI to let RF enter RX mode, because BB doesn't send 3-wire command when RX path is enable */
	//IGI = (u1Byte)ODM_GetBBReg(pDM_Odm, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm));
	//ODM_Write_DIG(pDM_Odm, IGI - 2);
	//ODM_Write_DIG(pDM_Odm, IGI);

	IGI = (u1Byte)ODM_GetBBReg(pDM_Odm, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm));
	ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm), IGI - 2);
	ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B, pDM_Odm), ODM_BIT(IGI, pDM_Odm), IGI - 2);
	ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_A, pDM_Odm), ODM_BIT(IGI, pDM_Odm), IGI);
	ODM_SetBBReg(pDM_Odm, ODM_REG(IGI_B, pDM_Odm), ODM_BIT(IGI, pDM_Odm), IGI);

	/* Modify CCA parameters */
	phydm_ccapar_by_rxpath_8822b(pDM_Odm);
	phydm_ccapar_by_rfe_8822b(pDM_Odm);
	phydm_rfe_8822b(pDM_Odm, central_ch_8822b);

	/* Dynamic spur detection by PSD and NBI/CSI mask */
	if (pDM_Odm->priv->pshare->rf_ft_var.mp_specific)
		phydm_dynamic_spur_det_elimitor(pDM_Odm);

	ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_trx_mode_8822b(): Success to set TRx mode setting (TX: 0x%x, RX: 0x%x)\n", TxPath, RxPath));
	return TRUE;
}


BOOLEAN
config_phydm_parameter_init(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_PARAMETER_INIT_E	type
	)
{
	if (type == ODM_PRE_SETTING) {
		ODM_SetBBReg(pDM_Odm, 0x808, (BIT28|BIT29), 0x0);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_init(): Pre setting: disable OFDM and CCK block\n"));
	} else if (type == ODM_POST_SETTING) {
		ODM_SetBBReg(pDM_Odm, 0x808, (BIT28|BIT29), 0x3);
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_init(): Post setting: enable OFDM and CCK block\n"));
	} else {
		ODM_RT_TRACE(pDM_Odm, ODM_PHY_CONFIG, ODM_DBG_TRACE, ("config_phydm_parameter_init(): Wrong type!!\n"));
		return FALSE;
	}

	//phydm_set_iqgen_8822B(pDM_Odm);
	return TRUE;
}

/* ======================================================================== */
#endif	/* RTL8822B_SUPPORT == 1 */

