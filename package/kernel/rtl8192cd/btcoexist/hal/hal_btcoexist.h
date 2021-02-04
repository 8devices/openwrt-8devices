/******************************************************************************
 *
 * Copyright(c) 2013 Realtek Corporation. All rights reserved.
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
#ifndef __HAL_BTCOEXIST_H__
#define __HAL_BTCOEXIST_H__


#include <8192cd.h>
#include "../btcoexist_init.h"
#include "../../8192cd_util.h"


void DBG_BT_INFO(u8 *dbgmsg);

void hal_btcoex_SetBTCoexist(BT_COEX_HAL_ADAPTER *padapter, u8 bBtExist);
u8 hal_btcoex_IsBtExist(BT_COEX_HAL_ADAPTER *padapter);
u8 hal_btcoex_IsBtDisabled(BT_COEX_HAL_ADAPTER *padapter);
void hal_btcoex_SetChipType(BT_COEX_HAL_ADAPTER *padapter, u8 chipType);
void hal_btcoex_SetPgAntNum(BT_COEX_HAL_ADAPTER *padapter, u8 antNum);

u8 hal_btcoex_Initialize(BT_COEX_HAL_ADAPTER *padapter);
void hal_btcoex_PowerOnSetting(BT_COEX_HAL_ADAPTER *padapter);
void hal_btcoex_PreLoadFirmware(BT_COEX_HAL_ADAPTER *padapter);
void hal_btcoex_InitHwConfig(BT_COEX_HAL_ADAPTER *padapter, u8 bWifiOnly);

void hal_btcoex_IpsNotify(BT_COEX_HAL_ADAPTER *padapter, u8 type);
void hal_btcoex_LpsNotify(BT_COEX_HAL_ADAPTER *padapter, u8 type);
void hal_btcoex_ScanNotify(BT_COEX_HAL_ADAPTER *padapter, u8 type);
void hal_btcoex_ConnectNotify(BT_COEX_HAL_ADAPTER *padapter, u8 action);
void hal_btcoex_MediaStatusNotify(BT_COEX_HAL_ADAPTER *padapter, u8 mediaStatus);
void hal_btcoex_SpecialPacketNotify(BT_COEX_HAL_ADAPTER *padapter, u8 pktType);
void hal_btcoex_IQKNotify(BT_COEX_HAL_ADAPTER *padapter, u8 state);
void hal_btcoex_BtInfoNotify(BT_COEX_HAL_ADAPTER *padapter, u8 length, u8 *tmpBuf);
void hal_btcoex_BtMpRptNotify(BT_COEX_HAL_ADAPTER *padapter, u8 length, u8 *tmpBuf);
void hal_btcoex_SuspendNotify(BT_COEX_HAL_ADAPTER *padapter, u8 state);
void hal_btcoex_HaltNotify(BT_COEX_HAL_ADAPTER *padapter);
void hal_btcoex_ScoreBoardStatusNotify(BT_COEX_HAL_ADAPTER *padapter, u8 length, u8 *tmpBuf);
void hal_btcoex_SwitchBtTRxMask(BT_COEX_HAL_ADAPTER *padapter);

void hal_btcoex_Hanlder(BT_COEX_HAL_ADAPTER *padapter);

s32 hal_btcoex_IsBTCoexRejectAMPDU(BT_COEX_HAL_ADAPTER *padapter);
s32 hal_btcoex_IsBTCoexCtrlAMPDUSize(BT_COEX_HAL_ADAPTER *padapter);
u32 hal_btcoex_GetAMPDUSize(BT_COEX_HAL_ADAPTER *padapter);
void hal_btcoex_SetManualControl(BT_COEX_HAL_ADAPTER *padapter, u8 bmanual);
u8 hal_btcoex_1Ant(BT_COEX_HAL_ADAPTER *padapter);
u8 hal_btcoex_IsBtControlLps(BT_COEX_HAL_ADAPTER *padapter);
u8 hal_btcoex_IsLpsOn(BT_COEX_HAL_ADAPTER *padapter);
u8 hal_btcoex_RpwmVal(BT_COEX_HAL_ADAPTER *padapter);
u8 hal_btcoex_LpsVal(BT_COEX_HAL_ADAPTER *padapter);
u32 hal_btcoex_GetRaMask(BT_COEX_HAL_ADAPTER *padapter);
void hal_btcoex_RecordPwrMode(BT_COEX_HAL_ADAPTER *padapter, u8 *pCmdBuf, u8 cmdLen);
void hal_btcoex_DisplayBtCoexInfo(BT_COEX_HAL_ADAPTER *padapter, u8 *pbuf, u32 bufsize);
void hal_btcoex_SetDBG(BT_COEX_HAL_ADAPTER *padapter, u32 *pDbgModule);
u32 hal_btcoex_GetDBG(BT_COEX_HAL_ADAPTER *padapter, u8 *pStrBuf, u32 bufSize);
u8 hal_btcoex_IncreaseScanDeviceNum(BT_COEX_HAL_ADAPTER *padapter);
u8 hal_btcoex_IsBtLinkExist(BT_COEX_HAL_ADAPTER *padapter);
void hal_btcoex_SetBtPatchVersion(BT_COEX_HAL_ADAPTER *padapter, u16 btHciVer, u16 btPatchVer);
void hal_btcoex_SetHciVersion(BT_COEX_HAL_ADAPTER *padapter, u16 hciVersion);
void hal_btcoex_SendScanNotify(BT_COEX_HAL_ADAPTER *padapter, u8 type);
void hal_btcoex_StackUpdateProfileInfo(void);
void hal_btcoex_pta_off_on_notify(BT_COEX_HAL_ADAPTER *padapter, u8 bBTON);
void hal_btcoex_SetAntIsolationType(BT_COEX_HAL_ADAPTER *padapter, u8 anttype);
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
int hal_btcoex_AntIsolationConfig_ParaFile(IN BT_COEX_HAL_ADAPTER *Adapter, IN char *pFileName);
int hal_btcoex_ParseAntIsolationConfigFile(BT_COEX_HAL_ADAPTER *Adapter, char	*buffer);
#endif /* CONFIG_LOAD_PHY_PARA_FROM_FILE */
u16 hal_btcoex_btreg_read(BT_COEX_HAL_ADAPTER *padapter, u8 type, u16 addr, u32 *data);
u16 hal_btcoex_btreg_write(BT_COEX_HAL_ADAPTER *padapter, u8 type, u16 addr, u16 val);
void hal_btcoex_set_rfe_type(u8 type);
void hal_btcoex_switchband_notify(u8 under_scan, u8 band_type);
#endif /* !__HAL_BTCOEX_H__ */
