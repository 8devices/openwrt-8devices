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
#if (RTL8197F_SUPPORT == 1)
#ifndef	__ODM_RTL8197F_H__
#define __ODM_RTL8197F_H__

s1Byte 
odm_CCKRSSI_8197F(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte	LNA_idx, 
	IN		u1Byte	VGA_idx
	);

VOID
phydm_phypara_a_cut(
	IN		PDM_ODM_T		pDM_Odm
	);

VOID
phydm_hwsetting_8197f(
	IN		PDM_ODM_T		pDM_Odm
	);

void
phydm_api_primary_cca(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u8				DSC_LSC_USC
	);


#endif	/* #define __ODM_RTL8197F_H__ */
#endif

