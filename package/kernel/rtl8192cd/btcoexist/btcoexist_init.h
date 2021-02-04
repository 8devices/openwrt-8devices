#ifndef __BTCOEXIST_INIT_H__
#define __BTCOEXIST_INIT_H__

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/circ_buf.h>
#include <asm/io.h>

/*kernel socket related*/
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/netlink.h>

#endif
#include <../8192cd_cfg.h>

#define H2C_BTMP_OPER_LEN			5

#define _FAIL		0
#define _SUCCESS	1
#define H2C_BT_MP_OPER 0x67


typedef struct rtl8192cd_priv BT_COEX_HAL_ADAPTER;


#define GET_BT_COEXIST_COMMON_DATA(adapter) (adapter.pshare->bt_coex_comm_data)
#if 0
#define rtw_get_hw_type(adapter) (((PRIV_INFO)adapter)->dvobj->HardwareType)
#endif


#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(adapter) __func__, adapter->pnetdev->name



typedef unsigned long _irqL;
typedef struct	semaphore _sema;

typedef	enum _BT_CoType{
	BT_2WIRE		= 0,		
	BT_ISSC_3WIRE	= 1,
	BT_ACCEL		= 2,
	BT_CSR_BC4		= 3,
	BT_CSR_BC8		= 4,
	BT_RTL8756		= 5,
	BT_RTL8723A		= 6,
	BT_RTL8821		= 7,
	BT_RTL8723B		= 8,
	BT_RTL8192E		= 9,
	BT_RTL8814A		= 10,
	BT_RTL8812A		= 11,
	BT_RTL8703B		= 12,
	BT_RTL8822B		= 13,
	BT_RTL8723D		= 14,
	BT_RTL8821C		= 15,
	BT_RTL8761     		= 16
} BT_CoType, *PBT_CoType;

enum Power_Mgnt
{
	PS_MODE_ACTIVE	= 0	,
	PS_MODE_MIN			,
};
#if 0
inline void _rtw_memmove(void *dst, const void *src, unsigned long sz)
{
#if defined(PLATFORM_LINUX)
	memmove(dst, src, sz);
#else
	#warning "no implementation\n"
#endif
}
#endif
typedef enum _RT_MEDIA_STATUS{
	RT_MEDIA_DISCONNECT = 0,
	RT_MEDIA_CONNECT       = 1
} RT_MEDIA_STATUS;


typedef struct timer_list _timer;

__inline static void _set_timer(_timer *ptimer, u32 delay_time)
{
	mod_timer(ptimer , (jiffies + (delay_time * HZ / 1000)));
}

__inline static void _cancel_timer(_timer *ptimer, u8 *bcancelled)
{
	del_timer_sync(ptimer);
	*bcancelled = 1;
}

#if 0
s32 rtw_hal_fill_h2c_cmd(PRIV_INFO padapter, u8 ElementID, u32 CmdLen, u8 *pCmdBuffer)
{
    s32 ret = _FAIL;
	u8 i;

	RT_TRACE(COMP_COEX, DBG_LOUD, ("[h2c] %02x: ", ElementID));
	for (i=0; i<CmdLen; i++)
		RT_TRACE(COMP_COEX, DBG_LOUD, ("%02x ", pCmdBuffer[i]));
	RT_TRACE(COMP_COEX, DBG_LOUD, ("\n"));
	ret = FillH2CCmd88XX(padapter, ElementID, CmdLen, pCmdBuffer);

	return ret;
#if 0    
	_adapter *pri_adapter = GET_PRIMARY_ADAPTER(padapter);

	if (pri_adapter->bFWReady == _TRUE)
		return padapter->HalFunc.fill_h2c_cmd(padapter, ElementID, CmdLen, pCmdBuffer);
	else if (padapter->registrypriv.mp_mode == 0)
		printk(FUNC_ADPT_FMT" FW doesn't exit when no MP mode, by pass H2C id:0x%02x\n"
			  , FUNC_ADPT_ARG(padapter), ElementID);
	return _FAIL;
#endif    
}
#endif


//Linux os related 
#ifdef __KERNEL__
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
	typedef struct mutex		_mutex;
#else
	typedef struct semaphore	_mutex;
#endif
#endif

#if DBG
#ifdef RT_TRACE
#undef RT_TRACE
#define RT_TRACE(dbgtype, dbgflag, printstr)\
do {\
	if (GLBtcDbgType[dbgtype] & BIT(dbgflag))\
	{\
		DbgPrint printstr;\
	}\
} while (0)
#endif
#else
#define RT_TRACE(dbgtype, dbgflag, printstr)
#endif

#if CONFIG_BT_COEXIST

#if 1

typedef struct _HCI_LINK_INFO {
	u2Byte					ConnectHandle;
	u1Byte					IncomingTrafficMode;
	u1Byte					OutgoingTrafficMode;
	u1Byte					BTProfile;
	u1Byte					BTCoreSpec;
	s1Byte					BT_RSSI;
	u1Byte					TrafficProfile;
	u1Byte					linkRole;
} HCI_LINK_INFO, *PHCI_LINK_INFO;

#define	MAX_BT_ACL_LINK_NUM				8

typedef struct _HCI_EXT_CONFIG {
	HCI_LINK_INFO				aclLink[MAX_BT_ACL_LINK_NUM];
	u1Byte					btOperationCode;
	u2Byte					CurrentConnectHandle;
	u1Byte					CurrentIncomingTrafficMode;
	u1Byte					CurrentOutgoingTrafficMode;

	u1Byte					NumberOfACL;
	u1Byte					NumberOfSCO;
	u1Byte					CurrentBTStatus;
	u2Byte					HCIExtensionVer;

	BOOLEAN					bEnableWifiScanNotify;
} HCI_EXT_CONFIG, *PHCI_EXT_CONFIG;

typedef struct _HCI_PHY_LINK_BSS_INFO {
	u2Byte						bdCap;			/* capability information */

	/* Qos related. Added by Annie, 2005-11-01. */
	/* BSS_QOS						BssQos;		 */

} HCI_PHY_LINK_BSS_INFO, *PHCI_PHY_LINK_BSS_INFO;


typedef enum _BT_CONNECT_TYPE {
	BT_CONNECT_AUTH_REQ								= 0x00,
	BT_CONNECT_AUTH_RSP								= 0x01,
	BT_CONNECT_ASOC_REQ								= 0x02,
	BT_CONNECT_ASOC_RSP								= 0x03,
	BT_DISCONNECT										= 0x04
} BT_CONNECT_TYPE, *PBT_CONNECT_TYPE;
#endif


typedef struct _BT_MGNT {
	BOOLEAN				bBTConnectInProgress;
	BOOLEAN				bLogLinkInProgress;
	BOOLEAN				bPhyLinkInProgress;
	BOOLEAN				bPhyLinkInProgressStartLL;
	u1Byte				BtCurrentPhyLinkhandle;
	u2Byte				BtCurrentLogLinkhandle;
	u1Byte				CurrentConnectEntryNum;
	u1Byte				DisconnectEntryNum;
	u1Byte				CurrentBTConnectionCnt;
	BT_CONNECT_TYPE		BTCurrentConnectType;
	BT_CONNECT_TYPE		BTReceiveConnectPkt;
	u1Byte				BTAuthCount;
	u1Byte				BTAsocCount;
	BOOLEAN				bStartSendSupervisionPkt;
	BOOLEAN				BtOperationOn;
	BOOLEAN				BTNeedAMPStatusChg;
	BOOLEAN				JoinerNeedSendAuth;
	HCI_PHY_LINK_BSS_INFO	bssDesc;
	HCI_EXT_CONFIG		ExtConfig;
	BOOLEAN				bNeedNotifyAMPNoCap;
	BOOLEAN				bCreateSpportQos;
	BOOLEAN				bSupportProfile;
	u1Byte				BTChannel;
	BOOLEAN				CheckChnlIsSuit;
	BOOLEAN				bBtScan;
	BOOLEAN				btLogoTest;
	BOOLEAN				bRfStatusNotified;
	BOOLEAN				bBtRsvedPageDownload;
} BT_MGNT, *PBT_MGNT;


struct bt_coex_info {
	/* For Kernel Socket */
	struct socket *udpsock;
	struct sockaddr_in wifi_sockaddr; /*wifi socket*/
	struct sockaddr_in bt_sockaddr;/* BT socket */
	struct sock *sk_store;/*back up socket for UDP RX int*/

	/* store which socket is OK */
	u8 sock_open;

	u8 BT_attend;
	u8 is_exist; /* socket exist */
	BT_MGNT BtMgnt;
	struct workqueue_struct *btcoex_wq;
	struct delayed_work recvmsg_work;
};
#endif



/* Some variables can't get from outsrc BT-Coex,
 * so we need to save here */
typedef struct _BT_COEXIST_STATUS {
	u8 bBtExist;
	u8 btTotalAntNum;
	u8 btChipType;
	u8 bInitlized;
	u8 btAntisolation;
} BT_COEXIST_STATUS, *PBT_COEXIST_STATUS;


struct bt_coexist_hal_ops {
	u32(*hal_init)(BT_COEX_HAL_ADAPTER *padapter);
	u32(*hal_deinit)(BT_COEX_HAL_ADAPTER *padapter);
	
	void(*set_hw_reg_handler)(BT_COEX_HAL_ADAPTER *padapter, u8 variable, u8 *val);
	void(*get_adapter_info)(BT_COEX_HAL_ADAPTER *padapter);
};

struct bt_coexist_common_data{
	u8	btcoex;
	u8	bt_iso;
	u8	bt_sco;
	u8	bt_ampdu;
	u8	ant_num;
	unsigned char	curr_bt_coex_ant;

	struct bt_coexist_hal_ops	bt_coex_hal_func;
	BT_COEXIST_STATUS	bt_coex_status;	
	
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	struct bt_coex_info	coex_info;
#endif

	/*** EEPROM SETTING ***/
	u8	EEPROMBluetoothCoexist;
	u8	EEPROMBluetoothType;
	u8	EEPROMBluetoothAntNum;

};

enum { /* for ips_mode */
	IPS_NONE = 0,
	IPS_NORMAL,
	IPS_LEVEL_2,
	IPS_NUM
};

int rtl8192cd_bt_coexist_func_init(BT_COEX_HAL_ADAPTER *padapter);


#endif
