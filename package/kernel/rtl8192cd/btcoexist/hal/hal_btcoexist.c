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
#define __HAL_BTCOEX_C__

#ifdef CONFIG_BT_COEXIST

#include "../../8192cd_btcoexist.h"
#include "hal_btcoexist.h"
#include "./hal_btc/hal_btc_precomp.h"
#include "./hal_btc/hal_btc_chip/hal_btc_8197f.h"

/* ************************************
 *		Global variables
 * ************************************ */
const char *const BtProfileString[] = {
	"NONE",
	"A2DP",
	"PAN",
	"HID",
	"SCO",
};

const char *const BtSpecString[] = {
	"1.0b",
	"1.1",
	"1.2",
	"2.0+EDR",
	"2.1+EDR",
	"3.0+HS",
	"4.0",
};

const char *const BtLinkRoleString[] = {
	"Master",
	"Slave",
};

const char *const h2cStaString[] = {
	"successful",
	"h2c busy",
	"rf off",
	"fw not read",
};

const char *const ioStaString[] = {
	"success",
	"can not IO",
	"rf off",
	"fw not read",
	"wait io timeout",
	"invalid len",
	"idle Q empty",
	"insert waitQ fail",
	"unknown fail",
	"wrong level",
	"h2c stopped",
};

const char *const GLBtcWifiBwString[] = {
	"11bg",
	"HT20",
	"HT40",
	"HT80",
	"HT160"
};

const char *const GLBtcWifiFreqString[] = {
	"2.4G",
	"5G"
};

const char *const GLBtcIotPeerString[] = {
	"UNKNOWN",
	"REALTEK",
	"REALTEK_92SE",
	"BROADCOM",
	"RALINK",
	"ATHEROS",
	"CISCO",
	"MERU",
	"MARVELL",
	"REALTEK_SOFTAP", /* peer is RealTek SOFT_AP, by Bohn, 2009.12.17 */
	"SELF_SOFTAP", /* Self is SoftAP */
	"AIRGO",
	"INTEL",
	"RTK_APCLIENT",
	"REALTEK_81XX",
	"REALTEK_WOW",
	"REALTEK_JAGUAR_BCUTAP",
	"REALTEK_JAGUAR_CCUTAP"
};

const char *const coexOpcodeString[] = {
	"Wifi status notify",
	"Wifi progress",
	"Wifi info",
	"Power state",
	"Set Control",
	"Get Control"
};

const char *const coexIndTypeString[] = {
	"bt info",
	"pstdma",
	"limited tx/rx",
	"coex table",
	"request"
};

const char *const coexH2cResultString[] = {
	"ok",
	"unknown",
	"un opcode",
	"opVer MM",
	"par Err",
	"par OoR",
	"reqNum MM",
	"halMac Fail",
	"h2c TimeOut",
	"Invalid c2h Len",
	"data overflow"
};

#define HALBTCOUTSRC_AGG_CHK_WINDOW_IN_MS	8000

struct btc_coexist GLBtCoexist;
BTC_OFFLOAD gl_coex_offload;
u8 GLBtcWiFiInScanState;
u8 GLBtcWiFiInIQKState;
u8 GLBtcWiFiInIPS;
u8 GLBtcWiFiInLPS;
u8 GLBtcBtCoexAliveRegistered;

/*
 * BT control H2C/C2H
 */
/* EXT_EID */
typedef enum _bt_ext_eid {
	C2H_WIFI_FW_ACTIVE_RSP	= 0,
	C2H_TRIG_BY_BT_FW
} BT_EXT_EID;

/* C2H_STATUS */
typedef enum _bt_c2h_status {
	BT_STATUS_OK = 0,
	BT_STATUS_VERSION_MISMATCH,
	BT_STATUS_UNKNOWN_OPCODE,
	BT_STATUS_ERROR_PARAMETER
} BT_C2H_STATUS;

/* C2H BT OP CODES */
typedef enum _bt_op_code {
	BT_OP_GET_BT_VERSION	= 0,
	BT_OP_WRITE_REG_ADDR	= 12,
	BT_OP_WRITE_REG_VALUE,
	BT_OP_READ_REG		= 17
} BT_OP_CODE;

#define BTC_MPOPER_TIMEOUT	50	/* unit: ms */

#define C2H_MAX_SIZE		16
u8 GLBtcBtMpOperSeq;
_mutex GLBtcBtMpOperLock;
_timer GLBtcBtMpOperTimer;
_sema GLBtcBtMpRptSema;
u8 GLBtcBtMpRptSeq;
u8 GLBtcBtMpRptStatus;
u8 GLBtcBtMpRptRsp[C2H_MAX_SIZE];
u8 GLBtcBtMpRptRspSize;
u8 GLBtcBtMpRptWait;
u8 GLBtcBtMpRptWiFiOK;
u8 GLBtcBtMpRptBTOK;

/*
 * Debug
 */
u32 GLBtcDbgType[COMP_MAX];
u8 GLBtcDbgBuf[BT_TMP_BUF_SIZE];
u1Byte	gl_btc_trace_buf[BT_TMP_BUF_SIZE];

void _rtw_up_sema(_sema	*sema)
{

#ifdef PLATFORM_LINUX

	up(sema);

#endif
#if 0
	sema_post(sema);
#endif
#ifdef PLATFORM_OS_XP

	KeReleaseSemaphore(sema, IO_NETWORK_INCREMENT, 1,  FALSE);

#endif

#ifdef PLATFORM_OS_CE
	ReleaseSemaphore(*sema,  1,  NULL);
#endif
}


u32 _rtw_down_sema(_sema *sema)
{

#ifdef PLATFORM_LINUX

	if (down_interruptible(sema))
		return _FAIL;
	else
		return _SUCCESS;

#endif
#if 0
	sema_wait(sema);
	return  _SUCCESS;
#endif
#ifdef PLATFORM_OS_XP

	if (STATUS_SUCCESS == KeWaitForSingleObject(sema, Executive, KernelMode, TRUE, NULL))
		return  _SUCCESS;
	else
		return _FAIL;
#endif

#ifdef PLATFORM_OS_CE
	if (WAIT_OBJECT_0 == WaitForSingleObject(*sema, INFINITE))
		return _SUCCESS;
	else
		return _FAIL;
#endif
}

inline void _rtw_memmove(void *dst, const void *src, unsigned long sz)
{
#if defined(PLATFORM_LINUX)
	memmove(dst, src, sz);
#else
	#warning "no implementation\n"
#endif
}

inline u32 rtw_systime_to_ms(u32 systime)
{
#ifdef PLATFORM_LINUX
	return systime * 1000 / HZ;
#endif
#ifdef PLATFORM_FREEBSD
	return systime * 1000;
#endif
#ifdef PLATFORM_WINDOWS
	return systime / 10000 ;
#endif
}

typedef struct _btcoexdbginfo {
	u8 *info;
	u32 size; /* buffer total size */
	u32 len; /* now used length */
} BTCDBGINFO, *PBTCDBGINFO;

BTCDBGINFO GLBtcDbgInfo;

#define	BT_Operation(Adapter)						_FALSE

static void DBG_BT_INFO_INIT(PBTCDBGINFO pinfo, u8 *pbuf, u32 size)
{
	if (NULL == pinfo)
		return;

	memset(pinfo, 0, sizeof(BTCDBGINFO));

	if (pbuf && size) {
		pinfo->info = pbuf;
		pinfo->size = size;
	}
}

void DBG_BT_INFO(u8 *dbgmsg)
{
	PBTCDBGINFO pinfo;
	u32 msglen, buflen;
	u8 *pbuf;


	pinfo = &GLBtcDbgInfo;

	if (NULL == pinfo->info)
		return;

	msglen = strlen(dbgmsg);
	if (pinfo->len + msglen > pinfo->size)
		return;

	pbuf = pinfo->info + pinfo->len;
	memcpy(pbuf, dbgmsg, msglen);
	pinfo->len += msglen;
}

/* ************************************
 *		Debug related function
 * ************************************ */
static u8 halbtcoutsrc_IsBtCoexistAvailable(PBTC_COEXIST pBtCoexist)
{
	if (!pBtCoexist->bBinded ||
	    NULL == pBtCoexist->Adapter)
		return _FALSE;
	return _TRUE;
}

static void halbtcoutsrc_DbgInit(void)
{
	u8	i;

	for (i = 0; i < COMP_MAX; i++)
		GLBtcDbgType[i] = 0;
}

static u8 halbtcoutsrc_IsCsrBtCoex(PBTC_COEXIST pBtCoexist)
{
	if (pBtCoexist->board_info.bt_chip_type == BTC_CHIP_CSR_BC4
	    || pBtCoexist->board_info.bt_chip_type == BTC_CHIP_CSR_BC8
	   )
		return _TRUE;
	return _FALSE;
}

static u8 halbtcoutsrc_IsHwMailboxExist(PBTC_COEXIST pBtCoexist)
{
	if (pBtCoexist->board_info.bt_chip_type == BTC_CHIP_CSR_BC4
	    || pBtCoexist->board_info.bt_chip_type == BTC_CHIP_CSR_BC8
	   )
		return _FALSE;
	else if (IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter))
			return _FALSE;
	else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter))
		return _FALSE;
	else
		return _TRUE;
}

static void halbtcoutsrc_LeaveLps(PBTC_COEXIST pBtCoexist)
{
	PADAPTER padapter;


	padapter = pBtCoexist->Adapter;

	pBtCoexist->bt_info.bt_ctrl_lps = _TRUE;
	pBtCoexist->bt_info.bt_lps_on = _FALSE;

	//rtw_btcoex_LPS_Leave(padapter);
}

void halbtcoutsrc_EnterLps(PBTC_COEXIST pBtCoexist)
{
	PADAPTER padapter;


	padapter = pBtCoexist->Adapter;

	if (pBtCoexist->bdontenterLPS == _FALSE) {
		pBtCoexist->bt_info.bt_ctrl_lps = _TRUE;
		pBtCoexist->bt_info.bt_lps_on = _TRUE;
#if 0
		rtw_btcoex_LPS_Enter(padapter);
#endif
	}
}

void halbtcoutsrc_NormalLps(PBTC_COEXIST pBtCoexist)
{
	PADAPTER padapter;



	padapter = pBtCoexist->Adapter;

	if (pBtCoexist->bt_info.bt_ctrl_lps) {
		pBtCoexist->bt_info.bt_lps_on = _FALSE;
#if 0        
		rtw_btcoex_LPS_Leave(padapter);
#endif
		pBtCoexist->bt_info.bt_ctrl_lps = _FALSE;

		/* recover the LPS state to the original */
#if 0
		padapter->HalFunc.UpdateLPSStatusHandler(
			padapter,
			pPSC->RegLeisurePsMode,
			pPSC->RegPowerSaveMode);
#endif
	}
}

/*
 *  Constraint:
 *	   1. this function will request pwrctrl->lock
 */
void halbtcoutsrc_LeaveLowPower(PBTC_COEXIST pBtCoexist)
{
#ifdef CONFIG_LPS_LCLK
	PADAPTER padapter;
	PHAL_DATA_TYPE pHalData;
	struct pwrctrl_priv *pwrctrl;
	s32 ready;
	u32 stime;
	s32 utime;
	u32 timeout; /* unit: ms */


	padapter = pBtCoexist->Adapter;
	pHalData = GET_HAL_DATA(padapter);
	pwrctrl = adapter_to_pwrctl(padapter);
	ready = _FAIL;
#ifdef LPS_RPWM_WAIT_MS
	timeout = LPS_RPWM_WAIT_MS;
#else /* !LPS_RPWM_WAIT_MS */
	timeout = 30;
#endif /* !LPS_RPWM_WAIT_MS */

	if (GLBtcBtCoexAliveRegistered == _TRUE)
		return;

	stime = rtw_get_current_time();
	do {
		ready = rtw_register_task_alive(padapter, BTCOEX_ALIVE);
		if (_SUCCESS == ready)
			break;

		utime = rtw_get_passing_time_ms(stime);
		if (utime > timeout)
			break;

		rtw_msleep_os(1);
	} while (1);

	GLBtcBtCoexAliveRegistered = _TRUE;
#endif /* CONFIG_LPS_LCLK */
}

/*
 *  Constraint:
 *	   1. this function will request pwrctrl->lock
 */
void halbtcoutsrc_NormalLowPower(PBTC_COEXIST pBtCoexist)
{
#ifdef CONFIG_LPS_LCLK
	PADAPTER padapter;

	if (GLBtcBtCoexAliveRegistered == _FALSE)
		return;

	padapter = pBtCoexist->Adapter;
	rtw_unregister_task_alive(padapter, BTCOEX_ALIVE);

	GLBtcBtCoexAliveRegistered = _FALSE;
#endif /* CONFIG_LPS_LCLK */
}

void halbtcoutsrc_DisableLowPower(PBTC_COEXIST pBtCoexist, u8 bLowPwrDisable)
{
	pBtCoexist->bt_info.bt_disable_low_pwr = bLowPwrDisable;
	if (bLowPwrDisable)
		halbtcoutsrc_LeaveLowPower(pBtCoexist);		/* leave 32k low power. */
	else
		halbtcoutsrc_NormalLowPower(pBtCoexist);	/* original 32k low power behavior. */
}

void halbtcoutsrc_AggregationCheck(PBTC_COEXIST pBtCoexist)
{
	PADAPTER padapter;
	BOOLEAN bNeedToAct = _FALSE;
	static u32 preTime = 0;
	u32 curTime = 0;

	padapter = pBtCoexist->Adapter;

	/* ===================================== */
	/* To void continuous deleteBA=>addBA=>deleteBA=>addBA */
	/* This function is not allowed to continuous called. */
	/* It can only be called after 8 seconds. */
	/* ===================================== */

	curTime = rtw_systime_to_ms(rtw_get_current_time());
	if ((curTime - preTime) < HALBTCOUTSRC_AGG_CHK_WINDOW_IN_MS)	/* over 8 seconds you can execute this function again. */
		return;
	else
		preTime = curTime;

	if (pBtCoexist->bt_info.reject_agg_pkt) {
		bNeedToAct = _TRUE;
		pBtCoexist->bt_info.pre_reject_agg_pkt = pBtCoexist->bt_info.reject_agg_pkt;
	} else {
		if (pBtCoexist->bt_info.pre_reject_agg_pkt) {
			bNeedToAct = _TRUE;
			pBtCoexist->bt_info.pre_reject_agg_pkt = pBtCoexist->bt_info.reject_agg_pkt;
		}

		if (pBtCoexist->bt_info.pre_bt_ctrl_agg_buf_size !=
		    pBtCoexist->bt_info.bt_ctrl_agg_buf_size) {
			bNeedToAct = _TRUE;
			pBtCoexist->bt_info.pre_bt_ctrl_agg_buf_size = pBtCoexist->bt_info.bt_ctrl_agg_buf_size;
		}

		if (pBtCoexist->bt_info.bt_ctrl_agg_buf_size) {
			if (pBtCoexist->bt_info.pre_agg_buf_size !=
			    pBtCoexist->bt_info.agg_buf_size)
				bNeedToAct = _TRUE;
			pBtCoexist->bt_info.pre_agg_buf_size = pBtCoexist->bt_info.agg_buf_size;
		}
	}
#if 0
	if (bNeedToAct)
		rtw_btcoex_rx_ampdu_apply(padapter);
#endif    
}

u8 halbtcoutsrc_IsWifiBusy(BT_COEX_HAL_ADAPTER *padapter)
{
#if 0    
	//chester modified

	if (rtw_mi_check_status(padapter, MI_AP_MODE))
		return _TRUE;
	if (rtw_mi_busy_traffic_check(padapter, _FALSE))
		return _TRUE;
#endif
	// AP mode always busy
	return _TRUE;
}

static u32 _halbtcoutsrc_GetWifiLinkStatus(BT_COEX_HAL_ADAPTER *padapter)
{
	//struct mlme_priv *pmlmepriv;
	u8 bp2p = _FALSE;
	u32 portConnectedStatus;
	struct stat_info* pstat;

	unsigned int opmode = padapter->pmib->dot11OperationEntry.opmode;


	//pmlmepriv = &padapter->mlmepriv;
	bp2p = _FALSE;
	portConnectedStatus = 0;

#ifdef CONFIG_P2P
	/*not ready*/
	if (!rtw_p2p_chk_state(&padapter->wdinfo, P2P_STATE_NONE))
		bp2p = _TRUE;
#endif /* CONFIG_P2P */

	if ((opmode & WIFI_ASOC_STATE) == _TRUE) {
		if ((opmode & WIFI_AP_STATE) == _TRUE) {
			if (_TRUE == bp2p)
				portConnectedStatus |= WIFI_P2P_GO_CONNECTED;
			else
				portConnectedStatus |= WIFI_AP_CONNECTED;
		} else {
			if (_TRUE == bp2p)
				portConnectedStatus |= WIFI_P2P_GC_CONNECTED;
			else
				portConnectedStatus |= WIFI_STA_CONNECTED;
		}
	}

	return portConnectedStatus;
}

u32 halbtcoutsrc_GetWifiLinkStatus(PBTC_COEXIST pBtCoexist)
{
	/* ================================= */
	/* return value: */
	/* [31:16]=> connected port number */
	/* [15:0]=> port connected bit define */
	/* ================================ */

	BT_COEX_HAL_ADAPTER *padapter;
	u32 retVal;
	u32 portConnectedStatus, numOfConnectedPort;
	//struct dvobj_priv *dvobj;
	//BT_COEX_HAL_ADAPTER *iface;
	//int i;

	padapter = pBtCoexist->Adapter;
	retVal = 0;
	//portConnectedStatus = 0;
	//numOfConnectedPort = 0;
	/*
	dvobj = adapter_to_dvobj(padapter);

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if ((iface) && rtw_is_adapter_up(iface)) {
			retVal = _halbtcoutsrc_GetWifiLinkStatus(iface);
			if (retVal) {
				portConnectedStatus |= retVal;
				numOfConnectedPort++;
			}
		}
	}
	*/
		
	//retVal = (numOfConnectedPort << 16) | portConnectedStatus;

	retVal = _halbtcoutsrc_GetWifiLinkStatus(padapter);

	return retVal;

}

static u8 _is_btfwver_valid(PBTC_COEXIST pBtCoexist, u16 btfwver)
{
	if (!btfwver)
		return _FALSE;
#if 0
	if (IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter))
		if (btfwver == 0x8723)
			return _FALSE;
#endif
	return _TRUE;
}

static void _btmpoper_timer_hdl(void *p)
{
	if (GLBtcBtMpRptWait) {
		GLBtcBtMpRptWait = 0;
		_rtw_up_sema(&GLBtcBtMpRptSema);
	}
}

s32 rtw_hal_fill_h2c_cmd(BT_COEX_HAL_ADAPTER *padapter, u8 ElementID, u32 CmdLen, u8 *pCmdBuffer)
{
	
	if(_GET_HAL_DATA(padapter)->bFWReady == _TRUE){
		return FillH2CCmd88XX(padapter, ElementID, CmdLen, pCmdBuffer);
	}
	else{ 
		printk(" FW doesn't exit when no MP mode, by pass H2C id:0x%02x\n",ElementID);
	}

	return _FAIL;

	/*
	printk(COMP_COEX, DBG_LOUD, ("[h2c] %02x: ", ElementID));
	for (i=0; i<CmdLen; i++)
		printk(COMP_COEX, DBG_LOUD, ("%02x ", pCmdBuffer[i]));
	printk(COMP_COEX, DBG_LOUD, ("\n"));
	ret = FillH2CCmd88XX(padapter, ElementID, CmdLen, pCmdBuffer);

	return ret;
	*/
}

/*
 * !IMPORTANT!
 *	Before call this function, caller should acquire "GLBtcBtMpOperLock"!
 *	Othrewise there will be racing problem and something may go wrong.
 */
static u8 _btmpoper_cmd(PBTC_COEXIST pBtCoexist, u8 opcode, u8 opcodever, u8 *cmd, u8 size)
{
	BT_COEX_HAL_ADAPTER *padapter;
	u8 buf[H2C_BTMP_OPER_LEN] = {0};
	u8 buflen;
	u8 seq;
	u8 timer_cancelled;
	s32 ret;


	if (!cmd && size)
		size = 0;
	if ((size + 2) > H2C_BTMP_OPER_LEN)
		return BT_STATUS_H2C_LENGTH_EXCEEDED;
	buflen = size + 2;

	seq = GLBtcBtMpOperSeq & 0xF;
	GLBtcBtMpOperSeq++;

	buf[0] = (opcodever & 0xF) | (seq << 4);
	buf[1] = opcode;
	if (cmd && size)
		memcpy(buf + 2, cmd, size);

	GLBtcBtMpRptWait = 1;
	GLBtcBtMpRptWiFiOK = 0;
	GLBtcBtMpRptBTOK = 0;
	GLBtcBtMpRptStatus = 0;
	padapter = (BT_COEX_HAL_ADAPTER *)pBtCoexist->Adapter;
	_set_timer(&GLBtcBtMpOperTimer, BTC_MPOPER_TIMEOUT);
	if (rtw_hal_fill_h2c_cmd(padapter, H2C_BT_MP_OPER, buflen, buf) == _FAIL) {
		_cancel_timer(&GLBtcBtMpOperTimer, &timer_cancelled);
		ret = BT_STATUS_H2C_FAIL;
		goto exit;
	}

	_rtw_down_sema(&GLBtcBtMpRptSema);
	/* GLBtcBtMpRptWait should be 0 here*/

	if (!GLBtcBtMpRptWiFiOK) {
		printk("%s: Didn't get H2C Rsp Event!\n", __FUNCTION__);
		ret = BT_STATUS_H2C_TIMTOUT;
		goto exit;
	}
	if (!GLBtcBtMpRptBTOK) {
		printk("%s: Didn't get BT response!\n", __FUNCTION__);
		ret = BT_STATUS_H2C_BT_NO_RSP;
		goto exit;
	}
	if (seq != GLBtcBtMpRptSeq) {
		printk("%s: Sequence number not match!(%d!=%d)!\n",
			 __FUNCTION__, seq, GLBtcBtMpRptSeq);
		ret = BT_STATUS_C2H_REQNUM_MISMATCH;
		goto exit;
	}

	switch (GLBtcBtMpRptStatus) {
	/* Examine the status reported from C2H */
	case BT_STATUS_OK:
		ret = BT_STATUS_BT_OP_SUCCESS;
		printk("%s: C2H status = BT_STATUS_BT_OP_SUCCESS\n", __FUNCTION__);
		break;
	case BT_STATUS_VERSION_MISMATCH:
		ret = BT_STATUS_OPCODE_L_VERSION_MISMATCH;
		printk("%s: C2H status = BT_STATUS_OPCODE_L_VERSION_MISMATCH\n", __FUNCTION__);
		break;
	case BT_STATUS_UNKNOWN_OPCODE:
		ret = BT_STATUS_UNKNOWN_OPCODE_L;
		printk("%s: C2H status = MP_BT_STATUS_UNKNOWN_OPCODE_L\n", __FUNCTION__);
		break;
	case BT_STATUS_ERROR_PARAMETER:
		ret = BT_STATUS_PARAMETER_FORMAT_ERROR_L;
		printk("%s: C2H status = MP_BT_STATUS_PARAMETER_FORMAT_ERROR_L\n", __FUNCTION__);
		break;
	default:
		ret = BT_STATUS_UNKNOWN_STATUS_L;
		printk("%s: C2H status = MP_BT_STATUS_UNKNOWN_STATUS_L\n", __FUNCTION__);
		break;
	}

exit:
	return ret;
}

u32 halbtcoutsrc_GetBtPatchVer(PBTC_COEXIST pBtCoexist)
{

	if (_is_btfwver_valid(pBtCoexist, pBtCoexist->bt_info.bt_real_fw_ver) == _TRUE)
		goto exit;

	if (halbtcoutsrc_IsHwMailboxExist(pBtCoexist) == _TRUE) {

		_irqL irqL;
		u8 ret;

		//_enter_critical_mutex(&GLBtcBtMpOperLock, &irqL);
		SMP_LOCK_BUF(&GLBtcBtMpOperLock);

		ret = _btmpoper_cmd(pBtCoexist, BT_OP_GET_BT_VERSION, 0, NULL, 0);
		if (BT_STATUS_BT_OP_SUCCESS == ret) {
			pBtCoexist->bt_info.bt_real_fw_ver = le16_to_cpu(*(u16 *)GLBtcBtMpRptRsp);
			pBtCoexist->bt_info.bt_fw_ver = *(GLBtcBtMpRptRsp + 2);
		}
		pBtCoexist->bt_info.get_bt_fw_ver_cnt++;

		//_exit_critical_mutex(&GLBtcBtMpOperLock, &irqL);
		SMP_UNLOCK_BUF(&GLBtcBtMpOperLock);

	} else {
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX

		u1Byte dataLen = 2;
		u1Byte buf[4] = {0};

		buf[0] = 0x0;	/* OP_Code */
		buf[1] = 0x0;	/* OP_Code_Length */
		BT_SendEventExtBtCoexControl(pBtCoexist->Adapter, _FALSE, dataLen, &buf[0]);
#endif /* !CONFIG_BT_COEXIST_SOCKET_TRX */
	}

exit:
	return pBtCoexist->bt_info.bt_real_fw_ver;
}

s32 halbtcoutsrc_GetWifiRssi(BT_COEX_HAL_ADAPTER *padapter)
{
#if 0

	PHAL_DATA_TYPE pHalData;
	s32 UndecoratedSmoothedPWDB = 0;
	
	pHalData = GET_HAL_DATA(padapter);

	UndecoratedSmoothedPWDB = pHalData->EntryMinUndecoratedSmoothedPWDB;

	return UndecoratedSmoothedPWDB;

#endif
	return padapter->pshare->rssi_min;
	

	
}

static u8 halbtcoutsrc_GetWifiScanAPNum(PADAPTER padapter)
{
	struct mlme_priv *pmlmepriv;
	struct mlme_ext_priv *pmlmeext;
	static u8 scan_AP_num = 0;

#if 0
	pmlmepriv = &padapter->mlmepriv;
	pmlmeext = &padapter->mlmeextpriv;

	if (GLBtcWiFiInScanState == _FALSE) {
		if (pmlmepriv->num_of_scanned > 0xFF)
			scan_AP_num = 0xFF;
		else
			scan_AP_num = (u8)pmlmepriv->num_of_scanned;
	}
#endif
	return scan_AP_num;
}

#define IsLegacyOnly(NetType)  ((NetType) == ((NetType) & (WIRELESS_11BG | WIRELESS_11A)))

u8 * is_sta_on_connecting_state(BT_COEX_HAL_ADAPTER *priv)
{
	struct stat_info	*pstat, *pstat_max=NULL;
	struct list_head	*phead, *plist;
	unsigned int		 max_tp;
	phead = &priv->asoc_list;
	plist = phead;

	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);

		if((pstat!=NULL) && ((pstat->state & WIFI_ASOC_STATE)|| (pstat->wpa_sta_info->state == PSK_STATE_PTKINITDONE))){
			return _TRUE;
		}		
	}
	
	return _FALSE;
}



u8 halbtcoutsrc_Get(void *pBtcContext, u8 getType, void *pOutBuf)
{
	PBTC_COEXIST pBtCoexist;
	BT_COEX_HAL_ADAPTER *padapter;
	//PHAL_DATA_TYPE pHalData;
	//struct mlme_ext_priv *mlmeext;
	u8 bSoftApExist, bVwifiExist;
	u8 *pu8;
	s32 *pS4Tmp;
	u32 *pU4Tmp;
	u8 *pU1Tmp;
	u8 ret;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return _FALSE;


	padapter = (BT_COEX_HAL_ADAPTER *)pBtCoexist->Adapter;
#if 0

	pHalData = GET_HAL_DATA(padapter);

	mlmeext = &padapter->mlmeextpriv;
#endif
	bSoftApExist = _FALSE;
	bVwifiExist = _FALSE;
	pu8 = (u8 *)pOutBuf;
	pS4Tmp = (s32 *)pOutBuf;
	pU4Tmp = (u32 *)pOutBuf;
	pU1Tmp = (u8 *)pOutBuf;
	ret = _TRUE;

	switch (getType) {
	case BTC_GET_BL_HS_OPERATION:
		*pu8 = _FALSE;
		ret = _FALSE;
		break;

	case BTC_GET_BL_HS_CONNECTING:
		*pu8 = _FALSE;
		ret = _FALSE;
		break;

	case BTC_GET_BL_WIFI_CONNECTED:
#if 0        
		//chester modified
		*pu8 = (rtw_mi_check_status(padapter, MI_LINKED)) ? _TRUE : _FALSE;
#endif
		//AP mode only
		if(padapter->assoc_num > 0)
			*pu8 = _TRUE;	
		else
			*pu8 = _FALSE;
		
		break;

	case BTC_GET_BL_WIFI_BUSY:
#if 0
		*pu8 = halbtcoutsrc_IsWifiBusy(padapter);
#endif
		//AP mode alway busy
		*pu8 = _TRUE;
		break;

	case BTC_GET_BL_WIFI_SCAN:
#if 0
		*pu8 = (rtw_mi_check_fwstate(padapter, WIFI_SITE_MONITOR)) ? _TRUE : _FALSE;
#else
		/* Use the value of the new variable GLBtcWiFiInScanState to judge whether WiFi is in scan state or not, since the originally used flag
			WIFI_SITE_MONITOR in fwstate may not be cleared in time */
		*pu8 = GLBtcWiFiInScanState;
#endif
		break;

	case BTC_GET_BL_WIFI_LINK:
#if 0        
		//chester modified
		*pu8 = (rtw_mi_check_status(padapter, MI_STA_LINKING)) ? _TRUE : _FALSE;
#endif
		*pu8 = is_sta_on_connecting_state(padapter);
		
		break;

	case BTC_GET_BL_WIFI_ROAM:
#if 0
		//chester modified
		*pu8 = (rtw_mi_check_status(padapter, MI_STA_LINKING)) ? _TRUE : _FALSE;
#endif
		*pu8 = is_sta_on_connecting_state(padapter);

	        break;

	case BTC_GET_BL_WIFI_4_WAY_PROGRESS:
		*pu8 = _FALSE;
		break;

	case BTC_GET_BL_WIFI_UNDER_5G:
        
		*pu8 = (padapter->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_5G) ? _TRUE : _FALSE;

		break;

	case BTC_GET_BL_WIFI_AP_MODE_ENABLE:
        	//chester modified
		//*pu8 = (rtw_mi_check_status(padapter, MI_AP_MODE)) ? _TRUE : _FALSE;
		*pu8 = 	_TRUE;
		
		break;

	case BTC_GET_BL_WIFI_ENABLE_ENCRYPTION:
        
		*pu8 = (padapter->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm  == 0) ? _FALSE : _TRUE;

		break;

	case BTC_GET_BL_WIFI_UNDER_B_MODE:

       		*pu8 = (padapter->pmib->dot11BssType.net_work_type & WIRELESS_11B) ? _TRUE : _FALSE;
		        
		break;

	case BTC_GET_BL_WIFI_IS_IN_MP_MODE:
#if 0        
		//chester modified
		if (padapter->registrypriv.mp_mode == 0)
			*pu8 = _FALSE;
		else
			*pu8 = _TRUE;
#endif        
		*pu8 = _FALSE;
		break;

	case BTC_GET_BL_EXT_SWITCH:
		*pu8 = _FALSE;
		break;
	case BTC_GET_BL_IS_ASUS_8723B:
		/* Always return FALSE in linux driver since this case is added only for windows driver */
		*pu8 = _FALSE;
		break;

	case BTC_GET_S4_WIFI_RSSI:
#if 0
		//chester modified
		*pS4Tmp = halbtcoutsrc_GetWifiRssi(padapter);
#endif
		*pS4Tmp = padapter->pshare->rssi_min;
		break;

	case BTC_GET_S4_HS_RSSI:
		*pS4Tmp = 0;
		ret = _FALSE;
		break;

	case BTC_GET_U4_WIFI_BW:
       
		if (IsLegacyOnly(padapter->pmib->dot11BssType.net_work_type))
			*pU4Tmp = BTC_WIFI_BW_LEGACY;
		else {
			switch (padapter->pshare->CurrentChannelBW) {
			case CHANNEL_WIDTH_20:
				*pU4Tmp = BTC_WIFI_BW_HT20;
				break;
			case CHANNEL_WIDTH_40:
				*pU4Tmp = BTC_WIFI_BW_HT40;
				break;
			case CHANNEL_WIDTH_80:
				*pU4Tmp = BTC_WIFI_BW_HT80;
				break;
			case CHANNEL_WIDTH_160:
				*pU4Tmp = BTC_WIFI_BW_HT160;
				break;
			default:
				printk("[BTCOEX] unkown bandwidth(%d)\n", padapter->pshare->CurrentChannelBW);
				*pU4Tmp = BTC_WIFI_BW_HT40;
				break;
			}

		}

		break;

	case BTC_GET_U4_WIFI_TRAFFIC_DIRECTION: 

#if 0
		//chester modified
		PRT_LINK_DETECT_T plinkinfo;
		plinkinfo = &padapter->mlmepriv.LinkDetectInfo;
		if (plinkinfo->NumTxOkInPeriod > plinkinfo->NumRxOkInPeriod)
			*pU4Tmp = BTC_WIFI_TRAFFIC_TX;
		else
			*pU4Tmp = BTC_WIFI_TRAFFIC_RX;
#endif        
		if(padapter->ext_stats.tx_avarage > padapter->ext_stats.rx_avarage)
			*pU4Tmp = BTC_WIFI_TRAFFIC_TX;
		else
			*pU4Tmp = BTC_WIFI_TRAFFIC_RX;

		break;

	case BTC_GET_U4_WIFI_FW_VER:
 		*pU4Tmp = padapter->pshare->fw_version << 16;
		*pU4Tmp |= padapter->pshare->fw_sub_version;
 		break;

	case BTC_GET_U4_WIFI_LINK_STATUS:
		*pU4Tmp = halbtcoutsrc_GetWifiLinkStatus(pBtCoexist);
		break;

	case BTC_GET_U4_BT_PATCH_VER:
		*pU4Tmp = halbtcoutsrc_GetBtPatchVer(pBtCoexist);
		break;

	case BTC_GET_U4_VENDOR:
		*pU4Tmp = BTC_VENDOR_OTHER;
		break;

	case BTC_GET_U4_WIFI_IQK_TOTAL:
 #if 0       
 		//chester modified
		*pU4Tmp = pHalData->odmpriv.nIQK_Cnt;
 #endif
 		*pU4Tmp = 0;
		break;

	case BTC_GET_U4_WIFI_IQK_OK:
#if 0        
 		//chester modified
		*pU4Tmp = pHalData->odmpriv.nIQK_OK_Cnt;
#endif
 		*pU4Tmp = 0;
		break;

	case BTC_GET_U4_WIFI_IQK_FAIL:
#if 0        
 		//chester modified
		*pU4Tmp = pHalData->odmpriv.nIQK_Fail_Cnt;
#endif
 		*pU4Tmp = 0;
		break;

	case BTC_GET_U1_WIFI_DOT11_CHNL:

		//*pU1Tmp = padapter->mlmeextpriv.cur_channel;
		*pU1Tmp = padapter->pmib->dot11RFEntry.dot11channel;

		break;

	case BTC_GET_U1_WIFI_CENTRAL_CHNL:
       
		//*pU1Tmp = pHalData->CurrentChannel;
		*pU1Tmp = padapter->pmib->dot11RFEntry.dot11channel;

		break;

	case BTC_GET_U1_WIFI_HS_CHNL:
		*pU1Tmp = 0;
		ret = _FALSE;
		break;

	case BTC_GET_U1_WIFI_P2P_CHNL:
#ifdef CONFIG_P2P
		{
			struct wifidirect_info *pwdinfo = &(padapter->wdinfo);
			
			*pU1Tmp = pwdinfo->operating_channel;
		}
#else
		*pU1Tmp = 0;
#endif
		break;

	case BTC_GET_U1_MAC_PHY_MODE:
		/*			*pU1Tmp = BTC_SMSP;
		 *			*pU1Tmp = BTC_DMSP;
		 *			*pU1Tmp = BTC_DMDP;
		 *			*pU1Tmp = BTC_MP_UNKNOWN; */
		break;

	case BTC_GET_U1_AP_NUM:
		*pU1Tmp = halbtcoutsrc_GetWifiScanAPNum(padapter);
		break;
		
	case BTC_GET_U1_ANT_TYPE:{

	        struct bt_coexist_common_data bt_coex_comm_data = padapter->pshare->bt_coex_comm_data;
		
		switch (bt_coex_comm_data.bt_coex_status.btAntisolation) {
		case 0:
			*pU1Tmp = (u1Byte)BTC_ANT_TYPE_0;
			pBtCoexist->board_info.ant_type = (u1Byte)BTC_ANT_TYPE_0;
			break;
		case 1:
			*pU1Tmp = (u1Byte)BTC_ANT_TYPE_1;
			pBtCoexist->board_info.ant_type = (u1Byte)BTC_ANT_TYPE_1;
			break;
		case 2:
			*pU1Tmp = (u1Byte)BTC_ANT_TYPE_2;
			pBtCoexist->board_info.ant_type = (u1Byte)BTC_ANT_TYPE_2;
			break;
		case 3:
			*pU1Tmp = (u1Byte)BTC_ANT_TYPE_3;
			pBtCoexist->board_info.ant_type = (u1Byte)BTC_ANT_TYPE_3;
			break;
		case 4:
			*pU1Tmp = (u1Byte)BTC_ANT_TYPE_4;
			pBtCoexist->board_info.ant_type = (u1Byte)BTC_ANT_TYPE_4;
			break;
		}
	}
		break;
	case BTC_GET_U1_IOT_PEER:
#if 0        
		//chester modified
		*pU1Tmp = mlmeext->mlmext_info.assoc_AP_vendor;
#endif
		break;

	/* =======1Ant=========== */
	case BTC_GET_U1_LPS_MODE:
#if 0        
		//chester modified
		*pU1Tmp = padapter->dvobj->pwrctl_priv.pwr_mode;
#endif
		break;

	default:
		ret = _FALSE;
		break;
	}

	return ret;
}

u8 halbtcoutsrc_Set(void *pBtcContext, u8 setType, void *pInBuf)
{
	PBTC_COEXIST pBtCoexist;
	PADAPTER padapter;
	PHAL_DATA_TYPE pHalData;
	u8 *pu8;
	u8 *pU1Tmp;
	u32	*pU4Tmp;
	u8 ret;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	padapter = pBtCoexist->Adapter;
	pHalData = GET_HAL_DATA(padapter);
	pu8 = (u8 *)pInBuf;
	pU1Tmp = (u8 *)pInBuf;
	pU4Tmp = (u32 *)pInBuf;
	ret = _TRUE;

	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return _FALSE;

	switch (setType) {
	/* set some u8 type variables. */
	case BTC_SET_BL_BT_DISABLE:
		pBtCoexist->bt_info.bt_disabled = *pu8;
		break;

	case BTC_SET_BL_BT_TRAFFIC_BUSY:
		pBtCoexist->bt_info.bt_busy = *pu8;
		break;

	case BTC_SET_BL_BT_LIMITED_DIG:
		pBtCoexist->bt_info.limited_dig = *pu8;
		break;

	case BTC_SET_BL_FORCE_TO_ROAM:
		pBtCoexist->bt_info.force_to_roam = *pu8;
		break;

	case BTC_SET_BL_TO_REJ_AP_AGG_PKT:
		pBtCoexist->bt_info.reject_agg_pkt = *pu8;
		break;

	case BTC_SET_BL_BT_CTRL_AGG_SIZE:
		pBtCoexist->bt_info.bt_ctrl_agg_buf_size = *pu8;
		break;

	case BTC_SET_BL_INC_SCAN_DEV_NUM:
		pBtCoexist->bt_info.increase_scan_dev_num = *pu8;
		break;

	case BTC_SET_BL_BT_TX_RX_MASK:
		pBtCoexist->bt_info.bt_tx_rx_mask = *pu8;
		break;

	case BTC_SET_BL_MIRACAST_PLUS_BT:
		pBtCoexist->bt_info.miracast_plus_bt = *pu8;
		break;

	/* set some u8 type variables. */
	case BTC_SET_U1_RSSI_ADJ_VAL_FOR_AGC_TABLE_ON:
		pBtCoexist->bt_info.rssi_adjust_for_agc_table_on = *pU1Tmp;
		break;

	case BTC_SET_U1_AGG_BUF_SIZE:
		pBtCoexist->bt_info.agg_buf_size = *pU1Tmp;
		break;

	/* the following are some action which will be triggered */
	case BTC_SET_ACT_GET_BT_RSSI:
#if 0
		BT_SendGetBtRssiEvent(padapter);
#else
		ret = _FALSE;
#endif
		break;

	case BTC_SET_ACT_AGGREGATE_CTRL:
		halbtcoutsrc_AggregationCheck(pBtCoexist);
		break;

	/* =======1Ant=========== */
	/* set some u8 type variables. */
	case BTC_SET_U1_RSSI_ADJ_VAL_FOR_1ANT_COEX_TYPE:
		pBtCoexist->bt_info.rssi_adjust_for_1ant_coex_type = *pU1Tmp;
		break;

	case BTC_SET_U1_LPS_VAL:
		pBtCoexist->bt_info.lps_val = *pU1Tmp;
		break;

	case BTC_SET_U1_RPWM_VAL:
		pBtCoexist->bt_info.rpwm_val = *pU1Tmp;
		break;

	/* the following are some action which will be triggered */
	case BTC_SET_ACT_LEAVE_LPS:
		halbtcoutsrc_LeaveLps(pBtCoexist);
		break;

	case BTC_SET_ACT_ENTER_LPS:
		halbtcoutsrc_EnterLps(pBtCoexist);
		break;

	case BTC_SET_ACT_NORMAL_LPS:
		halbtcoutsrc_NormalLps(pBtCoexist);
		break;

	case BTC_SET_ACT_DISABLE_LOW_POWER:
		halbtcoutsrc_DisableLowPower(pBtCoexist, *pu8);
		break;

	case BTC_SET_ACT_UPDATE_RAMASK:
		pBtCoexist->bt_info.ra_mask = *pU4Tmp;
#if 0
		if (check_fwstate(&padapter->mlmepriv, WIFI_ASOC_STATE) == _TRUE) {
			struct sta_info *psta;
			PWLAN_BSSID_EX cur_network;

			cur_network = &padapter->mlmeextpriv.mlmext_info.network;
			psta = rtw_get_stainfo(&padapter->stapriv, cur_network->MacAddress);
			rtw_hal_update_ra_mask(psta, 0);
		}
#endif		
		break;

	case BTC_SET_ACT_SEND_MIMO_PS: {
#if 0        
		u8 newMimoPsMode = 3;
		struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
		struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);

		/* *pU1Tmp = 0 use SM_PS static type */
		/* *pU1Tmp = 1 disable SM_PS */
		if (*pU1Tmp == 0)
			newMimoPsMode = WLAN_HT_CAP_SM_PS_STATIC;
		else if (*pU1Tmp == 1)
			newMimoPsMode = WLAN_HT_CAP_SM_PS_DISABLED;

		if (check_fwstate(&padapter->mlmepriv , WIFI_ASOC_STATE) == _TRUE) {
			/* issue_action_SM_PS(padapter, get_my_bssid(&(pmlmeinfo->network)), newMimoPsMode); */
			issue_action_SM_PS_wait_ack(padapter , get_my_bssid(&(pmlmeinfo->network)) , newMimoPsMode, 3 , 1);
		}
#endif        
	}
	break;

	case BTC_SET_ACT_CTRL_BT_INFO:
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
		{
			u8 dataLen = *pU1Tmp;
			u8 tmpBuf[BTC_TMP_BUF_SHORT];
			if (dataLen)
				memcpy(tmpBuf, pU1Tmp + 1, dataLen);
			BT_SendEventExtBtInfoControl(padapter, dataLen, &tmpBuf[0]);
		}
#else /* !CONFIG_BT_COEXIST_SOCKET_TRX */
		ret = _FALSE;
#endif /* CONFIG_BT_COEXIST_SOCKET_TRX */
		break;

	case BTC_SET_ACT_CTRL_BT_COEX:
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
		{
			u8 dataLen = *pU1Tmp;
			u8 tmpBuf[BTC_TMP_BUF_SHORT];
			if (dataLen)
				memcpy(tmpBuf, pU1Tmp + 1, dataLen);
			BT_SendEventExtBtCoexControl(padapter, _FALSE, dataLen, &tmpBuf[0]);
		}
#else /* !CONFIG_BT_COEXIST_SOCKET_TRX */
		ret = _FALSE;
#endif /* CONFIG_BT_COEXIST_SOCKET_TRX */
		break;
	case BTC_SET_ACT_CTRL_8723B_ANT:
#if 0
		{
			u1Byte	dataLen = *pU1Tmp;
			u1Byte	tmpBuf[BTC_TMP_BUF_SHORT];
			if (dataLen)
				PlatformMoveMemory(&tmpBuf[0], pU1Tmp + 1, dataLen);
			BT_Set8723bAnt(Adapter, dataLen, &tmpBuf[0]);
		}
#else
		ret = _FALSE;
#endif
		break;
	/* ===================== */
	default:
		ret = _FALSE;
		break;
	}

	return ret;
}


u8 halbtcoutsrc_UnderIps(PBTC_COEXIST pBtCoexist)
{
	PADAPTER padapter;
	struct pwrctrl_priv *pwrpriv;
	u8 bMacPwrCtrlOn;
#if 0
	padapter = pBtCoexist->Adapter;
	pwrpriv = &padapter->dvobj->pwrctl_priv;
	bMacPwrCtrlOn = _FALSE;

	if ((_TRUE == pwrpriv->bips_processing)
	    && (IPS_NONE != pwrpriv->ips_mode_req)
	   )
		return _TRUE;

	if (rf_off == pwrpriv->rf_pwrstate)
		return _TRUE;

	rtw_hal_get_hwreg(padapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);
	if (_FALSE == bMacPwrCtrlOn)
		return _TRUE;
#endif
	return _FALSE;
}

u8 halbtcoutsrc_UnderLps(PBTC_COEXIST pBtCoexist)
{
	return GLBtcWiFiInLPS;
}

u8 halbtcoutsrc_Under32K(PBTC_COEXIST pBtCoexist)
{
	/* todo: the method to check whether wifi is under 32K or not */
	return _FALSE;
}

void halbtcoutsrc_DisplayCoexStatistics(PBTC_COEXIST pBtCoexist)
{
#if 0
	PADAPTER padapter = (PADAPTER)pBtCoexist->Adapter;
	PBT_MGNT pBtMgnt = &padapter->MgntInfo.BtInfo.BtMgnt;
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	u8 *cliBuf = pBtCoexist->cliBuf;
	u1Byte			i, j;
	u1Byte			tmpbuf[BTC_TMP_BUF_SHORT];


	if (gl_coex_offload.cnt_h2c_sent) {
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Coex h2c notify]============");
		CL_PRINTF(cliBuf);

		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = H2c(%d)/Ack(%d)", "Coex h2c/c2h overall statistics",
			gl_coex_offload.cnt_h2c_sent, gl_coex_offload.cnt_c2h_ack);
		for (j = 0; j < COL_STATUS_MAX; j++) {
			if (gl_coex_offload.status[j]) {
				CL_SPRINTF(tmpbuf, BTC_TMP_BUF_SHORT, ", %s:%d", coexH2cResultString[j], gl_coex_offload.status[j]);
				CL_STRNCAT(cliBuf, BT_TMP_BUF_SIZE, tmpbuf, BTC_TMP_BUF_SHORT);
			}
		}
		CL_PRINTF(cliBuf);
	}
	for (i = 0; i < COL_OP_WIFI_OPCODE_MAX; i++) {
		if (gl_coex_offload.h2c_record[i].count) {
			/*==========================================*/
			/*	H2C result statistics*/
			/*==========================================*/
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = total:%d", coexOpcodeString[i], gl_coex_offload.h2c_record[i].count);
			for (j = 0; j < COL_STATUS_MAX; j++) {
				if (gl_coex_offload.h2c_record[i].status[j]) {
					CL_SPRINTF(tmpbuf, BTC_TMP_BUF_SHORT, ", %s:%d", coexH2cResultString[j], gl_coex_offload.h2c_record[i].status[j]);
					CL_STRNCAT(cliBuf, BT_TMP_BUF_SIZE, tmpbuf, BTC_TMP_BUF_SHORT);
				}
			}
			CL_PRINTF(cliBuf);
			/*==========================================*/
			/*	H2C/C2H content*/
			/*==========================================*/
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = ", "H2C / C2H content");
			for (j = 0; j < gl_coex_offload.h2c_record[i].h2c_len; j++) {
				CL_SPRINTF(tmpbuf, BTC_TMP_BUF_SHORT, "%02x ", gl_coex_offload.h2c_record[i].h2c_buf[j]);
				CL_STRNCAT(cliBuf, BT_TMP_BUF_SIZE, tmpbuf, 3);
			}
			if (gl_coex_offload.h2c_record[i].c2h_ack_len) {
				CL_STRNCAT(cliBuf, BT_TMP_BUF_SIZE, "/ ", 2);
				for (j = 0; j < gl_coex_offload.h2c_record[i].c2h_ack_len; j++) {
					CL_SPRINTF(tmpbuf, BTC_TMP_BUF_SHORT, "%02x ", gl_coex_offload.h2c_record[i].c2h_ack_buf[j]);
					CL_STRNCAT(cliBuf, BT_TMP_BUF_SIZE, tmpbuf, 3);
				}
			}
			CL_PRINTF(cliBuf);
			/*==========================================*/
		}
	}

	if (gl_coex_offload.cnt_c2h_ind) {
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Coex c2h indication]============");
		CL_PRINTF(cliBuf);

		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = Ind(%d)", "C2H indication statistics",
			   gl_coex_offload.cnt_c2h_ind);
		for (j = 0; j < COL_STATUS_MAX; j++) {
			if (gl_coex_offload.c2h_ind_status[j]) {
				CL_SPRINTF(tmpbuf, BTC_TMP_BUF_SHORT, ", %s:%d", coexH2cResultString[j], gl_coex_offload.c2h_ind_status[j]);
				CL_STRNCAT(cliBuf, BT_TMP_BUF_SIZE, tmpbuf, BTC_TMP_BUF_SHORT);
			}
		}
		CL_PRINTF(cliBuf);
	}
	for (i = 0; i < COL_IND_MAX; i++) {
		if (gl_coex_offload.c2h_ind_record[i].count) {
			/*==========================================*/
			/*	H2C result statistics*/
			/*==========================================*/
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = total:%d", coexIndTypeString[i], gl_coex_offload.c2h_ind_record[i].count);
			for (j = 0; j < COL_STATUS_MAX; j++) {
				if (gl_coex_offload.c2h_ind_record[i].status[j]) {
					CL_SPRINTF(tmpbuf, BTC_TMP_BUF_SHORT, ", %s:%d", coexH2cResultString[j], gl_coex_offload.c2h_ind_record[i].status[j]);
					CL_STRNCAT(cliBuf, BT_TMP_BUF_SIZE, tmpbuf, BTC_TMP_BUF_SHORT);
				}
			}
			CL_PRINTF(cliBuf);
			/*==========================================*/
			/*	content*/
			/*==========================================*/
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = ", "C2H indication content");
			for (j = 0; j < gl_coex_offload.c2h_ind_record[i].ind_len; j++) {
				CL_SPRINTF(tmpbuf, BTC_TMP_BUF_SHORT, "%02x ", gl_coex_offload.c2h_ind_record[i].ind_buf[j]);
				CL_STRNCAT(cliBuf, BT_TMP_BUF_SIZE, tmpbuf, 3);
			}
			CL_PRINTF(cliBuf);
			/*==========================================*/
		}
	}

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Statistics]============");
	CL_PRINTF(cliBuf);

#if (H2C_USE_IO_THREAD != 1)
	for (i = 0; i < H2C_STATUS_MAX; i++) {
		if (pHalData->h2cStatistics[i]) {
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = [%s] = %d", "H2C statistics", \
				   h2cStaString[i], pHalData->h2cStatistics[i]);
			CL_PRINTF(cliBuf);
		}
	}
#else
	for (i = 0; i < IO_STATUS_MAX; i++) {
		if (Adapter->ioComStr.ioH2cStatistics[i]) {
			CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = [%s] = %d", "H2C statistics", \
				ioStaString[i], Adapter->ioComStr.ioH2cStatistics[i]);
			CL_PRINTF(cliBuf);
		}
	}
#endif
#if 0
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x", "lastHMEBoxNum", \
		   pHalData->LastHMEBoxNum);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = 0x%x / 0x%x", "LastOkH2c/FirstFailH2c(fwNotRead)", \
		   pHalData->lastSuccessH2cEid, pHalData->firstFailedH2cEid);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d/ %d/ %d", "c2hIsr/c2hIntr/clr1AF/noRdy/noBuf", \
		pHalData->InterruptLog.nIMR_C2HCMD, DBG_Var.c2hInterruptCnt, DBG_Var.c2hClrReadC2hCnt,
		   DBG_Var.c2hNotReadyCnt, DBG_Var.c2hBufAlloFailCnt);
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d", "c2hPacket", \
		   DBG_Var.c2hPacketCnt);
	CL_PRINTF(cliBuf);
#endif
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d", "Periodical/ DbgCtrl", \
		pBtCoexist->statistics.cntPeriodical, pBtCoexist->statistics.cntDbgCtrl);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d/ %d", "PowerOn/InitHw/InitCoexDm/RfStatus", \
		pBtCoexist->statistics.cntPowerOn, pBtCoexist->statistics.cntInitHwConfig, pBtCoexist->statistics.cntInitCoexDm,
		   pBtCoexist->statistics.cntRfStatusNotify);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d/ %d/ %d", "Ips/Lps/Scan/Connect/Mstatus", \
		pBtCoexist->statistics.cntIpsNotify, pBtCoexist->statistics.cntLpsNotify,
		pBtCoexist->statistics.cntScanNotify, pBtCoexist->statistics.cntConnectNotify,
		   pBtCoexist->statistics.cntMediaStatusNotify);
	CL_PRINTF(cliBuf);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d", "Special pkt/Bt info/ bind",
		pBtCoexist->statistics.cntSpecialPacketNotify, pBtCoexist->statistics.cntBtInfoNotify,
		   pBtCoexist->statistics.cntBind);
	CL_PRINTF(cliBuf);
#endif
	PADAPTER		padapter = pBtCoexist->Adapter;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	u8				*cliBuf = pBtCoexist->cli_buf;
#if 0
	if (pHalData->EEPROMBluetoothCoexist == 1) 
#endif
        {
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s", "============[Coex Status]============");
		CL_PRINTF(cliBuf);
	//	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "IsBtDisabled", rtw_btcoex_IsBtDisabled(padapter));
		CL_PRINTF(cliBuf);
	//	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d ", "IsBtControlLps", rtw_btcoex_IsBtControlLps(padapter));
		CL_PRINTF(cliBuf);
	}
}

void halbtcoutsrc_DisplayBtLinkInfo(PBTC_COEXIST pBtCoexist)
{
#if 0
	PADAPTER padapter = (PADAPTER)pBtCoexist->Adapter;
	PBT_MGNT pBtMgnt = &padapter->MgntInfo.BtInfo.BtMgnt;
	u8 *cliBuf = pBtCoexist->cliBuf;
	u8 i;


	if (pBtCoexist->stack_info.profile_notified) {
		for (i = 0; i < pBtMgnt->ExtConfig.NumberOfACL; i++) {
			if (pBtMgnt->ExtConfig.HCIExtensionVer >= 1) {
				CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s/ %s/ %s", "Bt link type/spec/role", \
					BtProfileString[pBtMgnt->ExtConfig.aclLink[i].BTProfile],
					BtSpecString[pBtMgnt->ExtConfig.aclLink[i].BTCoreSpec],
					BtLinkRoleString[pBtMgnt->ExtConfig.aclLink[i].linkRole]);
				CL_PRINTF(cliBuf);
			} else {
				CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s/ %s", "Bt link type/spec", \
					BtProfileString[pBtMgnt->ExtConfig.aclLink[i].BTProfile],
					BtSpecString[pBtMgnt->ExtConfig.aclLink[i].BTCoreSpec]);
				CL_PRINTF(cliBuf);
			}
		}
	}
#endif
}

void halbtcoutsrc_DisplayWifiStatus(PBTC_COEXIST pBtCoexist)
{
	PADAPTER	padapter = pBtCoexist->Adapter;
#if 0    
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
#endif
	u8	*cliBuf = pBtCoexist->cli_buf;
	s32	wifiRssi = 0, btHsRssi = 0;
	BOOLEAN	bScan = _FALSE, bLink = _FALSE, bRoam = _FALSE, bWifiBusy = _FALSE, bWifiUnderBMode = _FALSE;
	u32	wifiBw = BTC_WIFI_BW_HT20, wifiTrafficDir = BTC_WIFI_TRAFFIC_TX, wifiFreq = BTC_FREQ_2_4G;
	u32	wifiLinkStatus = 0x0;
	BOOLEAN	bBtHsOn = _FALSE, bLowPower = _FALSE;
	u8	wifiChnl = 0, wifiP2PChnl = 0, nScanAPNum = 0, FwPSState;
	u32	iqk_cnt_total = 0, iqk_cnt_ok = 0, iqk_cnt_fail = 0;

	wifiLinkStatus = halbtcoutsrc_GetWifiLinkStatus(pBtCoexist);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d/ %d/ %d", "STA/vWifi/HS/p2pGo/p2pGc", \
		((wifiLinkStatus & WIFI_STA_CONNECTED) ? 1 : 0), ((wifiLinkStatus & WIFI_AP_CONNECTED) ? 1 : 0),
		((wifiLinkStatus & WIFI_HS_CONNECTED) ? 1 : 0), ((wifiLinkStatus & WIFI_P2P_GO_CONNECTED) ? 1 : 0),
		   ((wifiLinkStatus & WIFI_P2P_GC_CONNECTED) ? 1 : 0));
	CL_PRINTF(cliBuf);

	pBtCoexist->btc_get(pBtCoexist, BTC_GET_BL_WIFI_SCAN, &bScan);
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_BL_WIFI_LINK, &bLink);
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_BL_WIFI_ROAM, &bRoam);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d ", "Link/ Roam/ Scan", \
		bLink, bRoam, bScan);
	CL_PRINTF(cliBuf);	

	pBtCoexist->btc_get(pBtCoexist, BTC_GET_U4_WIFI_IQK_TOTAL, &iqk_cnt_total);
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_U4_WIFI_IQK_OK, &iqk_cnt_ok);
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_U4_WIFI_IQK_FAIL, &iqk_cnt_fail);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d/ %d/ %d ", "IQK Total/ OK/ Fail", \
		iqk_cnt_total, iqk_cnt_ok, iqk_cnt_fail);
	CL_PRINTF(cliBuf);
#if 0	
	if (wifiLinkStatus & WIFI_STA_CONNECTED) {
		CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s", "IOT Peer", GLBtcIotPeerString[padapter->mlmeextpriv.mlmext_info.assoc_AP_vendor]);
		CL_PRINTF(cliBuf);
	}
#endif 
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_S4_WIFI_RSSI, &wifiRssi);
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_U1_WIFI_DOT11_CHNL, &wifiChnl);
	if ((wifiLinkStatus & WIFI_P2P_GO_CONNECTED) || (wifiLinkStatus & WIFI_P2P_GC_CONNECTED)) 
		pBtCoexist->btc_get(pBtCoexist, BTC_GET_U1_WIFI_P2P_CHNL, &wifiP2PChnl);	
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %d dBm/ %d/ %d", "RSSI/ STA_Chnl/ P2P_Chnl", \
		wifiRssi -100, wifiChnl, wifiP2PChnl);
	CL_PRINTF(cliBuf);

	pBtCoexist->btc_get(pBtCoexist, BTC_GET_BL_WIFI_UNDER_5G, &wifiFreq);
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_U4_WIFI_BW, &wifiBw);
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_BL_WIFI_BUSY, &bWifiBusy);
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_U4_WIFI_TRAFFIC_DIRECTION, &wifiTrafficDir);
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_BL_WIFI_UNDER_B_MODE, &bWifiUnderBMode);
	pBtCoexist->btc_get(pBtCoexist, BTC_GET_U1_AP_NUM, &nScanAPNum);
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s / %s/ %s/ %d ", "Band/ BW/ Traffic/ APCnt", \
		GLBtcWifiFreqString[wifiFreq], ((bWifiUnderBMode) ? "11b" : GLBtcWifiBwString[wifiBw]),
		((!bWifiBusy) ? "idle" : ((BTC_WIFI_TRAFFIC_TX == wifiTrafficDir) ? "uplink" : "downlink")),
		   nScanAPNum);
	CL_PRINTF(cliBuf);

	/* power status */
	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %s%s%s", "Power Status", \
		((halbtcoutsrc_UnderIps(pBtCoexist) == _TRUE) ? "IPS ON" : "IPS OFF"),
		((halbtcoutsrc_UnderLps(pBtCoexist) == _TRUE) ? ", LPS ON" : ", LPS OFF"),
		((halbtcoutsrc_Under32K(pBtCoexist) == _TRUE) ? ", 32k" : ""));
	CL_PRINTF(cliBuf);

	CL_SPRINTF(cliBuf, BT_TMP_BUF_SIZE, "\r\n %-35s = %02x %02x %02x %02x %02x %02x (0x%x/0x%x)", "Power mode cmd(lps/rpwm)", \
		   pBtCoexist->pwrModeVal[0], pBtCoexist->pwrModeVal[1],
		   pBtCoexist->pwrModeVal[2], pBtCoexist->pwrModeVal[3],
		   pBtCoexist->pwrModeVal[4], pBtCoexist->pwrModeVal[5],
		   pBtCoexist->bt_info.lps_val,
		   pBtCoexist->bt_info.rpwm_val);
	CL_PRINTF(cliBuf);
}

void halbtcoutsrc_DisplayDbgMsg(void *pBtcContext, u8 dispType)
{
	PBTC_COEXIST pBtCoexist;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	switch (dispType) {
	case BTC_DBG_DISP_COEX_STATISTICS:
		halbtcoutsrc_DisplayCoexStatistics(pBtCoexist);
		break;
	case BTC_DBG_DISP_BT_LINK_INFO:
		halbtcoutsrc_DisplayBtLinkInfo(pBtCoexist);
		break;
	case BTC_DBG_DISP_WIFI_STATUS:
		halbtcoutsrc_DisplayWifiStatus(pBtCoexist);
		break;
	default:
		break;
	}
}

/* ************************************
 *		IO related function
 * ************************************ */
u8 halbtcoutsrc_Read1Byte(void *pBtcContext, u32 RegAddr)
{
	PBTC_COEXIST pBtCoexist;
	BT_COEX_HAL_ADAPTER* priv;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	priv = (BT_COEX_HAL_ADAPTER* )pBtCoexist->Adapter;

	return RTL_R8(RegAddr);

}

u16 halbtcoutsrc_Read2Byte(void *pBtcContext, u32 RegAddr)
{
	PBTC_COEXIST pBtCoexist;
	BT_COEX_HAL_ADAPTER* priv;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	priv = (BT_COEX_HAL_ADAPTER* )pBtCoexist->Adapter;

	return	RTL_R16(RegAddr);
	
}

u32 halbtcoutsrc_Read4Byte(void *pBtcContext, u32 RegAddr)
{
	PBTC_COEXIST pBtCoexist;
	BT_COEX_HAL_ADAPTER* priv;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	priv = (BT_COEX_HAL_ADAPTER* )pBtCoexist->Adapter;

	return	RTL_R32(RegAddr);

}

void halbtcoutsrc_Write1Byte(void *pBtcContext, u32 RegAddr, u8 Data)
{
	PBTC_COEXIST pBtCoexist;
	BT_COEX_HAL_ADAPTER* priv;

	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	priv = (BT_COEX_HAL_ADAPTER* )pBtCoexist->Adapter;

	RTL_W8(RegAddr,Data);
		
}

void halbtcoutsrc_BitMaskWrite1Byte(void *pBtcContext, u32 regAddr, u8 bitMask, u8 data1b)
{
	PBTC_COEXIST pBtCoexist;
	BT_COEX_HAL_ADAPTER* priv;
	u8 originalValue, bitShift;
	u8 i;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	priv = (BT_COEX_HAL_ADAPTER* )pBtCoexist->Adapter;
	originalValue = 0;
	bitShift = 0;

	if (bitMask != 0xff) {
		originalValue = RTL_R8(regAddr);
		
		for (i = 0; i <= 7; i++) {
			if ((bitMask >> i) & 0x1)
				break;
		}
		bitShift = i;

		data1b = (originalValue & ~bitMask) | ((data1b << bitShift) & bitMask);
	}

	//rtw_write8(padapter, regAddr, data1b);
	RTL_W8(regAddr,data1b);

}

void halbtcoutsrc_Write2Byte(void *pBtcContext, u32 RegAddr, u16 Data)
{
	PBTC_COEXIST pBtCoexist;
	BT_COEX_HAL_ADAPTER* priv;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	priv = (BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter;

	//rtw_write16(padapter, RegAddr, Data);
	RTL_W16(RegAddr, Data);

}

void halbtcoutsrc_Write4Byte(void *pBtcContext, u32 RegAddr, u32 Data)
{
	PBTC_COEXIST pBtCoexist;
	BT_COEX_HAL_ADAPTER* priv;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	priv = (BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter;

	//rtw_write32(padapter, RegAddr, Data);
	RTL_W32(RegAddr, Data);
	
}

void halbtcoutsrc_WriteLocalReg1Byte(void *pBtcContext, u32 RegAddr, u8 Data)
{
#define SDIO_LOCAL_BASE				0x10250000
	PBTC_COEXIST		pBtCoexist = (PBTC_COEXIST)pBtcContext;
	BT_COEX_HAL_ADAPTER*	priv = (BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter;

	if (BTC_INTF_SDIO == pBtCoexist->chip_interface){
		//w_write8(Adapter, SDIO_LOCAL_BASE | RegAddr, Data);
		RTL_W8(SDIO_LOCAL_BASE | RegAddr, Data);
	}
	else{
		//w_write8(Adapter, RegAddr, Data);
		RTL_W8(RegAddr, Data);
	}
 
}

void halbtcoutsrc_SetBbReg(void *pBtcContext, u32 RegAddr, u32 BitMask, u32 Data)
{
	PBTC_COEXIST pBtCoexist;
	PADAPTER padapter;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	padapter = pBtCoexist->Adapter;

	PHY_SetBBReg(padapter, RegAddr, BitMask, Data);
}


u32 halbtcoutsrc_GetBbReg(void *pBtcContext, u32 RegAddr, u32 BitMask)
{
	PBTC_COEXIST pBtCoexist;
	PADAPTER padapter;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	padapter = pBtCoexist->Adapter;

	return PHY_QueryBBReg(padapter, RegAddr, BitMask);
}

void halbtcoutsrc_SetRfReg(void *pBtcContext, u8 eRFPath, u32 RegAddr, u32 BitMask, u32 Data)
{
	PBTC_COEXIST pBtCoexist;
	PADAPTER padapter;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	padapter = pBtCoexist->Adapter;

	PHY_SetRFReg(padapter, eRFPath, RegAddr, BitMask, Data);
}

u32 halbtcoutsrc_GetRfReg(void *pBtcContext, u8 eRFPath, u32 RegAddr, u32 BitMask)
{
	PBTC_COEXIST pBtCoexist;
	PADAPTER padapter;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	padapter = pBtCoexist->Adapter;

	return PHY_QueryRFReg(padapter, eRFPath, RegAddr, BitMask,1);
}

u16 halbtcoutsrc_SetBtReg(void *pBtcContext, u8 RegType, u32 RegAddr, u32 Data)
{
	PBTC_COEXIST pBtCoexist;
	u16 ret = BT_STATUS_BT_OP_SUCCESS;

	pBtCoexist = (PBTC_COEXIST)pBtcContext;

	if (halbtcoutsrc_IsHwMailboxExist(pBtCoexist) == _TRUE) {
		u8 buf[3] = {0};
		_irqL irqL;
		u8 op_code;
		u8 status;
#if 0
		_enter_critical_mutex(&GLBtcBtMpOperLock, &irqL);

		Data = cpu_to_le32(Data);
		op_code = BT_OP_WRITE_REG_VALUE;
		status = _btmpoper_cmd(pBtCoexist, op_code, 0, (u8 *)&Data, 3);
		if (status != BT_STATUS_BT_OP_SUCCESS)
			ret = SET_BT_MP_OPER_RET(op_code, status);
		else {
			buf[0] = RegType;
			*(u16 *)(buf + 1) = cpu_to_le16((u16)RegAddr);
			op_code = BT_OP_WRITE_REG_ADDR;
			status = _btmpoper_cmd(pBtCoexist, op_code, 0, buf, 3);
			if (status != BT_STATUS_BT_OP_SUCCESS)
				ret = SET_BT_MP_OPER_RET(op_code, status);
		}

		_exit_critical_mutex(&GLBtcBtMpOperLock, &irqL);
#endif        
	} else
		ret = BT_STATUS_NOT_IMPLEMENT;

	return ret;
}

u8 halbtcoutsrc_SetBtAntDetection(void *pBtcContext, u8 txTime, u8 btChnl)
{
	/* Always return _FALSE since we don't implement this yet */
#if 0
	PBTC_COEXIST		pBtCoexist = (PBTC_COEXIST)pBtcContext;
	PADAPTER			Adapter = pBtCoexist->Adapter;
	u1Byte				btCanTx = 0;
	BOOLEAN			bStatus = FALSE;

	bStatus = NDBG_SetBtAntDetection(Adapter, txTime, btChnl, &btCanTx);
	if (bStatus && btCanTx)
		return _TRUE;
	else
		return _FALSE;
#else
	return _FALSE;
#endif
}

u16 halbtcoutsrc_GetBtReg(void *pBtcContext, u8 RegType, u32 RegAddr, u32 *data)
{
	PBTC_COEXIST pBtCoexist;
	u16 ret = BT_STATUS_BT_OP_SUCCESS;

	pBtCoexist = (PBTC_COEXIST)pBtcContext;

	if (halbtcoutsrc_IsHwMailboxExist(pBtCoexist) == _TRUE) {
		u8 buf[3] = {0};
		_irqL irqL;
		u8 op_code;
		u8 status;

		buf[0] = RegType;
		*(u16 *)(buf + 1) = cpu_to_le16((u16)RegAddr);
#if 0
		_enter_critical_mutex(&GLBtcBtMpOperLock, &irqL);

		op_code = BT_OP_READ_REG;
		status = _btmpoper_cmd(pBtCoexist, op_code, 0, buf, 3);
		if (status == BT_STATUS_BT_OP_SUCCESS)
			*data = le16_to_cpu(*(u16 *)GLBtcBtMpRptRsp);
		else
			ret = SET_BT_MP_OPER_RET(op_code, status);

		_exit_critical_mutex(&GLBtcBtMpOperLock, &irqL);
#endif
	} else
		ret = BT_STATUS_NOT_IMPLEMENT;

	return ret;
}

void halbtcoutsrc_FillH2cCmd(void *pBtcContext, u8 elementId, u32 cmdLen, u8 *pCmdBuffer)
{
	PBTC_COEXIST pBtCoexist;
	BT_COEX_HAL_ADAPTER *padapter;


	pBtCoexist = (PBTC_COEXIST)pBtcContext;
	padapter = (BT_COEX_HAL_ADAPTER *)pBtCoexist->Adapter;

	rtw_hal_fill_h2c_cmd(padapter, elementId, cmdLen, pCmdBuffer);
}

static void halbtcoutsrc_coex_offload_init(void)
{
	u1Byte	i;

	gl_coex_offload.h2c_req_num = 0;
	gl_coex_offload.cnt_h2c_sent = 0;
	gl_coex_offload.cnt_c2h_ack = 0;
	gl_coex_offload.cnt_c2h_ind = 0;

	for (i = 0; i < COL_MAX_H2C_REQ_NUM; i++)
		init_completion(&gl_coex_offload.c2h_event[i]);
}

static COL_H2C_STATUS halbtcoutsrc_send_h2c(PADAPTER Adapter, PCOL_H2C pcol_h2c, u16 h2c_cmd_len)
{
	COL_H2C_STATUS		h2c_status = COL_STATUS_C2H_OK;
	u8				i;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
	reinit_completion(&gl_coex_offload.c2h_event[pcol_h2c->req_num]);		/* set event to un signaled state */
#else
	INIT_COMPLETION(gl_coex_offload.c2h_event[pcol_h2c->req_num]);
#endif

	if (TRUE) {
#if 0	/*(USE_HAL_MAC_API == 1) */
		if (RT_STATUS_SUCCESS == HAL_MAC_Send_BT_COEX(&GET_HAL_MAC_INFO(Adapter), (pu1Byte)(pcol_h2c), (u4Byte)h2c_cmd_len, 1)) {
			if (!wait_for_completion_timeout(&gl_coex_offload.c2h_event[pcol_h2c->req_num], 20)) {
				h2c_status = COL_STATUS_H2C_TIMTOUT;
			}
		} else {
			h2c_status = COL_STATUS_H2C_HALMAC_FAIL;
		}
#endif
	}

	return h2c_status;
}

static COL_H2C_STATUS halbtcoutsrc_check_c2h_ack(PADAPTER Adapter, PCOL_SINGLE_H2C_RECORD pH2cRecord)
{
	COL_H2C_STATUS	c2h_status = COL_STATUS_C2H_OK;
	PCOL_H2C		p_h2c_cmd = (PCOL_H2C)&pH2cRecord->h2c_buf[0];
	u8			req_num = p_h2c_cmd->req_num;
	PCOL_C2H_ACK	p_c2h_ack = (PCOL_C2H_ACK)&gl_coex_offload.c2h_ack_buf[req_num];


	if ((COL_C2H_ACK_HDR_LEN + p_c2h_ack->ret_len) > gl_coex_offload.c2h_ack_len[req_num]) {
		c2h_status = COL_STATUS_COEX_DATA_OVERFLOW;
		return c2h_status;
	}
	/* else */
	{
		_rtw_memmove(&pH2cRecord->c2h_ack_buf[0], &gl_coex_offload.c2h_ack_buf[req_num], gl_coex_offload.c2h_ack_len[req_num]);
		pH2cRecord->c2h_ack_len = gl_coex_offload.c2h_ack_len[req_num];
	}


	if (p_c2h_ack->req_num != p_h2c_cmd->req_num) {
		c2h_status = COL_STATUS_C2H_REQ_NUM_MISMATCH;
	} else if (p_c2h_ack->opcode_ver != p_h2c_cmd->opcode_ver) {
		c2h_status = COL_STATUS_C2H_OPCODE_VER_MISMATCH;
	} else {
		c2h_status = p_c2h_ack->status;
	}

	return c2h_status;
}

COL_H2C_STATUS halbtcoutsrc_CoexH2cProcess(void *pBtCoexist,
		u8 opcode, u8 opcode_ver, u8 *ph2c_par, u8 h2c_par_len)
{
	PADAPTER			Adapter = ((struct btc_coexist *)pBtCoexist)->Adapter;
	u8				H2C_Parameter[BTC_TMP_BUF_SHORT] = {0};
	PCOL_H2C			pcol_h2c = (PCOL_H2C)&H2C_Parameter[0];
	u16				paraLen = 0;
	COL_H2C_STATUS		h2c_status = COL_STATUS_C2H_OK, c2h_status = COL_STATUS_C2H_OK;
	COL_H2C_STATUS		ret_status = COL_STATUS_C2H_OK;
	u16				i, col_h2c_len = 0;

	pcol_h2c->opcode = opcode;
	pcol_h2c->opcode_ver = opcode_ver;
	pcol_h2c->req_num = gl_coex_offload.h2c_req_num;
	gl_coex_offload.h2c_req_num++;
	gl_coex_offload.h2c_req_num %= 16;

	_rtw_memmove(&pcol_h2c->buf[0], ph2c_par, h2c_par_len);


	col_h2c_len = h2c_par_len + 2;	/* 2=sizeof(OPCode, OPCode_version and  Request number) */
	BT_PrintData(Adapter, "[COL], H2C cmd: ", col_h2c_len, H2C_Parameter);

	gl_coex_offload.cnt_h2c_sent++;

	gl_coex_offload.h2c_record[opcode].count++;
	gl_coex_offload.h2c_record[opcode].h2c_len = col_h2c_len;
	_rtw_memmove((PVOID)&gl_coex_offload.h2c_record[opcode].h2c_buf[0], (PVOID)pcol_h2c, col_h2c_len);

	h2c_status = halbtcoutsrc_send_h2c(Adapter, pcol_h2c, col_h2c_len);

	gl_coex_offload.h2c_record[opcode].c2h_ack_len = 0;

	if (COL_STATUS_C2H_OK == h2c_status) {
		/* if reach here, it means H2C get the correct c2h response, */
		c2h_status = halbtcoutsrc_check_c2h_ack(Adapter, &gl_coex_offload.h2c_record[opcode]);
		ret_status = c2h_status;
	} else {
		/* check h2c status error, return error status code to upper layer. */
		ret_status = h2c_status;
	}
	gl_coex_offload.h2c_record[opcode].status[ret_status]++;
	gl_coex_offload.status[ret_status]++;

	return ret_status;
}

u32 halbtcoutsrc_GetBtCoexSupportedFeature(void *pBtcContext)
{
	struct btc_coexist *pBtCoexist=(struct btc_coexist *)pBtcContext;
	PADAPTER			Adapter=pBtCoexist->Adapter;
	u4Byte				coexSupportedFeature=0x0;
	u1Byte				opcodeVer = 1;
	BOOLEAN				status = false;


	return coexSupportedFeature;
}

u32 halbtcoutsrc_GetBtCoexSupportedVersion(void *pBtcContext)
{
	struct btc_coexist *pBtCoexist=(struct btc_coexist *)pBtcContext;
	PADAPTER			Adapter=pBtCoexist->Adapter;
	u32				coexSupportedVersion=0x0;
	u8				opcodeVer = 1;
	u8				status = false;


	return coexSupportedVersion;
}	

u32 halbtcoutsrc_GetPhydmVersion(void *pBtcContext)
{
	struct btc_coexist *pBtCoexist = (struct btc_coexist *)pBtcContext;
	PADAPTER		Adapter = pBtCoexist->Adapter;

#ifdef CONFIG_RTL8192E
	return RELEASE_VERSION_8192E;
#endif

#ifdef CONFIG_RTL8821A
	return RELEASE_VERSION_8821A;
#endif

#ifdef CONFIG_RTL8723B
	return RELEASE_VERSION_8723B;
#endif

#ifdef CONFIG_RTL8812A
	return RELEASE_VERSION_8812A;
#endif

#ifdef CONFIG_RTL8703B
	return RELEASE_VERSION_8703B;
#endif

#ifdef CONFIG_RTL8822B
	return RELEASE_VERSION_8822B;
#endif

#ifdef CONFIG_RTL8723D
	return RELEASE_VERSION_8723D;
#endif

}

#if 0
static void BT_CoexOffloadRecordErrC2hAck(PADAPTER	Adapter)
{
	PADAPTER		pDefaultAdapter = GetDefaultAdapter(Adapter);

	if (pDefaultAdapter != Adapter)
		return;

	if (!hal_btcoex_IsBtExist(Adapter))
		return;

	gl_coex_offload.cnt_c2h_ack++;

	gl_coex_offload.status[COL_STATUS_INVALID_C2H_LEN]++;
}

static void BT_CoexOffloadC2hAckCheck(PADAPTER	Adapter, u8 *tmpBuf, u8 length)
{
	PADAPTER		pDefaultAdapter = GetDefaultAdapter(Adapter);
	PCOL_C2H_ACK	p_c2h_ack = NULL;
	u8			req_num = 0xff;

	if (pDefaultAdapter != Adapter)
		return;

	if (!hal_btcoex_IsBtExist(Adapter))
		return;

	gl_coex_offload.cnt_c2h_ack++;

	if (length < COL_C2H_ACK_HDR_LEN) {		/* c2h ack length must >= 3 (status, opcode_ver, req_num and ret_len) */
		gl_coex_offload.status[COL_STATUS_INVALID_C2H_LEN]++;
	} else {
		BT_PrintData(Adapter, "[COL], c2h ack:", length, tmpBuf);

		p_c2h_ack = (PCOL_C2H_ACK)tmpBuf;
		req_num = p_c2h_ack->req_num;

		_rtw_memmove(&gl_coex_offload.c2h_ack_buf[req_num][0], tmpBuf, length);
		gl_coex_offload.c2h_ack_len[req_num] = length;

		complete(&gl_coex_offload.c2h_event[req_num]);
	}
}

static void BT_CoexOffloadC2hIndCheck(PADAPTER Adapter, u8 *tmpBuf, u8 length)
{
	PADAPTER		pDefaultAdapter = GetDefaultAdapter(Adapter);
	PCOL_C2H_IND	p_c2h_ind = NULL;
	u8			ind_type = 0, ind_version = 0, ind_length = 0;

	if (pDefaultAdapter != Adapter)
		return;

	if (!hal_btcoex_IsBtExist(Adapter))
		return;

	gl_coex_offload.cnt_c2h_ind++;

	if (length < COL_C2H_IND_HDR_LEN) {		/* c2h indication length must >= 3 (type, version and length) */
		gl_coex_offload.c2h_ind_status[COL_STATUS_INVALID_C2H_LEN]++;
	} else {
		BT_PrintData(Adapter, "[COL], c2h indication:", length, tmpBuf);

		p_c2h_ind = (PCOL_C2H_IND)tmpBuf;
		ind_type = p_c2h_ind->type;
		ind_version = p_c2h_ind->version;
		ind_length = p_c2h_ind->length;

		_rtw_memmove(&gl_coex_offload.c2h_ind_buf[0], tmpBuf, length);
		gl_coex_offload.c2h_ind_len = length;

		/* log */
		gl_coex_offload.c2h_ind_record[ind_type].count++;
		gl_coex_offload.c2h_ind_record[ind_type].status[COL_STATUS_C2H_OK]++;
		_rtw_memmove(&gl_coex_offload.c2h_ind_record[ind_type].ind_buf[0], tmpBuf, length);
		gl_coex_offload.c2h_ind_record[ind_type].ind_len = length;

		gl_coex_offload.c2h_ind_status[COL_STATUS_C2H_OK]++;
		/*TODO: need to check c2h indication length*/
		/* TODO: Notification */
	}
}

void BT_CoexOffloadC2hCheck(PADAPTER Adapter, u8 *Buffer, u8 Length)
{
#if 0 /*(USE_HAL_MAC_API == 1)*/
	u8	c2hSubCmdId = 0, c2hAckLen = 0, h2cCmdId = 0, h2cSubCmdId = 0, c2hIndLen = 0;

	BT_PrintData(Adapter, "[COL], c2h packet:", Length - 2, Buffer + 2);
	c2hSubCmdId = (u1Byte)C2H_HDR_GET_C2H_SUB_CMD_ID(Buffer);

	if (c2hSubCmdId == C2H_SUB_CMD_ID_H2C_ACK_HDR ||
	    c2hSubCmdId == C2H_SUB_CMD_ID_BT_COEX_INFO) {
		if (c2hSubCmdId == C2H_SUB_CMD_ID_H2C_ACK_HDR) {
			/* coex c2h ack */
			h2cCmdId = (u1Byte)H2C_ACK_HDR_GET_H2C_CMD_ID(Buffer);
			h2cSubCmdId = (u1Byte)H2C_ACK_HDR_GET_H2C_SUB_CMD_ID(Buffer);
			if (h2cCmdId == 0xff && h2cSubCmdId == 0x60) {
				c2hAckLen = (u1Byte)C2H_HDR_GET_LEN(Buffer);
				if (c2hAckLen >= 8)
					BT_CoexOffloadC2hAckCheck(Adapter, &Buffer[12], (u1Byte)(c2hAckLen - 8));
				else
					BT_CoexOffloadRecordErrC2hAck(Adapter);
			}
		} else if (c2hSubCmdId == C2H_SUB_CMD_ID_BT_COEX_INFO) {
			/* coex c2h indication */
			c2hIndLen = (u1Byte)C2H_HDR_GET_LEN(Buffer);
			BT_CoexOffloadC2hIndCheck(Adapter, &Buffer[4], (u1Byte)c2hIndLen);
		}
	}
#endif
}
#endif

/* ************************************
 *		Extern functions called by other module
 * ************************************ */
u8 EXhalbtcoutsrc_BindBtCoexWithAdapter(void *padapter)
{
	PBTC_COEXIST		pBtCoexist = &GLBtCoexist;
	u8	antNum = 1, chipType = 0, singleAntPath = 0;
#if 0    
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA((PADAPTER)padapter);
#endif

	if (pBtCoexist->bBinded)
		return _FALSE;
	else
		pBtCoexist->bBinded = _TRUE;

	pBtCoexist->statistics.cnt_bind++;

	pBtCoexist->Adapter = padapter;

	pBtCoexist->stack_info.profile_notified = _FALSE;

	pBtCoexist->bt_info.bt_ctrl_agg_buf_size = _FALSE;
	pBtCoexist->bt_info.agg_buf_size = 5;

	pBtCoexist->bt_info.increase_scan_dev_num = _FALSE;
	pBtCoexist->bt_info.miracast_plus_bt = _FALSE;

	antNum = rtw_btcoex_get_pg_ant_num((PADAPTER)padapter);
	EXhalbtcoutsrc_SetAntNum(BT_COEX_ANT_TYPE_PG, antNum);

	if (antNum == 1) {
		singleAntPath = rtw_btcoex_get_pg_single_ant_path((PADAPTER)padapter);
		EXhalbtcoutsrc_SetSingleAntPath(singleAntPath);
	}

	/* set default antenna position to main  port */
	pBtCoexist->board_info.btdm_ant_pos = BTC_ANTENNA_AT_MAIN_PORT;

	pBtCoexist->board_info.btdm_ant_det_finish = _FALSE;
	pBtCoexist->board_info.btdm_ant_num_by_ant_det = 1;

	pBtCoexist->board_info.tfbga_package = rtw_btcoex_is_tfbga_package_type((PADAPTER)padapter);

	pBtCoexist->board_info.rfe_type = rtw_btcoex_get_pg_rfe_type((PADAPTER)padapter);

	pBtCoexist->board_info.ant_div_cfg = rtw_btcoex_get_ant_div_cfg((PADAPTER)padapter);

	return _TRUE;
}

u8 EXhalbtcoutsrc_InitlizeVariables(void *padapter)
{
	PBTC_COEXIST pBtCoexist = &GLBtCoexist;

	/* pBtCoexist->statistics.cntBind++; */

	halbtcoutsrc_DbgInit();

	halbtcoutsrc_coex_offload_init();

#ifdef CONFIG_PCI_HCI
	pBtCoexist->chip_interface = BTC_INTF_PCI;
#elif defined(CONFIG_USB_HCI)
	pBtCoexist->chip_interface = BTC_INTF_USB;
#elif defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
	pBtCoexist->chip_interface = BTC_INTF_SDIO;
#else
	pBtCoexist->chip_interface = BTC_INTF_UNKNOWN;
#endif

	EXhalbtcoutsrc_BindBtCoexWithAdapter(padapter);

	pBtCoexist->btc_read_1byte = halbtcoutsrc_Read1Byte;
	pBtCoexist->btc_write_1byte = halbtcoutsrc_Write1Byte;
	pBtCoexist->btc_write_1byte_bitmask = halbtcoutsrc_BitMaskWrite1Byte;
	pBtCoexist->btc_read_2byte = halbtcoutsrc_Read2Byte;
	pBtCoexist->btc_write_2byte = halbtcoutsrc_Write2Byte;
	pBtCoexist->btc_read_4byte = halbtcoutsrc_Read4Byte;
	pBtCoexist->btc_write_4byte = halbtcoutsrc_Write4Byte;
	pBtCoexist->btc_write_local_reg_1byte = halbtcoutsrc_WriteLocalReg1Byte;

	pBtCoexist->btc_set_bb_reg = halbtcoutsrc_SetBbReg;
	pBtCoexist->btc_get_bb_reg = halbtcoutsrc_GetBbReg;

	pBtCoexist->btc_set_rf_reg = halbtcoutsrc_SetRfReg;
	pBtCoexist->btc_get_rf_reg = halbtcoutsrc_GetRfReg;

	pBtCoexist->btc_fill_h2c = halbtcoutsrc_FillH2cCmd;
	pBtCoexist->btc_disp_dbg_msg = halbtcoutsrc_DisplayDbgMsg;

	pBtCoexist->btc_get = halbtcoutsrc_Get;
	pBtCoexist->btc_set = halbtcoutsrc_Set;
	pBtCoexist->btc_get_bt_reg = halbtcoutsrc_GetBtReg;
	pBtCoexist->btc_set_bt_reg = halbtcoutsrc_SetBtReg;
	pBtCoexist->btc_set_bt_ant_detection = halbtcoutsrc_SetBtAntDetection;
	pBtCoexist->btc_coex_h2c_process = halbtcoutsrc_CoexH2cProcess;
	pBtCoexist->btc_get_bt_coex_supported_feature = halbtcoutsrc_GetBtCoexSupportedFeature;
	pBtCoexist->btc_get_bt_coex_supported_version= halbtcoutsrc_GetBtCoexSupportedVersion;
	pBtCoexist->btc_get_bt_phydm_version = halbtcoutsrc_GetPhydmVersion;

	pBtCoexist->cli_buf = &GLBtcDbgBuf[0];

	GLBtcWiFiInScanState = _FALSE;

	GLBtcWiFiInIQKState = _FALSE;

	GLBtcWiFiInIPS = _FALSE;

	GLBtcWiFiInLPS = _FALSE;

	GLBtcBtCoexAliveRegistered = _FALSE;

	/* BT Control H2C/C2H*/
	GLBtcBtMpOperSeq = 0;
#if 0    
	_rtw_mutex_init(&GLBtcBtMpOperLock);
	_init_timer(&GLBtcBtMpOperTimer, ((PADAPTER)padapter)->pnetdev, _btmpoper_timer_hdl, pBtCoexist);
	_rtw_init_sema(&GLBtcBtMpRptSema, 0);
#endif
	GLBtcBtMpRptSeq = 0;
	GLBtcBtMpRptStatus = 0;
	memset(GLBtcBtMpRptRsp, 0, C2H_MAX_SIZE);

	GLBtcBtMpRptRspSize = 0;
	GLBtcBtMpRptWait = 0;
	GLBtcBtMpRptWiFiOK = 0;
	GLBtcBtMpRptBTOK = 0;

	return _TRUE;
}

void EXhalbtcoutsrc_PowerOnSetting(PBTC_COEXIST pBtCoexist)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
/*

	if (IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2)
			ex_halbtc8723b2ant_power_on_setting(pBtCoexist);
		else if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8723b1ant_power_on_setting(pBtCoexist);
	}

	if (IS_HARDWARE_TYPE_8723D(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2)
			ex_halbtc8723d2ant_power_on_setting(pBtCoexist);
		else if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8723d1ant_power_on_setting(pBtCoexist);
	}

	if (IS_HARDWARE_TYPE_8822B(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8822b1ant_power_on_setting(pBtCoexist);
	}

	if (IS_HARDWARE_TYPE_8821C(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2)
			ex_halbtc8821c2ant_power_on_setting(pBtCoexist);
		else if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8821c1ant_power_on_setting(pBtCoexist);
	}
	*/
}

void EXhalbtcoutsrc_PreLoadFirmware(PBTC_COEXIST pBtCoexist)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->statistics.cnt_pre_load_firmware++;
/*
	if (IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2)
			ex_halbtc8723b2ant_pre_load_firmware(pBtCoexist);
		else if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8723b1ant_pre_load_firmware(pBtCoexist);
	}

	if (IS_HARDWARE_TYPE_8723D(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2)
			ex_halbtc8723d2ant_pre_load_firmware(pBtCoexist);
		else if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8723d1ant_pre_load_firmware(pBtCoexist);
	}

	if (IS_HARDWARE_TYPE_8821C(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2)
			ex_halbtc8821c2ant_pre_load_firmware(pBtCoexist);
		else if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8821c1ant_pre_load_firmware(pBtCoexist);
	}
	*/
}

void EXhalbtcoutsrc_init_hw_config(PBTC_COEXIST pBtCoexist, u8 bWifiOnly)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->statistics.cnt_init_hw_config++;

	if (IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_init_hw_config(pBtCoexist, bWifiOnly);
		
	}else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_init_hw_config(pBtCoexist, bWifiOnly);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_init_hw_config(pBtCoexist, bWifiOnly);
		}
	}

}

void EXhalbtcoutsrc_init_coex_dm(PBTC_COEXIST pBtCoexist)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->statistics.cnt_init_coex_dm++;

	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_init_coex_dm(pBtCoexist);
	}else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_init_coex_dm(pBtCoexist);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_init_coex_dm(pBtCoexist);
		}
	}

	pBtCoexist->initilized = _TRUE;
}

void EXhalbtcoutsrc_ips_notify(PBTC_COEXIST pBtCoexist, u8 type)
{
	u8	ipsType;

	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->statistics.cnt_ips_notify++;
	if (pBtCoexist->manual_control)
		return;

	if (IPS_NONE == type) {
		ipsType = BTC_IPS_LEAVE;
		GLBtcWiFiInIPS = _FALSE;
	} else {
		ipsType = BTC_IPS_ENTER;
		GLBtcWiFiInIPS = _TRUE;
	}

	/* All notify is called in cmd thread, don't need to leave low power again
	*	halbtcoutsrc_LeaveLowPower(pBtCoexist); */
	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_ips_notify(pBtCoexist, ipsType);
	} else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_ips_notify(pBtCoexist, ipsType);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_ips_notify(pBtCoexist, ipsType);
		}
	} 

	/*	halbtcoutsrc_NormalLowPower(pBtCoexist); */
}

void EXhalbtcoutsrc_lps_notify(PBTC_COEXIST pBtCoexist, u8 type)
{
	u8 lpsType;


	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->statistics.cnt_lps_notify++;
	if (pBtCoexist->manual_control)
		return;

	if (PS_MODE_ACTIVE == type) {
		lpsType = BTC_LPS_DISABLE;
		GLBtcWiFiInLPS = _FALSE;
	} else {
		lpsType = BTC_LPS_ENABLE;
		GLBtcWiFiInLPS = _TRUE;
	}

	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_lps_notify(pBtCoexist, lpsType);
	} else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_lps_notify(pBtCoexist, lpsType);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_lps_notify(pBtCoexist, lpsType);
		}
	} 

}

void EXhalbtcoutsrc_scan_notify(PBTC_COEXIST pBtCoexist, u8 type)
{
	u8	scanType;

	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cnt_scan_notify++;
	if (pBtCoexist->manual_control)
		return;

	if (type) {
		scanType = BTC_SCAN_START;
		GLBtcWiFiInScanState = _TRUE;
	} else {
		scanType = BTC_SCAN_FINISH;
		GLBtcWiFiInScanState = _FALSE;
	}

	/* All notify is called in cmd thread, don't need to leave low power again
	*	halbtcoutsrc_LeaveLowPower(pBtCoexist); */

	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_scan_notify(pBtCoexist, scanType);
	} else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_scan_notify(pBtCoexist, scanType);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_scan_notify(pBtCoexist, scanType);
		}
	} 

	/*	halbtcoutsrc_NormalLowPower(pBtCoexist); */
}

void EXhalbtcoutsrc_connect_notify(PBTC_COEXIST pBtCoexist, u8 action)
{
	u8	assoType;

	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cnt_connect_notify++;
	if (pBtCoexist->manual_control)
		return;

	if (action)
		assoType = BTC_ASSOCIATE_START;
	else
		assoType = BTC_ASSOCIATE_FINISH;

	/* All notify is called in cmd thread, don't need to leave low power again
	*	halbtcoutsrc_LeaveLowPower(pBtCoexist); */
	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_connect_notify(pBtCoexist, assoType);
	} else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_connect_notify(pBtCoexist, assoType);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_connect_notify(pBtCoexist, assoType);
		}
	} 

	/*	halbtcoutsrc_NormalLowPower(pBtCoexist); */
}

void EXhalbtcoutsrc_media_status_notify(PBTC_COEXIST pBtCoexist, RT_MEDIA_STATUS mediaStatus)
{
	u8 mStatus;

	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->statistics.cnt_media_status_notify++;
	if (pBtCoexist->manual_control)
		return;

	if (RT_MEDIA_CONNECT == mediaStatus)
		mStatus = BTC_MEDIA_CONNECT;
	else
		mStatus = BTC_MEDIA_DISCONNECT;

	/* All notify is called in cmd thread, don't need to leave low power again
	*	halbtcoutsrc_LeaveLowPower(pBtCoexist); */
	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_media_status_notify(pBtCoexist, mStatus);
	}else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_media_status_notify(pBtCoexist, mStatus);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_media_status_notify(pBtCoexist, mStatus);
		}
	} 

	/*	halbtcoutsrc_NormalLowPower(pBtCoexist); */
}

void EXhalbtcoutsrc_specific_packet_notify(PBTC_COEXIST pBtCoexist, u8 pktType)
{
	u8	packetType;

	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cnt_specific_packet_notify++;
	if (pBtCoexist->manual_control)
		return;

	if (PACKET_DHCP == pktType)
		packetType = BTC_PACKET_DHCP;
	else if (PACKET_EAPOL == pktType)
		packetType = BTC_PACKET_EAPOL;
	else if (PACKET_ARP == pktType)
		packetType = BTC_PACKET_ARP;
	else {
		packetType = BTC_PACKET_UNKNOWN;
		return;
	}

	/* All notify is called in cmd thread, don't need to leave low power again
	*	halbtcoutsrc_LeaveLowPower(pBtCoexist); */
	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_specific_packet_notify(pBtCoexist, packetType);
	}else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_specific_packet_notify(pBtCoexist, packetType);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_specific_packet_notify(pBtCoexist, packetType);
		}
	}

	/*	halbtcoutsrc_NormalLowPower(pBtCoexist); */
}

void EXhalbtcoutsrc_bt_info_notify(PBTC_COEXIST pBtCoexist, u8 *tmpBuf, u8 length)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->statistics.cnt_bt_info_notify++;

	/* All notify is called in cmd thread, don't need to leave low power again
	*	halbtcoutsrc_LeaveLowPower(pBtCoexist); */
	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_bt_info_notify(pBtCoexist, tmpBuf, length);
	}else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_bt_info_notify(pBtCoexist, tmpBuf, length);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_bt_info_notify(pBtCoexist, tmpBuf, length);
		}
	} 

	/*	halbtcoutsrc_NormalLowPower(pBtCoexist); */
}

VOID
EXhalbtcoutsrc_RfStatusNotify(
	IN	PBTC_COEXIST		pBtCoexist,
	IN	u1Byte				type
)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cnt_rf_status_notify++;

	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		//null
	}else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		//null
	} 
	
}

void EXhalbtcoutsrc_StackOperationNotify(PBTC_COEXIST pBtCoexist, u8 type)
{
	//empdy
}

void EXhalbtcoutsrc_halt_notify(PBTC_COEXIST pBtCoexist)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_halt_notify(pBtCoexist);
	} else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_halt_notify(pBtCoexist);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_halt_notify(pBtCoexist);
		}
	} 

	pBtCoexist->bBinded = FALSE;
}

void EXhalbtcoutsrc_SwitchBtTRxMask(PBTC_COEXIST pBtCoexist)
{
#if 0    
	if (IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2) {
			halbtcoutsrc_SetBtReg(pBtCoexist, 0, 0x3c, 0x01); /* BT goto standby while GNT_BT 1-->0 */
		} else if (pBtCoexist->board_info.btdm_ant_num == 1) {
			halbtcoutsrc_SetBtReg(pBtCoexist, 0, 0x3c, 0x15); /* BT goto standby while GNT_BT 1-->0 */
		}
	}
#endif    
}

void EXhalbtcoutsrc_pnp_notify(PBTC_COEXIST pBtCoexist, u8 pnpState)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	/*  */
	/* currently only 1ant we have to do the notification, */
	/* once pnp is notified to sleep state, we have to leave LPS that we can sleep normally. */
	/*  */
	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_pnp_notify(pBtCoexist, pnpState);
	} else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_pnp_notify(pBtCoexist, pnpState);
		}
	} 
}

void EXhalbtcoutsrc_ScoreBoardStatusNotify(PBTC_COEXIST pBtCoexist, u8 *tmpBuf, u8 length)
{
#if 0
	if (IS_HARDWARE_TYPE_8703B(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8703b1ant_ScoreBoardStatusNotify(pBtCoexist, tmpBuf, length);
	} else if (IS_HARDWARE_TYPE_8723D(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 1)
			;/*ex_halbtc8723d1ant_ScoreBoardStatusNotify(pBtCoexist, tmpBuf, length);*/
	}
#endif
}

void EXhalbtcoutsrc_CoexDmSwitch(PBTC_COEXIST pBtCoexist)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cnt_coex_dm_switch++;
#if 0

	halbtcoutsrc_LeaveLowPower(pBtCoexist);

	if (IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 1) {
			pBtCoexist->stop_coex_dm = TRUE;
			ex_halbtc8723b1ant_coex_dm_reset(pBtCoexist);
			EXhalbtcoutsrc_SetAntNum(BT_COEX_ANT_TYPE_DETECTED, 2);
			ex_halbtc8723b2ant_init_hw_config(pBtCoexist, FALSE);
			ex_halbtc8723b2ant_init_coex_dm(pBtCoexist);
			pBtCoexist->stop_coex_dm = FALSE;
		}
	} else if (IS_HARDWARE_TYPE_8723D(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 1) {
			pBtCoexist->stop_coex_dm = TRUE;
			ex_halbtc8723d1ant_coex_dm_reset(pBtCoexist);
			EXhalbtcoutsrc_SetAntNum(BT_COEX_ANT_TYPE_DETECTED, 2);
			ex_halbtc8723d2ant_init_hw_config(pBtCoexist, FALSE);
			ex_halbtc8723d2ant_init_coex_dm(pBtCoexist);
			pBtCoexist->stop_coex_dm = FALSE;
		}
	}

	halbtcoutsrc_NormalLowPower(pBtCoexist);
#endif
}

void EXhalbtcoutsrc_periodical(PBTC_COEXIST pBtCoexist)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	pBtCoexist->statistics.cnt_periodical++;

	/* Periodical should be called in cmd thread, */
	/* don't need to leave low power again
	*	halbtcoutsrc_LeaveLowPower(pBtCoexist); */
	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_periodical(pBtCoexist);
	} else if (IS_HARDWARE_TYPE_8812((BT_COEX_HAL_ADAPTER *)pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_periodical(pBtCoexist);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_periodical(pBtCoexist);
		}
	} 

		halbtcoutsrc_NormalLowPower(pBtCoexist); 
}

void EXhalbtcoutsrc_dbg_control(PBTC_COEXIST pBtCoexist, u8 opCode, u8 opLen, u8 *pData)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->statistics.cnt_dbg_ctrl++;

	/* This function doesn't be called yet, */
	/* default no need to leave low power to avoid deadlock
	*	halbtcoutsrc_LeaveLowPower(pBtCoexist); */

	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		ex_halbtc8197F_dbg_control(pBtCoexist, opCode, opLen, pData);
	} else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_dbg_control(pBtCoexist, opCode, opLen, pData);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_dbg_control(pBtCoexist, opCode, opLen, pData);
		}
	} 
	/*	halbtcoutsrc_NormalLowPower(pBtCoexist); */
}

#if 0
VOID
EXhalbtcoutsrc_AntennaDetection(
	IN	PBTC_COEXIST			pBtCoexist,
	IN	u4Byte					centFreq,
	IN	u4Byte					offset,
	IN	u4Byte					span,
	IN	u4Byte					seconds
)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	/* Need to refine the following power save operations to enable this function in the future */
#if 0
	IPSDisable(pBtCoexist->Adapter, FALSE, 0);
	LeisurePSLeave(pBtCoexist->Adapter, LPS_DISABLE_BT_COEX);
#endif

	if (IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8723b1ant_AntennaDetection(pBtCoexist, centFreq, offset, span, seconds);
	}

	/* IPSReturn(pBtCoexist->Adapter, 0xff); */
}
#endif

void EXhalbtcoutsrc_StackUpdateProfileInfo(void)
{
   
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	PBTC_COEXIST pBtCoexist = &GLBtCoexist;
	BT_COEX_HAL_ADAPTER * padapter = (BT_COEX_HAL_ADAPTER *)GLBtCoexist.Adapter;
	PBT_MGNT pBtMgnt = &padapter->pshare->bt_coex_comm_data.coex_info.BtMgnt;
	u8 i;

	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->stack_info.profile_notified = _TRUE;

	pBtCoexist->stack_info.num_of_link =
		pBtMgnt->ExtConfig.NumberOfACL + pBtMgnt->ExtConfig.NumberOfSCO;

	/* reset first */
	pBtCoexist->stack_info.bt_link_exist = _FALSE;
	pBtCoexist->stack_info.sco_exist = _FALSE;
	pBtCoexist->stack_info.acl_exist = _FALSE;
	pBtCoexist->stack_info.a2dp_exist = _FALSE;
	pBtCoexist->stack_info.hid_exist = _FALSE;
	pBtCoexist->stack_info.num_of_hid = 0;
	pBtCoexist->stack_info.pan_exist = _FALSE;

	if (!pBtMgnt->ExtConfig.NumberOfACL)
		pBtCoexist->stack_info.min_bt_rssi = 0;

	if (pBtCoexist->stack_info.num_of_link) {
		pBtCoexist->stack_info.bt_link_exist = _TRUE;
		if (pBtMgnt->ExtConfig.NumberOfSCO)
			pBtCoexist->stack_info.sco_exist = _TRUE;
		if (pBtMgnt->ExtConfig.NumberOfACL)
			pBtCoexist->stack_info.acl_exist = _TRUE;
	}

	for (i = 0; i < pBtMgnt->ExtConfig.NumberOfACL; i++) {
		if (BT_PROFILE_A2DP == pBtMgnt->ExtConfig.aclLink[i].BTProfile)
			pBtCoexist->stack_info.a2dp_exist = _TRUE;
		else if (BT_PROFILE_PAN == pBtMgnt->ExtConfig.aclLink[i].BTProfile)
			pBtCoexist->stack_info.pan_exist = _TRUE;
		else if (BT_PROFILE_HID == pBtMgnt->ExtConfig.aclLink[i].BTProfile) {
			pBtCoexist->stack_info.hid_exist = _TRUE;
			pBtCoexist->stack_info.num_of_hid++;
		} else
			pBtCoexist->stack_info.unknown_acl_exist = _TRUE;
	}
#endif /* CONFIG_BT_COEXIST_SOCKET_TRX */

}

void EXhalbtcoutsrc_UpdateMinBtRssi(s8 btRssi)
{
	PBTC_COEXIST pBtCoexist = &GLBtCoexist;

	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->stack_info.min_bt_rssi = btRssi;
}

void EXhalbtcoutsrc_SetHciVersion(u16 hciVersion)
{
	PBTC_COEXIST pBtCoexist = &GLBtCoexist;

	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->stack_info.hci_version = hciVersion;
}

void EXhalbtcoutsrc_SetBtPatchVersion(u16 btHciVersion, u16 btPatchVersion)
{
	PBTC_COEXIST pBtCoexist = &GLBtCoexist;

	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	pBtCoexist->bt_info.bt_real_fw_ver = btPatchVersion;
	pBtCoexist->bt_info.bt_hci_ver = btHciVersion;
}

#if 0
void EXhalbtcoutsrc_SetBtExist(u8 bBtExist)
{
	GLBtCoexist.boardInfo.bBtExist = bBtExist;
}
#endif
void EXhalbtcoutsrc_SetChipType(u8 chipType)
{
	switch (chipType) {
	default:
	case BT_2WIRE:
	case BT_ISSC_3WIRE:
	case BT_ACCEL:
	case BT_RTL8756:
		GLBtCoexist.board_info.bt_chip_type = BTC_CHIP_UNDEF;
		break;
	case BT_CSR_BC4:
		GLBtCoexist.board_info.bt_chip_type = BTC_CHIP_CSR_BC4;
		break;
	case BT_CSR_BC8:
		GLBtCoexist.board_info.bt_chip_type = BTC_CHIP_CSR_BC8;
		break;
	case BT_RTL8723A:
		GLBtCoexist.board_info.bt_chip_type = BTC_CHIP_RTL8723A;
		break;
	case BT_RTL8821:
		GLBtCoexist.board_info.bt_chip_type = BTC_CHIP_RTL8821;
		break;
	case BT_RTL8723B:
		GLBtCoexist.board_info.bt_chip_type = BTC_CHIP_RTL8723B;
		break;
	}
}

void EXhalbtcoutsrc_SetAntNum(u8 type, u8 antNum)
{
	if (BT_COEX_ANT_TYPE_PG == type) {
		GLBtCoexist.board_info.pg_ant_num = antNum;
		GLBtCoexist.board_info.btdm_ant_num = antNum;
#if 0
/* The antenna position: Main (default) or Aux for pgAntNum=2 && btdmAntNum =1 */
		/* The antenna position should be determined by auto-detect mechanism */
		/* The following is assumed to main, and those must be modified if y auto-detect mechanism is ready */
		if ((GLBtCoexist.board_info.pg_ant_num == 2) && (GLBtCoexist.board_info.btdm_ant_num == 1))
			GLBtCoexist.board_info.btdm_ant_pos = BTC_ANTENNA_AT_MAIN_PORT;
		else
			GLBtCoexist.board_info.btdm_ant_pos = BTC_ANTENNA_AT_MAIN_PORT;
#endif
	} else if (BT_COEX_ANT_TYPE_ANTDIV == type) {
		GLBtCoexist.board_info.btdm_ant_num = antNum;
		/* GLBtCoexist.boardInfo.btdmAntPos = BTC_ANTENNA_AT_MAIN_PORT;	 */
	} else if (BT_COEX_ANT_TYPE_DETECTED == type) {
		GLBtCoexist.board_info.btdm_ant_num = antNum;
		/* GLBtCoexist.boardInfo.btdmAntPos = BTC_ANTENNA_AT_MAIN_PORT; */
	}
}

/*
 * Currently used by 8723b only, S0 or S1
 *   */
void EXhalbtcoutsrc_SetSingleAntPath(u8 singleAntPath)
{
	GLBtCoexist.board_info.single_ant_path = singleAntPath;
}

void EXhalbtcoutsrc_DisplayBtCoexInfo(PBTC_COEXIST pBtCoexist)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	//halbtcoutsrc_LeaveLowPower(pBtCoexist);

	if (IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER *)pBtCoexist->Adapter)) {
		ex_halbtc8197F_display_coex_info(pBtCoexist);
	}else if (IS_HARDWARE_TYPE_8812((BT_COEX_HAL_ADAPTER *)pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
			//ex_halbtc8812a2ant_display_coex_info(pBtCoexist);
		}
		else if (pBtCoexist->board_info.btdm_ant_num == 1){
			//ex_halbtc8812a1ant_display_coex_info(pBtCoexist);
		}
	}

    
	halbtcoutsrc_NormalLowPower(pBtCoexist);
}

void EXhalbtcoutsrc_DisplayAntDetection(PBTC_COEXIST pBtCoexist)
{
	if (!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;

	halbtcoutsrc_LeaveLowPower(pBtCoexist);
#if 0
	if (IS_HARDWARE_TYPE_8723B(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8723b1ant_display_ant_detection(pBtCoexist);
	} else if (IS_HARDWARE_TYPE_8821C(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2)
			ex_halbtc8821c2ant_display_ant_detection(pBtCoexist);
		else if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8821c1ant_display_ant_detection(pBtCoexist);
	}
#endif
	halbtcoutsrc_NormalLowPower(pBtCoexist);
}

void ex_halbtcoutsrc_pta_off_on_notify(PBTC_COEXIST pBtCoexist, u8 bBTON)
{
	if(IS_HARDWARE_TYPE_8197F((BT_COEX_HAL_ADAPTER*)pBtCoexist->Adapter)){
		//null
	}else if (IS_HARDWARE_TYPE_8812(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2){
//			ex_halbtc8812a2ant_pta_off_on_notify(pBtCoexist, (bBTON == _TRUE) ? BTC_BT_ON : BTC_BT_OFF);
		}
	}

}

void EXhalbtcoutsrc_set_rfe_type(u8 type)
{
	GLBtCoexist.board_info.rfe_type= type;
}

void EXhalbtcoutsrc_switchband_notify(struct btc_coexist *pBtCoexist, u8 type)
{
	if(!halbtcoutsrc_IsBtCoexistAvailable(pBtCoexist))
		return;
	
	if(pBtCoexist->manual_control)
		return;


	halbtcoutsrc_LeaveLowPower(pBtCoexist);
#if 0
	if(IS_HARDWARE_TYPE_8822B(pBtCoexist->Adapter)) {
		if(pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8822b1ant_switchband_notify(pBtCoexist, type);
	} else if (IS_HARDWARE_TYPE_8821C(pBtCoexist->Adapter)) {
		if (pBtCoexist->board_info.btdm_ant_num == 2)
			ex_halbtc8821c2ant_switchband_notify(pBtCoexist, type);
		else if (pBtCoexist->board_info.btdm_ant_num == 1)
			ex_halbtc8821c1ant_switchband_notify(pBtCoexist, type);
	}
#endif
	halbtcoutsrc_NormalLowPower(pBtCoexist);
}

static void halbt_init_hw_config92C(BT_COEX_HAL_ADAPTER * padapter)
{
	PHAL_DATA_TYPE pHalData;
	u8 u1Tmp;

#if 0
	pHalData = GET_HAL_DATA(padapter);
	if ((pHalData->bt_coexist.btChipType == BT_CSR_BC4) ||
	    (pHalData->bt_coexist.btChipType == BT_CSR_BC8)) {
		if (pHalData->rf_type == RF_1T1R) {
			/* Config to 1T1R */
			u1Tmp = rtw_read8(padapter, rOFDM0_TRxPathEnable);
			u1Tmp &= ~BIT(1);
			rtw_write8(padapter, rOFDM0_TRxPathEnable, u1Tmp);
			RT_DISP(FBT, BT_TRACE, ("[BTCoex], BT write 0xC04 = 0x%x\n", u1Tmp));

			u1Tmp = rtw_read8(padapter, rOFDM1_TRxPathEnable);
			u1Tmp &= ~BIT(1);
			rtw_write8(padapter, rOFDM1_TRxPathEnable, u1Tmp);
			RT_DISP(FBT, BT_TRACE, ("[BTCoex], BT write 0xD04 = 0x%x\n", u1Tmp));
		}
	}
#endif
}

static void halbt_init_hw_config92D(BT_COEX_HAL_ADAPTER * padapter)
{
	PHAL_DATA_TYPE pHalData;
	u8 u1Tmp;
#if 0
	pHalData = GET_HAL_DATA(padapter);
	if ((pHalData->bt_coexist.btChipType == BT_CSR_BC4) ||
	    (pHalData->bt_coexist.btChipType == BT_CSR_BC8)) {
		if (pHalData->rf_type == RF_1T1R) {
			/* Config to 1T1R */
			u1Tmp = rtw_read8(padapter, rOFDM0_TRxPathEnable);
			u1Tmp &= ~BIT(1);
			rtw_write8(padapter, rOFDM0_TRxPathEnable, u1Tmp);
			RT_DISP(FBT, BT_TRACE, ("[BTCoex], BT write 0xC04 = 0x%x\n", u1Tmp));

			u1Tmp = rtw_read8(padapter, rOFDM1_TRxPathEnable);
			u1Tmp &= ~BIT(1);
			rtw_write8(padapter, rOFDM1_TRxPathEnable, u1Tmp);
			RT_DISP(FBT, BT_TRACE, ("[BTCoex], BT write 0xD04 = 0x%x\n", u1Tmp));
		}
	}
#endif
}

/*
 * Description:
 *	Run BT-Coexist mechansim or not
 *
 */
void hal_btcoex_SetBTCoexist(BT_COEX_HAL_ADAPTER *padapter, u8 bBtExist)
{
	//bt_coex_comm_data = (padapter.pshare->bt_coex_comm_data);

	padapter->pshare->bt_coex_comm_data.bt_coex_status.bBtExist = bBtExist;
	
}

/*
 * Dewcription:
 *	Check is co-exist mechanism enabled or not
 *
 * Return:
 *	_TRUE	Enable BT co-exist mechanism
 *	_FALSE	Disable BT co-exist mechanism
 */
u8 hal_btcoex_IsBtExist(BT_COEX_HAL_ADAPTER *padapter)
{
	//PHAL_DATA_TYPE	pHalData;


	//pHalData = GET_HAL_DATA(padapter);
	//    return true;
    
	//return pHalData->bt_coexist.bBtExist;

	struct bt_coexist_common_data bt_coex_comm_data = padapter->pshare->bt_coex_comm_data ;

	return bt_coex_comm_data.bt_coex_status.bBtExist;

}

u8 hal_btcoex_IsBtDisabled(BT_COEX_HAL_ADAPTER *padapter)
{
	if (!hal_btcoex_IsBtExist(padapter))
		return _TRUE;

	if (GLBtCoexist.bt_info.bt_disabled)
		return _TRUE;
	else
		return _FALSE;
}

void hal_btcoex_SetChipType(BT_COEX_HAL_ADAPTER * padapter, u8 chipType)
{
	//PHAL_DATA_TYPE	pHalData;

	//pHalData = GET_HAL_DATA(padapter);
   
	//pHalData->bt_coexist.btChipType = chipType;

	padapter->pshare->bt_coex_comm_data.bt_coex_status.btChipType = chipType;

}

void hal_btcoex_SetPgAntNum(BT_COEX_HAL_ADAPTER * padapter, u8 antNum)
{
	//PHAL_DATA_TYPE	pHalData;

	//pHalData = GET_HAL_DATA(padapter);

	//pHalData->bt_coexist.btTotalAntNum = antNum;

	padapter->pshare->bt_coex_comm_data.bt_coex_status.btTotalAntNum = antNum;

}

u8 hal_btcoex_Initialize(BT_COEX_HAL_ADAPTER *padapter)
{
//	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	u8 ret;

	memset(&GLBtCoexist, 0, sizeof(GLBtCoexist));
	
	hal_btcoex_SetBTCoexist(padapter, rtw_btcoex_get_bt_coexist(padapter));
	hal_btcoex_SetChipType(padapter, rtw_btcoex_get_chip_type(padapter));
	hal_btcoex_SetPgAntNum(padapter, rtw_btcoex_get_pg_ant_num(padapter));
	
	ret = EXhalbtcoutsrc_InitlizeVariables((void *)padapter);

	return ret;
}

void hal_btcoex_PowerOnSetting(BT_COEX_HAL_ADAPTER * padapter)
{
	EXhalbtcoutsrc_PowerOnSetting(&GLBtCoexist);
}

void hal_btcoex_PreLoadFirmware(BT_COEX_HAL_ADAPTER * padapter)
{
	EXhalbtcoutsrc_PreLoadFirmware(&GLBtCoexist);
}

void hal_btcoex_InitHwConfig(BT_COEX_HAL_ADAPTER * padapter, u8 bWifiOnly)
{
	if (!hal_btcoex_IsBtExist(padapter))
		return;
	
	EXhalbtcoutsrc_init_hw_config(&GLBtCoexist, bWifiOnly);
	EXhalbtcoutsrc_init_coex_dm(&GLBtCoexist);
}

void hal_btcoex_IpsNotify(BT_COEX_HAL_ADAPTER * padapter, u8 type)
{
	EXhalbtcoutsrc_ips_notify(&GLBtCoexist, type);
}

void hal_btcoex_LpsNotify(BT_COEX_HAL_ADAPTER * padapter, u8 type)
{
	EXhalbtcoutsrc_lps_notify(&GLBtCoexist, type);
}

void hal_btcoex_ScanNotify(BT_COEX_HAL_ADAPTER * padapter, u8 type)
{
	EXhalbtcoutsrc_scan_notify(&GLBtCoexist, type);
}

void hal_btcoex_ConnectNotify(BT_COEX_HAL_ADAPTER * padapter, u8 action)
{
	EXhalbtcoutsrc_connect_notify(&GLBtCoexist, action);
}

void hal_btcoex_MediaStatusNotify(BT_COEX_HAL_ADAPTER * padapter, u8 mediaStatus)
{
	EXhalbtcoutsrc_media_status_notify(&GLBtCoexist, mediaStatus);
}

void hal_btcoex_SpecialPacketNotify(BT_COEX_HAL_ADAPTER * padapter, u8 pktType)
{
	EXhalbtcoutsrc_specific_packet_notify(&GLBtCoexist, pktType);
}

void hal_btcoex_IQKNotify(BT_COEX_HAL_ADAPTER * padapter, u8 state)
{
	GLBtcWiFiInIQKState = state;
}

void hal_btcoex_BtInfoNotify(BT_COEX_HAL_ADAPTER * padapter, u8 length, u8 *tmpBuf)
{
	if (GLBtcWiFiInIQKState == _TRUE)
		return;

	EXhalbtcoutsrc_bt_info_notify(&GLBtCoexist, tmpBuf, length);
}

void hal_btcoex_BtMpRptNotify(BT_COEX_HAL_ADAPTER * padapter, u8 length, u8 *tmpBuf)
{
	u8 extid, status, len, seq;


	if (!GLBtcBtMpRptWait)
		return;

	if ((length < 3) || (!tmpBuf))
		return;

	extid = tmpBuf[0];
	/* not response from BT FW then exit*/
	switch (extid) {
#if 0        
	case C2H_WIFI_FW_ACTIVE_RSP:
		GLBtcBtMpRptWiFiOK = 1;
		return;

	case C2H_TRIG_BY_BT_FW:
		_cancel_timer_ex(&GLBtcBtMpOperTimer);
		GLBtcBtMpRptWait = 0;
		GLBtcBtMpRptBTOK = 1;
		break;
#endif
	default:
		return;
	}

	status = tmpBuf[1] & 0xF;
	len = tmpBuf[1] >> 4;
	seq = tmpBuf[2] >> 4;

	GLBtcBtMpRptSeq = seq;
	GLBtcBtMpRptStatus = status;
	memcpy(GLBtcBtMpRptRsp, tmpBuf + 3, len);
	GLBtcBtMpRptRspSize = len;
	_rtw_up_sema(&GLBtcBtMpRptSema);
}

void hal_btcoex_SuspendNotify(BT_COEX_HAL_ADAPTER * padapter, u8 state)
{
	if (state == 1)
		state = BTC_WIFI_PNP_SLEEP;
	else
		state = BTC_WIFI_PNP_WAKE_UP;

	EXhalbtcoutsrc_pnp_notify(&GLBtCoexist, state);
}

void hal_btcoex_HaltNotify(BT_COEX_HAL_ADAPTER * padapter)
{
	EXhalbtcoutsrc_halt_notify(&GLBtCoexist);
}

void hal_btcoex_ScoreBoardStatusNotify(BT_COEX_HAL_ADAPTER * padapter, u8 length, u8 *tmpBuf)
{
	EXhalbtcoutsrc_ScoreBoardStatusNotify(&GLBtCoexist, tmpBuf, length);
}

void hal_btcoex_SwitchBtTRxMask(BT_COEX_HAL_ADAPTER * padapter)
{
	EXhalbtcoutsrc_SwitchBtTRxMask(&GLBtCoexist);
}

void hal_btcoex_Hanlder(BT_COEX_HAL_ADAPTER * padapter)
{
	EXhalbtcoutsrc_periodical(&GLBtCoexist);
}

s32 hal_btcoex_IsBTCoexRejectAMPDU(BT_COEX_HAL_ADAPTER * padapter)
{
	return (s32)GLBtCoexist.bt_info.reject_agg_pkt;
}

s32 hal_btcoex_IsBTCoexCtrlAMPDUSize(BT_COEX_HAL_ADAPTER * padapter)
{
	return (s32)GLBtCoexist.bt_info.bt_ctrl_agg_buf_size;
}

u32 hal_btcoex_GetAMPDUSize(BT_COEX_HAL_ADAPTER * padapter)
{
	return (u32)GLBtCoexist.bt_info.agg_buf_size;
}

void hal_btcoex_SetManualControl(BT_COEX_HAL_ADAPTER * padapter, u8 bmanual)
{
	GLBtCoexist.manual_control = bmanual;
}

u8 hal_btcoex_1Ant(BT_COEX_HAL_ADAPTER * padapter)
{
	if (hal_btcoex_IsBtExist(padapter) == _FALSE)
		return _FALSE;

	if (GLBtCoexist.board_info.btdm_ant_num == 1)
		return _TRUE;

	return _FALSE;
}

u8 hal_btcoex_IsBtControlLps(BT_COEX_HAL_ADAPTER * padapter)
{
	if (GLBtCoexist.bdontenterLPS == _TRUE)
		return _TRUE;
	
	if (hal_btcoex_IsBtExist(padapter) == _FALSE)
		return _FALSE;

	if (GLBtCoexist.bt_info.bt_disabled)
		return _FALSE;

	if (GLBtCoexist.bt_info.bt_ctrl_lps)
		return _TRUE;

	return _FALSE;
}

u8 hal_btcoex_IsLpsOn(BT_COEX_HAL_ADAPTER * padapter)
{
	if (GLBtCoexist.bdontenterLPS == _TRUE)
		return _FALSE;
	
	if (hal_btcoex_IsBtExist(padapter) == _FALSE)
		return _FALSE;

	if (GLBtCoexist.bt_info.bt_disabled)
		return _FALSE;

	if (GLBtCoexist.bt_info.bt_lps_on)
		return _TRUE;

	return _FALSE;
}

u8 hal_btcoex_RpwmVal(BT_COEX_HAL_ADAPTER * padapter)
{
	return GLBtCoexist.bt_info.rpwm_val;
}

u8 hal_btcoex_LpsVal(BT_COEX_HAL_ADAPTER * padapter)
{
	return GLBtCoexist.bt_info.lps_val;
}

u32 hal_btcoex_GetRaMask(BT_COEX_HAL_ADAPTER * padapter)
{
	if (!hal_btcoex_IsBtExist(padapter))
		return 0;

	if (GLBtCoexist.bt_info.bt_disabled)
		return 0;

	/* Modify by YiWei , suggest by Cosa and Jenyu
	 * Remove the limit antenna number , because 2 antenna case (ex: 8192eu)also want to get BT coex report rate mask.
	 */
	/*if (GLBtCoexist.board_info.btdm_ant_num != 1)
		return 0;*/

	return GLBtCoexist.bt_info.ra_mask;
}

void hal_btcoex_RecordPwrMode(BT_COEX_HAL_ADAPTER * padapter, u8 *pCmdBuf, u8 cmdLen)
{

	memcpy(GLBtCoexist.pwrModeVal, pCmdBuf, cmdLen);
}

void hal_btcoex_DisplayBtCoexInfo(BT_COEX_HAL_ADAPTER * padapter, u8 *pbuf, u32 bufsize)
{
	PBTCDBGINFO pinfo;


	pinfo = &GLBtcDbgInfo;
	DBG_BT_INFO_INIT(pinfo, pbuf, bufsize);
	EXhalbtcoutsrc_DisplayBtCoexInfo(&GLBtCoexist);
	DBG_BT_INFO_INIT(pinfo, NULL, 0);
}

void hal_btcoex_SetDBG(BT_COEX_HAL_ADAPTER * padapter, u32 *pDbgModule)
{
	u32 i;


	if (NULL == pDbgModule)
		return;

	for (i = 0; i < COMP_MAX; i++)
		GLBtcDbgType[i] = pDbgModule[i];
}

u32 hal_btcoex_GetDBG(BT_COEX_HAL_ADAPTER * padapter, u8 *pStrBuf, u32 bufSize)
{
	s32 count;
	u8 *pstr;
	u32 leftSize;


	if ((NULL == pStrBuf) || (0 == bufSize))
		return 0;

	count = 0;
	pstr = pStrBuf;
	leftSize = bufSize;
	/*	RTW_INFO(FUNC_ADPT_FMT ": bufsize=%d\n", FUNC_ADPT_ARG(padapter), bufSize); */

	count = snprintf(pstr, leftSize, "#define DBG\t%d\n", DBG);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;

	count = snprintf(pstr, leftSize, "BTCOEX Debug Setting:\n");
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;

	count = snprintf(pstr, leftSize,
			    "COMP_COEX: 0x%08X\n\n",
			    GLBtcDbgType[COMP_COEX]);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;

#if 0
	count = rtw_snprintf(pstr, leftSize, "INTERFACE Debug Setting Definition:\n");
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[0]=%d for INTF_INIT\n",
		    GLBtcDbgType[BTC_MSG_INTERFACE] & INTF_INIT ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[2]=%d for INTF_NOTIFY\n\n",
		    GLBtcDbgType[BTC_MSG_INTERFACE] & INTF_NOTIFY ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;

	count = rtw_snprintf(pstr, leftSize, "ALGORITHM Debug Setting Definition:\n");
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[0]=%d for BT_RSSI_STATE\n",
		GLBtcDbgType[BTC_MSG_ALGORITHM] & ALGO_BT_RSSI_STATE ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[1]=%d for WIFI_RSSI_STATE\n",
		GLBtcDbgType[BTC_MSG_ALGORITHM] & ALGO_WIFI_RSSI_STATE ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[2]=%d for BT_MONITOR\n",
		    GLBtcDbgType[BTC_MSG_ALGORITHM] & ALGO_BT_MONITOR ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[3]=%d for TRACE\n",
		    GLBtcDbgType[BTC_MSG_ALGORITHM] & ALGO_TRACE ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[4]=%d for TRACE_FW\n",
		    GLBtcDbgType[BTC_MSG_ALGORITHM] & ALGO_TRACE_FW ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[5]=%d for TRACE_FW_DETAIL\n",
		GLBtcDbgType[BTC_MSG_ALGORITHM] & ALGO_TRACE_FW_DETAIL ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[6]=%d for TRACE_FW_EXEC\n",
		GLBtcDbgType[BTC_MSG_ALGORITHM] & ALGO_TRACE_FW_EXEC ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[7]=%d for TRACE_SW\n",
		    GLBtcDbgType[BTC_MSG_ALGORITHM] & ALGO_TRACE_SW ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[8]=%d for TRACE_SW_DETAIL\n",
		GLBtcDbgType[BTC_MSG_ALGORITHM] & ALGO_TRACE_SW_DETAIL ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
	count = rtw_snprintf(pstr, leftSize, "\tbit[9]=%d for TRACE_SW_EXEC\n",
		GLBtcDbgType[BTC_MSG_ALGORITHM] & ALGO_TRACE_SW_EXEC ? 1 : 0);
	if ((count < 0) || (count >= leftSize))
		goto exit;
	pstr += count;
	leftSize -= count;
#endif

exit:
	count = pstr - pStrBuf;
	/*	RTW_INFO(FUNC_ADPT_FMT ": usedsize=%d\n", FUNC_ADPT_ARG(padapter), count); */

	return count;
}

u8 hal_btcoex_IncreaseScanDeviceNum(BT_COEX_HAL_ADAPTER * padapter)
{
	if (!hal_btcoex_IsBtExist(padapter))
		return _FALSE;

	if (GLBtCoexist.bt_info.increase_scan_dev_num)
		return _TRUE;

	return _FALSE;
}

u8 hal_btcoex_IsBtLinkExist(BT_COEX_HAL_ADAPTER * padapter)
{
	if (GLBtCoexist.bt_link_info.bt_link_exist)
		return _TRUE;

	return _FALSE;
}

void hal_btcoex_SetBtPatchVersion(BT_COEX_HAL_ADAPTER * padapter, u16 btHciVer, u16 btPatchVer)
{
	EXhalbtcoutsrc_SetBtPatchVersion(btHciVer, btPatchVer);
}

void hal_btcoex_SetHciVersion(BT_COEX_HAL_ADAPTER * padapter, u16 hciVersion)
{
	EXhalbtcoutsrc_SetHciVersion(hciVersion);
}

void hal_btcoex_StackUpdateProfileInfo(void)
{
	EXhalbtcoutsrc_StackUpdateProfileInfo();
}

void hal_btcoex_pta_off_on_notify(BT_COEX_HAL_ADAPTER * padapter, u8 bBTON)
{
	ex_halbtcoutsrc_pta_off_on_notify(&GLBtCoexist, bBTON);
}

/*
 *	Description:
 *	Setting BT coex antenna isolation type .
 *	coex mechanisn/ spital stream/ best throughput
 *	anttype = 0	,	PSTDMA	/	2SS	/	0.5T	,	bad isolation , WiFi/BT ANT Distance<15cm , (<20dB) for 2,3 antenna
 *	anttype = 1	,	PSTDMA	/	1SS	/	0.5T	,	normal isolaiton , 50cm>WiFi/BT ANT Distance>15cm , (>20dB) for 2 antenna
 *	anttype = 2	,	TDMA	/	2SS	/	T ,		normal isolaiton , 50cm>WiFi/BT ANT Distance>15cm , (>20dB) for 3 antenna
 *	anttype = 3	,	no TDMA	/	1SS	/	0.5T	,	good isolation , WiFi/BT ANT Distance >50cm , (>40dB) for 2 antenna
 *	anttype = 4	,	no TDMA	/	2SS	/	T ,		good isolation , WiFi/BT ANT Distance >50cm , (>40dB) for 3 antenna
 *	wifi only throughput ~ T
 *	wifi/BT share one antenna with SPDT
 */
void hal_btcoex_SetAntIsolationType(BT_COEX_HAL_ADAPTER * padapter, u8 anttype)
{
	//PHAL_DATA_TYPE pHalData;
	PBTC_COEXIST	pBtCoexist = &GLBtCoexist;

	/*RTW_INFO("####%s , anttype = %d  , %d\n" , __func__ , anttype , __LINE__); */
	//pHalData = GET_HAL_DATA(padapter);
	
	//pHalData->bt_coexist.btAntisolation = anttype;

	struct bt_coexist_common_data bt_coex_comm_data = padapter->pshare->bt_coex_comm_data;

	bt_coex_comm_data.bt_coex_status.btAntisolation = anttype;

	switch (bt_coex_comm_data.bt_coex_status.btAntisolation) {
	case 0:
		pBtCoexist->board_info.ant_type = (u1Byte)BTC_ANT_TYPE_0;
		break;
	case 1:
		pBtCoexist->board_info.ant_type = (u1Byte)BTC_ANT_TYPE_1;
		break;
	case 2:
		pBtCoexist->board_info.ant_type = (u1Byte)BTC_ANT_TYPE_2;
		break;
	case 3:
		pBtCoexist->board_info.ant_type = (u1Byte)BTC_ANT_TYPE_3;
		break;
	case 4:
		pBtCoexist->board_info.ant_type = (u1Byte)BTC_ANT_TYPE_4;
		break;
	}

}

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
int
hal_btcoex_ParseAntIsolationConfigFile(
	PADAPTER		Adapter,
	char			*buffer
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u32	i = 0 , j = 0;
	char	*szLine , *ptmp;
	int rtStatus = _SUCCESS;
	char param_value_string[10];
	u8 param_value;
	u8 anttype = 4;

	u8 ant_num = 3 , ant_distance = 50 , rfe_type = 1;

	typedef struct ant_isolation {
		char *param_name;  /* antenna isolation config parameter name */
		u8 *value; /* antenna isolation config parameter value */
	} ANT_ISOLATION;

	ANT_ISOLATION ant_isolation_param[] = {
		{"ANT_NUMBER" , &ant_num},
		{"ANT_DISTANCE" , &ant_distance},
		{"RFE_TYPE" , &rfe_type},
		{NULL , 0}
	};



	/* RTW_INFO("===>Hal_ParseAntIsolationConfigFile()\n" ); */

	ptmp = buffer;
	for (szLine = GetLineFromBuffer(ptmp) ; szLine != NULL; szLine = GetLineFromBuffer(ptmp)) {
		/* skip comment */
		if (IsCommentString(szLine))
			continue;

		/* RTW_INFO("%s : szLine = %s , strlen(szLine) = %d\n" , __func__ , szLine , strlen(szLine));*/
		for (j = 0 ; ant_isolation_param[j].param_name != NULL ; j++) {
			if (strstr(szLine , ant_isolation_param[j].param_name) != NULL) {
				i = 0;
				while (i < strlen(szLine)) {
					if (szLine[i] != '"')
						++i;
					else {
						/* skip only has one " */
						if (strpbrk(szLine , "\"") == strrchr(szLine , '"')) {
							RTW_INFO("Fail to parse parameters , format error!\n");
							break;
						}
						_rtw_memset((PVOID)param_value_string , 0 , 10);
						if (!ParseQualifiedString(szLine , &i , param_value_string , '"' , '"')) {
							RTW_INFO("Fail to parse parameters\n");
							return _FAIL;
						} else if (!GetU1ByteIntegerFromStringInDecimal(param_value_string , ant_isolation_param[j].value))
							RTW_INFO("Fail to GetU1ByteIntegerFromStringInDecimal\n");

						break;
					}
				}
			}
		}
	}

	/* YiWei 20140716 , for BT coex antenna isolation control */
	/* rfe_type = 0 was SPDT , rfe_type = 1 was coupler */
	if (ant_num == 3 && ant_distance >= 50)
		anttype = 3;
	else if (ant_num == 2 && ant_distance >= 50 && rfe_type == 1)
		anttype = 2;
	else if (ant_num == 3 && ant_distance >= 15 && ant_distance < 50)
		anttype = 2;
	else if (ant_num == 2 && ant_distance >= 15 && ant_distance < 50 && rfe_type == 1)
		anttype = 2;
	else if ((ant_num == 2 && ant_distance < 15 && rfe_type == 1) || (ant_num == 3 && ant_distance < 15))
		anttype = 1;
	else if (ant_num == 2 && rfe_type == 0)
		anttype = 0;
	else
		anttype = 0;

	hal_btcoex_SetAntIsolationType(Adapter, anttype);

	RTW_INFO("%s : ant_num = %d\n" , __func__ , ant_num);
	RTW_INFO("%s : ant_distance = %d\n" , __func__ , ant_distance);
	RTW_INFO("%s : rfe_type = %d\n" , __func__ , rfe_type);
	/* RTW_INFO("<===Hal_ParseAntIsolationConfigFile()\n"); */
	return rtStatus;
}


int
hal_btcoex_AntIsolationConfig_ParaFile(
	IN	PADAPTER	Adapter,
	IN	char		*pFileName
)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(Adapter);
	int	rlen = 0 , rtStatus = _FAIL;

	_rtw_memset(pHalData->para_file_buf , 0 , MAX_PARA_FILE_BUF_LEN);

	rtw_get_phy_file_path(Adapter, pFileName);
	if (rtw_is_file_readable(rtw_phy_para_file_path) == _TRUE) {
		rlen = rtw_retrieve_from_file(rtw_phy_para_file_path, pHalData->para_file_buf, MAX_PARA_FILE_BUF_LEN);
		if (rlen > 0)
			rtStatus = _SUCCESS;
	}


	if (rtStatus == _SUCCESS) {
		/*RTW_INFO("%s(): read %s ok\n", __func__ , pFileName);*/
		rtStatus = hal_btcoex_ParseAntIsolationConfigFile(Adapter , pHalData->para_file_buf);
	} else
		RTW_INFO("%s(): No File %s, Load from *** Array!\n" , __func__ , pFileName);

	return rtStatus;
}
#endif /* CONFIG_LOAD_PHY_PARA_FROM_FILE */

u16 hal_btcoex_btreg_read(BT_COEX_HAL_ADAPTER * padapter, u8 type, u16 addr, u32 *data)
{
	u16 ret = 0;

	halbtcoutsrc_LeaveLowPower(&GLBtCoexist);

	ret = halbtcoutsrc_GetBtReg(&GLBtCoexist, type, addr, data);

	halbtcoutsrc_NormalLowPower(&GLBtCoexist);

	return ret;
}

u16 hal_btcoex_btreg_write(BT_COEX_HAL_ADAPTER *padapter, u8 type, u16 addr, u16 val)
{
	u16 ret = 0;

	halbtcoutsrc_LeaveLowPower(&GLBtCoexist);

	ret = halbtcoutsrc_SetBtReg(&GLBtCoexist, type, addr, val);

	halbtcoutsrc_NormalLowPower(&GLBtCoexist);

	return ret;
}

void hal_btcoex_set_rfe_type(u8 type)
{
	EXhalbtcoutsrc_set_rfe_type(type);
}
void hal_btcoex_switchband_notify(u8 under_scan, u8 band_type)
{
	switch (band_type) {
	case RF88XX_BAND_ON_2_4G:
		if (under_scan)
			EXhalbtcoutsrc_switchband_notify(&GLBtCoexist, BTC_SWITCH_TO_24G);
		else
			EXhalbtcoutsrc_switchband_notify(&GLBtCoexist, BTC_SWITCH_TO_24G_NoForScan);
		break;
	case RF88XX_BAND_ON_5G:
		EXhalbtcoutsrc_switchband_notify(&GLBtCoexist, BTC_SWITCH_TO_5G);
		break;
	default:
		printk("[BTCOEX] unkown switch band type\n");
		break;
	}
}
#endif /* CONFIG_BT_COEXIST */
