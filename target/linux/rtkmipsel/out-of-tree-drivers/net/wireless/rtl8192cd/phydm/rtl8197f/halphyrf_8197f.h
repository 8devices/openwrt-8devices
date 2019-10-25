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

#ifndef __HAL_PHY_RF_8197F_H__
#define __HAL_PHY_RF_8197F_H__

/*--------------------------Define Parameters-------------------------------*/
#if (DM_ODM_SUPPORT_TYPE & ODM_CE)
#define	IQK_DELAY_TIME_97F		15		/*ms*/
#else
#define	IQK_DELAY_TIME_97F		10
#endif

#define AVG_THERMAL_NUM_8197F	4
#define RF_T_METER_8197F		0x42
#define DPK_THRESHOLD_8197F		7


void 
ConfigureTxpowerTrack_8197F(
	PTXPWRTRACK_CFG	pConfig
	);

VOID
ODM_TxPwrTrackSetPwr8197F(
	PVOID				pDM_VOID,
	PWRTRACK_METHOD	Method,
	u1Byte				RFPath,
	u1Byte				ChannelMappedIndex
	);

VOID
GetDeltaSwingTable_8197F(
	PVOID		pDM_VOID,
	pu1Byte		*TemperatureUP_A,
	pu1Byte		*TemperatureDOWN_A,
	pu1Byte		*TemperatureUP_B,
	pu1Byte		*TemperatureDOWN_B,
	pu1Byte		*TemperatureUP_CCK_A,
	pu1Byte		*TemperatureDOWN_CCK_A,
	pu1Byte		*TemperatureUP_CCK_B,
	pu1Byte		*TemperatureDOWN_CCK_B
	);




VOID
PHY_SetRFPathSwitch_8197F(
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	IN PDM_ODM_T		pDM_Odm,
#else
	IN	PADAPTER	pAdapter,
#endif
	IN	BOOLEAN		bMain
	);

#endif	/* #ifndef __HAL_PHY_RF_8197F_H__ */

