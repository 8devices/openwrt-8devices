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

/*============================================================
// include files
============================================================*/

#include "mp_precomp.h"
#include "../phydm_precomp.h"

#if (RTL8197F_SUPPORT == 1)

s1Byte 
odm_CCKRSSI_8197F(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte	LNA_idx, 
	IN		u1Byte	VGA_idx
	)
{
	s1Byte	rx_pwr_all;
	s1Byte	diff_para;

	if ((pDM_Odm->BoardType == (ODM_BOARD_EXT_TRSW|ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_PA)) && (pDM_Odm->PackageType == 1))
		diff_para = -7;
	else
		diff_para = 7;
	
	switch (LNA_idx) {
	case 7:
		rx_pwr_all = -52 + diff_para  - 2*(VGA_idx);
		break;

	case 6:
		rx_pwr_all = -42 + diff_para - 2*(VGA_idx); 
		break;

	case 5:
		rx_pwr_all = -32 + diff_para - 2*(VGA_idx); 
		break;

	case 4:
		rx_pwr_all = -26 + diff_para - 2*(VGA_idx); 
		break;

	case 3:
		rx_pwr_all = -18 + diff_para - 2*(VGA_idx); 
		break;

	case 2:
		rx_pwr_all = -12 + diff_para - 2*(VGA_idx);
		break;

	case 1:
		rx_pwr_all = -2 + diff_para - 2*(VGA_idx);
		break;
		
	case 0:
		rx_pwr_all = 2 + diff_para - 2*(VGA_idx);
		break;

	default:
	break;			

	}

	return rx_pwr_all;

}

VOID
phydm_phypara_a_cut(
	IN		PDM_ODM_T		pDM_Odm
	)
{

	ODM_SetBBReg(pDM_Odm, 0x97c, 0xff000000, 0x20);/*97f A-cut workaround*/

	ODM_SetBBReg(pDM_Odm, 0xa9c, BIT17, 0);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xa0c, bMaskByte2, 0x7e);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xa0c, bMaskByte1, 0x0);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xa84, bMaskByte2, 0x1f);/*97f A-cut workaround*/

	ODM_SetBBReg(pDM_Odm, 0x824, BIT14, 0x1);/*pathA r_rxhp_tx*/
	ODM_SetBBReg(pDM_Odm, 0x824, BIT17, 0x1);/*pathA r_rxhp_t2r*/
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT14, 0x1);/*pathB r_rxhp_tx*/
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT17, 0x1);/*pathB r_rxhp_t2r*/
	ODM_SetBBReg(pDM_Odm, 0xc5c, BIT2, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc5c, BIT5, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc5c, BIT8, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc5c, BIT11, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc5c, BIT14, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xce0, BIT2, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc7c, BIT24, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc7c, BIT27, 0x1);/*97f A-cut workaround*/

}


VOID
phydm_dynamic_disable_ecs(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	PFALSE_ALARM_STATISTICS 	FalseAlmCnt = (PFALSE_ALARM_STATISTICS)PhyDM_Get_Structure( pDM_Odm, PHYDM_FALSEALMCNT);

	if ((pDM_Odm->is_disable_dym_ecs == TRUE) || (pDM_Odm->mp_mode == TRUE))			/*use mib to disable this dym function*/
		return;
	
	if ((pDM_Odm->RSSI_Min < 30) || (FalseAlmCnt->Cnt_all * 4 >= FalseAlmCnt->Cnt_CCA_all))
		ODM_SetBBReg(pDM_Odm, 0x9ac, BIT17, 0);
	else if ((pDM_Odm->RSSI_Min >= 34) && (FalseAlmCnt->Cnt_all * 5 <= FalseAlmCnt->Cnt_CCA_all))
		ODM_SetBBReg(pDM_Odm, 0x9ac, BIT17, 1);
}

void
phydm_dynamic_ant_weighting(
	PDM_ODM_T		p_dm_odm
)
{
	u8 rssi_l2h = 43, rssi_h2l = 37;

	if (p_dm_odm->is_disable_dym_ant_weighting)
		return;

	/* force AGC weighting */
	ODM_SetBBReg(p_dm_odm, 0xc54, BIT0, 1);
	/* MRC by AGC table */
	ODM_SetBBReg(p_dm_odm, 0xce8, BIT30, 1);
	/*Enable Enable antenna_weighting_shift mechanism*/
	ODM_SetBBReg(p_dm_odm, 0xd5c, BIT29, 1);

	if (p_dm_odm->RSSI_Min_ByPath != 0xFF) {
		if (p_dm_odm->RSSI_Min_ByPath >= rssi_l2h) {
			ODM_SetBBReg(p_dm_odm, 0xd5c, BIT31 | BIT30, 0);	/* equal weighting */
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_ANT_WEIGHTING, ODM_DBG_LOUD, ("RSSI_Min_ByPath = %d, is larger than rssi_l2h = %d, adapt equal weighting for 2SS\n",
				p_dm_odm->RSSI_Min_ByPath,
				rssi_l2h));
		}
		else if (p_dm_odm->RSSI_Min_ByPath <= rssi_h2l) {
			/*fix sec_min_wgt = 1/2*/
			ODM_SetBBReg(p_dm_odm, 0xd5c, BIT31, 0);
			ODM_SetBBReg(p_dm_odm, 0xd5c, BIT30, 1);
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_ANT_WEIGHTING, ODM_DBG_LOUD, ("RSSI_Min_ByPath = %d, is smaller than rssi_h2l = %d, adapt AGC weighting\n",
				p_dm_odm->RSSI_Min_ByPath,
				rssi_h2l));
		}
	} else {
		/*fix sec_min_wgt = 1/2*/
		ODM_SetBBReg(p_dm_odm, 0xd5c, BIT31, 0);
		ODM_SetBBReg(p_dm_odm, 0xd5c, BIT30, 1);		
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_ANT_WEIGHTING, ODM_DBG_LOUD, ("RSSI_Min_ByPath = %d, adapt AGC weighting\n",
			p_dm_odm->RSSI_Min_ByPath));
	}
}


phydm_hwsetting_8197f(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	phydm_dynamic_disable_ecs(pDM_Odm);
	phydm_dynamic_ant_weighting(pDM_Odm);
}

/*DSC(double side band) = 0 or 3*/
/*LSC(left side band) = 1*/
/*USC(right side band) = 2*/
phydm_api_primary_cca(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u8				DSC_LSC_USC
	)
{
	u32	dsc_lsc_usc;
	
	if(pDM_Odm->is_disable_primary_cca == TRUE)
		return;

	dsc_lsc_usc = ODM_GetBBReg(pDM_Odm, 0xc6c, BIT8|BIT7);

	if ((dsc_lsc_usc == 0) | (dsc_lsc_usc == 3))
		pDM_Odm->original_pd_threshold = ODM_GetBBReg(pDM_Odm, 0xc84, BIT31|BIT30|BIT29|BIT28);

	ODM_SetBBReg(pDM_Odm, 0xc6c, BIT8|BIT7, DSC_LSC_USC);
	
	if((DSC_LSC_USC == 0) | (DSC_LSC_USC == 3))
		ODM_SetBBReg(pDM_Odm, 0xc84, BIT31|BIT30|BIT29|BIT28, pDM_Odm->original_pd_threshold);
	else
		ODM_SetBBReg(pDM_Odm, 0xc84, BIT31|BIT30|BIT29|BIT28, 0);
}
#endif	/* RTL8197F_SUPPORT == 1 */

