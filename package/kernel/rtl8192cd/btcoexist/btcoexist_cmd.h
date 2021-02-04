#ifndef __BTCOEXIST_CMD_H__
#define __BTCOEXIST_CMD_H__

#include "./btcoexist_init.h"

#ifdef PLATFORM_LINUX
#define rtw_warn_on(condition) WARN_ON(condition)
#else
#define rtw_warn_on(condition) do {} while (0)
#endif

struct btinfo {
 u8 cid;
 u8 len;

 u8 bConnection:1;
 u8 bSCOeSCO:1;
 u8 bInQPage:1;
 u8 bACLBusy:1;
 u8 bSCOBusy:1;
 u8 bHID:1;
 u8 bA2DP:1;
 u8 bFTP:1;

 u8 retry_cnt:4;
 u8 rsvd_34:1;
 u8 rsvd_35:1;
 u8 rsvd_36:1;
 u8 rsvd_37:1;

 u8 rssi;

 u8 rsvd_50:1;
 u8 rsvd_51:1;
 u8 rsvd_52:1;
 u8 rsvd_53:1;
 u8 rsvd_54:1;
 u8 rsvd_55:1;
 u8 eSCO_SCO:1;
 u8 Master_Slave:1;

 u8 rsvd_6;
 u8 rsvd_7;
};

struct btinfo_8761ATV {
	u8 cid;
	u8 len;

	u8 bConnection:1;
	u8 bSCOeSCO:1;
	u8 bInQPage:1;
	u8 bACLBusy:1;
	u8 bSCOBusy:1;
	u8 bHID:1;
	u8 bA2DP:1;
	u8 bFTP:1;

	u8 retry_cnt:4;
	u8 rsvd_34:1;
	u8 bPage:1;
	u8 TRxMask:1;
	u8 Sniff_attempt:1;

	u8 rssi;

	u8 A2dp_rate:1;
	u8 ReInit:1;
	u8 MaxPower:1;
	u8 bEnIgnoreWlanAct:1;
	u8 TxPowerLow:1;
	u8 TxPowerHigh:1;
	u8 eSCO_SCO:1;
	u8 Master_Slave:1;

	u8 ACL_TRx_TP_low;
	u8 ACL_TRx_TP_high;
};


/*=============================efuse cmd*/


/*=============================proc cmd*/
#define __user

void btinfo_evt_dump(struct btinfo_8761ATV *info);

int proc_get_btcoex_dbg(struct seq_file *m, void *v);
ssize_t proc_set_btcoex_dbg(struct file *file, const char __user *buffer, size_t count, loff_t *pos, void *data);
//int proc_get_btcoex_info(struct seq_file *m, void *v);

//static void rtw_btinfo_hdl(BT_COEX_HAL_ADAPTER *adapter, u8 *buf, u16 buf_len);


/*mp cmd*/



#endif
