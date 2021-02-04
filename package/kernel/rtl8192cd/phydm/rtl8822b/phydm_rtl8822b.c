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

#if (RTL8822B_SUPPORT == 1)


VOID
phydm_dynamic_switch_htstf_mumimo_8822b(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	/*if rssi > 40dBm, enable HT-STF gain controller, otherwise, if rssi < 40dBm, disable the controller*/
	/*add by Chun-Hung Ho 20160711 */
		if (pDM_Odm->RSSI_Min >= 40)
			ODM_SetBBReg(pDM_Odm, 0x8d8, BIT17, 0x1);
		else if (pDM_Odm->RSSI_Min < 35)
			ODM_SetBBReg(pDM_Odm, 0x8d8, BIT17, 0x0);

		ODM_RT_TRACE(pDM_Odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("%s, RSSI_Min = %d\n", __func__, pDM_Odm->RSSI_Min));
}		

VOID
phydm_somlrxhp_setting(
	IN		PDM_ODM_T		pDM_Odm,
	BOOLEAN switch_soml
)
{
	if (switch_soml == TRUE) {
		ODM_SetBBReg(pDM_Odm, 0x19a8, bMaskDWord, 0xd90a0000);
		/* Following are RxHP settings for T2R as always low, workaround for OTA test, required to classify */
		ODM_SetBBReg(pDM_Odm, 0xc04, (BIT21|BIT18), 0x0);
		ODM_SetBBReg(pDM_Odm, 0xe04, (BIT21|BIT18), 0x0);
	} else {
		ODM_SetBBReg(pDM_Odm, 0x19a8, bMaskDWord, 0x090a0000);
		ODM_SetBBReg(pDM_Odm, 0xc04, (BIT21|BIT18), 0x0);
		ODM_SetBBReg(pDM_Odm, 0xe04, (BIT21|BIT18), 0x0);
	}
		
	/* Dynamic RxHP setting with SoML on/off apply on all RFE type */
	if ((switch_soml == FALSE) && ((pDM_Odm->RFEType == 1) || (pDM_Odm->RFEType == 6) || (pDM_Odm->RFEType == 7) || (pDM_Odm->RFEType == 9))) {
		ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108000);
		ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x0);
	}
		
	if (*pDM_Odm->pChannel <= 14) {
		if ((switch_soml == TRUE) && (!((pDM_Odm->RFEType == 3) || (pDM_Odm->RFEType == 5) || (pDM_Odm->RFEType == 8)))) {
			ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108000);
			ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x0);
		}
	} else if (*pDM_Odm->pChannel > 35) {
		if (switch_soml == TRUE) {
			ODM_SetBBReg(pDM_Odm, 0x8cc, bMaskDWord, 0x08108000);
			ODM_SetBBReg(pDM_Odm, 0x8d8, BIT27, 0x0);
		}
	}

}

VOID
phydm_hwsetting_8822b(
	IN		PDM_ODM_T		pDM_Odm
	)
{
	phydm_dynamic_switch_htstf_mumimo_8822b(pDM_Odm);
}

#endif	/* RTL8822B_SUPPORT == 1 */

