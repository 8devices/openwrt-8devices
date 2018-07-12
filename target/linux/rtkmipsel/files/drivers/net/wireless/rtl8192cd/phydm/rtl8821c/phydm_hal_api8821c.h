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
#ifndef __INC_PHYDM_API_H_8821C__
#define __INC_PHYDM_API_H_8821C__

#if (RTL8821C_SUPPORT == 1)

#define	PHY_CONFIG_VERSION_8821C			"2.1.5"	/*2016.04.12     (HW user guide version: R02, SW user guide version: R01, Modification: R06)*/

#define	INVALID_RF_DATA					0xffffffff
#define	INVALID_TXAGC_DATA				0xff

#define	config_phydm_read_rf_check_8821c(data)			(data != INVALID_RF_DATA)
#define	config_phydm_read_txagc_check_8821c(data)		(data != INVALID_TXAGC_DATA)

u4Byte
config_phydm_read_rf_reg_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		RFPath,
	IN	u4Byte					RegAddr,
	IN	u4Byte					BitMask
	);

BOOLEAN
config_phydm_write_rf_reg_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		RFPath,
	IN	u4Byte					RegAddr,
	IN	u4Byte					BitMask,
	IN	u4Byte					Data
	);

BOOLEAN
config_phydm_write_txagc_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u4Byte					PowerIndex,
	IN	ODM_RF_RADIO_PATH_E		Path,	
	IN	u1Byte					HwRate
	);

u1Byte
config_phydm_read_txagc_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E		Path,
	IN	u1Byte					HwRate
	);

BOOLEAN
config_phydm_switch_band_8821c(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch
	);

BOOLEAN
config_phydm_switch_channel_8821c(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch
	);

BOOLEAN
config_phydm_switch_bandwidth_8821c(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	);

BOOLEAN
config_phydm_switch_channel_bw_8821c(	
	IN	PDM_ODM_T				pDM_Odm,
	IN	u1Byte					central_ch,
	IN	u1Byte					primary_ch_idx,
	IN	ODM_BW_E				bandwidth
	);

BOOLEAN
config_phydm_trx_mode_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_RF_PATH_E			TxPath,
	IN	ODM_RF_PATH_E			RxPath,
	IN	BOOLEAN					bTx2Path
	);

BOOLEAN
config_phydm_parameter_init_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	ODM_PARAMETER_INIT_E	type
	);


/* ======================================================================== */
/* These following functions can be used for PHY DM only*/

BOOLEAN
phydm_write_txagc_1byte_8821c(
	IN	PDM_ODM_T				pDM_Odm,
	IN	u4Byte					PowerIndex,
	IN	ODM_RF_RADIO_PATH_E		Path,	
	IN	u1Byte					HwRate
	);

/* ======================================================================== */

u4Byte
query_phydm_trx_capability_8821c(
	IN	PDM_ODM_T				pDM_Odm
);

u4Byte
query_phydm_stbc_capability_8821c(
	IN	PDM_ODM_T				pDM_Odm
);

u4Byte
query_phydm_ldpc_capability_8821c(
	IN	PDM_ODM_T				pDM_Odm
);

u4Byte
query_phydm_txbf_parameters_8821c(
	IN	PDM_ODM_T				pDM_Odm
);

u4Byte
query_phydm_txbf_capability_8821c(
	IN	PDM_ODM_T				pDM_Odm
);

#endif	/* RTL8821C_SUPPORT == 1 */
#endif	/*  __INC_PHYDM_API_H_8821C__ */


