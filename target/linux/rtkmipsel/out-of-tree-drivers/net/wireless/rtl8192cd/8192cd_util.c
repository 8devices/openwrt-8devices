/*
 *  Utility routines
 *
 *  $Id: 8192cd_util.c,v 1.52.2.24 2011/01/10 06:55:07 chuangsw Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_UTILS_C_

#ifdef __KERNEL__

#include <linux/version.h>

#include <linux/circ_buf.h>
#include <linux/sched.h>
#include <linux/if_arp.h>
#include <net/ipv6.h>
#include <net/protocol.h>
#include <net/ndisc.h>
#include <linux/icmpv6.h>
#include <linux/vmalloc.h>


#elif defined(__ECOS)
#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#ifdef __KERNEL__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#define ipv6_addr_copy(a1,a2) memcpy(a1, a2, sizeof(struct in6_addr))
#endif
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_util.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"
#include "WlanHAL/HalDbgCmd.h"
#if defined(RTK_NL80211)
#include "8192cd_cfg80211.h"
#endif

#ifdef RTL8192CD_VARIABLE_USED_DMEM
#include "./8192cd_dmem.h"
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
#ifdef __KERNEL__
#include <linux/if_vlan.h>
#endif
#endif
#if defined(CONFIG_RTL_FASTBRIDGE)
#include <net/rtl/features/fast_bridge.h>
#endif

#ifdef __OSK__
#include "ccb.h"
#endif

#if defined(CONFIG_RTL_SIMPLE_CONFIG)
#if !defined(__ECOS)
#include <linux/netdevice.h>
#endif
#include "./8192cd_profile.h"
extern unsigned char g_sc_ifname[32];
#endif
#ifdef CONFIG_DUAL_CPU
#include "dual_cpu.h"
#endif
#if defined(USE_PID_NOTIFY) && defined(LINUX_2_6_27_)
struct pid *_wlanapp_pid;
struct pid *_wlanwapi_pid;
#ifdef CONFIG_IEEE80211R
struct pid *_wlanft_pid;
#endif
#endif

unsigned char Realtek_OUI[] = {0x00, 0xe0, 0x4c};
unsigned char dot11_rate_table[] = {2,4,11,22,12,18,24,36,48,72,96,108,0}; // last element must be zero!!
unsigned char oui_rfc1042[] = {0x00, 0x00, 0x00};
unsigned char oui_8021h[] = {0x00, 0x00, 0xf8};
unsigned char oui_cisco[] = {0x00, 0x00, 0x0c};
unsigned char SNAP_ETH_TYPE_IPX[2] = {0x81, 0x37};
unsigned char SNAP_ETH_TYPE_APPLETALK_AARP[2] = {0x80, 0xf3};
unsigned char SNAP_ETH_TYPE_APPLETALK_DDP[2] = {0x80, 0x9B};
unsigned char SNAP_HDR_APPLETALK_DDP[3] = {0x08, 0x00, 0x07}; // Datagram Delivery Protocol


#if defined(RTK_WLAN_EVENT_INDICATE)
extern struct sock *get_nl_eventd_sk(void);
extern int get_nl_eventd_pid(void);
extern void rtk_eventd_netlink_send(int pid, struct sock *nl_sk, int eventID, char *ifname, char *data, int data_len);
#endif

#ifdef __ECOS
#ifndef VLAN_HLEN
#define VLAN_HLEN 4
#endif
#endif

void mem_dump(unsigned char *ptitle, unsigned char *pbuf, int len)
{
	char tmpbuf[100];
	int i, n = 0;
	
	if (ptitle)
		sprintf(tmpbuf, "%s", ptitle);
	else
		tmpbuf[0] = '\0';
	
	for (i = 0; i < len; ++i ) {
		if (!(i & 0x0f)) {
			printk("%s\n", tmpbuf);
			n = sprintf(tmpbuf, "%03X:\t", i);
		}
		n += sprintf((tmpbuf+n), " %02X", pbuf[i]);
	}
	printk("%s\n", tmpbuf);
}

struct rtl_arphdr
{
	//for corss platform
    __be16          ar_hrd;         /* format of hardware address   */
    __be16          ar_pro;         /* format of protocol address   */
    unsigned char   ar_hln;         /* length of hardware address   */
    unsigned char   ar_pln;         /* length of protocol address   */
    __be16          ar_op;          /* ARP opcode (command)         */
};

#ifdef DBG_MEMORY_LEAK
#include <asm/atomic.h>
atomic_t _malloc_cnt = ATOMIC_INIT(0);
atomic_t _malloc_size = ATOMIC_INIT(0);
#endif /* DBG_MEMORY_LEAK */

#ifdef __KERNEL__
inline u8* _rtw_vmalloc(u32 sz)
{
	u8 *pbuf;

	pbuf = vmalloc(sz);

#ifdef DBG_MEMORY_LEAK
	if (pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;
}

inline u8* _rtw_zvmalloc(u32 sz)
{
	u8 	*pbuf;

	pbuf = _rtw_vmalloc(sz);
	if (pbuf != NULL) {
		memset(pbuf, 0, sz);
	}

	return pbuf;
}

inline void _rtw_vmfree(const void *pbuf, u32 sz)
{
	if (pbuf)
	{
		vfree(pbuf);

#ifdef DBG_MEMORY_LEAK
		atomic_dec(&_malloc_cnt);
		atomic_sub(sz, &_malloc_size);
#endif /* DBG_MEMORY_LEAK */
	}
}
#endif // __KERNEL__

#if defined(__ECOS)
u8* _rtw_malloc_flag(u32 sz, int gfp)
#else
u8* _rtw_malloc_flag(u32 sz, gfp_t gfp)
#endif
{
	u8 *pbuf = NULL;

	pbuf = kmalloc(sz, gfp);

#ifdef DBG_MEMORY_LEAK
	if (pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;
}


u8* _rtw_malloc(u32 sz)
{
	u8 *pbuf = NULL;

	pbuf = kmalloc(sz, /*GFP_KERNEL*/GFP_ATOMIC);

#ifdef DBG_MEMORY_LEAK
	if (pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;
}

u8* _rtw_zmalloc(u32 sz)
{
	u8 *pbuf = _rtw_malloc(sz);

	if (pbuf != NULL) {
		memset(pbuf, 0, sz);
	}

	return pbuf;
}

void* rtw_malloc2d(int h, int w, int size)
{
	int j;

	void **a = (void **) rtw_zmalloc( h*sizeof(void *) + h*w*size );
	if(a == NULL)
	{
		return NULL;
	}

	for( j=0; j<h; j++ )
		a[j] = ((char *)(a+h)) + j*w*size;

	return a;
}

void rtw_mfree2d(void *pbuf, int h, int w, int size)
{
	rtw_mfree((u8 *)pbuf, h*sizeof(void*) + w*h*size);
}

void _rtw_mfree(const void *pbuf, u32 sz)
{
	if (pbuf)
	{
		kfree(pbuf);
		
#ifdef DBG_MEMORY_LEAK
		atomic_dec(&_malloc_cnt);
		atomic_sub(sz, &_malloc_size);
#endif /* DBG_MEMORY_LEAK */
	}
}

static __inline__ void release_buf_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf, struct list_head	*phead, unsigned int *count)
{
	struct list_head *plist;
#ifdef SMP_SYNC
	unsigned long flags;
#endif

	if (pbuf == NULL)
	{
		DEBUG_ERR("Release Null Buf!\n");
		return;
	}

#if 0
	if (*count >= PRE_ALLOCATED_HDR) {
		_DEBUG_ERR("over size free buf phead=%lX, *count=%d\n", (unsigned long)phead, *count);
		return;
	}
#endif

	plist = (struct list_head *)((unsigned long)pbuf - sizeof(struct list_head));

	SMP_LOCK_BUF(flags);
	
	*count = *count + 1;
	list_add_tail(plist, phead);
	
	SMP_UNLOCK_BUF(flags);
}

static __inline__ unsigned char *get_buf_from_poll(struct rtl8192cd_priv *priv, struct list_head *phead, unsigned int *count)
{
	unsigned char *buf;
	struct list_head *plist;
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif
	if(priv)
	SMP_LOCK_BUF(flags);

	if (list_empty(phead)) {
		if(priv)
		SMP_UNLOCK_BUF(flags);
//		_DEBUG_ERR("phead=%lX buf is empty now!\n", (unsigned long)phead);
		return NULL;
	}

	if (*count == 0) {
		if(priv)
		SMP_UNLOCK_BUF(flags);
		_DEBUG_ERR("phead=%lX under-run!\n", (unsigned long)phead);
		return NULL;
	}

	*count = *count - 1;
	plist = phead->next;
	list_del_init(plist);

	if(priv)
	SMP_UNLOCK_BUF(flags);
	
	buf = (UINT8 *)((unsigned long)plist + sizeof (struct list_head));
	return buf;
}

__inline__ unsigned int orSTABitMap(STA_BITMAP *map)
{
    return (
        map->_staMap_
#if (NUM_STAT >32)
        || map->_staMap_ext_1
#if (NUM_STAT >64)		
        || map->_staMap_ext_2 || map->_staMap_ext_3
#endif
#endif
    );	
}

__inline__ unsigned int orForce20_Switch20Map(struct rtl8192cd_priv *priv)
{
    return (orSTABitMap(&priv->force_20_sta) || orSTABitMap(&priv->switch_20_sta));
}

/* return 1 or 0*/
unsigned char getSTABitMap(STA_BITMAP *map, int bitIdx)
{
    unsigned int ret = 0;
    bitIdx--;

    if (bitIdx < 32)
        ret = map->_staMap_ & BIT(bitIdx);
#if (NUM_STAT >32)		
    else if (bitIdx <= 64)
        ret = map->_staMap_ext_1 & BIT(bitIdx - 32);
#if (NUM_STAT >64)		
    else if (bitIdx <= 96)
        ret  = map->_staMap_ext_2 & BIT(bitIdx - 64);
    else if (bitIdx <= 128)
        ret = map->_staMap_ext_3 & BIT(bitIdx -96);
#endif	
#endif	

    return (ret?1:0);
}


void setSTABitMap(STA_BITMAP *map, int bitIdx)
{
    bitIdx--;

    if (bitIdx < 32)
        map->_staMap_ |= BIT(bitIdx);
#if (NUM_STAT >32)		
    else if (bitIdx <= 64)
        map->_staMap_ext_1 |= BIT(bitIdx - 32);
#if (NUM_STAT >64)		
    else if (bitIdx <= 96)
        map->_staMap_ext_2 |= BIT(bitIdx - 64);
    else if (bitIdx <= 128)
        map->_staMap_ext_3 |= BIT(bitIdx -96);
#endif	
#endif	
}



void clearSTABitMap(STA_BITMAP* map, int bitIdx)
{
    bitIdx--;

    if (bitIdx < 32)
        map->_staMap_ &= ~ BIT(bitIdx);
#if (NUM_STAT >32)		
    else if (bitIdx < 64)
        map->_staMap_ext_1 &= ~ BIT(bitIdx - 32);
#if (NUM_STAT >64)		
    else if (bitIdx < 96)
        map->_staMap_ext_2 &= ~ BIT(bitIdx - 64);
    else if (bitIdx < 128)
        map->_staMap_ext_3 &= ~ BIT(bitIdx - 96);
#endif	
#endif	
}

#ifdef PRIV_STA_BUF
struct priv_obj_buf {
	unsigned char magic[8];
	struct list_head	list;
	struct aid_obj obj;
};

#if defined(WIFI_WMM) && defined(WMM_APSD)
struct priv_apsd_que {
	unsigned char magic[8];
	struct list_head	list;
	struct apsd_pkt_queue que;
};
#endif

#if defined(WIFI_WMM)
struct priv_dz_mgt_que {
	unsigned char magic[8];
	struct list_head	list;
	struct dz_mgmt_queue que;
};
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
struct priv_wpa_buf {
	unsigned char magic[8];
	struct list_head	list;
	WPA_STA_INFO wpa;
};
#endif

#define MAGIC_CODE_BUF			"8192"

#define MAX_PRIV_OBJ_NUM		NUM_STAT

#ifdef CONCURRENT_MODE
static struct priv_obj_buf obj_buf[NUM_WLAN_IFACE][MAX_PRIV_OBJ_NUM];
static struct list_head objbuf_list[NUM_WLAN_IFACE];
static int free_obj_buf_num[NUM_WLAN_IFACE];


#if defined(WIFI_WMM) && defined(WMM_APSD)
	#define MAX_PRIV_QUE_NUM	(MAX_PRIV_OBJ_NUM*4)
	static struct priv_apsd_que que_buf[NUM_WLAN_IFACE][MAX_PRIV_QUE_NUM];
	static struct list_head quebuf_list[NUM_WLAN_IFACE];
	static int free_que_buf_num[NUM_WLAN_IFACE];
#endif

#if defined(WIFI_WMM)
	#define MAX_MGT_QUE_NUM	(MAX_PRIV_OBJ_NUM)
	static struct priv_dz_mgt_que mgt_que_buf[NUM_WLAN_IFACE][MAX_MGT_QUE_NUM];
	static struct list_head mgt_quebuf_list[NUM_WLAN_IFACE];
	static int free_mgt_que_buf_num[NUM_WLAN_IFACE];
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	#define MAX_PRIV_WPA_NUM	MAX_PRIV_OBJ_NUM
	static struct priv_wpa_buf wpa_buf[NUM_WLAN_IFACE][MAX_PRIV_WPA_NUM];
	static struct list_head wpabuf_list[NUM_WLAN_IFACE];
	static int free_wpa_buf_num[NUM_WLAN_IFACE];
#endif

#else
static struct priv_obj_buf obj_buf[MAX_PRIV_OBJ_NUM];
static struct list_head objbuf_list;
static int free_obj_buf_num;


#if defined(WIFI_WMM) && defined(WMM_APSD)
	#define MAX_PRIV_QUE_NUM	(MAX_PRIV_OBJ_NUM*4)
	static struct priv_apsd_que que_buf[MAX_PRIV_QUE_NUM];
	static struct list_head quebuf_list;
	static int free_que_buf_num;
#endif

#if defined(WIFI_WMM)
	#define MAX_MGT_QUE_NUM	(MAX_PRIV_OBJ_NUM)
	static struct priv_dz_mgt_que mgt_que_buf[MAX_MGT_QUE_NUM];
	static struct list_head mgt_quebuf_list;
	static int free_mgt_que_buf_num;
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	#define MAX_PRIV_WPA_NUM	MAX_PRIV_OBJ_NUM
	static struct priv_wpa_buf wpa_buf[MAX_PRIV_WPA_NUM];
	static struct list_head wpabuf_list;
	static int free_wpa_buf_num;
#endif
#endif

void init_priv_sta_buf(struct rtl8192cd_priv *priv)
{
	int i;
#ifdef CONCURRENT_MODE
	int idx = priv->pshare->wlandev_idx;
	memset(&obj_buf[idx], '\0', sizeof(struct priv_obj_buf)*MAX_PRIV_OBJ_NUM);
	INIT_LIST_HEAD(&objbuf_list[idx]);
	for (i=0; i<MAX_PRIV_OBJ_NUM; i++)  {
		memcpy(obj_buf[idx][i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&obj_buf[idx][i].list);
		list_add_tail(&obj_buf[idx][i].list, &objbuf_list[idx]);
	}
	free_obj_buf_num[idx] = i;

#if defined(WIFI_WMM) && defined(WMM_APSD)
	memset(&que_buf[idx], '\0', sizeof(struct priv_apsd_que)*MAX_PRIV_QUE_NUM);
	INIT_LIST_HEAD(&quebuf_list[idx]);
	for (i=0; i<MAX_PRIV_QUE_NUM; i++)  {
		memcpy(que_buf[idx][i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&que_buf[idx][i].list);
		list_add_tail(&que_buf[idx][i].list, &quebuf_list[idx]);
	}
	free_que_buf_num[idx] = i;
#endif

#if defined(WIFI_WMM)
	memset(&mgt_que_buf[idx], '\0', sizeof(struct priv_dz_mgt_que)*MAX_MGT_QUE_NUM);
	INIT_LIST_HEAD(&mgt_quebuf_list[idx]);
	for (i=0; i<MAX_MGT_QUE_NUM; i++)  {
		memcpy(mgt_que_buf[idx][i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&mgt_que_buf[idx][i].list);
		list_add_tail(&mgt_que_buf[idx][i].list, &mgt_quebuf_list[idx]);
	}
	free_mgt_que_buf_num[idx] = i;
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	memset(&wpa_buf[idx], '\0', sizeof(struct priv_wpa_buf)*MAX_PRIV_WPA_NUM);
	INIT_LIST_HEAD(&wpabuf_list[idx]);
	for (i=0; i<MAX_PRIV_WPA_NUM; i++)  {
		memcpy(wpa_buf[idx][i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&wpa_buf[idx][i].list);
		list_add_tail(&wpa_buf[idx][i].list, &wpabuf_list[idx]);
	}
	free_wpa_buf_num[idx] = i;
#endif
#else	
	memset(obj_buf, '\0', sizeof(struct priv_obj_buf)*MAX_PRIV_OBJ_NUM);
	INIT_LIST_HEAD(&objbuf_list);
	for (i=0; i<MAX_PRIV_OBJ_NUM; i++)  {
		memcpy(obj_buf[i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&obj_buf[i].list);
		list_add_tail(&obj_buf[i].list, &objbuf_list);
	}
	free_obj_buf_num = i;

#if defined(WIFI_WMM) && defined(WMM_APSD)
	memset(que_buf, '\0', sizeof(struct priv_apsd_que)*MAX_PRIV_QUE_NUM);
	INIT_LIST_HEAD(&quebuf_list);
	for (i=0; i<MAX_PRIV_QUE_NUM; i++)  {
		memcpy(que_buf[i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&que_buf[i].list);
		list_add_tail(&que_buf[i].list, &quebuf_list);
	}
	free_que_buf_num = i;
#endif

#if defined(WIFI_WMM) 
	memset(&mgt_que_buf, '\0', sizeof(struct priv_dz_mgt_que)*MAX_MGT_QUE_NUM);
	INIT_LIST_HEAD(&mgt_quebuf_list);
	for (i=0; i<MAX_MGT_QUE_NUM; i++)  {
		memcpy(mgt_que_buf[i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&mgt_que_buf[i].list);
		list_add_tail(&mgt_que_buf[i].list, &mgt_quebuf_list);
	}
	free_mgt_que_buf_num = i;
#endif

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
	memset(wpa_buf, '\0', sizeof(struct priv_wpa_buf)*MAX_PRIV_WPA_NUM);
	INIT_LIST_HEAD(&wpabuf_list);
	for (i=0; i<MAX_PRIV_WPA_NUM; i++)  {
		memcpy(wpa_buf[i].magic, MAGIC_CODE_BUF, 4);
		INIT_LIST_HEAD(&wpa_buf[i].list);
		list_add_tail(&wpa_buf[i].list, &wpabuf_list);
	}
	free_wpa_buf_num = i;
#endif
#endif
}

struct aid_obj *alloc_sta_obj(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long flags=0;
#endif
	struct aid_obj *priv_obj;

    if(priv)
	    SAVE_INT_AND_CLI(flags);

#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
#ifdef CONCURRENT_MODE
	priv_obj = (struct aid_obj  *)get_buf_from_poll(priv, &objbuf_list[priv->pshare->wlandev_idx], (unsigned int *)&free_obj_buf_num[priv->pshare->wlandev_idx]);
#else	
	priv_obj = (struct aid_obj  *)get_buf_from_poll(priv, &objbuf_list, (unsigned int *)&free_obj_buf_num);
#endif
#else
#ifdef CONCURRENT_MODE
	priv_obj = (struct aid_obj  *)get_buf_from_poll(NULL, &objbuf_list[priv->pshare->wlandev_idx], (unsigned int *)&free_obj_buf_num[priv->pshare->wlandev_idx]);
#else	
	priv_obj = (struct aid_obj  *)get_buf_from_poll(NULL, &objbuf_list, (unsigned int *)&free_obj_buf_num);
#endif
#endif

   if(priv)
        RESTORE_INT(flags);

#if defined(__KERNEL__) || defined(__OSK__)
	if (priv_obj == NULL)
		return ((struct aid_obj *)kmalloc(sizeof(struct aid_obj), GFP_ATOMIC));
	else
#endif		
		return priv_obj;
}

void free_sta_obj(struct rtl8192cd_priv *priv, struct aid_obj *obj)
{
	unsigned long offset = (unsigned long)(&((struct priv_obj_buf *)0)->obj);
	struct priv_obj_buf *priv_obj = (struct priv_obj_buf *)(((unsigned long)obj) - offset);
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	if (!memcmp(priv_obj->magic, MAGIC_CODE_BUF, 4) &&
			((unsigned long)&priv_obj->obj) ==  ((unsigned long)obj)) {
		SAVE_INT_AND_CLI(flags);
#ifdef CONCURRENT_MODE
		release_buf_to_poll(priv, (unsigned char *)obj, &objbuf_list[priv->pshare->wlandev_idx], (unsigned int *)&free_obj_buf_num[priv->pshare->wlandev_idx]);
#else
		release_buf_to_poll(priv, (unsigned char *)obj, &objbuf_list, (unsigned int *)&free_obj_buf_num);
#endif
		RESTORE_INT(flags);
	}
	else
#ifdef __ECOS
		ASSERT(0);
#else
		kfree(obj);
#endif
}

#if defined(WIFI_WMM) && defined(WMM_APSD)
static struct apsd_pkt_queue *alloc_sta_que(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	struct apsd_pkt_queue *priv_que;
	SAVE_INT_AND_CLI(flags);
#ifdef CONCURRENT_MODE
	priv_que = (struct apsd_pkt_queue*)get_buf_from_poll(priv, &quebuf_list[priv->pshare->wlandev_idx], (unsigned int *)&free_que_buf_num[priv->pshare->wlandev_idx]);
#else	
	priv_que = (struct apsd_pkt_queue*)get_buf_from_poll(priv, &quebuf_list, (unsigned int *)&free_que_buf_num);
#endif
	RESTORE_INT(flags);

#if defined(__KERNEL__) || defined(__OSK__)
	if (priv_que == NULL)
		return ((struct apsd_pkt_queue *)kmalloc(sizeof(struct apsd_pkt_queue), GFP_ATOMIC));
	else
#endif		
		return priv_que;
}

void free_sta_que(struct rtl8192cd_priv *priv, struct apsd_pkt_queue *que)
{
	unsigned long offset = (unsigned long)(&((struct priv_apsd_que *)0)->que);
	struct priv_apsd_que *priv_que = (struct priv_apsd_que *)(((unsigned long)que) - offset);
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	if (!memcmp(priv_que->magic, MAGIC_CODE_BUF, 4) &&
			((unsigned long)&priv_que->que) ==  ((unsigned long)que)) {
		SAVE_INT_AND_CLI(flags);
#ifdef CONCURRENT_MODE
		release_buf_to_poll(priv, (unsigned char *)que, &quebuf_list[priv->pshare->wlandev_idx], (unsigned int *)&free_que_buf_num[priv->pshare->wlandev_idx]);
#else			
		release_buf_to_poll(priv, (unsigned char *)que, &quebuf_list, (unsigned int *)&free_que_buf_num);
#endif
		RESTORE_INT(flags);		
	}
	else
#ifdef __ECOS
		ASSERT(0);
#else
		kfree(que);
#endif
}
#endif // defined(WIFI_WMM) && defined(WMM_APSD)

#if defined(WIFI_WMM)
static struct dz_mgmt_queue *alloc_sta_mgt_que(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	struct dz_mgmt_queue *priv_que;

	SAVE_INT_AND_CLI(flags);
#ifdef CONCURRENT_MODE
	priv_que = (struct dz_mgmt_queue*)get_buf_from_poll(priv, &mgt_quebuf_list[priv->pshare->wlandev_idx], (unsigned int *)&free_mgt_que_buf_num[priv->pshare->wlandev_idx]);
#else
	priv_que = (struct dz_mgmt_queue*)get_buf_from_poll(priv, &mgt_quebuf_list, (unsigned int *)&free_mgt_que_buf_num);
#endif
	RESTORE_INT(flags);

#if defined(__KERNEL__) || defined(__OSK__)
	if (priv_que == NULL)
		return ((struct dz_mgmt_queue *)kmalloc(sizeof(struct dz_mgmt_queue), GFP_ATOMIC));
	else
#endif		
		return priv_que;
}

void free_sta_mgt_que(struct rtl8192cd_priv *priv, struct dz_mgmt_queue *que)
{
	unsigned long offset = (unsigned long)(&((struct priv_dz_mgt_que *)0)->que);
	struct priv_dz_mgt_que *priv_que = (struct priv_dz_mgt_que *)(((unsigned long)que) - offset);
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	if (!memcmp(priv_que->magic, MAGIC_CODE_BUF, 4) &&
			((unsigned long)&priv_que->que) ==  ((unsigned long)que)) {
		SAVE_INT_AND_CLI(flags);
#ifdef CONCURRENT_MODE
		release_buf_to_poll(priv, (unsigned char *)que, &mgt_quebuf_list[priv->pshare->wlandev_idx], (unsigned int *)&free_mgt_que_buf_num[priv->pshare->wlandev_idx]);
#else
		release_buf_to_poll(priv, (unsigned char *)que, &mgt_quebuf_list, (unsigned int *)&free_mgt_que_buf_num);
#endif
		RESTORE_INT(flags);		
	}
	else
		kfree(que);
}
#endif // defined(WIFI_WMM) 


#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
static WPA_STA_INFO *alloc_wpa_buf(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	WPA_STA_INFO *priv_buf;
	SAVE_INT_AND_CLI(flags);
#ifdef CONCURRENT_MODE
	priv_buf = (WPA_STA_INFO *)get_buf_from_poll(priv, &wpabuf_list[priv->pshare->wlandev_idx], (unsigned int *)&free_wpa_buf_num[priv->pshare->wlandev_idx]);
#else
	priv_buf = (WPA_STA_INFO *)get_buf_from_poll(priv, &wpabuf_list, (unsigned int *)&free_wpa_buf_num);
#endif
	RESTORE_INT(flags);

#ifdef __ECOS
	ASSERT(priv_buf != NULL);
	return priv_buf;

#else	
	if (priv_buf == NULL)
		return ((WPA_STA_INFO *)kmalloc(sizeof(WPA_STA_INFO), GFP_ATOMIC));
	else
		return priv_buf;
#endif		
}

void free_wpa_buf(struct rtl8192cd_priv *priv, WPA_STA_INFO *buf)
{
	unsigned long offset = (unsigned long)(&((struct priv_wpa_buf *)0)->wpa);
	struct priv_wpa_buf *priv_buf = (struct priv_wpa_buf *)(((unsigned long)buf) - offset);
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	if (!memcmp(priv_buf->magic, MAGIC_CODE_BUF, 4) &&
			((unsigned long)&priv_buf->wpa) ==  ((unsigned long)buf)) {
		SAVE_INT_AND_CLI(flags);
#ifdef CONCURRENT_MODE
		release_buf_to_poll(priv, (unsigned char *)buf, &wpabuf_list[priv->pshare->wlandev_idx], (unsigned int *)&free_wpa_buf_num[priv->pshare->wlandev_idx]);
#else
		release_buf_to_poll(priv, (unsigned char *)buf, &wpabuf_list, (unsigned int *)&free_wpa_buf_num);
#endif
		RESTORE_INT(flags);
	}
	else
#ifdef __ECOS
		ASSERT(0);
#else	
		kfree(buf);
#endif
}
#endif // INCLUDE_WPA_PSK
#endif // PRIV_STA_BUF


#ifdef CONFIG_RTL8190_PRIV_SKB
#ifdef CONCURRENT_MODE
static struct sk_buff *dev_alloc_skb_priv(struct rtl8192cd_priv *priv, unsigned int size);
#else
static struct sk_buff *dev_alloc_skb_priv(struct rtl8192cd_priv *priv, unsigned int size);
#endif
#endif


int enque(struct rtl8192cd_priv *priv, int *head, int *tail, unsigned long ffptr, int ffsize, void *elm)
{
	// critical section!
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);
	if (CIRC_SPACE(*head, *tail, ffsize) == 0) {
		RESTORE_INT(flags);
		return FALSE;
	}

	*(unsigned long *)(ffptr + (*head)*(sizeof(void *))) = (unsigned long)elm;
	*head = (*head + 1) & (ffsize - 1);
	RESTORE_INT(flags);
	return TRUE;
}


void* deque(struct rtl8192cd_priv *priv, int *head, int *tail, unsigned long ffptr, int ffsize)
{
	// critical section!
	unsigned int  i;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	void *elm;
	
	SAVE_INT_AND_CLI(flags);
	if (CIRC_CNT(*head, *tail, ffsize) == 0) {
		RESTORE_INT(flags);
		return NULL;
	}

	i = *tail;
	*tail = (*tail + 1) & (ffsize - 1);
	elm = (void*)(*(unsigned long *)(ffptr + i*(sizeof(void *))));
	RESTORE_INT(flags);
	
	return elm;
}


void initque(struct rtl8192cd_priv *priv, int *head, int *tail)
{
	// critical section!
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);
	*head = *tail = 0;
	RESTORE_INT(flags);
}


int	isFFempty(int head, int tail)
{
	return (head == tail);
}

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
void _init_txservq(struct tx_servq *ptxservq, int q_num)
{
	_rtw_init_listhead(&ptxservq->tx_pending);
	_rtw_init_queue(&ptxservq->xframe_queue);
	ptxservq->q_num = q_num;
#ifdef CONFIG_SDIO_HCI
	ptxservq->ts_used = 0;
#endif
}

#ifdef CONFIG_TCP_ACK_TXAGG
void _init_tcpack_servq(struct tcpack_servq *tcpackq, int q_num)
{
	_rtw_init_listhead(&tcpackq->tx_pending);
	_rtw_init_queue(&tcpackq->xframe_queue);
	tcpackq->q_num = q_num;
}
#endif
#endif // CONFIG_USB_HCI || CONFIG_SDIO_HCI

#ifdef CONFIG_RTK_MESH
unsigned int find_rate_MP(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct ht_cap_elmt * peer_ht_cap,  int peer_ht_cap_len, char *peer_rate,int peer_rate_len,int mode, int isBasicRate)
{
	unsigned int len, i, hirate, lowrate, rate_limit, OFDM_only=0;
	unsigned char *rateset, *p;

	if ((get_rf_mimo_mode(priv)== MIMO_1T2R) || (get_rf_mimo_mode(priv)== MIMO_1T1R)) //eric-8814 ?? 3t3r ??
		rate_limit = 8;
	else if (get_rf_mimo_mode(priv)== MIMO_2T2R)
		rate_limit = 16;
	else if (get_rf_mimo_mode(priv)== MIMO_3T3R)
		rate_limit = 24; 
	else
		rate_limit = 16;

	if (pstat) {
		rateset = pstat->bssrateset;
		len = pstat->bssratelen;
	}
	else {
		rateset = peer_rate;
		len = peer_rate_len;
	}

	hirate = _1M_RATE_;
	lowrate = _54M_RATE_;
	if (priv->pshare->curr_band == BAND_5G)
		OFDM_only = 1;

	for(i=0,p=rateset; i<len; i++,p++)
	{
		if (*p == 0x00)
			break;

		if ((isBasicRate & 1) && !(*p & 0x80))
			continue;

		if ((isBasicRate & 2) && !is_CCK_rate(*p & 0x7f))
			continue;

		if ((*p & 0x7f) > hirate)
			if (!OFDM_only || !is_CCK_rate(*p & 0x7f))
				hirate = (*p & 0x7f);

		if ((*p & 0x7f) < lowrate)
			if (!OFDM_only || !is_CCK_rate(*p & 0x7f))
				lowrate = (*p & 0x7f);
	}

	if (pstat) {
		if ((mode == 1) && (isBasicRate == 0) && pstat->ht_cap_len) {
			for (i=0; i<rate_limit; i++)
			{
				if (pstat->ht_cap_buf.support_mcs[i/8] & BIT(i%8)) {
					hirate = i;
					hirate += HT_RATE_ID;
				}
			}
		}
	}
	else {
		if ((mode == 1) && (isBasicRate == 0) && priv->ht_cap_len && peer_ht_cap_len) {
			for (i=0; i<rate_limit; i++)
			{
				if (peer_ht_cap->support_mcs[i/8] & BIT(i%8)) {
					hirate = i;
					hirate += HT_RATE_ID;
				}
			}
		}
	}

	if (mode == 0)
		return lowrate;
	else
		return hirate;
}

#endif


// rateset: is the rateset for searching
// mode: 0: find the lowest rate, 1: find the highest rate
// isBasicRate: bit0-1: find from basic rate set, bit0-0: find from supported rate set. bit1-1: find CCK only
unsigned int find_rate(struct rtl8192cd_priv *priv, struct stat_info *pstat, int mode, int isBasicRate)
{
	unsigned int len, i, hirate, lowrate, rate_limit, OFDM_only=0;
	unsigned char *rateset, *p;
#ifdef CLIENT_MODE
	unsigned char totalrateset[32];
#endif

	if ((get_rf_mimo_mode(priv)== MIMO_1T2R) || (get_rf_mimo_mode(priv)== MIMO_1T1R)) //eric-8814 ?? 3t3r ??
		rate_limit = 8;
	else if (get_rf_mimo_mode(priv)== MIMO_2T2R)
		rate_limit = 16;
	else if (get_rf_mimo_mode(priv)== MIMO_3T3R)
		rate_limit = 24; 
	else
		rate_limit = 16;

	if (pstat) {
		rateset = pstat->bssrateset;
		len = pstat->bssratelen;
	} else {
#ifdef CLIENT_MODE
		if ((OPMODE & WIFI_STATION_STATE) && priv->pmib->dot11Bss.supportrate) {
			int i=0;
			len = 0;
			for (i=0; dot11_rate_table[i]; i++) {
				if (priv->pmib->dot11Bss.supportrate & BIT(i)) {
					totalrateset[len] = dot11_rate_table[i];
					if (priv->pmib->dot11Bss.basicrate & BIT(i))
						totalrateset[len] |= 0x80;
					len++;
				}
			}
			rateset = totalrateset;
		} else
#endif
		{
			rateset = AP_BSSRATE;
			len = AP_BSSRATE_LEN;
		}
	}

	hirate = _1M_RATE_;
	lowrate = _54M_RATE_;
	if (priv->pshare->curr_band == BAND_5G
#if defined(RTK_5G_SUPPORT) 
		|| priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G
#endif
		)
		OFDM_only = 1;

	for(i=0,p=rateset; i<len; i++,p++)
	{
		if (*p == 0x00)
			break;

		if ((isBasicRate & 1) && !(*p & 0x80))
			continue;

		if ((isBasicRate & 2) && !is_CCK_rate(*p & 0x7f))
			continue;

		if ((*p & 0x7f) > hirate)
			if (!OFDM_only || !is_CCK_rate(*p & 0x7f))
				hirate = (*p & 0x7f);

		if ((*p & 0x7f) < lowrate)
			if (!OFDM_only || !is_CCK_rate(*p & 0x7f))
				lowrate = (*p & 0x7f);
	}

	if (pstat) {
		if ((mode == 1) && (isBasicRate == 0) && pstat->ht_cap_len && (!should_restrict_Nrate(priv, pstat))) {
			for (i=0; i<rate_limit; i++)
			{
				if (pstat->ht_cap_buf.support_mcs[i/8] & BIT(i&0x7)) {
					hirate = i;
					hirate += HT_RATE_ID;
				}
			}
		}
	}
	else {
		if ((mode == 1) && (isBasicRate == 0) && priv->ht_cap_len) {
			for (i=0; i<rate_limit; i++)
			{
				if (priv->ht_cap_buf.support_mcs[i/8] & BIT(i%8)) {
					hirate = i;
					hirate += HT_RATE_ID;
				}
			}
		}
	}

	if (mode == 0)
		return lowrate;
	else
		return hirate;
}


UINT8 get_rate_from_bit_value(int bit_val)
{
	int i;

	if (bit_val == 0)
		return 0;
	
#ifdef RTK_AC_SUPPORT 	//vht rate 
	if(bit_val & BIT(31)) {
		i = bit_val - BIT(31);

		if(i < VHT_RATE_NUM)
			return (VHT_RATE_ID + i);
		else
			return _NSS1_MCS0_RATE_; //unknown rate value 
	}
#endif

	if(bit_val & BIT(28)) {
		i = bit_val - BIT(28);

		if((i+16) < HT_RATE_NUM)
			return (_MCS16_RATE_ + i);
		else
			return _MCS0_RATE_; //unknown rate value 
	}
	
	i = 0;
	while ((bit_val & BIT(i)) == 0)
		i++;

	if (i < 12)
		return dot11_rate_table[i];
	else if (i < 28)
		return ((i - 12) + HT_RATE_ID);
	
	return 0;
}


int get_rate_index_from_ieee_value(UINT8 val)
{
	int i;
	for (i=0; dot11_rate_table[i]; i++) {
		if (val == dot11_rate_table[i]) {
			return i;
		}
	}
	_DEBUG_ERR("Local error, invalid input rate for get_rate_index_from_ieee_value() [%d]!!\n", val);
	return 0;
}


int get_bit_value_from_ieee_value(UINT8 val)
{
	int i=0;
	while(dot11_rate_table[i] != 0) {
		if (dot11_rate_table[i] == val)
			return BIT(i);
		i++;
	}
	return 0;
}

BOOLEAN CheckCts2SelfEnable(UINT8 rtsTxRate)
{
	return (rtsTxRate <= _11M_RATE_) ? 1 :0;
}

UINT8 find_rts_rate(struct rtl8192cd_priv *priv, UINT8 TxRate, BOOLEAN bErpProtect)
{
	UINT8 rtsTxRate = _6M_RATE_;

	if(bErpProtect) // use CCK rate as RTS
	{
		rtsTxRate = _1M_RATE_;
	}
	else
	{
		switch (TxRate) 
		{
			case _NSS3_MCS9_RATE_:
			case _NSS3_MCS8_RATE_:
			case _NSS3_MCS7_RATE_:
			case _NSS3_MCS6_RATE_:
			case _NSS3_MCS5_RATE_:
			case _NSS3_MCS4_RATE_:
			case _NSS3_MCS3_RATE_:
			case _NSS2_MCS9_RATE_:
			case _NSS2_MCS8_RATE_:
			case _NSS2_MCS7_RATE_:
			case _NSS2_MCS6_RATE_:
			case _NSS2_MCS5_RATE_:
			case _NSS2_MCS4_RATE_:
			case _NSS2_MCS3_RATE_:
			case _NSS1_MCS9_RATE_:
			case _NSS1_MCS8_RATE_:
			case _NSS1_MCS7_RATE_:
			case _NSS1_MCS6_RATE_:
			case _NSS1_MCS5_RATE_:
			case _NSS1_MCS4_RATE_:
			case _NSS1_MCS3_RATE_:
			case _MCS23_RATE_:
			case _MCS22_RATE_:
			case _MCS21_RATE_:
			case _MCS20_RATE_:
			case _MCS19_RATE_:
			case _MCS15_RATE_:
			case _MCS14_RATE_:
			case _MCS13_RATE_:
			case _MCS12_RATE_:
			case _MCS11_RATE_:
			case _MCS7_RATE_:
			case _MCS6_RATE_:
			case _MCS5_RATE_:
			case _MCS4_RATE_:
			case _MCS3_RATE_:
			case _54M_RATE_:
			case _48M_RATE_:
			case _36M_RATE_:
			case _24M_RATE_:		
				rtsTxRate = _24M_RATE_;
				break;
			case _NSS3_MCS2_RATE_:
			case _NSS3_MCS1_RATE_:				
			case _NSS2_MCS2_RATE_:
			case _NSS2_MCS1_RATE_:
			case _NSS1_MCS2_RATE_:
			case _NSS1_MCS1_RATE_:
			case _MCS18_RATE_:
			case _MCS17_RATE_:				
			case _MCS10_RATE_:
			case _MCS9_RATE_:
			case _MCS2_RATE_:
			case _MCS1_RATE_:
			case _18M_RATE_:
			case _12M_RATE_:
				rtsTxRate = _12M_RATE_;
				break;
			case _NSS3_MCS0_RATE_:
			case _NSS2_MCS0_RATE_:
			case _NSS1_MCS0_RATE_:
			case _MCS16_RATE_:
			case _MCS8_RATE_:
			case _MCS0_RATE_:
			case _9M_RATE_:
			case _6M_RATE_:
				rtsTxRate = _6M_RATE_;
				break;
			case _11M_RATE_:
			case _5M_RATE_:
			case _2M_RATE_:
			case _1M_RATE_:
				rtsTxRate = _1M_RATE_;
				break;
			default:
				rtsTxRate = _6M_RATE_;
				break;
		}
	}

	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G) {
	           if(rtsTxRate < _6M_RATE_)
	                     rtsTxRate = _6M_RATE_;
	}

	return rtsTxRate;
}

void init_stainfo(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	struct wifi_mib	*pmib = priv->pmib;
	unsigned long	offset;
	int i, j;
#if !defined(SMP_SYNC) && defined(CONFIG_RTL_WAPI_SUPPORT)
	unsigned long		flags;
#endif

#ifdef WDS
	static unsigned char bssrateset[32];
	unsigned int	bssratelen=0;
	unsigned int	current_tx_rate=0;
#endif
	unsigned short	bk_aid;
	unsigned char		bk_hwaddr[MACADDRLEN];


	// init linked list header
	// BUT do NOT init hash_list
	INIT_LIST_HEAD(&pstat->asoc_list);
	INIT_LIST_HEAD(&pstat->auth_list);
	INIT_LIST_HEAD(&pstat->sleep_list);
	INIT_LIST_HEAD(&pstat->defrag_list);
	INIT_LIST_HEAD(&pstat->wakeup_list);
	INIT_LIST_HEAD(&pstat->frag_list);

#ifdef CONFIG_PCI_HCI
	// to avoid add RAtid fail
	INIT_LIST_HEAD(&pstat->addRAtid_list);
	INIT_LIST_HEAD(&pstat->addrssi_list);
#endif

#ifdef CONFIG_RTK_MESH
	INIT_LIST_HEAD(&pstat->mesh_mp_ptr);
#endif	// CONFIG_RTK_MESH

#ifdef A4_STA
	INIT_LIST_HEAD(&pstat->a4_sta_list);
#endif
#if defined(CONFIG_PCI_HCI)
	skb_queue_head_init(&pstat->dz_queue);
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	INIT_LIST_HEAD(&pstat->pspoll_list);

	for (i = 0; i < MAX_STA_TX_SERV_QUEUE; ++i) {
		_init_txservq(&pstat->tx_queue[i], i);
#ifdef CONFIG_TCP_ACK_TXAGG
		_init_tcpack_servq(&pstat->tcpack_queue[i], i);
#endif
	}
	
	pstat->pending_cmd = 0;
	pstat->asoc_list_refcnt = 0;
#ifdef __ECOS
	cyg_flag_init(&pstat->asoc_unref_done);
#else
	init_completion(&pstat->asoc_unref_done);
#endif
#endif

  
#ifdef RTK_ATM
    if(priv->pshare->rf_ft_var.atm_en){
        SMP_LOCK_XMIT(flags);
    	for(i=BK_QUEUE;i<HIGH_QUEUE;i++) {	
            pstat->atm_swq.q_aggnum[i] = 2;			
            skb_queue_head_init(&pstat->atm_swq.swq_queue[i]);
        }
        SMP_UNLOCK_XMIT(flags);   
    }    
#endif

#ifdef SW_TX_QUEUE
    SMP_LOCK_XMIT(flags);
    for(i=BK_QUEUE;i<HIGH_QUEUE;i++) {	
    	pstat->swq.q_aggnum[i] = 2;			
        skb_queue_head_init(&pstat->swq.swq_queue[i]);
    }
    SMP_UNLOCK_XMIT(flags);   
#endif

	pstat->cnt_sleep = 0;

#ifdef DOT11K
    if(priv->pmib->dot11StationConfigEntry.dot11RadioMeasurementActivated) {
        pstat->rm_timer.data = (unsigned long) pstat;        
        pstat->rm_timer.function = rm_beacon_expire;          
        init_timer(&pstat->rm_timer);       
    }
#endif


	// we do NOT reset MAC here

#if defined(WIFI_WMM)
#ifdef DZ_ADDBA_RSP
	pstat->dz_addba.used = 0;
#endif
#endif

#ifdef WDS
	if (pstat->state & WIFI_WDS) {
		bssratelen = pstat->bssratelen;
		memcpy(bssrateset, pstat->bssrateset, bssratelen);
		current_tx_rate = pstat->current_tx_rate;
	}
#endif

	// zero out all the rest
	bk_aid = pstat->aid;
	memcpy(bk_hwaddr, pstat->hwaddr, MACADDRLEN);

	offset = (unsigned long)(&((struct stat_info *)0)->auth_seq);
	memset((void *)((unsigned long)pstat + offset), 0, sizeof(struct stat_info)-offset);
	
	pstat->aid = bk_aid;
	memcpy(pstat->hwaddr, bk_hwaddr, MACADDRLEN);

#ifdef WDS
	if (bssratelen) {
		pstat->bssratelen = bssratelen;
		memcpy(pstat->bssrateset, bssrateset, bssratelen);
		pstat->current_tx_rate = current_tx_rate;
		pstat->state |= WIFI_WDS;
	}
#endif

	// some variables need initial value
	pstat->ieee8021x_ctrlport = pmib->dot118021xAuthEntry.dot118021xDefaultPort;
	pstat->expire_to = priv->expire_to;
	for (i=0; i<8; i++)
		for (j=0; j<TUPLE_WINDOW; j++)
			pstat->tpcache[i][j] = 0xffff;
			// Stanldy mesh: pstat->tpcache[i][j] = j+1 is best solution, because its a hash table, fill slot[i] with i+1 can prevent collision,fix the packet loss of first unicast
	pstat->tpcache_mgt = 0xffff;
#ifdef CLIENT_MODE
	pstat->tpcache_mcast = 0xffff;
#endif
#ifdef GBWC
	for (i=0; i<priv->pmib->gbwcEntry.GBWCNum; i++) {
		if (!memcmp(pstat->hwaddr, priv->pmib->gbwcEntry.GBWCAddr[i], MACADDRLEN)) {
			pstat->GBWC_in_group = TRUE;
			break;
		}
	}
#endif

// button 2009.05.21
#if defined(INCLUDE_WPA_PSK) && !defined(RTK_NL80211)
	pstat->wpa_sta_info->clientHndshkProcessing = pstat->wpa_sta_info->clientHndshkDone = FALSE;
#endif

#ifdef CONFIG_RTK_MESH
	pstat->mesh_neighbor_TBL.BSexpire_LLSAperiod = jiffies + MESH_EXPIRE_TO;
#endif	//CONFIG_RTK_MESH

	memset(pstat->rc_entry, 0, sizeof(pstat->rc_entry));

#ifdef SUPPORT_TX_AMSDU
	for (i=0; i<8; i++)
		skb_queue_head_init(&pstat->amsdu_tx_que[i]);
#endif

#if defined (HW_ANT_SWITCH) && (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
	pstat->CurAntenna = priv->pshare->rf_ft_var.CurAntenna;
#endif


#ifdef CONFIG_IEEE80211W
	init_timer(&pstat->SA_timer);
	pstat->SA_timer.data = (unsigned long) pstat;
	pstat->SA_timer.function = rtl8192cd_sa_query_timer;
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
//	if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
	{
		SAVE_INT_AND_CLI(flags);
//		wapiAssert(pstat->wapiInfo!=NULL);
		if (pstat->wapiInfo==NULL)
		{
			pstat->wapiInfo = (wapiStaInfo*)kmalloc(sizeof(wapiStaInfo), GFP_ATOMIC);
			if (pstat->wapiInfo==NULL)
			{
				printk("Err: kmalloc wapiStaInfo fail!\n");
			}
		}
		if (pstat->wapiInfo!=NULL)
		{
			pstat->wapiInfo->priv = priv;
			wapiStationInit(pstat);
			pstat->wapiInfo->wapiType = priv->pmib->wapiInfo.wapiType;
		}
		RESTORE_INT(flags);
	}
#endif

#ifdef USE_OUT_SRC		
#ifdef _OUTSRC_COEXIST
	if(IS_OUTSRC_CHIP(priv))
#endif
	ODM_CmnInfoPtrArrayHook(ODMPTR , ODM_CMNINFO_STA_STATUS, pstat->aid, pstat);
#endif

#ifdef MULTI_MAC_CLONE
	if ((OPMODE & WIFI_STATION_STATE) && priv->pmib->ethBrExtInfo.macclone_enable) 
	{
		pstat->mclone_id = ACTIVE_ID;
		memcpy(pstat->sa_addr, GET_MY_HWADDR, MACADDRLEN);		
	}
#endif

#ifdef RTK_ATM	
	pstat->atm_sta_time = 0;
	pstat->atm_match_sta_time = 0;
#endif

#ifdef SBWC
	pstat->SBWC_mode = SBWC_MODE_DISABLE;
	pstat->SBWC_tx_limit = 0;	
	pstat->SBWC_rx_limit = 0;
	pstat->SBWC_tx_limit_byte = 0;	
	pstat->SBWC_rx_limit_byte = 0;
	pstat->SBWC_tx_count = 0;
	pstat->SBWC_rx_count = 0;
	pstat->SBWC_txq.head = 0;
	pstat->SBWC_txq.tail = 0;
	pstat->SBWC_rxq.head = 0;
	pstat->SBWC_rxq.tail = 0;
	pstat->SBWC_consuming_q = FALSE;

	for (i = 0 ; i != priv->pmib->sbwcEntry.count ; ++i)
	{
		if (!memcmp(priv->pmib->sbwcEntry.entry[i].mac, pstat->hwaddr, MACADDRLEN))
		{
			pstat->SBWC_tx_limit = priv->pmib->sbwcEntry.entry[i].tx_lmt;
			pstat->SBWC_rx_limit = priv->pmib->sbwcEntry.entry[i].rx_lmt;
			pstat->SBWC_tx_limit_byte = ((pstat->SBWC_tx_limit * 1024 / 8) / (HZ / SBWC_TO));
			pstat->SBWC_rx_limit_byte = ((pstat->SBWC_rx_limit * 1024 / 8) / (HZ / SBWC_TO));
			break;
		}
	}
#endif

}

#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
void dump_sta_dz_queue_num(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#if defined(WIFI_WMM) && defined(WMM_APSD)
	int				hd, tl;
#endif

	// free all skb in dz_queue
	
	printk("---------------------------------------\n");
#if defined(CONFIG_PCI_HCI)
	printk("pstat->dz_queue:%d\n",skb_queue_len(&pstat->dz_queue));
#endif

#ifdef SW_TX_QUEUE
	printk("swq.be_queue:%d\n",skb_queue_len(&pstat->swq.swq_queue[BE_QUEUE]));
	printk("swq.bk_queue:%d\n",skb_queue_len(&pstat->swq.swq_queue[BK_QUEUE]));
	printk("swq.vi_queue:%d\n",skb_queue_len(&pstat->swq.swq_queue[VI_QUEUE]));
	printk("swq.vo_queue:%d\n",skb_queue_len(&pstat->swq.swq_queue[VO_QUEUE]));
#endif

#if defined(WIFI_WMM) && defined(WMM_APSD)
	hd = pstat->VO_dz_queue->head;
	tl = pstat->VO_dz_queue->tail;
	printk("VO_dz_queue:%d\n",CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE));
	hd = pstat->VI_dz_queue->head;
	tl = pstat->VI_dz_queue->tail;
	printk("VI_dz_queue:%d\n",CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE));
	hd = pstat->BE_dz_queue->head;
	tl = pstat->BE_dz_queue->tail;
	printk("BE_dz_queue:%d\n",CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE));
	hd = pstat->BK_dz_queue->head;
	tl = pstat->BK_dz_queue->tail;
	printk("BK_dz_queue:%d\n",CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE));
#endif

#if defined(WIFI_WMM)
	hd = pstat->MGT_dz_queue->head;
	tl = pstat->MGT_dz_queue->tail;
	printk("BK_dz_queue:%d\n",CIRC_CNT(hd, tl, NUM_DZ_MGT_QUEUE));
#endif
	
	return;

}
#endif

#ifdef CONFIG_PCI_HCI
void free_sta_tx_skb(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#ifdef WIFI_WMM
    int				hd, tl;
#endif
    struct sk_buff	*pskb;
#if defined(SW_TX_QUEUE) || defined(RTK_ATM)
    int i;
#endif
    SMP_LOCK_XMIT(x);    

    // free all skb in dz_queue
    while (skb_queue_len(&pstat->dz_queue)) {
        pskb = (struct sk_buff	*)skb_dequeue(&pstat->dz_queue);
        rtl_kfree_skb(priv, pskb, _SKB_TX_);
    }

#ifdef RTK_ATM
    if(priv->pshare->rf_ft_var.atm_en) {
        for(i = BK_QUEUE; i < HIGH_QUEUE; i++) {
            while (skb_queue_len(&pstat->atm_swq.swq_queue[i])) {
                pskb = skb_dequeue(&pstat->atm_swq.swq_queue[i]);
                rtl_kfree_skb(priv, pskb, _SKB_TX_);
            }
        }
    }
#endif

#ifdef SW_TX_QUEUE
    for(i = BK_QUEUE; i < HIGH_QUEUE; i++) {
        while (skb_queue_len(&pstat->swq.swq_queue[i])) {
            pskb = skb_dequeue(&pstat->swq.swq_queue[i]);
            priv->ext_stats.swq_drop_pkt++;
            rtl_kfree_skb(priv, pskb, _SKB_TX_);
        }
    }
#endif

#if defined(WIFI_WMM) && defined(WMM_APSD)
    hd = pstat->VO_dz_queue->head;
    tl = pstat->VO_dz_queue->tail;
    while (CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE)) {
        pskb = pstat->VO_dz_queue->pSkb[tl];
        rtl_kfree_skb(priv, pskb, _SKB_TX_);
        tl++;
        tl = tl & (NUM_APSD_TXPKT_QUEUE - 1);
    }
    pstat->VO_dz_queue->head = 0;
    pstat->VO_dz_queue->tail = 0;

    hd = pstat->VI_dz_queue->head;
    tl = pstat->VI_dz_queue->tail;
    while (CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE)) {
        pskb = pstat->VI_dz_queue->pSkb[tl];
        rtl_kfree_skb(priv, pskb, _SKB_TX_);
        tl++;
        tl = tl & (NUM_APSD_TXPKT_QUEUE - 1);
    }
    pstat->VI_dz_queue->head = 0;
    pstat->VI_dz_queue->tail = 0;

    hd = pstat->BE_dz_queue->head;
    tl = pstat->BE_dz_queue->tail;
    while (CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE)) {
        pskb = pstat->BE_dz_queue->pSkb[tl];
        rtl_kfree_skb(priv, pskb, _SKB_TX_);
        tl++;
        tl = tl & (NUM_APSD_TXPKT_QUEUE - 1);
    }
    pstat->BE_dz_queue->head = 0;
    pstat->BE_dz_queue->tail = 0;

    hd = pstat->BK_dz_queue->head;
    tl = pstat->BK_dz_queue->tail;
    while (CIRC_CNT(hd, tl, NUM_APSD_TXPKT_QUEUE)) {
        pskb = pstat->BK_dz_queue->pSkb[tl];
        rtl_kfree_skb(priv, pskb, _SKB_TX_);
        tl++;
        tl = tl & (NUM_APSD_TXPKT_QUEUE - 1);
    }
    pstat->BK_dz_queue->head = 0;
    pstat->BK_dz_queue->tail = 0;
#endif
#if defined(WIFI_WMM)
    hd = pstat->MGT_dz_queue->head;
    tl = pstat->MGT_dz_queue->tail;
    while (CIRC_CNT(hd, tl, NUM_DZ_MGT_QUEUE)) {
        struct tx_insn *ptx_insn = pstat->MGT_dz_queue->ptx_insn[tl];
        release_mgtbuf_to_poll(priv, ptx_insn->pframe);
        release_wlanhdr_to_poll(priv, ptx_insn->phdr);
        kfree(ptx_insn);
        tl++;
        tl = tl & (NUM_DZ_MGT_QUEUE - 1);
    }
    pstat->MGT_dz_queue->head = 0;
    pstat->MGT_dz_queue->tail = 0;


#ifdef DZ_ADDBA_RSP
    pstat->dz_addba.used = 0;
#endif
#endif

#ifdef SBWC
	while (CIRC_CNT(pstat->SBWC_txq.head, pstat->SBWC_txq.tail, NUM_TXPKT_QUEUE))
	{
		struct sk_buff *pskb = pstat->SBWC_txq.pSkb[pstat->SBWC_txq.tail];
		rtl_kfree_skb(priv, pskb, _SKB_TX_);
		pstat->SBWC_txq.tail++;
		pstat->SBWC_txq.tail = pstat->SBWC_txq.tail & (NUM_TXPKT_QUEUE - 1);
	}
	while (CIRC_CNT(pstat->SBWC_rxq.head, pstat->SBWC_rxq.tail, NUM_TXPKT_QUEUE))
	{
		struct sk_buff *pskb = pstat->SBWC_rxq.pSkb[pstat->SBWC_rxq.tail];
		rtl_kfree_skb(priv, pskb, _SKB_RX_);
		pstat->SBWC_rxq.tail++;
		pstat->SBWC_rxq.tail = pstat->SBWC_rxq.tail & (NUM_TXPKT_QUEUE - 1);
	}
#endif

    SMP_UNLOCK_XMIT(x);

}
#endif // CONFIG_PCI_HCI

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
void free_sta_tx_skb(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int i;
	for (i = 0; i < MAX_STA_TX_SERV_QUEUE; ++i) {
#ifdef CONFIG_TCP_ACK_TXAGG
		rtw_tcpack_servq_flush(priv, &pstat->tcpack_queue[i]);
#endif
		rtw_txservq_flush(priv, &pstat->tx_queue[i]);
	}
}
#endif

#ifdef RX_BUFFER_GATHER_REORDER
extern void flush_rx_list_inq(struct rtl8192cd_priv *priv,struct rx_frinfo *pfrinfo);
#endif

void free_sta_skb(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int	 i, j;
	struct list_head frag_list;
	struct sk_buff	*pskb;
	unsigned long flags;
	
#if defined(SUPPORT_RX_AMSDU_AMPDU) && defined(RX_BUFFER_GATHER_REORDER)
	struct rx_frinfo *pfrinfo;
#endif	

	free_sta_tx_skb(priv,pstat);
	
	// free all skb in frag_list
	INIT_LIST_HEAD(&frag_list);

	DEFRAG_LOCK(flags);
	list_splice_init(&pstat->frag_list, &frag_list);
	DEFRAG_UNLOCK(flags);
	
	unchainned_all_frag(priv, &frag_list);

	// free all skb in rc queue
	SMP_LOCK_REORDER_CTRL(flags);
	for (i=0; i<8; i++) {
		pstat->rc_entry[i].start_rcv = FALSE;
		for (j=0; j<RC_ENTRY_NUM; j++) {
			if (pstat->rc_entry[i].packet_q[j]) {
				pskb = pstat->rc_entry[i].packet_q[j];
				#if defined(SUPPORT_RX_AMSDU_AMPDU) && defined(RX_BUFFER_GATHER_REORDER)
				pfrinfo=pstat->rc_entry[i].frinfo_q[j];
				if(pfrinfo && pfrinfo->gather_flag == GATHER_FIRST) {
					flush_rx_list_inq(priv,pfrinfo);
					pstat->rc_entry[i].frinfo_q[j]=NULL;
				} 
				#endif
				rtl_kfree_skb(priv, pskb, _SKB_RX_);
				pstat->rc_entry[i].packet_q[j] = NULL;
			}
		}
	}
	SMP_UNLOCK_REORDER_CTRL(flags);
}

#ifdef THERMAL_CONTROL
#ifdef TX_SHORTCUT
void clear_short_cut_cache(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	memset(pstat->tx_sc_ent, 0, sizeof(pstat->tx_sc_ent));
}
#endif
#endif

void release_stainfo(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int				i;
	unsigned long		flags;

	if (priv->pshare->is_40m_bw && (pstat->IOTPeer == HT_IOT_PEER_MARVELL))

	{
	    clearSTABitMap(&priv->pshare->marvellMapBit, pstat->aid);

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_RTL_8723B_SUPPORT)
		if((GET_CHIP_VER(priv)==VERSION_8812E)||(GET_CHIP_VER(priv)==VERSION_8881A) || (GET_CHIP_VER(priv)==VERSION_8723B)){
		}
        else
#endif
		if ((orSTABitMap(&priv->pshare->marvellMapBit) == 0) &&
			 (priv->pshare->Reg_RRSR_2 != 0) && (priv->pshare->Reg_81b != 0))
		{
#if defined(CONFIG_PCI_HCI)
			RTL_W8(RRSR+2, priv->pshare->Reg_RRSR_2);
			RTL_W8(0x81b, priv->pshare->Reg_81b);
			priv->pshare->Reg_RRSR_2 = 0;
			priv->pshare->Reg_81b = 0;
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			notify_40M_RRSR_SC_change(priv);
#endif
		}
	}

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	update_intel_sta_bitmap(priv, pstat, 1);
#endif
#if defined(WIFI_11N_2040_COEXIST_EXT)
	update_40m_staMap(priv, pstat, 1);
#endif

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	// avoid xmit_tasklet deadlock when receiving auth req under heavy TX traffic
#ifdef WDS
	pstat->state &= WIFI_WDS;
#else
	pstat->state = 0;
#endif
#endif

	// flush the stainfo cache
	//if (!memcmp(pstat->hwaddr, priv->stainfo_cache.hwaddr, MACADDRLEN))
	//	memset(&(priv->stainfo_cache), 0, sizeof(priv->stainfo_cache));
	if (pstat == priv->pstat_cache)
		priv->pstat_cache = NULL;

	// delete all list
	// BUT do NOT delete hash list
	asoc_list_del(priv, pstat);
	auth_list_del(priv, pstat);
	sleep_list_del(priv, pstat);
	wakeup_list_del(priv, pstat);

	DEFRAG_LOCK(flags);
	if (!list_empty(&(pstat->defrag_list)))
		list_del_init(&(pstat->defrag_list));
	DEFRAG_UNLOCK(flags);

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	rtw_pspoll_sta_delete(priv, pstat);
#endif

#ifdef CONFIG_PCI_HCI
	// to avoid add RAtid fail
	if (!list_empty(&(pstat->addRAtid_list)))
		list_del_init(&(pstat->addRAtid_list));

	if (!list_empty(&(pstat->addrssi_list)))
		list_del_init(&(pstat->addrssi_list));
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
	#if defined(CONFIG_USB_HCI)
	rtw_flush_h2c_cmd_queue(priv, pstat);
	#endif
#elif defined(CONFIG_RTL_88E_SUPPORT)
	#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	rtw_flush_cmd_queue(priv, pstat);
	#endif
#endif


#ifdef CONFIG_IEEE80211W
	if (timer_pending(&pstat->SA_timer))
	    del_timer(&pstat->SA_timer);
#endif

	// free all queued skb
	free_sta_skb(priv, pstat);

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	for (i = 0; i < MAX_STA_TX_SERV_QUEUE; ++i) {
		_rtw_spinlock_free(&(pstat->tx_queue[i].xframe_queue.lock));
#ifdef CONFIG_TCP_ACK_TXAGG
		_rtw_spinlock_free(&(pstat->tcpack_queue[i].xframe_queue.lock));
#endif
	}
#endif

#ifdef CONFIG_RTK_MESH
	SMP_LOCK_MESH_MP_HDR(flags);
	if (!list_empty(&(pstat->mesh_mp_ptr)))
		list_del_init(&(pstat->mesh_mp_ptr));
	SMP_UNLOCK_MESH_MP_HDR(flags);

    if(netif_running(GET_ROOT(priv)->mesh_dev)) {
        if(isMeshPoint(pstat)) {
            clear_route_info(GET_ROOT(priv), pstat->hwaddr);
        }
        else {
            remove_proxy_entry(GET_ROOT(priv), NULL, pstat->hwaddr);        
        }
    }
#endif

#ifdef A4_STA
    if(pstat->state & WIFI_A4_STA) {
        a4_sta_cleanup(priv, pstat);
    }

    if (!list_empty(&pstat->a4_sta_list))
        list_del_init(&pstat->a4_sta_list);	
#endif

#ifdef TV_MODE
    if(priv->tv_mode_status & BIT1) { /*TV mode is auto, check if there is STA that support tv auto*/
        tv_mode_auto_support_check(priv);
    }
#endif

	// remove key in CAM
	if (pstat->dot11KeyMapping.keyInCam == TRUE) {
		if (GET_ROOT(priv)->drv_state & DRV_STATE_OPEN) {
#ifdef USE_WEP_4_KEYS
			if((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) ||
		 		(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)) {
			int keyid=0;
			for(;keyid<4; keyid++) {
				if (CamDeleteOneEntry(priv, pstat->hwaddr, keyid, 0)) 
					priv->pshare->CamEntryOccupied--;				
			}
			pstat->dot11KeyMapping.keyInCam = FALSE;
			} else
#endif
			{
			if (CamDeleteOneEntry(priv, pstat->hwaddr, 0, 0)) {
				pstat->dot11KeyMapping.keyInCam = FALSE;
				priv->pshare->CamEntryOccupied--;
			}
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
			/*	for wapi, one state take two cam entry	*/
			if (CamDeleteOneEntry(priv, pstat->hwaddr, 0, 0)) {
				pstat->dot11KeyMapping.keyInCam = FALSE;
				priv->pshare->CamEntryOccupied--;
			}
#endif
			}
		}
	}
	SMP_LOCK_REORDER_CTRL(flags);
	for (i=0; i<RC_TIMER_NUM; i++) {
		if (priv->pshare->rc_timer[i].pstat == pstat)
			priv->pshare->rc_timer[i].pstat = NULL;
		if (priv->pshare->rc_timer_cli[i].pstat == pstat)
			priv->pshare->rc_timer_cli[i].pstat = NULL;
	}
	SMP_UNLOCK_REORDER_CTRL(flags);
#ifdef SW_TX_QUEUE
    SMP_LOCK_XMIT(flags);  
    for (i = BK_QUEUE; i <= VO_QUEUE; i++) {
        if(pstat->swq.swq_timer_id[i]) {
            rtl8192cd_swq_deltimer(priv, pstat, i);
        }
    }
    SMP_UNLOCK_XMIT(flags);    
#endif

#ifdef DOT11K
    if(priv->pmib->dot11StationConfigEntry.dot11RadioMeasurementActivated) {         
        /*terminate the measurement request to other STA*/
        if (timer_pending(&pstat->rm_timer))
            del_timer_sync(&pstat->rm_timer);

        /*terminate the measurement process requested by other STA*/
        if(pstat == priv->rm.req_pstat) {
            rm_terminate(priv);            
            priv->rm.req_pstat = NULL;
        }
    }
#endif

#ifdef WDS
	pstat->state &= WIFI_WDS;
#else
	pstat->state = 0;
#endif

#ifdef INDICATE_LINK_CHANGE
	indicate_sta_link_change(priv, pstat, DECREASE, __FUNCTION__);
#endif

#ifdef TX_SHORTCUT
	memset(pstat->tx_sc_ent, 0, sizeof(pstat->tx_sc_ent));
#endif

#ifdef RX_SHORTCUT
	for (i=0; i<RX_SC_ENTRY_NUM; i++)
		pstat->rx_sc_ent[i].rx_payload_offset = 0;
#endif

#if defined(INCLUDE_WPA_PSK) && !defined(RTK_NL80211)
	if (timer_pending(&pstat->wpa_sta_info->resendTimer)) {
		del_timer(&pstat->wpa_sta_info->resendTimer);
	}
#endif

	release_remapAid(priv, pstat);

#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
	if(IS_OUTSRC_CHIP(priv))
#endif
	ODM_CmnInfoPtrArrayHook(ODMPTR , ODM_CMNINFO_STA_STATUS, pstat->aid, 0);
#endif
#ifdef SUPPORT_TX_AMSDU
	for (i=0; i<8; i++)
		free_skb_queue(priv, &pstat->amsdu_tx_que[i]);
#endif

#if (BEAMFORMING_SUPPORT == 1)
	if (priv->pmib->dot11RFEntry.txbf == 1 && (priv->pshare->WlanSupportAbility & WLAN_BEAMFORMING_SUPPORT) )
	{
        PRT_BEAMFORMING_INFO 	pBeamformingInfo = &(priv->pshare->BeamformingInfo);

		ODM_RT_TRACE(ODMPTR, PHYDM_COMP_TXBF, ODM_DBG_LOUD, ("%s,\n", __FUNCTION__));

        pBeamformingInfo->CurDelBFerBFeeEntrySel = BFerBFeeEntry;

		if(Beamforming_DeInitEntry(priv, pstat->hwaddr))
			Beamforming_Notify(priv);
	}
#endif
#if 1
#if defined(BR_SHORTCUT) && defined(RTL_CACHED_BR_STA)
	release_brsc_cache(pstat->hwaddr);
#endif
#if defined(CONFIG_RTL_FASTBRIDGE)
		rtl_fb_del_entry(pstat->hwaddr);
#endif
#else
#ifdef BR_SHORTCUT
	clear_shortcut_cache();
#endif
#endif
#if defined(CONFIG_RTL_88E_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL)
	if(pstat->txpause_flag && priv->pshare->paused_sta_num)
		priv->pshare->paused_sta_num--;
#endif		
#ifdef CONFIG_RTL_WAPI_SUPPORT
	free_sta_wapiInfo(priv, pstat);
#endif
}


struct	stat_info *alloc_stainfo(struct rtl8192cd_priv *priv, unsigned char *hwaddr, int id)
{
#if 1//!defined(SMP_SYNC) || (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	unsigned long	flags;
#endif
    unsigned int	i;
	struct list_head	*phead, *plist;
	struct stat_info	*pstat;

	SAVE_INT_AND_CLI(flags);

	if (id < 0) { // not from FAST_RECOVERY
	// any free sta info?
		for(i=0; i<NUM_STAT; i++) {
			if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == FALSE))
			{
				priv->pshare->aidarray[i]->priv = priv;
				priv->pshare->aidarray[i]->used = TRUE;
				pstat = &(priv->pshare->aidarray[i]->station);
				memcpy(pstat->hwaddr, hwaddr, MACADDRLEN);
				init_stainfo(priv, pstat);
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
				if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef RATEADAPTIVE_BY_ODM
					ODM_RAInfo_Init(ODMPTR, pstat->aid);
#else
					priv->pshare->RaInfo[pstat->aid].pstat = pstat;			
					RateAdaptiveInfoInit(&priv->pshare->RaInfo[pstat->aid]);
#endif			
				}
#endif

				// insert to hash list
				hash_list_add(priv, pstat);

				RESTORE_INT(flags);
				return pstat;
			}
		}

		// allocate new sta info
		for(i=0; i<NUM_STAT; i++) {
			if (priv->pshare->aidarray[i] == NULL)
				break;
		}
	}
	else
		i = id;

	if (i < NUM_STAT) {
#ifdef RTL8192CD_VARIABLE_USED_DMEM
			priv->pshare->aidarray[i] = (struct aid_obj *)rtl8192cd_dmem_alloc(AID_OBJ, &i);
#else
#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i] = alloc_sta_obj(priv);
#else
			priv->pshare->aidarray[i] = (struct aid_obj *)kmalloc(sizeof(struct aid_obj), GFP_ATOMIC);
#endif
#endif
			if (priv->pshare->aidarray[i] == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i], 0, sizeof(struct aid_obj));

#ifdef CONFIG_PCI_HCI
#if defined(WIFI_WMM) && defined(WMM_APSD)
#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.VO_dz_queue = alloc_sta_que(priv);
#else
			priv->pshare->aidarray[i]->station.VO_dz_queue = (struct apsd_pkt_queue *)kmalloc(sizeof(struct apsd_pkt_queue), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.VO_dz_queue == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.VO_dz_queue, 0, sizeof(struct apsd_pkt_queue));

#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.VI_dz_queue = alloc_sta_que(priv);
#else
			priv->pshare->aidarray[i]->station.VI_dz_queue = (struct apsd_pkt_queue *)kmalloc(sizeof(struct apsd_pkt_queue), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.VI_dz_queue == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.VI_dz_queue, 0, sizeof(struct apsd_pkt_queue));

#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.BE_dz_queue = alloc_sta_que(priv);
#else
			priv->pshare->aidarray[i]->station.BE_dz_queue = (struct apsd_pkt_queue *)kmalloc(sizeof(struct apsd_pkt_queue), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.BE_dz_queue == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.BE_dz_queue, 0, sizeof(struct apsd_pkt_queue));

#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.BK_dz_queue = alloc_sta_que(priv);
#else
			priv->pshare->aidarray[i]->station.BK_dz_queue = (struct apsd_pkt_queue *)kmalloc(sizeof(struct apsd_pkt_queue), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.BK_dz_queue == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.BK_dz_queue, 0, sizeof(struct apsd_pkt_queue));
#endif

#if defined(WIFI_WMM)
#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.MGT_dz_queue = alloc_sta_mgt_que(priv);
#else
			priv->pshare->aidarray[i]->station.MGT_dz_queue = (struct dz_mgmt_queue *)kmalloc(sizeof(struct dz_mgmt_queue), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.MGT_dz_queue == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.MGT_dz_queue, 0, sizeof(struct dz_mgmt_queue));
#endif
#endif // CONFIG_PCI_HCI

#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
#ifdef PRIV_STA_BUF
			priv->pshare->aidarray[i]->station.wpa_sta_info = alloc_wpa_buf(priv);
#else
			priv->pshare->aidarray[i]->station.wpa_sta_info = (WPA_STA_INFO *)kmalloc(sizeof(WPA_STA_INFO), GFP_ATOMIC);
#endif
			if (priv->pshare->aidarray[i]->station.wpa_sta_info == NULL)
				goto no_free_memory;
			memset(priv->pshare->aidarray[i]->station.wpa_sta_info, 0, sizeof(WPA_STA_INFO));
#endif

#if defined(WIFI_HAPD) || defined(RTK_NL80211)
			memset(priv->pshare->aidarray[i]->station.wpa_ie, 0, 256);
#ifndef HAPD_DRV_PSK_WPS
			memset(priv->pshare->aidarray[i]->station.wps_ie, 0, 256);
#endif
#endif

			priv->pshare->aidarray[i]->priv = priv;
			INIT_LIST_HEAD(&(priv->pshare->aidarray[i]->station.hash_list));
			priv->pshare->aidarray[i]->station.aid = i + 1; //aid 0 is reserved for AP
			priv->pshare->aidarray[i]->used = TRUE;
			pstat = &(priv->pshare->aidarray[i]->station);
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
			if (GET_CHIP_VER(priv)==VERSION_8188E) {

#ifdef RATEADAPTIVE_BY_ODM
				ODM_RAInfo_Init(ODMPTR, pstat->aid);
#else
				priv->pshare->RaInfo[i + 1].pstat = pstat;
				RateAdaptiveInfoInit(&priv->pshare->RaInfo[i + 1]);
#endif
			}
#endif

			memcpy(pstat->hwaddr, hwaddr, MACADDRLEN);
			init_stainfo(priv, pstat);

			// insert to hash list
			hash_list_add(priv, pstat);

			RESTORE_INT(flags);
			return pstat;
	}

	// no more free sta info, check idle sta
	for(i=0; i<NUM_STAT; i++) {
		pstat = &(priv->pshare->aidarray[i]->station);
		if ((pstat->expire_to == 0)
#ifdef WDS
#ifdef LAZY_WDS
			&& ((pstat->state & WIFI_WDS_LAZY) ||
				(!(pstat->state & WIFI_WDS_LAZY) && !(pstat->state & WIFI_WDS)))
#else
			&& !(pstat->state & WIFI_WDS)
#endif
#endif
		)
		{
			release_stainfo(priv->pshare->aidarray[i]->priv, pstat);
			hash_list_del(priv->pshare->aidarray[i]->priv, pstat);

			priv->pshare->aidarray[i]->used = TRUE;
			priv->pshare->aidarray[i]->priv = priv;
			memcpy(pstat->hwaddr, hwaddr, MACADDRLEN);
			init_stainfo(priv, pstat);
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
			if (GET_CHIP_VER(priv)==VERSION_8188E)
#ifdef RATEADAPTIVE_BY_ODM
				ODM_RAInfo_Init(ODMPTR, pstat->aid);
#else
				RateAdaptiveInfoInit(&priv->pshare->RaInfo[pstat->aid]);
#endif
#endif
			// insert to hash list
			hash_list_add(priv, pstat);

			RESTORE_INT(flags);
			return pstat;
		}
	}

	RESTORE_INT(flags);
	DEBUG_ERR("AID buf is not enough\n");
	return	(struct stat_info *)NULL;

no_free_memory:

	if (priv->pshare->aidarray[i]) {
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
		if (priv->pshare->aidarray[i]->station.wpa_sta_info)
#ifdef PRIV_STA_BUF
			free_wpa_buf(priv, priv->pshare->aidarray[i]->station.wpa_sta_info);
#else
			kfree(priv->pshare->aidarray[i]->station.wpa_sta_info);
#endif
#endif

#ifdef CONFIG_PCI_HCI
#if defined(WIFI_WMM) && defined(WMM_APSD)
#ifdef PRIV_STA_BUF
		if (priv->pshare->aidarray[i]->station.VO_dz_queue)
			free_sta_que(priv, priv->pshare->aidarray[i]->station.VO_dz_queue);
		if (priv->pshare->aidarray[i]->station.VI_dz_queue)
			free_sta_que(priv, priv->pshare->aidarray[i]->station.VI_dz_queue);
		if (priv->pshare->aidarray[i]->station.BE_dz_queue)
			free_sta_que(priv, priv->pshare->aidarray[i]->station.BE_dz_queue);
		if (priv->pshare->aidarray[i]->station.BK_dz_queue)
			free_sta_que(priv, priv->pshare->aidarray[i]->station.BK_dz_queue);
#else
		if (priv->pshare->aidarray[i]->station.VO_dz_queue)
			kfree(priv->pshare->aidarray[i]->station.VO_dz_queue);
		if (priv->pshare->aidarray[i]->station.VI_dz_queue)
			kfree(priv->pshare->aidarray[i]->station.VI_dz_queue);
		if (priv->pshare->aidarray[i]->station.BE_dz_queue)
			kfree(priv->pshare->aidarray[i]->station.BE_dz_queue);
		if (priv->pshare->aidarray[i]->station.BK_dz_queue)
			kfree(priv->pshare->aidarray[i]->station.BK_dz_queue);
#endif
#endif

#if defined(WIFI_WMM)
#ifdef PRIV_STA_BUF
		if (priv->pshare->aidarray[i]->station.MGT_dz_queue)
			free_sta_mgt_que(priv, priv->pshare->aidarray[i]->station.MGT_dz_queue);
#else
		if (priv->pshare->aidarray[i]->station.MGT_dz_queue)
			kfree(priv->pshare->aidarray[i]->station.MGT_dz_queue);

#endif
#endif
#endif // CONFIG_PCI_HCI

#ifdef RTL8192CD_VARIABLE_USED_DMEM
		rtl8192cd_dmem_free(AID_OBJ, &i);
#else
#ifdef PRIV_STA_BUF
		free_sta_obj(priv, priv->pshare->aidarray[i]);
#else
		kfree(priv->pshare->aidarray[i]);
#endif
#endif
		priv->pshare->aidarray[i] = NULL;
	}

	RESTORE_INT(flags);
	DEBUG_ERR("No free memory to allocate station info\n");
	return NULL;
}


int del_station(struct rtl8192cd_priv *priv, struct stat_info *pstat, int send_disasoc)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	DOT11_DISASSOCIATION_IND Disassociation_Ind;

	if (!netif_running(priv->dev))
		return 0;

	if (pstat == NULL)
		return 0;

	if (!list_empty(&pstat->asoc_list))
	{
		if (IEEE8021X_FUN)
		{
#ifndef WITHOUT_ENQUEUE
			memcpy((void *)Disassociation_Ind.MACAddr, (void *)pstat->hwaddr, MACADDRLEN);
			Disassociation_Ind.EventId = DOT11_EVENT_DISASSOCIATION_IND;
			Disassociation_Ind.IsMoreEvent = 0;
			Disassociation_Ind.Reason = _STATS_OTHER_;
			Disassociation_Ind.tx_packets = pstat->tx_pkts;
			Disassociation_Ind.rx_packets = pstat->rx_pkts;
			Disassociation_Ind.tx_bytes   = pstat->tx_bytes;
			Disassociation_Ind.rx_bytes   = pstat->rx_bytes;
			DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&Disassociation_Ind,
						sizeof(DOT11_DISASSOCIATION_IND));
#endif
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
			psk_indicate_evt(priv, DOT11_EVENT_DISASSOCIATION_IND, pstat->hwaddr, NULL, 0);
#endif

#ifdef RTK_NL80211
			event_indicate_cfg80211(priv, pstat->hwaddr, CFG80211_DEL_STA, NULL);
#endif
#ifdef WIFI_HAPD
			event_indicate_hapd(priv, pstat->hwaddr, HAPD_EXIRED, NULL);
#ifdef HAPD_DRV_PSK_WPS
			event_indicate(priv, pstat->hwaddr, 2);
#endif
#else
			event_indicate(priv, pstat->hwaddr, 2);
#endif
		}

		if (send_disasoc)
			issue_disassoc(priv, pstat->hwaddr, _RSON_UNSPECIFIED_);

		if (pstat->expire_to > 0)
		{
			SAVE_INT_AND_CLI(flags);
			cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
			check_sta_characteristic(priv, pstat, DECREASE);
			RESTORE_INT(flags);

			LOG_MSG("A STA is deleted by application program - %02X:%02X:%02X:%02X:%02X:%02X\n",
				pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2], pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);
		}
	}

	free_stainfo(priv, pstat);

#ifdef CLIENT_MODE
	if (OPMODE & WIFI_STATION_STATE) {
		OPMODE_VAL(OPMODE & ~(WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE));
		start_clnt_lookup(priv, DONTRESCAN);
	}
#endif

	return 1;
}


int	free_stainfo(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#ifndef SMP_SYNC
	unsigned long	flags;
#endif
	unsigned int	i;

	if (pstat == (struct stat_info *)NULL)
	{
		DEBUG_ERR("illegal free an NULL stat obj\n");
		return FAIL;
	}

	for(i=0; i<NUM_STAT; i++)
	{
		if (priv->pshare->aidarray[i] &&
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			(priv->pshare->aidarray[i]->priv == priv) &&
#endif
			(priv->pshare->aidarray[i]->used == TRUE) &&
			(&(priv->pshare->aidarray[i]->station) == pstat))
		{
			DEBUG_INFO("free station info of %02X%02X%02X%02X%02X%02X\n",
				pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
				pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);

			SAVE_INT_AND_CLI(flags);
#ifdef WDS
#ifdef LAZY_WDS
			if (!(pstat->state & WIFI_WDS) || (pstat->state & WIFI_WDS_LAZY))
#else
			if (!(pstat->state & WIFI_WDS))
#endif
#endif
			{
				priv->pshare->aidarray[i]->used = FALSE;
				// remove from hash_list
				hash_list_del(priv, pstat);
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT) && !defined(RATEADAPTIVE_BY_ODM)
				if (GET_CHIP_VER(priv)==VERSION_8188E)
					priv->pshare->RaInfo[pstat->aid].pstat = NULL;
#endif
			}

			release_stainfo(priv, pstat);
			RESTORE_INT(flags);
			return SUCCESS;
		}
	}

#if defined(CONFIG_RTL_WAPI_SUPPORT)
	wapiAssert(pstat->wapiInfo==NULL);
#endif
	DEBUG_ERR("pstat can not be freed \n");
	return	FAIL;
}


/* any station allocated can be searched by hash list */
#ifdef CONFIG_WLAN_HAL_8197F)
#ifdef __OSK__
__IRAM_WIFI_PRI1
#else
__MIPS16
__IRAM_IN_865X
#endif
struct stat_info *get_stainfo_hash(struct rtl8192cd_priv *priv, unsigned char *hwaddr)
{
	struct list_head	*phead, *plist;
	struct stat_info	*pstat;
	unsigned int	index;
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif

	SMP_LOCK_HASH_LIST(flags);

	index = wifi_mac_hash(hwaddr);
	phead = &priv->stat_hash[index];
	
	plist = phead->next;
	//check whether plist and phead is null
	while (plist && phead && plist != phead)
	{
		pstat = list_entry(plist, struct stat_info ,hash_list);
		plist = plist->next;
		
		if (isEqualMACAddr(pstat->hwaddr, hwaddr)) { // if found the matched address
#ifdef MULTI_MAC_CLONE
			if (!(priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) ||
				((priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) &&
										!priv->pmib->ethBrExtInfo.macclone_enable) ||
					((priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) &&
						priv->pmib->ethBrExtInfo.macclone_enable & (pstat->mclone_id == ACTIVE_ID))) 
#endif		
			{
				priv->pstat_cache = pstat;
				goto exit;
			}
		}
#ifdef CONFIG_PCI_HCI
		if (plist == plist->next)
			break;
#endif
	}
	
	pstat = NULL;
	
exit:
	SMP_UNLOCK_HASH_LIST(flags);
	
	return pstat;
}

#else
#ifdef __OSK__
__IRAM_WIFI_PRI1
#else
__MIPS16
__IRAM_IN_865X
#endif
struct stat_info *get_stainfo(struct rtl8192cd_priv *priv, unsigned char *hwaddr)
{
	struct list_head	*phead, *plist;
	struct stat_info	*pstat;
	unsigned int	index;
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif

#ifdef RTK_NL80211
	if(hwaddr == NULL)
		return (struct stat_info *)NULL;
#endif

	//if (!memcmp(hwaddr, priv->stainfo_cache.hwaddr, MACADDRLEN) &&  priv->stainfo_cache.pstat)
	pstat = priv->pstat_cache;

#ifdef MULTI_MAC_CLONE
	if ((priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) && MCLONE_NUM > 0) {
	    if (pstat && !memcmp(hwaddr, pstat->hwaddr, MACADDRLEN) && pstat->mclone_id == ACTIVE_ID)
		return pstat;
	}
	else
#endif
	{
    if(pstat && isEqualMACAddr(hwaddr, pstat->hwaddr))
		return pstat;
	}

	SMP_LOCK_HASH_LIST(flags);

	index = wifi_mac_hash(hwaddr);
	phead = &priv->stat_hash[index];
	
	plist = phead->next;
	while (plist != phead)
	{
		pstat = list_entry(plist, struct stat_info ,hash_list);
		plist = plist->next;
		
		if (!(memcmp((void *)pstat->hwaddr, (void *)hwaddr, MACADDRLEN))) { // if found the matched address
#ifdef MULTI_MAC_CLONE
			if (!(priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) ||
				((priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) &&
										!priv->pmib->ethBrExtInfo.macclone_enable) ||
					((priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) &&
						priv->pmib->ethBrExtInfo.macclone_enable & (pstat->mclone_id == ACTIVE_ID))) 
#endif		
			{
				priv->pstat_cache = pstat;
				goto exit;
			}
		}
#ifdef CONFIG_PCI_HCI
		if (plist == plist->next)
			break;
#endif
	}
	
	pstat = NULL;
	
exit:
	SMP_UNLOCK_HASH_LIST(flags);
	
	return pstat;
}
#endif


#ifdef __OSK__
__IRAM_WIFI_PRI1
#else
__MIPS16
__IRAM_IN_865X
#endif
struct stat_info *get_stainfo_fast(struct rtl8192cd_priv *priv, unsigned char *hwaddr, unsigned char macID)
{
    //struct aid_obj  *obj;

#ifdef CONFIG_8814_AP_MAC_VERI
    return NULL;
#endif //#ifdef CONFIG_8814_AP_MAC_VERI

#ifdef HW_FILL_MACID
    if (IS_SUPPORT_HW_FILL_MACID(priv)) {    
        if(macID > 0)
        {
            if((macID >= 0x7E))
            {
                if(macID == 0x7E)
                    printk("Serious issue, HW detect macid fail = 0x7F \n");
                return get_stainfo(priv, hwaddr);
            }
            else
            {
                //if(get_stainfo(priv, hwaddr)!= &(priv->pshare->aidarray[macID-1]->station))
                //{
                //    printk("SW HW getSTA error SW = %x HW = %x \n",get_stainfo(priv, hwaddr),&(priv->pshare->aidarray[macID-1]->station));
                //}
                return &(priv->pshare->aidarray[macID-1]->station);
            }
            /*
            obj = priv->pshare->aidarray[macID-1];

            if(obj->priv == priv)
                return &(priv->pshare->aidarray[macID-1]->station);
            else
            {
                printk("obj error at macID %x \n",macID);
                return (struct stat_info *)NULL;
            }*/
            
        }else {
            return get_stainfo(priv, hwaddr);
        }        
    }else        
#endif
    {
        return get_stainfo(priv, hwaddr);
    }   
}


#ifdef HW_FILL_MACID
__MIPS16
__IRAM_IN_865X
struct stat_info *get_HW_mapping_sta(struct rtl8192cd_priv *priv, unsigned char macID)
{
    struct aid_obj  *obj;
    
    if(macID > 0)
    {
        if((macID >= 0x7E))
            return (struct stat_info *)NULL;
        else
            return &(priv->pshare->aidarray[macID-1]->station);

        /*
        obj = priv->pshare->aidarray[macID-1];

        if(obj->priv == priv)
            return &(priv->pshare->aidarray[macID-1]->station);
        else
        {
            printk("obj error at macID %x \n",macID);
            return (struct stat_info *)NULL;
        }*/
        
    }
}
#endif // #ifdef HW_FILL_MACID

/* aid is only meaningful for assocated stations... */
struct stat_info *get_aidinfo(struct rtl8192cd_priv *priv, unsigned int aid)
{
	struct list_head	*plist, *phead;
	struct stat_info	*pstat = NULL;
#if defined(SMP_SYNC) //&& (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	unsigned long flags = 0;
#endif

	if (aid == 0)
		return (struct stat_info *)NULL;
	
	SMP_LOCK_ASOC_LIST(flags);

	phead = &priv->asoc_list;
	plist = phead->next;

	while (plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;
		if (pstat->aid == aid)
			goto exit;
	}
	pstat = (struct stat_info *)NULL;
exit:
	SMP_UNLOCK_ASOC_LIST(flags);
	
	return pstat;
}

#if 1
struct stat_info *get_macidinfo(struct rtl8192cd_priv *priv, unsigned int aid)
{
	struct list_head	*plist, *phead;
	struct stat_info	*pstat = NULL;
#if defined(SMP_SYNC)// && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	unsigned long flags = 0;
#endif

	if (aid == 0)
		return (struct stat_info *)NULL;
	
	SMP_LOCK_ASOC_LIST(flags);

	phead = &priv->asoc_list;
	plist = phead->next;

	while (plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;
		if (REMAP_AID(pstat) == aid)
			goto exit;
	}
	pstat = (struct stat_info *)NULL;
exit:
	SMP_UNLOCK_ASOC_LIST(flags);
	
	return pstat;
}


struct stat_info *get_macIDinfo(struct rtl8192cd_priv *rpriv, unsigned int macid)
{
	struct rtl8192cd_priv *priv = GET_ROOT(rpriv);
	struct stat_info	*pstat = NULL;
#ifdef MBSSID
	unsigned int i;
#endif

	pstat = get_macidinfo(priv, macid);
	if(pstat)
		return pstat;
	
#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))	{
		pstat = get_macidinfo(GET_VXD_PRIV(priv), macid);
		if(pstat)
			return pstat;
	}
#endif

#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) 		{
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i])) {
				pstat = get_macidinfo(priv->pvap_priv[i], macid);
				if(pstat)
					return pstat;	
			}
		}
	}
#endif
	return pstat;
}

#endif

#if !(defined(CONFIG_WLAN_HAL_8197F)) || defined(CONFIG_WLAN_STATS_EXTENTION)
int IS_BSSID(struct rtl8192cd_priv *priv, unsigned char *da)
{
	unsigned char *bssid;
	bssid = priv->pmib->dot11StationConfigEntry.dot11Bssid;

	//if (!memcmp(da, bssid, 6))
	if (isEqualMACAddr(da, bssid))
		return TRUE;
	else
		return FALSE;
}


int IS_MCAST(unsigned char *da)
{
	if ((*da) & 0x01)
		return TRUE;
	else
		return FALSE;
}

int IS_BCAST2(unsigned char *da)
{
     if ((*da) == 0xff)
         return TRUE;
     else
         return FALSE;
}
#endif

int p80211_stt_findproto(UINT16 proto)
{
	/* Always return found for now.	This is the behavior used by the */
	/*  Zoom Win95 driver when 802.1h mode is selected */
	/* TODO: If necessary, add an actual search we'll probably
		 need this to match the CMAC's way of doing things.
		 Need to do some testing to confirm.
	*/

	if (proto == 0x80f3 ||   /* APPLETALK */
		proto == 0x8137 ) /* DIX II IPX */
		return 1;

	return 0;
}


void eth_2_llc(struct wlan_ethhdr_t *pethhdr, struct llc_snap *pllc_snap)
{
	pllc_snap->llc_hdr.dsap=pllc_snap->llc_hdr.ssap=0xAA;
	pllc_snap->llc_hdr.ctl=0x03;

	if (p80211_stt_findproto(ntohs(pethhdr->type))) {
		memcpy((void *)pllc_snap->snap_hdr.oui, oui_8021h, WLAN_IEEE_OUI_LEN);
	}
	else {
		memcpy((void *)pllc_snap->snap_hdr.oui, oui_rfc1042, WLAN_IEEE_OUI_LEN);
	}
	pllc_snap->snap_hdr.type = pethhdr->type;
}


void eth2_2_wlanhdr(struct rtl8192cd_priv *priv, struct wlan_ethhdr_t *pethhdr, struct tx_insn *txcfg)
{
	unsigned char *pframe = txcfg->phdr;
	unsigned int to_fr_ds = get_tofr_ds(pframe);

	switch (to_fr_ds)
	{
		case 0x00:
			memcpy(GetAddr1Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
			memcpy(GetAddr2Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
			memcpy(GetAddr3Ptr(pframe), BSSID, WLAN_ADDR_LEN);
			break;
		case 0x01:
			{
#ifdef MCAST2UI_REFINE
                                if (txcfg->fr_type == _SKB_FRAME_TYPE_)
					memcpy(GetAddr1Ptr(pframe), (const void *) &((struct sk_buff *)txcfg->pframe)->cb[10], WLAN_ADDR_LEN);
                                else
#endif
				memcpy(GetAddr1Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
				memcpy(GetAddr2Ptr(pframe), BSSID, WLAN_ADDR_LEN);
				memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
			}
			break;
		case 0x02:
			{
				memcpy(GetAddr1Ptr(pframe), BSSID, WLAN_ADDR_LEN);
				memcpy(GetAddr2Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
				memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
			}
			break;
		case 0x03:

#ifdef CONFIG_RTK_MESH
            if(txcfg->is_11s) {
                memcpy(GetAddr1Ptr(pframe), txcfg->nhop_11s, WLAN_ADDR_LEN);
                memcpy(GetAddr2Ptr(pframe), GET_MY_HWADDR, WLAN_ADDR_LEN);
                memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
                memcpy(GetAddr4Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
            } else
#endif // CONFIG_RTK_MESH
#ifdef A4_STA
            if (priv->pshare->rf_ft_var.a4_enable && txcfg->pstat && (txcfg->pstat->state & WIFI_A4_STA)) {
                memcpy(GetAddr1Ptr(pframe), txcfg->pstat->hwaddr, WLAN_ADDR_LEN);
                memcpy(GetAddr2Ptr(pframe), GET_MY_HWADDR, WLAN_ADDR_LEN);
		if(txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
			memcpy(GetAddr3Ptr(pframe), (const void *)GET_MY_HWADDR, WLAN_ADDR_LEN);
			memcpy(GetAddr4Ptr(pframe), (const void *)GET_MY_HWADDR, WLAN_ADDR_LEN);
		} else {
                	memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
	                memcpy(GetAddr4Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
		}
            }
            else
#endif
            {            
#ifdef WDS
                #ifdef MP_TEST
    			if (OPMODE & WIFI_MP_STATE)
    				memcpy(GetAddr1Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
    			else
                #endif
    				memcpy(GetAddr1Ptr(pframe), priv->pmib->dot11WdsInfo.entry[txcfg->wdsIdx].macAddr, WLAN_ADDR_LEN);

                #ifdef MP_TEST
    			if (OPMODE & WIFI_MP_STATE)
    				memcpy(GetAddr2Ptr(pframe), priv->dev->dev_addr, WLAN_ADDR_LEN);
    			else
                #endif
                #ifdef __DRAYTEK_OS__
    				memcpy(GetAddr2Ptr(pframe), priv->dev->dev_addr, WLAN_ADDR_LEN);
                #else
    				memcpy(GetAddr2Ptr(pframe), priv->wds_dev[txcfg->wdsIdx]->dev_addr , WLAN_ADDR_LEN);
                #endif
    			memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
    			memcpy(GetAddr4Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
#else // not WDS   			
    			DEBUG_ERR("no support for WDS!\n");
    			memcpy(GetAddr1Ptr(pframe), (const void *)pethhdr->daddr, WLAN_ADDR_LEN);
    			memcpy(GetAddr2Ptr(pframe), (const void *)BSSID, WLAN_ADDR_LEN);
    			memcpy(GetAddr3Ptr(pframe), (const void *)pethhdr->saddr, WLAN_ADDR_LEN);
#endif	// WDS
            }
			break;
	}
}


int skb_p80211_to_ether(struct net_device *dev, int wep_mode, struct rx_frinfo *pfrinfo)
{
    UINT	to_fr_ds;
    INT		payload_length;
    INT		payload_offset, trim_pad;
    UINT8	daddr[WLAN_ETHADDR_LEN];
    UINT8	saddr[WLAN_ETHADDR_LEN];
    UINT8	*pframe;
#ifdef CONFIG_RTK_MESH
    INT 	mesh_header_len=0;
#endif
    struct wlan_hdr *w_hdr;
    struct wlan_ethhdr_t   *e_hdr;
    struct wlan_llc_t      *e_llc;
    struct wlan_snap_t     *e_snap;
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

    int wlan_pkt_format;
    struct sk_buff *skb = get_pskb(pfrinfo);

#ifdef RX_SHORTCUT
    extern int get_rx_sc_free_entry(struct stat_info *pstat, struct rx_frinfo *pfrinfo);
    int privacy, idx=0;
    struct rx_sc_entry *prxsc_entry = NULL;
    struct wlan_hdr wlanhdr;
    struct stat_info 	*pstat;
#endif // RX_SHORTCUT

    pframe = get_pframe(pfrinfo);
    to_fr_ds = get_tofr_ds(pframe);
    payload_offset = get_hdrlen(priv, pframe);
    trim_pad = 0; // _CRCLNG_ has beed subtracted in isr
    w_hdr = (struct wlan_hdr *)pframe;

#ifdef CONFIG_RTK_MESH
    if( pfrinfo->is_11s &8 )
    {
        struct wlan_ethhdr_t eth;
        struct  MESH_HDR* mhdr = (struct MESH_HDR*) (pframe+sizeof(struct wlan_ethhdr_t));
        const short mlen =  (mhdr->mesh_flag &1) ? 16 : 4;
        memcpy( &eth, pframe, sizeof(struct wlan_ethhdr_t));
        if( mlen &16 )
            memcpy(&eth, mhdr->DestMACAddr, WLAN_ETHADDR_LEN<<1 );
        memcpy(skb_pull(skb, mlen), &eth, sizeof(struct wlan_ethhdr_t));
        return SUCCESS;
    }
#endif

#ifdef RX_SHORTCUT
    pstat = get_stainfo(priv, GetAddr2Ptr(skb->data));
#endif // RX_SHORTCUT


	if ( to_fr_ds == 0x00) {
		memcpy(daddr, (const void *)w_hdr->addr1, WLAN_ETHADDR_LEN);
		memcpy(saddr, (const void *)w_hdr->addr2, WLAN_ETHADDR_LEN);
	}
	else if( to_fr_ds == 0x01) {
		memcpy(daddr, (const void *)w_hdr->addr1, WLAN_ETHADDR_LEN);
		memcpy(saddr, (const void *)w_hdr->addr3, WLAN_ETHADDR_LEN);
	}
	else if( to_fr_ds == 0x02) {
		memcpy(daddr, (const void *)w_hdr->addr3, WLAN_ETHADDR_LEN);
		memcpy(saddr, (const void *)w_hdr->addr2, WLAN_ETHADDR_LEN);
	}
	else {
#ifdef CONFIG_RTK_MESH
		// WIFI_11S_MESH = WIFI_QOS_DATA
		if(pfrinfo->is_11s &1) {
			memcpy(daddr, (const void *)pfrinfo->mesh_header.DestMACAddr, WLAN_ETHADDR_LEN);
			memcpy(saddr, (const void *)pfrinfo->mesh_header.SrcMACAddr, WLAN_ETHADDR_LEN);
			mesh_header_len = 16;				
		}
		else
#endif // CONFIG_RTK_MESH
		{
			memcpy(daddr, (const void *)w_hdr->addr3, WLAN_ETHADDR_LEN);
			memcpy(saddr, (const void *)w_hdr->addr4, WLAN_ETHADDR_LEN);
		}
	}

	if (GetPrivacy(pframe)) {
#ifdef CONFIG_RTL_WAPI_SUPPORT
		if ((wep_mode == _WAPI_SMS4_)) {
			payload_offset += (WAPI_EXT_LEN-WAPI_ALIGNMENT_OFFSET);
			trim_pad += (SMS4_MIC_LEN);
		} else
#endif
		if (((wep_mode == _WEP_40_PRIVACY_) || (wep_mode == _WEP_104_PRIVACY_))) {
			payload_offset += 4;
			trim_pad += 4;
		}
		else if ((wep_mode == _TKIP_PRIVACY_)) {
			payload_offset += 8;
			trim_pad += (8 + 4);
		}
		else if ((wep_mode == _CCMP_PRIVACY_)) {
			payload_offset += 8;
			trim_pad += 8;
		}
		else {
			DEBUG_ERR("drop pkt due to unallowed wep_mode privacy=%d\n", wep_mode);
			return FAIL;
		}
	}

#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
	skb->len -= WAPI_ALIGNMENT_OFFSET;
#endif

	payload_length = skb->len - payload_offset - trim_pad;

#ifdef CONFIG_RTK_MESH
    payload_length -=	mesh_header_len;
#endif

	if (payload_length <= 0) {
		DEBUG_ERR("drop pkt due to payload_length<=0\n");
		return FAIL;
	}

	e_hdr = (struct wlan_ethhdr_t *) (pframe + payload_offset);
	e_llc = (struct wlan_llc_t *) (pframe + payload_offset);
	e_snap = (struct wlan_snap_t *) (pframe + payload_offset + sizeof(struct wlan_llc_t));

	if ((e_llc->dsap==0xaa) && (e_llc->ssap==0xaa) && (e_llc->ctl==0x03))
	{
		if (!memcmp(e_snap->oui, oui_rfc1042, WLAN_IEEE_OUI_LEN)) {
			wlan_pkt_format = WLAN_PKT_FORMAT_SNAP_RFC1042;
			if(!memcmp(&e_snap->type, SNAP_ETH_TYPE_IPX, 2))
				wlan_pkt_format = WLAN_PKT_FORMAT_IPX_TYPE4;
			else if(!memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_AARP, 2))
				wlan_pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		}
		else if (!memcmp(e_snap->oui, SNAP_HDR_APPLETALK_DDP, WLAN_IEEE_OUI_LEN) &&
				 !memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_DDP, 2))
			wlan_pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		else if (!memcmp(e_snap->oui, oui_8021h, WLAN_IEEE_OUI_LEN))
			wlan_pkt_format = WLAN_PKT_FORMAT_SNAP_TUNNEL;
		else if (!memcmp(e_snap->oui, oui_cisco, WLAN_IEEE_OUI_LEN))
			wlan_pkt_format = WLAN_PKT_FORMAT_CDP;
		else {
#ifdef CONFIG_WLAN_STATS_EXTENTION
			priv->ext_stats.unknown_pro_pkts_cnt++;
#endif
			DEBUG_ERR("drop pkt due to invalid frame format!\n");
			return FAIL;
		}
	}
	else if ((memcmp(daddr, e_hdr->daddr, WLAN_ETHADDR_LEN) == 0) &&
			 (memcmp(saddr, e_hdr->saddr, WLAN_ETHADDR_LEN) == 0))
		wlan_pkt_format = WLAN_PKT_FORMAT_ENCAPSULATED;
	else
		wlan_pkt_format = WLAN_PKT_FORMAT_OTHERS;

	DEBUG_INFO("Convert 802.11 to 802.3 in format %d\n", wlan_pkt_format);

	if ((wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042) ||
		(wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL) ||
		(wlan_pkt_format == WLAN_PKT_FORMAT_CDP)) {
		/* Test for an overlength frame */
		payload_length = payload_length - sizeof(struct wlan_llc_t) - sizeof(struct wlan_snap_t);

		if ((payload_length+WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("SNAP frame too large (%d>%d)\n",
				(payload_length+WLAN_ETHHDR_LEN), WLAN_MAX_ETHFRM_LEN);
		}

#ifdef RX_SHORTCUT
		if (!priv->pmib->dot11OperationEntry.disable_rxsc && pstat) {
#ifdef CONFIG_RTK_MESH
			if (pfrinfo->is_11s)
				privacy = get_sta_encrypt_algthm(priv, pstat);
			else
#endif // CONFIG_RTK_MESH
#ifdef WDS
			if (pfrinfo->to_fr_ds == 3 && priv->pmib->dot11WdsInfo.wdsEnabled)
				privacy = priv->pmib->dot11WdsInfo.wdsPrivacy;
			else
#endif
				privacy = get_sta_encrypt_algthm(priv, pstat);
			if ((GetFragNum(pframe)==0) &&
#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)
                (!pfrinfo->is_11s || !IS_MCAST(pfrinfo->da)) &&
#endif
				((privacy == 0) ||
#ifdef CONFIG_RTL_WAPI_SUPPORT
				(privacy==_WAPI_SMS4_) ||
#endif
#ifdef CONFIG_IEEE80211W
				(!UseSwCrypto(priv, pstat, IS_MCAST(GetAddr1Ptr(pframe)), 0))))	
#else
				(!UseSwCrypto(priv, pstat, IS_MCAST(GetAddr1Ptr(pframe))))))
#endif
			{
				idx = get_rx_sc_free_entry(pstat, pfrinfo);
				prxsc_entry = &pstat->rx_sc_ent[idx];
				memcpy((void *)&wlanhdr, pframe, pfrinfo->hdr_len);
			}
		}
#endif // RX_SHORTCUT


		/* chop 802.11 header from skb. */
		skb_pull(skb, payload_offset);

		if ((wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042) ||
			(wlan_pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL))
		{
			/* chop llc header from skb. */
			skb_pull(skb, sizeof(struct wlan_llc_t));

			/* chop snap header from skb. */
			skb_pull(skb, sizeof(struct wlan_snap_t));
		}

#ifdef CONFIG_RTK_MESH
		/* chop mesh header from skb. */
		skb_pull(skb, mesh_header_len);
#endif

		/* create 802.3 header at beginning of skb. */
		e_hdr = (struct wlan_ethhdr_t *)skb_push(skb, WLAN_ETHHDR_LEN);
		if (wlan_pkt_format == WLAN_PKT_FORMAT_CDP)
			e_hdr->type = payload_length;
		else
			e_hdr->type = e_snap->type;
		memcpy((void *)e_hdr->daddr, daddr, WLAN_ETHADDR_LEN);
		memcpy((void *)e_hdr->saddr, saddr, WLAN_ETHADDR_LEN);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length + WLAN_ETHHDR_LEN);

#ifdef RX_SHORTCUT
		if (prxsc_entry) {
			if ((e_hdr->type != htons(0x888e)) && // for WIFI_SIMPLE_CONFIG
#ifdef CONFIG_RTL_WAPI_SUPPORT
				(e_hdr->type != htons(ETH_P_WAPI)) &&
#endif
				(e_hdr->type != htons(ETH_P_ARP)) &&
				(wlan_pkt_format != WLAN_PKT_FORMAT_CDP)) {
				memcpy((void *)&prxsc_entry->rx_wlanhdr, &wlanhdr, pfrinfo->hdr_len);
				memcpy((void *)&prxsc_entry->rx_ethhdr, (const void *)e_hdr, sizeof(struct wlan_ethhdr_t));
				prxsc_entry->rx_payload_offset = payload_offset;
				prxsc_entry->rx_trim_pad = trim_pad;
				pstat->rx_privacy = GetPrivacy(pframe);

#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)
				if ( (pfrinfo->is_11s &3) && (pfrinfo->mesh_header.mesh_flag &1))
				{
					memcpy((void *)&prxsc_entry->rx_wlanhdr.meshhdr.DestMACAddr, (const void *)pfrinfo->mesh_header.DestMACAddr, WLAN_ETHADDR_LEN);
					memcpy((void *)&prxsc_entry->rx_wlanhdr.meshhdr.SrcMACAddr, (const void *)pfrinfo->mesh_header.SrcMACAddr, WLAN_ETHADDR_LEN);
				}
#endif
			}
		}
#endif
	}
	else if ((wlan_pkt_format == WLAN_PKT_FORMAT_OTHERS) ||
			 (wlan_pkt_format == WLAN_PKT_FORMAT_APPLETALK) ||
			 (wlan_pkt_format == WLAN_PKT_FORMAT_IPX_TYPE4)) {

		/* Test for an overlength frame */
		if ( (payload_length + WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("IPX/AppleTalk frame too large (%d>%d)\n",
				(payload_length + WLAN_ETHHDR_LEN), WLAN_MAX_ETHFRM_LEN);
		}

		/* chop 802.11 header from skb. */
		skb_pull(skb, payload_offset);

#ifdef CONFIG_RTK_MESH
		/* chop mesh header from skb. */
		skb_pull(skb, mesh_header_len);
#endif

		/* create 802.3 header at beginning of skb. */
		e_hdr = (struct wlan_ethhdr_t *)skb_push(skb, WLAN_ETHHDR_LEN);
		memcpy((void *)e_hdr->daddr, daddr, WLAN_ETHADDR_LEN);
		memcpy((void *)e_hdr->saddr, saddr, WLAN_ETHADDR_LEN);
		e_hdr->type = htons(payload_length);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length+WLAN_ETHHDR_LEN);
	}
	else if (wlan_pkt_format == WLAN_PKT_FORMAT_ENCAPSULATED) {

		if ( payload_length > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("Encapsulated frame too large (%d>%d)\n",
				payload_length, WLAN_MAX_ETHFRM_LEN);
		}

		/* Chop off the 802.11 header. */
		skb_pull(skb, payload_offset);

#ifdef CONFIG_RTK_MESH
		/* chop mesh header from skb. */
		skb_pull(skb, mesh_header_len);
#endif

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length);
	}

#if defined(__KERNEL__) || defined(__OSK__)
#ifdef LINUX_2_6_22_
	skb_reset_mac_header(skb);
#else
	skb->mac.raw = (unsigned char *) skb->data; /* new MAC header */
#endif
#endif

	return SUCCESS;
}


int strip_amsdu_llc(struct rtl8192cd_priv *priv, struct sk_buff *skb, struct stat_info *pstat)
{
	INT		payload_length;
	INT		payload_offset;
	UINT8	daddr[WLAN_ETHADDR_LEN];
	UINT8	saddr[WLAN_ETHADDR_LEN];
	struct wlan_ethhdr_t	*e_hdr;
	struct wlan_llc_t		*e_llc;
	struct wlan_snap_t		*e_snap;
	int		pkt_format;

	memcpy(daddr, skb->data, MACADDRLEN);
	memcpy(saddr, skb->data+MACADDRLEN, MACADDRLEN);
	payload_length = skb->len - WLAN_ETHHDR_LEN;
	payload_offset = WLAN_ETHHDR_LEN;

	e_hdr = (struct wlan_ethhdr_t *) (skb->data + payload_offset);
	e_llc = (struct wlan_llc_t *) (skb->data + payload_offset);
	e_snap = (struct wlan_snap_t *) (skb->data + payload_offset + sizeof(struct wlan_llc_t));

	if ((e_llc->dsap==0xaa) && (e_llc->ssap==0xaa) && (e_llc->ctl==0x03))
	{
		if (!memcmp(e_snap->oui, oui_rfc1042, WLAN_IEEE_OUI_LEN)) {
			pkt_format = WLAN_PKT_FORMAT_SNAP_RFC1042;
			if(!memcmp(&e_snap->type, SNAP_ETH_TYPE_IPX, 2))
				pkt_format = WLAN_PKT_FORMAT_IPX_TYPE4;
			else if(!memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_AARP, 2))
				pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		}
		else if (!memcmp(e_snap->oui, SNAP_HDR_APPLETALK_DDP, WLAN_IEEE_OUI_LEN) &&
				 !memcmp(&e_snap->type, SNAP_ETH_TYPE_APPLETALK_DDP, 2))
			pkt_format = WLAN_PKT_FORMAT_APPLETALK;
		else if (!memcmp(e_snap->oui, oui_8021h, WLAN_IEEE_OUI_LEN))
			pkt_format = WLAN_PKT_FORMAT_SNAP_TUNNEL;
		else if (!memcmp(e_snap->oui, oui_cisco, WLAN_IEEE_OUI_LEN))
			pkt_format = WLAN_PKT_FORMAT_CDP;
		else {
			DEBUG_ERR("drop pkt due to invalid frame format!\n");
			return FAIL;
		}
	}
	else if ((memcmp(daddr, e_hdr->daddr, WLAN_ETHADDR_LEN) == 0) &&
			 (memcmp(saddr, e_hdr->saddr, WLAN_ETHADDR_LEN) == 0))
		pkt_format = WLAN_PKT_FORMAT_ENCAPSULATED;
	else
		pkt_format = WLAN_PKT_FORMAT_OTHERS;

	DEBUG_INFO("Convert 802.11 to 802.3 in format %d\n", pkt_format);

	if ((pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042) ||
		(pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL) ||
		(pkt_format == WLAN_PKT_FORMAT_CDP)) {
		/* Test for an overlength frame */
		payload_length = payload_length - sizeof(struct wlan_llc_t) - sizeof(struct wlan_snap_t);

		if ((payload_length+WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("SNAP frame too large (%d>%d)\n",
				(payload_length+WLAN_ETHHDR_LEN), WLAN_MAX_ETHFRM_LEN);
		}

		/* chop 802.11 header from skb. */
		skb_pull(skb, payload_offset);

		if ((pkt_format == WLAN_PKT_FORMAT_SNAP_RFC1042) ||
			(pkt_format == WLAN_PKT_FORMAT_SNAP_TUNNEL))
		{
			/* chop llc header from skb. */
			skb_pull(skb, sizeof(struct wlan_llc_t));

			/* chop snap header from skb. */
			skb_pull(skb, sizeof(struct wlan_snap_t));
		}

		/* create 802.3 header at beginning of skb. */
		e_hdr = (struct wlan_ethhdr_t *)skb_push(skb, WLAN_ETHHDR_LEN);
		if (pkt_format == WLAN_PKT_FORMAT_CDP)
			e_hdr->type = payload_length;
		else
			e_hdr->type = e_snap->type;
		memcpy((void *)e_hdr->daddr, daddr, WLAN_ETHADDR_LEN);
		memcpy((void *)e_hdr->saddr, saddr, WLAN_ETHADDR_LEN);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length + WLAN_ETHHDR_LEN);
	}
	else if ((pkt_format == WLAN_PKT_FORMAT_OTHERS) ||
			 (pkt_format == WLAN_PKT_FORMAT_APPLETALK) ||
			 (pkt_format == WLAN_PKT_FORMAT_IPX_TYPE4)) {

		/* Test for an overlength frame */
		if ( (payload_length + WLAN_ETHHDR_LEN) > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("IPX/AppleTalk frame too large (%d>%d)\n",
				(payload_length + WLAN_ETHHDR_LEN), WLAN_MAX_ETHFRM_LEN);
		}

		/* chop 802.11 header from skb. */
		skb_pull(skb, payload_offset);

		/* create 802.3 header at beginning of skb. */
		e_hdr = (struct wlan_ethhdr_t *)skb_push(skb, WLAN_ETHHDR_LEN);
		memcpy((void *)e_hdr->daddr, daddr, WLAN_ETHADDR_LEN);
		memcpy((void *)e_hdr->saddr, saddr, WLAN_ETHADDR_LEN);
		e_hdr->type = htons(payload_length);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length+WLAN_ETHHDR_LEN);
	}
	else if (pkt_format == WLAN_PKT_FORMAT_ENCAPSULATED) {

		if ( payload_length > WLAN_MAX_ETHFRM_LEN ) {
			/* A bogus length ethfrm has been sent. */
			/* Is someone trying an oflow attack? */
			DEBUG_WARN("Encapsulated frame too large (%d>%d)\n",
				payload_length, WLAN_MAX_ETHFRM_LEN);
		}

		/* Chop off the 802.11 header. */
		skb_pull(skb, payload_offset);

		/* chop off the 802.11 CRC */
		skb_trim(skb, payload_length);
	}

#if defined(__KERNEL__) || defined(__OSK__)
#ifdef LINUX_2_6_22_
	skb_reset_mac_header(skb);
#else
	skb->mac.raw = (unsigned char *) skb->data; /* new MAC header */
#endif
#endif

	return SUCCESS;
}


unsigned int get_sta_encrypt_algthm(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned int privacy = 0;

#ifdef CONFIG_RTK_MESH
	if( isMeshPoint(pstat) )
		return priv->pmib->dot11sKeysTable.dot11Privacy ;
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (pstat&&pstat->wapiInfo&&pstat->wapiInfo->wapiType!=wapiDisable)
	{
		return _WAPI_SMS4_;
	}
	else
#endif
	{
	if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm) {
		if (pstat)
			privacy = pstat->dot11KeyMapping.dot11Privacy;
		else
			DEBUG_ERR("pstat == NULL\n");
	}
		else
		{
		// legacy system
		privacy = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm; //could be wep40 or wep104
	}
	}

	return privacy;
}


unsigned int get_mcast_encrypt_algthm(struct rtl8192cd_priv *priv)
{
	unsigned int privacy;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (!IS_ROOT_INTERFACE(priv) && !IEEE8021X_FUN &&
		((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) ||
		 (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_))){
		 privacy = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
	}else
#endif
#ifdef CONFIG_RTL_WAPI_SUPPORT
	if (priv->pmib->wapiInfo.wapiType!=wapiDisable)
	{
		return _WAPI_SMS4_;
	} else
#endif
	{
	if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm) {
		// check station info
		privacy = priv->pmib->dot11GroupKeysTable.dot11Privacy;
	}
	else {	// legacy system
		privacy = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;//must be wep40 or wep104
	}
	}
	return privacy;
}


unsigned int get_privacy(struct rtl8192cd_priv *priv, struct stat_info *pstat,
				unsigned int *iv, unsigned int *icv, unsigned int *mic)
{
	unsigned int privacy;
	*iv = 0;
	*icv = 0;
	*mic = 0;

	privacy = get_sta_encrypt_algthm(priv, pstat);

	switch (privacy)
	{
#ifdef CONFIG_RTL_WAPI_SUPPORT
	case _WAPI_SMS4_:
		*iv = WAPI_PN_LEN+2;
		*icv = 0;
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)		
	#ifdef CONFIG_IEEE80211W
			if(!(UseSwCrypto(priv, pstat, (pstat ? FALSE : TRUE), 0)))
	#else
			if(!(UseSwCrypto(priv, pstat, (pstat ? FALSE : TRUE))))
	#endif
				*mic = 0;	//HW will take care of the mic
			else
				*mic = SMS4_MIC_LEN;
#else
		*mic = SMS4_MIC_LEN;
#endif
		break;
#endif
	case _NO_PRIVACY_:
		*iv  = 0;
		*icv = 0;
		*mic = 0;
		break;
	case _WEP_40_PRIVACY_:
	case _WEP_104_PRIVACY_:
		*iv = 4;
		*icv = 4;
		*mic = 0;
		break;
	case _TKIP_PRIVACY_:
		*iv = 8;
		*icv = 4;
		*mic = 0;	// mic of TKIP is msdu based
		break;
	case _CCMP_PRIVACY_:
		*iv = 8;
		*icv = 0;
		*mic = 8;
		break;
	default:
		DEBUG_WARN("un-awared encrypted type %d\n", privacy);
		*iv = *icv = *mic = 0;
		break;
	}

	return privacy;
}


unsigned int get_mcast_privacy(struct rtl8192cd_priv *priv, unsigned int *iv, unsigned int *icv,
				unsigned int *mic)
{
	unsigned int privacy;
	*iv  = 0;
	*icv = 0;
	*mic = 0;

	privacy = get_mcast_encrypt_algthm(priv);

	switch (privacy)
	{
#ifdef CONFIG_RTL_WAPI_SUPPORT
	case _WAPI_SMS4_:
		*iv = WAPI_PN_LEN+2;
		*icv = 0;
#if defined(CONFIG_RTL_HW_WAPI_SUPPORT)
#ifdef CONFIG_IEEE80211W
	if(!(UseSwCrypto(priv, NULL, TRUE, 0)))
#else
 	if(!(UseSwCrypto(priv, NULL, TRUE)))
#endif
		*mic = 0;	//HW will take care of the mic
	else
		*mic = SMS4_MIC_LEN;
#else
		*mic = SMS4_MIC_LEN;
#endif
		break;
#endif
	case _NO_PRIVACY_:
		*iv = 0;
		*icv = 0;
		*mic = 0;
		break;
	case _WEP_40_PRIVACY_:
	case _WEP_104_PRIVACY_:
		*iv = 4;
		*icv = 4;
		*mic = 0;
		break;
	case _TKIP_PRIVACY_:
		*iv = 8;
		*icv = 4;
		*mic = 0; // mic of TKIP is msdu based
		break;
	case _CCMP_PRIVACY_:
		*iv = 8;
		*icv = 0;
		*mic = 8;
		break;
	default:
		DEBUG_WARN("un-awared encrypted type %d\n", privacy);
		*iv = 0;
		*icv = 0;
		*mic = 0;
		break;
	}

	return privacy;
}

unsigned char * get_da(unsigned char *pframe)
{
	unsigned char 	*da;
	unsigned int	to_fr_ds	= (GetToDs(pframe) << 1) | GetFrDs(pframe);

	switch (to_fr_ds) {
		case 0x00:	// ToDs=0, FromDs=0
			da = GetAddr1Ptr(pframe);
			break;
		case 0x01:	// ToDs=0, FromDs=1
			da = GetAddr1Ptr(pframe);
			break;
		case 0x02:	// ToDs=1, FromDs=0
			da = GetAddr3Ptr(pframe);
			break;
		default:	// ToDs=1, FromDs=1
			da = GetAddr3Ptr(pframe);
			break;
	}

	return da;
}


unsigned char * get_sa(unsigned char *pframe)
{
	unsigned char 	*sa;
	unsigned int	to_fr_ds	= (GetToDs(pframe) << 1) | GetFrDs(pframe);

	switch (to_fr_ds) {
		case 0x00:	// ToDs=0, FromDs=0
			sa = GetAddr2Ptr(pframe);
			break;
		case 0x01:	// ToDs=0, FromDs=1
			sa = GetAddr3Ptr(pframe);
			break;
		case 0x02:	// ToDs=1, FromDs=0
			sa = GetAddr2Ptr(pframe);
			break;
		default:	// ToDs=1, FromDs=1
			{
				unsigned char *qosCtrl=GetQosControl(pframe);

				if((qosCtrl[0] & BIT(7)))	//AMSDU, 8021.11ac Table 8-19 Address field
					sa = GetAddr2Ptr(pframe);
				else
					sa = GetAddr4Ptr(pframe);
			}
			break;
	}

	return sa;
}

#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028)) 
__MIPS16
#endif
__IRAM_IN_865X
unsigned char get_hdrlen(struct rtl8192cd_priv *priv, UINT8 *pframe)
{
    if (GetFrameType(pframe) == WIFI_DATA_TYPE)
    {
#ifdef CONFIG_RTK_MESH
        if ((get_tofr_ds(pframe) == 0x03) && ( (GetFrameSubType(pframe) == WIFI_11S_MESH) || (GetFrameSubType(pframe) == WIFI_11S_MESH_ACTION)))
        {
            if(GetFrameSubType(pframe) == WIFI_11S_MESH)  // DATA frame, qos might be on (TRUE on 8186)
            {
                return WLAN_HDR_A4_QOS_LEN;
            } // WIFI_11S_MESH
            else // WIFI_11S_MESH_ACTION frame, although qos flag is on, the qos field(2bytes) is not used for 8186
            {
                if(is_mesh_6addr_format_without_qos(pframe)) {
                    return WLAN_HDR_A6_MESH_DATA_LEN;
                } else {
                    return WLAN_HDR_A4_MESH_DATA_LEN;
                }
            }
        } // end of get_tofr_ds == 0x03 & (MESH DATA or MESH ACTION)
        else
#endif // CONFIG_RTK_MESH
        if (is_qos_data(pframe)) {            
            if (get_tofr_ds(pframe) == 0x03) {
                if(GetOrder(pframe))                    
                    return WLAN_HDR_A4_QOS_HT_LEN;
                else
                    return WLAN_HDR_A4_QOS_LEN;
            }
            else {
                if(GetOrder(pframe))                    
                    return WLAN_HDR_A3_QOS_HT_LEN;
                else
                    return WLAN_HDR_A3_QOS_LEN;
            }             
        }
        else {
            if (get_tofr_ds(pframe) == 0x03)
                return WLAN_HDR_A4_LEN;
            else
                return WLAN_HDR_A3_LEN;
        }
    }
    else if (GetFrameType(pframe) == WIFI_MGT_TYPE) {
        if(GetOrder(pframe))
            return 	WLAN_HDR_A3_HT_LEN;
        else
            return 	WLAN_HDR_A3_LEN;
    }
    else if (GetFrameType(pframe) == WIFI_CTRL_TYPE)
    {
        if (GetFrameSubType(pframe) == WIFI_PSPOLL)
            return 16;
        else if (GetFrameSubType(pframe) == WIFI_BLOCKACK_REQ)
            return 16;
        else if (GetFrameSubType(pframe) == WIFI_BLOCKACK)
            return 16;
        else
        {
            DEBUG_INFO("unallowed control pkt type! 0x%04X\n", GetFrameSubType(pframe));
            return 0;
        }
    }
    else
    {
        DEBUG_INFO("unallowed pkt type! 0x%04X\n", GetFrameType(pframe));
        return 0;
    }
}


unsigned char *get_mgtbuf_from_poll(struct rtl8192cd_priv *priv)
{
	unsigned char *ret;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);
	
	ret = get_buf_from_poll(priv, &priv->pshare->wlanbuf_list, (unsigned int *)&priv->pshare->pwlanbuf_poll->count);

	RESTORE_INT(flags);
	return ret;
}


void release_mgtbuf_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);

	release_buf_to_poll(priv, pbuf, &priv->pshare->wlanbuf_list, (unsigned int *)&priv->pshare->pwlanbuf_poll->count);

	RESTORE_INT(flags);
}


unsigned char *get_wlanhdr_from_poll(struct rtl8192cd_priv *priv)
{
	unsigned char *pbuf;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);

	pbuf = get_buf_from_poll(priv, &priv->pshare->wlan_hdrlist, (unsigned int *)&priv->pshare->pwlan_hdr_poll->count);
#ifdef TX_EARLY_MODE
	pbuf += 8;
#endif

	RESTORE_INT(flags);
	return pbuf;
}


void release_wlanhdr_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	if (pbuf == NULL)
	{
		DEBUG_ERR("Err: Free Null Buf!\n");
		return;
	}
	
	SAVE_INT_AND_CLI(flags);

#ifdef TX_EARLY_MODE
	pbuf -= 8;
#endif
	release_buf_to_poll(priv, pbuf, &priv->pshare->wlan_hdrlist, (unsigned int *)&priv->pshare->pwlan_hdr_poll->count);

	RESTORE_INT(flags);
}


//__MIPS16
__IRAM_IN_865X
unsigned char *get_wlanllchdr_from_poll(struct rtl8192cd_priv *priv)
{
	unsigned char *pbuf;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);

	pbuf = get_buf_from_poll(priv, &priv->pshare->wlanllc_hdrlist, (unsigned int *)&priv->pshare->pwlanllc_hdr_poll->count);
#ifdef TX_EARLY_MODE
	pbuf += 8;
#endif

	RESTORE_INT(flags);
	return pbuf;
}

#ifdef __OSK__
__IRAM_WIFI_PRI5
#endif
void release_wlanllchdr_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);

#ifdef TX_EARLY_MODE
	pbuf -= 8;
#endif
	release_buf_to_poll(priv, pbuf, &priv->pshare->wlanllc_hdrlist, (unsigned int *)&priv->pshare->pwlanllc_hdr_poll->count);

	RESTORE_INT(flags);
}


unsigned char *get_icv_from_poll(struct rtl8192cd_priv *priv)
{
	unsigned char *ret;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);

	ret = get_buf_from_poll(priv, &priv->pshare->wlanicv_list, (unsigned int *)&priv->pshare->pwlanicv_poll->count);

	RESTORE_INT(flags);
	return ret;
}


void release_icv_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);

	release_buf_to_poll(priv, pbuf, &priv->pshare->wlanicv_list, (unsigned int *)&priv->pshare->pwlanicv_poll->count);

	RESTORE_INT(flags);
}


unsigned char *get_mic_from_poll(struct rtl8192cd_priv *priv)
{
	unsigned char *ret;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);

	ret = get_buf_from_poll(priv, &priv->pshare->wlanmic_list, (unsigned int *)&priv->pshare->pwlanmic_poll->count);

	RESTORE_INT(flags);
	return ret;
}


void release_mic_to_poll(struct rtl8192cd_priv *priv, unsigned char *pbuf)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);

	release_buf_to_poll(priv, pbuf, &priv->pshare->wlanmic_list, (unsigned int *)&priv->pshare->pwlanmic_poll->count);

	RESTORE_INT(flags);
}


unsigned short get_pnl(union PN48 *ptsc)
{
	return (((ptsc->_byte_.TSC1) << 8) | (ptsc->_byte_.TSC0));
}


unsigned int get_pnh(union PN48 *ptsc)
{
	return 	(((ptsc->_byte_.TSC5) << 24) |
			 ((ptsc->_byte_.TSC4) << 16) |
			 ((ptsc->_byte_.TSC3) << 8) |
			  (ptsc->_byte_.TSC2));
}

#ifdef __OSK__
__IRAM_WIFI_PRI5
#endif
#ifdef CONFIG_IEEE80211W
int UseSwCrypto(struct rtl8192cd_priv *priv, struct stat_info *pstat, int isMulticast, int isPMF)
#else
int UseSwCrypto(struct rtl8192cd_priv *priv, struct stat_info *pstat, int isMulticast)
#endif
{
#ifdef CONFIG_IEEE80211W
		if(isPMF)
			return 1;
#endif

	if (SWCRYPTO)
		return 1;
	else // hw crypto
	{
#ifdef CONFIG_RTK_MESH
	if( isMeshPoint(pstat) )
		return 0;
//		return	(pstat->dot11KeyMapping.keyInCam || isMulticast) ? 0 : 1;

#endif

#ifdef WDS
		if (pstat && (pstat->state & WIFI_WDS) && !(pstat->state & WIFI_ASOC_STATE)) {
			if (!pstat->dot11KeyMapping.keyInCam)
				return 1;
			else
				return 0;
		}
#endif

			if (priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm 
#ifdef CONFIG_RTL_WAPI_SUPPORT
			|| priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WAPI_SMS4_
#endif
				) {
				if (isMulticast) { // multicast
					if (!priv->pmib->dot11GroupKeysTable.keyInCam)
						return 1;
					else
						return 0;
				}
				else {
				if (!pstat->dot11KeyMapping.keyInCam)
					return 1;
				else // key is in CAM
					return 0;
				}
			}
			else { // legacy 802.11 auth (wep40 || wep104)
#ifdef MBSSID
				if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
				{
					if (GET_ROOT(priv)->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) {
						if (isMulticast)
							return 1;
						else {
						if (!pstat->dot11KeyMapping.keyInCam)
							return 1;
						else // key is in CAM
							return 0;
						}
					}
				}
#endif

#ifdef USE_WEP_DEFAULT_KEY
				if (GET_ROOT(priv)->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE)
                {
                    if (pstat && (pstat->state & WIFI_ASOC_STATE))
                        return 0;
                }

				if (isMulticast && 	!priv->pmib->dot11GroupKeysTable.keyInCam)
					return 1;
#else			
				if (isMulticast) {
					if (!priv->pmib->dot11GroupKeysTable.keyInCam)
						return 1;
				}
				else {
					if (!pstat->dot11KeyMapping.keyInCam)
						return 1;				
				}			
#endif			
				return 0;
			}
		}
	}


void check_protection_shortslot(struct rtl8192cd_priv *priv)
{
#ifdef RTK_5G_SUPPORT //eric-8822
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
		return;
#endif
	if (priv->pmib->dot11ErpInfo.nonErpStaNum == 0 &&
			priv->pmib->dot11ErpInfo.olbcDetected == 0)
	{
		if (priv->pmib->dot11ErpInfo.protection) {
			priv->pmib->dot11ErpInfo.protection = 0;
			//priv->pshare->phw->RTSInitRate_Candidate = 0x8;	// 24Mbps
		}
	}
	else
	{
		if (!priv->pmib->dot11StationConfigEntry.protectionDisabled &&
					priv->pmib->dot11ErpInfo.protection == 0) {
			priv->pmib->dot11ErpInfo.protection = 1;
			//priv->pshare->phw->RTSInitRate_Candidate = 0x3; // 11Mbps
		}
	}

	if (priv->pmib->dot11ErpInfo.nonErpStaNum == 0)
	{
		if (priv->pmib->dot11ErpInfo.shortSlot == 0)
		{
			priv->pmib->dot11ErpInfo.shortSlot = 1;
#ifdef MBSSID
			if ((IS_ROOT_INTERFACE(priv))
#ifdef UNIVERSAL_REPEATER
				|| (IS_VXD_INTERFACE(priv))
#endif
				)
#endif
			set_slot_time(priv, priv->pmib->dot11ErpInfo.shortSlot);
			SET_SHORTSLOT_IN_BEACON_CAP;
			DEBUG_INFO("set short slot time\n");
		}
	}
	else
	{
		if (priv->pmib->dot11ErpInfo.shortSlot)
		{
			priv->pmib->dot11ErpInfo.shortSlot = 0;
#ifdef MBSSID
			if ((IS_ROOT_INTERFACE(priv))
#ifdef UNIVERSAL_REPEATER
				|| (IS_VXD_INTERFACE(priv))
#endif
				)
#endif
			set_slot_time(priv, priv->pmib->dot11ErpInfo.shortSlot);
			RESET_SHORTSLOT_IN_BEACON_CAP;
			DEBUG_INFO("reset short slot time\n");
		}
	}
}


void check_sta_characteristic(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act)
{
	if (act == INCREASE) {
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && !isErpSta(pstat)) {
			priv->pmib->dot11ErpInfo.nonErpStaNum++;
			check_protection_shortslot(priv);

			if (!pstat->useShortPreamble)
				priv->pmib->dot11ErpInfo.longPreambleStaNum++;
		}

		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (pstat->ht_cap_len == 0))
			priv->ht_legacy_sta_num++;
	}
	else {
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) && !isErpSta(pstat)) {
			priv->pmib->dot11ErpInfo.nonErpStaNum--;
			check_protection_shortslot(priv);

			if (!pstat->useShortPreamble && priv->pmib->dot11ErpInfo.longPreambleStaNum > 0)
				priv->pmib->dot11ErpInfo.longPreambleStaNum--;
		}

		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (pstat->ht_cap_len == 0))
			priv->ht_legacy_sta_num--;
	}
}

int should_forbid_Nmode(struct rtl8192cd_priv *priv)
{
	if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N))
		return 0;

	if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _NO_PRIVACY_)
		return 0;

	if (!(priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(3)))
		return 0;

	// if pure TKIP, change N mode to G mode
	if (priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(1)) {
		if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK ||
			priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm) {

			if ((priv->pmib->dot1180211AuthEntry.dot11WPACipher == 2) &&
				(priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher == 0))
				return 1;
			else if ((priv->pmib->dot1180211AuthEntry.dot11WPACipher == 0) &&
				(priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher == 2))
				return 1;
			else if ((priv->pmib->dot1180211AuthEntry.dot11WPACipher == 2) &&
				(priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher == 2))
				return 1;
		}
	}

	// if WEP, forbid  N mode
	if ((priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(0)) &&
		((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) ||
		 (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)))
		return 1;

	return 0;
}

#ifdef RTK_AC_SUPPORT //for 11ac logo
int is_mixed_mode(struct rtl8192cd_priv *priv)
{
	if((priv->pmib->dot1180211AuthEntry.dot11EnablePSK == 3)
		&& (priv->pmib->dot1180211AuthEntry.dot11WPACipher & BIT(1))
		&& (priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher & BIT(1))
		&& (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == 2))
	{
		return 1;
	}
	else
		return 0;
}
#endif

int should_restrict_Nrate(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	if (OPMODE & WIFI_AP_STATE)
	{
		if (pstat->is_legacy_encrpt == 1) {
			if (priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(1)) {
				if (!pstat->is_realtek_sta || (priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(2)))
					return 1;
			}
		}
		else if (pstat->is_legacy_encrpt == 2) {
			if (priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(0)) {
				if (!pstat->is_realtek_sta || (priv->pmib->dot11nConfigEntry.dot11nLgyEncRstrct & BIT(2)))
					return 1;
			}
		}

#ifdef RTK_AC_SUPPORT //for 11ac logo  // Cheat for mixed mode
	if(AC_SIGMA_MODE != AC_SIGMA_NONE) {
		if(is_mixed_mode(priv))
			return 1;
	}
#endif
		
#ifdef WDS
		else if (pstat->state & WIFI_WDS) {
			if ((priv->pmib->dot11WdsInfo.wdsPrivacy == _WEP_40_PRIVACY_) ||
				(priv->pmib->dot11WdsInfo.wdsPrivacy == _WEP_104_PRIVACY_) ||
				(priv->pmib->dot11WdsInfo.wdsPrivacy == _TKIP_PRIVACY_))
				return 1;
		}
#endif
	}
// Client mode IOT issue, Button 2009.07.17
#ifdef CLIENT_MODE
	else if(OPMODE & WIFI_STATION_STATE)
	{

		if(!pstat->is_realtek_sta && (pstat->IOTPeer != HT_IOT_PEER_MARVELL) && pstat->is_legacy_encrpt)


		return 1;
	}
#endif

	return 0;
}


#ifdef WDS
int getWdsIdxByDev(struct rtl8192cd_priv *priv, struct net_device *dev)
{
	int i;

#ifdef LAZY_WDS
	int max_num;
	if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE) 
		max_num = NUM_WDS;
	else
		max_num = priv->pmib->dot11WdsInfo.wdsNum;

	for (i=0; i<max_num; i++) {	
#else

	for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++) {
#endif		

		if (dev == priv->wds_dev[i])
			return i;
	}
	return -1;
}


struct net_device *getWdsDevByAddr(struct rtl8192cd_priv *priv, unsigned char *addr)
{
	int i;

#ifdef LAZY_WDS
	int max_num;
	if (priv->pmib->dot11WdsInfo.wdsEnabled == WDS_LAZY_ENABLE) 
		max_num = NUM_WDS;
	else
		max_num = priv->pmib->dot11WdsInfo.wdsNum;

	for (i=0; i<max_num; i++) {	
#else

	for (i=0; i<priv->pmib->dot11WdsInfo.wdsNum; i++) {
#endif		
			if (!memcmp(priv->pmib->dot11WdsInfo.entry[i].macAddr, addr, 6))
				return priv->wds_dev[i];
	}
	return NULL;
}
#endif // WDS


void validate_oper_rate(struct rtl8192cd_priv *priv)
{
	unsigned int supportedRates;
	unsigned int basicRates;

	if (OPMODE & WIFI_AP_STATE)
	{
		supportedRates = priv->pmib->dot11StationConfigEntry.dot11SupportedRates;
		basicRates = priv->pmib->dot11StationConfigEntry.dot11BasicRates;

		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) {
			if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G)) {
				// if use B only, mask G high rate
				supportedRates &= 0xf;
				basicRates &= 0xf;
			}
		}
		else {
			// if use A or G mode, mask B low rate
			supportedRates &= 0xff0;
			basicRates &= 0xff0;
		}

		if (supportedRates == 0) {
			if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G | WIRELESS_11A))
				supportedRates = 0xff0;
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B)
				supportedRates |= 0xf;

			PRINT_INFO("invalid supproted rate, use default value [%x]!\n", supportedRates);
		}

		if (basicRates == 0) {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A)
				//basicRates = 0x1f0;
				//11a basic rate is 6/12/24M 
				basicRates = 0x150;
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B)
				basicRates = 0xf;
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
				if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B))
					basicRates = 0x1f0;
			}

			PRINT_INFO("invalid basic rate, use default value [%x]!\n", basicRates);
		}

		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) {
				if ((basicRates & 0xf) == 0)		// if no CCK rates. jimmylin 2004/12/02
					basicRates |= 0xf;
				if ((supportedRates & 0xf) == 0)	// if no CCK rates. jimmylin 2004/12/02
					supportedRates |= 0xf;
			}
			if ((supportedRates & 0xff0) == 0) {	// no ERP rate existed
				supportedRates |= 0xff0;

				PRINT_INFO("invalid supported rate for 11G, use default value [%x]!\n",
																	supportedRates);
			}
		}

		priv->supported_rates = supportedRates;
		priv->basic_rates = basicRates;

		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			if (priv->pmib->dot11nConfigEntry.dot11nSupportedMCS == 0)
				priv->pmib->dot11nConfigEntry.dot11nSupportedMCS = 0xffff;
		}
	}
#ifdef CLIENT_MODE
	else
	{
		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {
			if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11B | WIRELESS_11G))
				priv->dual_band = 1;
			else
				priv->dual_band = 0;
		}
		else
			priv->dual_band = 0;

		if (priv->dual_band) {
			// for 2.4G band
			supportedRates = priv->pmib->dot11StationConfigEntry.dot11SupportedRates;
			basicRates = priv->pmib->dot11StationConfigEntry.dot11BasicRates;

			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) {
				if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G)) {
					supportedRates &= 0xf;
					basicRates &= 0xf;
				}
				if ((supportedRates & 0xf) == 0)
					supportedRates |= 0xf;
				if ((basicRates & 0xf) == 0)
					basicRates |= 0xf;
			}
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) {
				if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B)) {
					supportedRates &= 0xff0;
					basicRates &= 0xff0;
				}
				if ((supportedRates & 0xff0) == 0)
					supportedRates |= 0xff0;
				if ((basicRates & 0xff0) == 0)
					basicRates |= 0x1f0;
			}

			priv->supported_rates = supportedRates;
			priv->basic_rates = basicRates;

			// for 5G band
			supportedRates = priv->pmib->dot11StationConfigEntry.dot11SupportedRates;
			basicRates = priv->pmib->dot11StationConfigEntry.dot11BasicRates;

			supportedRates &= 0xff0;
			basicRates &= 0xff0;
			if (supportedRates == 0)
				supportedRates |= 0xff0;
			if (basicRates == 0)
				basicRates |= 0x1f0;

			priv->supported_rates_alt = supportedRates;
			priv->basic_rates_alt = basicRates;
		}
		else {
			supportedRates = priv->pmib->dot11StationConfigEntry.dot11SupportedRates;
			basicRates = priv->pmib->dot11StationConfigEntry.dot11BasicRates;

			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11B) {
				if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11G)) {
					supportedRates &= 0xf;
					basicRates &= 0xf;
				}
				if ((supportedRates & 0xf) == 0)
					supportedRates |= 0xf;
				if ((basicRates & 0xf) == 0)
					basicRates |= 0xf;
			}
			if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11G | WIRELESS_11A)) {
				if (!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11B)) {
					supportedRates &= 0xff0;
					basicRates &= 0xff0;
				}
				if ((supportedRates & 0xff0) == 0)
					supportedRates |= 0xff0;
				if ((basicRates & 0xff0) == 0)
					basicRates |= 0x1f0;
			}

			priv->supported_rates = supportedRates;
			priv->basic_rates = basicRates;
		}

		if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
			if (priv->pmib->dot11nConfigEntry.dot11nSupportedMCS == 0)
				priv->pmib->dot11nConfigEntry.dot11nSupportedMCS = 0xffff;
		}
	}
#endif
#if defined(RTK_AC_SUPPORT)
	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC) {
		
		if (IS_TEST_CHIP(priv))	{
			if(get_rf_mimo_mode(priv) == MIMO_1T1R) {
				priv->pmib->dot11acConfigEntry.dot11VHT_TxMap &= 0x0ff;
				priv->pmib->dot11acConfigEntry.dot11SupportedVHT = 0xfffc;
			} else if ((get_rf_mimo_mode(priv) == MIMO_3T3R)||(get_rf_mimo_mode(priv) == MIMO_4T4R)) {  //eric_8814
				priv->pmib->dot11acConfigEntry.dot11VHT_TxMap &= 0x3fffffff;
				priv->pmib->dot11acConfigEntry.dot11SupportedVHT = 0xffea;
			} else {
				priv->pmib->dot11acConfigEntry.dot11VHT_TxMap &= 0xfffff;
				priv->pmib->dot11acConfigEntry.dot11SupportedVHT = 0xfffa;
			} 
		} else {
			if(get_rf_mimo_mode(priv) == MIMO_1T1R) {
				priv->pmib->dot11acConfigEntry.dot11VHT_TxMap &= 0x3ff;
				priv->pmib->dot11acConfigEntry.dot11SupportedVHT |= 0xfffc;

				if(!priv->pmib->dot11acConfigEntry.dot11VHT_TxMap)
					priv->pmib->dot11acConfigEntry.dot11VHT_TxMap = 0x3ff;
				if(priv->pmib->dot11acConfigEntry.dot11SupportedVHT == 0xffff)
					priv->pmib->dot11acConfigEntry.dot11SupportedVHT = 0xfffe;
			} else if (get_rf_mimo_mode(priv) == MIMO_2T2R) { //eric_8814
				priv->pmib->dot11acConfigEntry.dot11VHT_TxMap &= 0xfffff;
				priv->pmib->dot11acConfigEntry.dot11SupportedVHT |= 0xfff0;

				if(!priv->pmib->dot11acConfigEntry.dot11VHT_TxMap)
					priv->pmib->dot11acConfigEntry.dot11VHT_TxMap = 0xfffff;
				if(priv->pmib->dot11acConfigEntry.dot11SupportedVHT == 0xffff)
					priv->pmib->dot11acConfigEntry.dot11SupportedVHT = 0xfffa;
			} else if ((get_rf_mimo_mode(priv) == MIMO_3T3R) ||(get_rf_mimo_mode(priv) == MIMO_4T4R)) {  //eric_8814
				priv->pmib->dot11acConfigEntry.dot11VHT_TxMap &= 0x3fffffff;
				priv->pmib->dot11acConfigEntry.dot11SupportedVHT |= 0xffc0;
				
				if(!priv->pmib->dot11acConfigEntry.dot11VHT_TxMap)
					priv->pmib->dot11acConfigEntry.dot11VHT_TxMap = 0x3fffffff;
				if(priv->pmib->dot11acConfigEntry.dot11SupportedVHT == 0xffff)
					priv->pmib->dot11acConfigEntry.dot11SupportedVHT = 0xffda;
			}			
		}
	} 

#endif	
}


void get_oper_rate(struct rtl8192cd_priv *priv)
{
	unsigned int supportedRates=0;
	unsigned int basicRates=0;
	unsigned char val;
	int i, idx=0;

	memset(AP_BSSRATE, 0, sizeof(AP_BSSRATE));
	AP_BSSRATE_LEN = 0;

	if (OPMODE & WIFI_AP_STATE) {
		supportedRates = priv->supported_rates;
		basicRates = priv->basic_rates;
	}
#ifdef CLIENT_MODE
	else {
		if (priv->dual_band && (priv->pshare->curr_band == BAND_5G)) {
			supportedRates = priv->supported_rates_alt;
			basicRates = priv->basic_rates_alt;
		}
		else {
			supportedRates = priv->supported_rates;
			basicRates = priv->basic_rates;
		}
	}
#endif

	for (i=0; dot11_rate_table[i]; i++) {
		int bit_mask = 1 << i;
		if (supportedRates & bit_mask) {
			val = dot11_rate_table[i];

#ifdef SUPPORT_SNMP_MIB
			SNMP_MIB_ASSIGN(dot11SupportedDataRatesSet[i], ((unsigned int)val));
			SNMP_MIB_ASSIGN(dot11OperationalRateSet[i], ((unsigned char)val));
#endif

			if (basicRates & bit_mask)
				val |= 0x80;

			AP_BSSRATE[idx] = val;
			AP_BSSRATE_LEN++;
			idx++;
		}
	}

#ifdef SUPPORT_SNMP_MIB
	SNMP_MIB_ASSIGN(dot11SupportedDataRatesNum, ((unsigned char)AP_BSSRATE_LEN));
#endif

}


// bssrate_ie: _SUPPORTEDRATES_IE_ get supported rate set
// bssrate_ie: _EXT_SUPPORTEDRATES_IE_ get extended supported rate set
int get_bssrate_set(struct rtl8192cd_priv *priv, int bssrate_ie, unsigned char **pbssrate, int *bssrate_len)
{
	int i;

#ifdef CONFIG_RTL_92D_SUPPORT
	if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
#else
	if ((priv->pshare->curr_band == BAND_5G)||(priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80)) //AC2G_256QAM
#endif
	{

#ifdef P2P_SUPPORT			
		if(bssrate_ie == _SUPPORTED_RATES_NO_CCK_ ){
				*pbssrate = &dot11_rate_table[4];
				*bssrate_len = 8;
				return TRUE;
		}
#endif			
	
		if (bssrate_ie == _SUPPORTEDRATES_IE_ 	)
		{

			for(i=0; i<AP_BSSRATE_LEN; i++)									
				if (!is_CCK_rate(AP_BSSRATE[i] & 0x7f))
					break;

			if (i == AP_BSSRATE_LEN)
				return FALSE;
			else {
				*pbssrate = &AP_BSSRATE[i];
				*bssrate_len = AP_BSSRATE_LEN - i;
				return TRUE;
			}
		}
		else
			return FALSE;
	}
	else
	{
		if (bssrate_ie == _SUPPORTEDRATES_IE_)
		{
			*pbssrate = AP_BSSRATE;
			if (AP_BSSRATE_LEN > 8)
				*bssrate_len = 8;
			else
				*bssrate_len = AP_BSSRATE_LEN;
			return TRUE;
		}
#ifdef P2P_SUPPORT
		else if( bssrate_ie == _SUPPORTED_RATES_NO_CCK_){
				*pbssrate = &dot11_rate_table[4];
				*bssrate_len = 8;
				return TRUE;
		}
#endif		
		else
		{
			if (AP_BSSRATE_LEN > 8) {
				*pbssrate = &AP_BSSRATE[8];
				*bssrate_len = AP_BSSRATE_LEN - 8;
				return TRUE;
			}
			else
				return FALSE;
		}
	}
}


struct channel_list{
	unsigned char	channel[31];
	unsigned char	len;
};
static struct channel_list reg_channel_2_4g[] = {
	/* FCC */		{{1,2,3,4,5,6,7,8,9,10,11},11},
	/* IC */		{{1,2,3,4,5,6,7,8,9,10,11},11},
	/* ETSI */		{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* SPAIN */		{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* FRANCE */	{{10,11,12,13},4},
	/* MKK */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* ISRAEL */	{{3,4,5,6,7,8,9,10,11,12,13},11},
	/* MKK1 */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* MKK2 */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* MKK3 */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* NCC (Taiwan) */	{{1,2,3,4,5,6,7,8,9,10,11},11},
	/* RUSSIAN */	{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* CN */		{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* Global */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* World_wide */	{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
	/* Test */		{{1,2,3,4,5,6,7,8,9,10,11,12,13,14},14},
	/* 5M10M */		{{},0},
	/* SG */		{{1,2,3,4,5,6,7,8,9,10,11},11},
	/* KR */		{{1,2,3,4,5,6,7,8,9,10,11,12,13},13},
};

#ifdef DFS
static struct channel_list reg_channel_5g_full_band[] = {
	/* FCC */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165},20},
	/* IC */		{{36,40,44,48,52,56,60,64,149,153,157,161},12},
	/* ETSI */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* SPAIN */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* FRANCE */	{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* MKK */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* ISRAEL */	{{36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140},19},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48,52,56,60,64},8},
	/* NCC (Taiwan) */	{{56,60,64,100,104,108,112,116,136,140,149,153,157,161,165},15},
	/* RUSSIAN */	{{36,40,44,48,52,56,60,64,132,136,140,149,153,157,161,165},16},
	/* CN */		{{36,40,44,48,52,56,60,64,149,153,157,161,165},13},
	/* Global */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165},20},
	/* World_wide */	{{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165},20},
	/* Test */		{{36,40,44,48, 52,56,60,64, 100,104,108,112, 116,120,124,128, 132,136,140,144, 149,153,157,161, 165,169,173,177}, 28},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{36,40,44,48,149,153,157,161,165},9},
	/* KR */		{{36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165},20},
};

struct channel_list reg_channel_5g_not_dfs_band[] = {
	/* FCC */		{{36,40,44,48,149,153,157,161,165},9},
	/* IC */		{{36,40,44,48,149,153,157,161},8},
	/* ETSI */		{{36,40,44,48},4},
	/* SPAIN */		{{36,40,44,48},4},
	/* FRANCE */	{{36,40,44,48},4},
	/* MKK */		{{36,40,44,48},4},
	/* ISRAEL */	{{36,40,44,48},4},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48},4},
	/* NCC (Taiwan) */	{{56,60,64,149,153,157,161,165},8},
	/* RUSSIAN */	{{36,40,44,48,149,153,157,161,165},9},
	/* CN */		{{36,40,44,48,149,153,157,161,165},9},
	/* Global */		{{36,40,44,48,149,153,157,161,165},9},
	/* World_wide */	{{36,40,44,48,149,153,157,161,165},9},
	/* Test */		{{36,40,44,48, 149,153,157,161, 165,169,173,177}, 12},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{36,40,44,48,149,153,157,161,165},9},
	/* KR */		{{36,40,44,48,149,153,157,161,165},9},
};
#else

// Exclude DFS channels
static struct channel_list reg_channel_5g_full_band[] = {
	/* FCC */		{{36,40,44,48,149,153,157,161,165},9},
	/* IC */		{{36,40,44,48,149,153,157,161},8},
	/* ETSI */		{{36,40,44,48},4},
	/* SPAIN */		{{36,40,44,48},4},
	/* FRANCE */	{{36,40,44,48},4},
	/* MKK */		{{36,40,44,48},4},
	/* ISRAEL */	{{36,40,44,48},4},
	/* MKK1 */		{{34,38,42,46},4},
	/* MKK2 */		{{36,40,44,48},4},
	/* MKK3 */		{{36,40,44,48},4},
	/* NCC (Taiwan) */	{{56,60,64,149,153,157,161,165},8},
	/* RUSSIAN */	{{36,40,44,48,149,153,157,161,165},9},
	/* CN */		{{36,40,44,48,149,153,157,161,165},9},
	/* Global */		{{36,40,44,48,149,153,157,161,165},9},
	/* World_wide */	{{36,40,44,48,149,153,157,161,165},9},
	/* Test */		{{36,40,44,48, 52,56,60,64, 100,104,108,112, 116,120,124,128, 132,136,140,144, 149,153,157,161, 165,169,173,177}, 28},
	/* 5M10M */		{{146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170}, 25},
	/* SG */		{{36,40,44,48,149,153,157,161,165},9},
	/* KR */		{{36,40,44,48,149,153,157,161,165},9},	
};
#endif


int is_available_channel(struct rtl8192cd_priv *priv, unsigned char channel) {
	
	if(priv->pmib->dot11DFSEntry.disable_DFS == 1) {
		if((channel >= 52 && channel <= 140))
			return 0;
	}
	
	if((priv->pmib->dot11RFEntry.band5GSelected & PHY_BAND_5G_1) &&
				(channel >= 36 && channel <= 48))
		return 1;
	else if((priv->pmib->dot11RFEntry.band5GSelected & PHY_BAND_5G_2) &&
				(channel >= 52 && channel <= 64))
		return 1;
	else if((priv->pmib->dot11RFEntry.band5GSelected & PHY_BAND_5G_3) &&
				(channel >= 100 && channel <= 144))
		return 1;
	else if((priv->pmib->dot11RFEntry.band5GSelected & PHY_BAND_5G_4) &&
				(channel >= 149 && channel <= 177))
		return 1;
	else if((priv->pmib->dot11RFEntry.band5GSelected & PHY_BAND_5G_4) &&
				(channel >= 146 && channel <= 177) && 
				(priv->pmib->dot11nConfigEntry.dot11nUse40M == 4 ||priv->pmib->dot11nConfigEntry.dot11nUse40M == 5))
		return 1;
	else
		return 0;
}


int is_available_NonDFS_channel(struct rtl8192cd_priv *priv, unsigned char channel)
{
	if((priv->pmib->dot11RFEntry.band5GSelected == PHY_BAND_5G_3))
		return 0;
	else
		return 1;
}


#define MAX_NUM_80M_CH 7
unsigned int CH_80m[MAX_NUM_80M_CH]={36,52,100,116,132,149,165};

int is80MChannel(unsigned int chnl_list[], unsigned int chnl_num,unsigned int channel)
{
	int idx;
	int chNO;
	int baseCH=0;
	idx = -1;

	if( (CH_80m[MAX_NUM_80M_CH-1] <= channel) && ((CH_80m[MAX_NUM_80M_CH-1]+8) >= channel))
		baseCH = CH_80m[MAX_NUM_80M_CH-1];
	else
	{
		for(chNO=0;chNO<MAX_NUM_80M_CH-1;chNO++) {
			if(CH_80m[chNO] <= channel && CH_80m[chNO+1] > channel) {
				baseCH = CH_80m[chNO];			
				break;
			}
		}
	}

	if(baseCH == 0)
		_DEBUG_ERR("Channel is out of scope\n");
	
	for(idx=0;idx<chnl_num;idx++) {
		// available_chnl is sorted.
		if(chnl_list[idx] == baseCH)
			break;
	}

	if(idx == chnl_num || idx + 3 >= chnl_num)
		return 0;

	if(chnl_list[idx+1] == baseCH + 4 &&
	   chnl_list[idx+2] == baseCH + 8 &&
	   chnl_list[idx+3] == baseCH + 12)
	   return 1;
	else
	   return 0;
}


#define MAX_NUM_40M_CH 14
unsigned int CH_40m[MAX_NUM_40M_CH]={36,44,52,60,100,108,116,124,132,140,149,157,165,173};

int is40MChannel(unsigned int chnl_list[], unsigned int chnl_num,unsigned int channel)
{
	int idx;
	int chNO;
	int baseCH=0;
	idx = -1;

	if( (CH_40m[MAX_NUM_40M_CH-1] <= channel) && ((CH_40m[MAX_NUM_40M_CH-1]+4) >= channel))
		baseCH = CH_40m[MAX_NUM_40M_CH-1];
	else
	{
		for(chNO=0;chNO<MAX_NUM_40M_CH-1;chNO++) {
			if(CH_40m[chNO] <= channel && CH_40m[chNO+1] > channel) {
				baseCH = CH_40m[chNO];			
				break;
			}
		}
	}

	if(baseCH == 0)
		_DEBUG_ERR("Channel is out of scope\n");
	
	for(idx=0;idx<chnl_num;idx++) {
		// available_chnl is sorted.
		if(chnl_list[idx] == baseCH)
			break;
	}

	if(idx == chnl_num || idx + 1 >= chnl_num)
		return 0;

	if(chnl_list[idx+1] == baseCH + 4)
	   return 1;
	else
	   return 0;
}


int find80MChannel(unsigned int chnl_list[], unsigned int chnl_num) {
	int i,j;	
	unsigned int random;
	unsigned int num;
#ifdef __ECOS
	{
		unsigned char random_buf[4];
		get_random_bytes(random_buf, 4);
		random = random_buf[3];
	}
#else
		get_random_bytes(&random, 4);
#endif
		num = random % chnl_num;
	for(i=num;i<chnl_num+num;i++) {
		j=i;
		if (j>=chnl_num)
			j=j-chnl_num;
		if(is80MChannel(chnl_list,chnl_num,chnl_list[j]))
			return chnl_list[j];
	}
	return -1;
}


int find40MChannel(unsigned int chnl_list[], unsigned int chnl_num)
{
	int i,j;	
	unsigned int random;
	unsigned int num;
#ifdef __ECOS
	{
		unsigned char random_buf[4];
		get_random_bytes(random_buf, 4);
		random = random_buf[3];
	}
#else
		get_random_bytes(&random, 4);
#endif
		num = random % chnl_num;
	
	for(i=num;i<chnl_num+num;i++) {
		j=i;
		if (j>=chnl_num)
			j=j-chnl_num;
		if(is40MChannel(chnl_list,chnl_num,chnl_list[j]))
			return chnl_list[j];
	}
	return -1;
}


int get_available_channel(struct rtl8192cd_priv *priv)
{
	int i, reg;
	struct channel_list *ch_5g_lst=NULL;

	priv->available_chnl_num = 0;
	reg = priv->pmib->dot11StationConfigEntry.dot11RegDomain;

	if ((reg < DOMAIN_FCC) || (reg >= DOMAIN_MAX))
		return FAIL;

	if (priv->pmib->dot11BssType.net_work_type & (WIRELESS_11B | WIRELESS_11G) || 
		((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
			!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11A))) {
#if 0 //def AC2G_256QAM
		if(is_ac2g(priv)) //if 2.4G + 11ac mode, force available channels = 1, 5, 9, 13
		{
				priv->available_chnl[0]=1;
				priv->available_chnl[1]=5;
				priv->available_chnl[2]=9;
				priv->available_chnl[3]=13;
				priv->available_chnl_num=4;
		}
		else
#endif
		{
			for (i=0; i<reg_channel_2_4g[reg-1].len; i++)
				priv->available_chnl[i] = reg_channel_2_4g[reg-1].channel[i];
			priv->available_chnl_num += reg_channel_2_4g[reg-1].len;
		}
	}

	if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A) {

		ch_5g_lst = reg_channel_5g_full_band;

		for (i=0; i<ch_5g_lst[reg-1].len; i++) {
				if(is_available_channel(priv,ch_5g_lst[reg-1].channel[i]))
					priv->available_chnl[priv->available_chnl_num++] = ch_5g_lst[reg-1].channel[i];
		}
		
		//for (i=0; i<ch_5g_lst[reg-1].len; i++)
		//	priv->available_chnl[priv->available_chnl_num+i] = ch_5g_lst[reg-1].channel[i];
		//priv->available_chnl_num += ch_5g_lst[reg-1].len;

#ifdef DFS
		/* remove the blocked channels from available_chnl[32] */
		if (priv->NOP_chnl_num)
			for (i=0; i<priv->NOP_chnl_num; i++)
				RemoveChannel(priv, priv->available_chnl, &priv->available_chnl_num, priv->NOP_chnl[i]);

		priv->Not_DFS_chnl_num = 0;
		for (i=0; i<reg_channel_5g_not_dfs_band[reg-1].len; i++) {
			if(is_available_NonDFS_channel(priv,reg_channel_5g_not_dfs_band[reg-1].channel[i]))
				priv->Not_DFS_chnl[priv->Not_DFS_chnl_num++] = reg_channel_5g_not_dfs_band[reg-1].channel[i];
			
		}
		
		
#endif
	}

// add by david ---------------------------------------------------
	if (priv->pmib->dot11RFEntry.dot11ch_low ||  priv->pmib->dot11RFEntry.dot11ch_hi) {
		unsigned int tmpbuf[100];
		int num=0;
		for (i=0; i<priv->available_chnl_num; i++) {
			if ( (priv->pmib->dot11RFEntry.dot11ch_low &&
					priv->available_chnl[i] < priv->pmib->dot11RFEntry.dot11ch_low) ||
				(priv->pmib->dot11RFEntry.dot11ch_hi &&
					priv->available_chnl[i] > priv->pmib->dot11RFEntry.dot11ch_hi))
				continue;
			else
				tmpbuf[num++] = priv->available_chnl[i];
		}
		if (num) {
			memcpy(priv->available_chnl, tmpbuf, num*4);
			priv->available_chnl_num = num;
		}
	}
//------------------------------------------------------ 2007-04-14

	return SUCCESS;
}


void cnt_assoc_num(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act, char *func)
{
#ifdef CONFIG_RTL_92D_SUPPORT
	int i;
#endif

#ifdef INDICATE_LINK_CHANGE
	if (!IEEE8021X_FUN && ((_NO_PRIVACY_ == priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm) ||
			(_WEP_40_PRIVACY_ == priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm) ||
			(_WEP_104_PRIVACY_ == priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm))) {
		indicate_sta_link_change(priv, pstat, act, func);
	}
#endif
	
	if (act == INCREASE) {
		if (priv->assoc_num <= NUM_STAT) {
			priv->assoc_num++;
#ifdef TLN_STATS
			if (priv->assoc_num > priv->wifi_stats.max_sta) {
				priv->wifi_stats.max_sta = priv->assoc_num;
				priv->wifi_stats.max_sta_timestamp = priv->up_time;
			}
#endif
			priv->pshare->total_assoc_num++;
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
			if (GET_CHIP_VER(priv) == VERSION_8188E) {
				update_remapAid(priv, pstat);
	#if defined(CONFIG_PCI_HCI)
				RTL8188E_AssignTxReportMacId(priv);
				if (priv->pshare->total_assoc_num == 1)
					RTL8188E_ResumeTxReport(priv);
	#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
				notify_tx_report_change(priv);
	#endif
			}
#endif
#if 0
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (IS_ROOT_INTERFACE(priv))
#endif
			{
				if (priv->assoc_num > 1)
					check_DIG_by_rssi(priv, 0);	// force DIG temporary off for association after the fist one
			}
#endif
			if (pstat->ht_cap_len) {
				priv->pshare->ht_sta_num++;
				if (priv->pshare->iot_mode_enable && (priv->pshare->ht_sta_num == 1)
#ifdef RTL_MANUAL_EDCA
						&& (priv->pmib->dot11QosEntry.ManualEDCA == 0)
#endif
						) {
#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
						if(IS_OUTSRC_CHIP(priv))
#endif
						IotEdcaSwitch(GET_ROOT(priv), priv->pshare->iot_mode_enable);
#endif
#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
						if(!IS_OUTSRC_CHIP(priv))
#endif
						IOT_EDCA_switch(priv, priv->pmib->dot11BssType.net_work_type, priv->pshare->iot_mode_enable);
#endif
				}

#ifdef WIFI_11N_2040_COEXIST
				if (priv->pmib->dot11nConfigEntry.dot11nCoexist && priv->pshare->is_40m_bw &&
					(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N|WIRELESS_11G))) {
					if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_40M_INTOLERANT_)) {
						if (OPMODE & WIFI_AP_STATE) {
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)	
							unsigned int force_20_stamap;
							force_20_stamap = orSTABitMap(&priv->force_20_sta);
							setSTABitMap(&priv->force_20_sta, pstat->aid);
							// force all STA switch TXBW to 20M
							if (0 == force_20_stamap)
								update_RAMask_to_FW(priv, 1);
#else
							setSTABitMap(&priv->force_20_sta, pstat->aid);
#endif
#if defined(WIFI_11N_2040_COEXIST_EXT)
							clearSTABitMap(&priv->pshare->_40m_staMap, pstat->aid);

#endif
						} 
					}
				}
#endif

				check_NAV_prot_len(priv, pstat, 0);
			}
#ifdef MULTI_STA_REFINE
			if( priv->pshare->total_assoc_num >10 ) {
				priv->ht_protection = 1;
				priv->ht_legacy_sta_num=100;
			}
#endif	
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)					
			if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
				UpdateHalMSRRPT8812(priv, pstat, INCREASE);		
				pstat->txpdrop_flag =0;	
				RTL8812_MACID_PAUSE(priv, 0, REMAP_AID(pstat));
				if(priv->pshare->paused_sta_num && pstat->txpause_flag) {
					priv->pshare->paused_sta_num--;	
					pstat->txpause_flag =0;
	        	}   				
			}
#endif
#ifdef CONFIG_WLAN_HAL
            if (IS_HAL_CHIP(priv)) {
               if(pstat->aid <128)
               {
                    GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat, INCREASE);				
					pstat->txpdrop_flag =0;
                    GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat));
					if(priv->pshare->paused_sta_num && pstat->txpause_flag) {
						priv->pshare->paused_sta_num--;	
						pstat->txpause_flag =0;
		        	}            
                    pstat->bDrop = 0; 
                }
            }
#endif

#ifdef HW_FILL_MACID
        if (IS_SUPPORT_HW_FILL_MACID(priv))  {
        //Init fill mac address,Init fill CRC5
        GET_HAL_INTERFACE(priv)->SetTxRPTHandler(priv, REMAP_AID(pstat), TXRPT_VAR_MAC_ADDRESS, &pstat->hwaddr);                         
        GET_HAL_INTERFACE(priv)->SetCRC5ToRPTBufferHandler(priv,CRC5(&pstat->hwaddr,6), REMAP_AID(pstat),1);                           
        }       
#endif //#ifdef HW_FILL_MACID            
		} else {
			DEBUG_ERR("Association Number Error (%d)!\n", NUM_STAT);
		}
	} else {
		if (priv->assoc_num > 0) {
			priv->assoc_num--;
			priv->pshare->total_assoc_num--;			
#if defined(CONFIG_RTL_88E_SUPPORT) && defined(TXREPORT)
			if (GET_CHIP_VER(priv) == VERSION_8188E) {
	#if defined(CONFIG_PCI_HCI)
				if (!priv->pshare->total_assoc_num)
					RTL8188E_SuspendTxReport(priv);
				else
					RTL8188E_AssignTxReportMacId(priv);
	#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
				notify_tx_report_change(priv);
	#endif
			}
#endif
#if defined(CONFIG_POWER_SAVE)
			if (priv->pshare->total_assoc_num == 0 && GET_ROOT(priv)->pmib->dot11OperationEntry.ps_level == 2)
			{
				rtw_lock_suspend_timeout(priv, 2000);
			}
#endif

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			if (0 == priv->assoc_num) {
				rtw_txservq_flush(priv, &priv->tx_mc_queue);
			}
#endif

#if 0
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (IS_ROOT_INTERFACE(priv))
#endif
				if (!priv->assoc_num) {
#ifdef INTERFERENCE_CONTROL
					if (priv->pshare->rf_ft_var.nbi_filter_enable)
						check_NBI_by_rssi(priv, 0xFF);	// force NBI on while no station associated
#else
	#if defined(CONFIG_PCI_HCI)
					check_DIG_by_rssi(priv, 0);	// force DIG off while no station associated
	#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
					notify_check_DIG_by_rssi(priv, 0);
	#endif
#endif
				}
#endif
			if (pstat->ht_cap_len) {
				if (--priv->pshare->ht_sta_num < 0) {
					printk("ht_sta_num error\n");  // this should not happen
				} else {
					if (priv->pshare->iot_mode_enable && !priv->pshare->ht_sta_num
#ifdef RTL_MANUAL_EDCA
							&& (priv->pmib->dot11QosEntry.ManualEDCA == 0)
#endif
							) {
#ifdef USE_OUT_SRC
#ifdef _OUTSRC_COEXIST
							if(IS_OUTSRC_CHIP(priv))
#endif
							IotEdcaSwitch(priv, priv->pshare->iot_mode_enable);
#endif

#if !defined(USE_OUT_SRC) || defined(_OUTSRC_COEXIST)
#ifdef _OUTSRC_COEXIST
							if(!IS_OUTSRC_CHIP(priv))
#endif
							IOT_EDCA_switch(priv, priv->pmib->dot11BssType.net_work_type, priv->pshare->iot_mode_enable);
#endif
					}
#ifdef WIFI_11N_2040_COEXIST
					if (priv->pmib->dot11nConfigEntry.dot11nCoexist && priv->pshare->is_40m_bw &&
						(priv->pmib->dot11BssType.net_work_type & (WIRELESS_11N|WIRELESS_11G))) {
						if (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_40M_INTOLERANT_)) {
							if (OPMODE & WIFI_AP_STATE) {
                                clearSTABitMap(&priv->force_20_sta, pstat->aid);
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
                                update_RAMask_to_FW(priv, 0);
#endif
								SetTxPowerLevel(priv, priv->pmib->dot11RFEntry.dot11channel);
							}
						}
					}
#endif

					check_NAV_prot_len(priv, pstat, 1);
				}
			}
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT) 				
			if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
				UpdateHalMSRRPT8812(priv, pstat, DECREASE);
				pstat->txpdrop_flag =1;					
			}
#endif
#if defined(CONFIG_WLAN_HAL_8881A) || defined(CONFIG_WLAN_HAL_8192EE)
            if ((GET_CHIP_VER(priv) == VERSION_8881A) || (GET_CHIP_VER(priv) == VERSION_8192E)) {
                if(pstat->aid <127)
                {
                    GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat, DECREASE);
                    pstat->bDrop = 1;                      
                }
            }
#endif
#ifdef HW_FILL_MACID
        if (IS_SUPPORT_HW_FILL_MACID(priv))  {
            // Set hwaddr zero
            u1Byte hwaddr = {0x00,0x00,0x00,0x00,0x00,0x00};
            GET_HAL_INTERFACE(priv)->SetTxRPTHandler(priv, REMAP_AID(pstat), TXRPT_VAR_MAC_ADDRESS, &hwaddr);                         
            GET_HAL_INTERFACE(priv)->SetCRC5ToRPTBufferHandler(priv,CRC5(&hwaddr,6), REMAP_AID(pstat),0); 
        }
#endif //#ifdef HW_FILL_MACID                                                  
		} else {
			DEBUG_ERR("Association Number Error (0)!\n");
		}
	}
#if defined(MULTI_STA_REFINE) && defined(TX_SHORTCUT)
	if(((act == INCREASE) && ((priv->pshare->total_assoc_num==11) || (priv->pshare->total_assoc_num==31)))
	|| ((act == DECREASE) && ((priv->pshare->total_assoc_num==9)  || (priv->pshare->total_assoc_num==29)))){
		clearAllTxShortCut(priv);
	}
#endif

#ifdef CONFIG_RTL_92D_SUPPORT
	if (GET_CHIP_VER(priv) == VERSION_8192D){
		for (i=NUM_STAT-1; i>=0 ; i--){
			if (priv->pshare->aidarray[i]!=NULL){
				if (priv->pshare->aidarray[i]->used)
						break;
			}
		}
		priv->pshare->max_fw_macid = priv->pshare->aidarray[i]->station.aid+1; // fw check macid num from 1~32, so we add 1 to index.
		if (priv->pshare->max_fw_macid > (NUM_STAT+1))
			priv->pshare->max_fw_macid = (NUM_STAT+1);
	}
#endif

	DEBUG_INFO("assoc_num%s(%d) in %s %02X%02X%02X%02X%02X%02X\n",
		act?"++":"--",
		priv->assoc_num,
		func,
		pstat->hwaddr[0],
		pstat->hwaddr[1],
		pstat->hwaddr[2],
		pstat->hwaddr[3],
		pstat->hwaddr[4],
		pstat->hwaddr[5]);
}

#ifdef INDICATE_LINK_CHANGE
void indicate_sta_link_change(struct rtl8192cd_priv *priv, struct stat_info *pstat, int act, char *func)
{
	DOT11_LINK_CHANGE_IND LinkChange_Ind;
	
	if ((INCREASE == act) && !pstat->link_up) {
		pstat->link_up = 1;
		// do something about STA addition
	} else if ((DECREASE == act) && pstat->link_up) {
		pstat->link_up = 0;
		// do something about STA removal
	} else {
		return;
	}
	
	memcpy(LinkChange_Ind.MACAddr, pstat->hwaddr, MACADDRLEN);
	LinkChange_Ind.EventId = DOT11_EVENT_LINK_CHANGE_IND;
	LinkChange_Ind.IsMoreEvent = 0;
	LinkChange_Ind.LinkStatus = pstat->link_up;
	DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (UINT8 *)&LinkChange_Ind,
				sizeof(DOT11_LINK_CHANGE_IND));

	DEBUG_INFO("%s%s in %s %02X%02X%02X%02X%02X%02X\n",
		__func__,
		act ? "++" : "--",
		func,
		pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
		pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);
}
#endif // INDICATE_LINK_CHANGE


/*
 * Use this function to get the number of associated station, no matter
 * it is expired or not. And don't count WDS peers in.
 */
int get_assoc_sta_num(struct rtl8192cd_priv *priv, int mode)
{
	struct list_head *phead, *plist;
	struct stat_info *pstat;
	int sta_num;
#if defined(SMP_SYNC)// && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	unsigned long flags = 0;
#endif

	sta_num = 0;
	phead = &priv->asoc_list;
	
	SMP_LOCK_ASOC_LIST(flags);
	
	plist = phead->next;
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;

#ifdef CONFIG_RTK_MESH        
		if(mode == 1) {
			if(isPossibleNeighbor(pstat))
			continue;
		}
#endif

		if ((pstat->state & WIFI_ASOC_STATE)
#ifdef NOT_RTK_BSP
			&& pstat->expire_to
#endif
			)
		{ 
			sta_num++;
		} 
	}
	
	SMP_UNLOCK_ASOC_LIST(flags);
	return sta_num;
}

#if defined(CONFIG_VERIWAVE_CHECK)
//rootPriv should be root and not NULL
void update_wifi_allitf_txrx_stats(struct rtl8192cd_priv *rootPriv)
{
	struct rtl8192cd_priv *priv = rootPriv;
	unsigned int rx_pkt=0, tx_pkt=0;
	unsigned int rx_bytes=0, tx_bytes=0;
#if defined(MBSSID) || defined(UNIVERSAL_REPEATER)
	unsigned char i=1;
#endif

	if (priv == NULL)
		return;

#ifdef INTERFERENCE_CONTROL
	//priv->pshare->fa_all_counter = priv->pshare->ofdm_FA_total_cnt,
#else
	//priv->pshare->fa_all_counter = priv->pshare->ofdm_FA_cnt1+priv->pshare->ofdm_FA_cnt2+priv->pshare->ofdm_FA_cnt3+priv->pshare->ofdm_FA_cnt4,
#endif
	//priv->pshare->fa_all_counter += priv->pshare->cck_FA_cnt;

	rx_pkt = priv->net_stats.rx_packets;
	tx_pkt = priv->net_stats.tx_packets;
	rx_bytes = priv->net_stats.rx_bytes;
	tx_bytes = priv->net_stats.tx_bytes;  

#ifdef MBSSID
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++){
		priv = rootPriv->pvap_priv[i];
		if (IS_DRV_OPEN(priv)) {
			rx_pkt += priv->net_stats.rx_packets;
			tx_pkt += priv->net_stats.tx_packets;
			rx_bytes += priv->net_stats.rx_bytes;
			tx_bytes += priv->net_stats.tx_bytes;
		}
	}
#endif

#ifdef UNIVERSAL_REPEATER
	priv = GET_VXD_PRIV(rootPriv);
	if (IS_DRV_OPEN(priv)){
		rx_pkt += priv->net_stats.rx_packets;
		tx_pkt += priv->net_stats.tx_packets;
		rx_bytes += priv->net_stats.rx_bytes;
		tx_bytes += priv->net_stats.tx_bytes;
	}
#endif

	rootPriv->pshare->wifi_rx_pkts[0] = rootPriv->pshare->wifi_rx_pkts[1];
	rootPriv->pshare->wifi_rx_pkts[1] = rootPriv->pshare->wifi_rx_pkts[2];
	rootPriv->pshare->wifi_tx_pkts[0] = rootPriv->pshare->wifi_tx_pkts[1];
	rootPriv->pshare->wifi_tx_pkts[1] = rootPriv->pshare->wifi_tx_pkts[2];

	rootPriv->pshare->wifi_rx_pkts[2] = rx_pkt;
	rootPriv->pshare->wifi_tx_pkts[2] = tx_pkt;

	rootPriv->pshare->wifi_rx_bytes[0] = rootPriv->pshare->wifi_rx_bytes[1];
	rootPriv->pshare->wifi_rx_bytes[1] = rootPriv->pshare->wifi_rx_bytes[2];
	rootPriv->pshare->wifi_tx_bytes[0] = rootPriv->pshare->wifi_tx_bytes[1];
	rootPriv->pshare->wifi_tx_bytes[1] = rootPriv->pshare->wifi_tx_bytes[2];

	rootPriv->pshare->wifi_rx_bytes[2] = rx_bytes;
	rootPriv->pshare->wifi_tx_bytes[2] = tx_bytes;
}
#endif

void event_indicate(struct rtl8192cd_priv *priv, unsigned char *mac, int event)
{
#ifdef __KERNEL__
#ifdef USE_CHAR_DEV
	if (priv->pshare->chr_priv && priv->pshare->chr_priv->asoc_fasync)
		kill_fasync(&priv->pshare->chr_priv->asoc_fasync, SIGIO, POLL_IN);
#endif
#ifdef USE_PID_NOTIFY
	if (priv->pshare->wlanapp_pid > 0)
#ifdef LINUX_2_6_27_
	{
		kill_pid(_wlanapp_pid, SIGIO, 1);
	}
#else
		kill_proc(priv->pshare->wlanapp_pid, SIGIO, 1);
#endif
#endif
#endif

#ifdef __DRAYTEK_OS__
	if (event == 2)
		cb_disassoc_indicate(priv->dev, mac);
#endif

#ifdef GREEN_HILL
	extern void indicate_to_upper(int reason, unsigned char *addr);
	if (event > 0)
		indicate_to_upper(event, mac);
#endif

#ifdef __ECOS
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_IWCONTROL
    extern cyg_flag_t iw_flag;
    cyg_flag_setbits(&iw_flag, 0x1);

#else
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WPS
	extern cyg_flag_t wsc_flag;
	cyg_flag_setbits(&wsc_flag, 0x1);
#endif
#endif
#endif

#ifdef __OSK__
	extern void rtk_wlan_event(int event, void *);
	rtk_wlan_event(event, priv->dev);
#endif
}

#ifdef WIFI_HAPD
int event_indicate_hapd(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra)
{
	struct net_device	*dev = (struct net_device *)priv->dev;
	union iwreq_data wreq;

	if(OPMODE & WIFI_AP_STATE)
	{

		printk("event_indicate_hapd +++, event =0x%x\n", event);

	memset(&wreq, 0, sizeof(wreq));

	if(event == HAPD_EXIRED)
		{
			memcpy(wreq.addr.sa_data, mac, 6);
			wireless_send_event(dev, IWEVEXPIRED, &wreq, NULL);
			return 0;
		}
	else if(event == HAPD_REGISTERED)
		{
			memcpy(wreq.addr.sa_data, mac, 6);
			wireless_send_event(dev, IWEVREGISTERED, &wreq, NULL);
			return 0;
		}
	else if(event == HAPD_MIC_FAILURE)
		{
			char buf[6];
			memcpy(buf, mac, 6);
			wreq.data.flags = event;
			wreq.data.length = 6;
			wireless_send_event(dev, IWEVCUSTOM, &wreq, buf);
			return 0;
		}
	else if(event == HAPD_WPS_PROBEREQ)
		{
			wreq.data.flags = event;
			wreq.data.length = sizeof(struct _DOT11_PROBE_REQUEST_IND);
			wireless_send_event(dev, IWEVGENIE, &wreq, extra); //IW_CUSTOM_MAX is 256, can NOT afford  _DOT11_PROBE_REQUEST_IND
					return 0;
		}		
	else{
			//Not used yet
			wreq.data.flags = event;
			wireless_send_event(dev, IWEVCUSTOM, &wreq, extra);
			return 0;
		}
	}

	return -1;
}
#endif

#ifdef WIFI_WPAS
int event_indicate_wpas(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra)
{
	struct net_device	*dev = (struct net_device *)priv->dev;
	union iwreq_data wreq;

	if(OPMODE & WIFI_STATION_STATE)
	{
		printk("event_indicate_wpas +++ event = 0x%x\n", event);
		
		memset(&wreq, 0, sizeof(wreq));

		if(event == WPAS_EXIRED)
			{
				memcpy(wreq.addr.sa_data, mac, 6);
				wireless_send_event(dev, IWEVEXPIRED, &wreq, NULL);
				return 0;
			}
		else if(event == WPAS_REGISTERED)
			{
				memcpy(wreq.addr.sa_data, mac, 6);
				wireless_send_event(dev, SIOCGIWAP, &wreq, NULL);
				return 0;
			}
		else if(event == WPAS_MIC_FAILURE)
			{
#ifdef WIFI_WPAS_CLI
				struct iw_michaelmicfailure ev;
				memset(&ev, 0, sizeof(ev));
//				if (hdr->addr1[0] & 0x01)
//					ev.flags |= IW_MICFAILURE_GROUP;
//				else
//					ev.flags |= IW_MICFAILURE_PAIRWISE;
				ev.src_addr.sa_family = ARPHRD_ETHER;
				memcpy(ev.src_addr.sa_data, mac, ETH_ALEN);
				wreq.data.length = sizeof(ev);
				wireless_send_event(dev, IWEVMICHAELMICFAILURE, &wreq, (char *)&ev);
#else
				char buf[6];
				memcpy(buf, mac, 6);
				wreq.data.flags = event;
				wreq.data.length = 6;
				wireless_send_event(dev, IWEVCUSTOM, &wreq, buf);
				return 0;
#endif
			}
		else if(event == WPAS_ASSOC_INFO)
			{
				wreq.data.flags = event;
				wreq.data.length = sizeof(struct _WPAS_ASSOCIATION_INFO);
				wireless_send_event(dev, IWEVCUSTOM, &wreq, extra); //IW_CUSTOM_MAX is 256, can NOT afford  _DOT11_PROBE_REQUEST_IND
				return 0;
			}	
		else if(event == WPAS_SCAN_DONE)
			{
				wireless_send_event(dev, SIOCGIWSCAN, &wreq, NULL);
				return 0;
			}
#ifdef WIFI_WPAS_CLI
		else if (event == WPAS_DISCON) 
			{
				wreq.ap_addr.sa_family = ARPHRD_ETHER;
				memset(wreq.ap_addr.sa_data, 0, ETH_ALEN);
				wireless_send_event(dev, SIOCGIWAP, &wreq, NULL);
				return 0;
			}
#endif		
		else
			{
				//Not used yet
				wreq.data.flags = event;
				wireless_send_event(dev, IWEVCUSTOM, &wreq, extra);
				return 0;
			}
	}

	return -1;
}
#endif


#ifdef CONFIG_RTL_WAPI_SUPPORT
void wapi_event_indicate(struct rtl8192cd_priv *priv)
{
	#ifdef LINUX_2_6_27_
	struct pid *pid;
	#endif

	if (priv->pshare->wlanwapi_pid > 0)
	{
#ifdef LINUX_2_6_27_
		kill_pid(_wlanwapi_pid, SIGIO, 1);
#else
		kill_proc(priv->pshare->wlanwapi_pid, SIGIO, 1);
#endif
	}
}
#endif

#ifdef RSSI_MONITOR_NCR
void rssi_event_indicate(struct rtl8192cd_priv *priv)
{
	if (priv->pshare->wlanrssim_pid > 0)
	{
#ifdef LINUX_2_6_27_
		kill_pid(priv->pshare->_wlanrssim_pid, SIGIO, 1);
#else
		kill_proc(priv->pshare->wlanrssim_pid, SIGIO, 1);
#endif
	}
}

void rssi_monitor(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char rssi)
{
	rssim_msg message;
	message.event = 0;

	if(rssi > priv->pshare->rf_ft_var.rssi_monitor_thd1 && pstat->rssi_report==0) {
		message.event = 1;
		pstat->rssi_report= rssi;
	} else if(rssi < priv->pshare->rf_ft_var.rssi_monitor_thd1-5 && pstat->rssi_report!=0) {
//		event = 2;			// not report leave event
		pstat->rssi_report= 0;
	} else if(pstat->rssi_report){
		if(RTL_ABS(rssi, pstat->rssi_report) > priv->pshare->rf_ft_var.rssi_monitor_thd2) {
			if(pstat->rssim_type !=1) {	// type 1: not report variation
				message.event = 3;
				pstat->rssi_report= rssi;
			}
		}
	}

	if(message.event) {		
		memcpy(message.hwaddr, pstat->hwaddr, MACADDRLEN);
		message.rssi = rssi;
		message.timestamp = jiffies;
		DOT11_EnQueue((unsigned long)priv, priv->rssimEvent_queue, &message, sizeof(message));
		rssi_event_indicate(priv);
	}
}
#endif

#ifdef USE_WEP_DEFAULT_KEY
void init_DefaultKey_Enc(struct rtl8192cd_priv *priv, unsigned char *key, int algorithm)
{
	unsigned char defaultmac[4][6];
	int i;

	memset(defaultmac, 0, sizeof(defaultmac));
	for(i=0; i<4; i++)
		defaultmac[i][5] = i;

	for(i=0; i<4; i++)
	{
		CamDeleteOneEntry(priv, defaultmac[i], i, 1);
		if (key == NULL)
			CamAddOneEntry(priv, defaultmac[i], i,
					(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm)<<2,
					1, priv->pmib->dot11DefaultKeysTable.keytype[i].skey);
		else
			CamAddOneEntry(priv, defaultmac[i], i,
					algorithm<<2,
					1, key);
	}
	priv->pshare->CamEntryOccupied += 4;
}
#endif


#ifdef UNIVERSAL_REPEATER
//
// Disable AP function in virtual interface
//
void disable_vxd_ap(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	if ((priv==NULL) || !(priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE))
		return;

	if (!(priv->drv_state & DRV_STATE_VXD_AP_STARTED))
		return;
	else
		priv->drv_state &= ~DRV_STATE_VXD_AP_STARTED;

	DEBUG_INFO("Disable vxd AP\n");

	if (IS_DRV_OPEN(priv))
		rtl8192cd_close(priv->dev);

	SAVE_INT_AND_CLI(flags);

#ifdef CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
    	GET_HAL_INTERFACE(priv)->DisableVXDAPHandler(priv);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
#if defined(CONFIG_PCI_HCI) || (defined(CONFIG_USB_HCI) && defined(CONFIG_SUPPORT_USB_INT) && defined(CONFIG_INTERRUPT_BASED_TXBCN))
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		priv->pshare->InterruptMask &= ~(HIMR_88E_BcnInt | HIMR_88E_TBDOK | HIMR_88E_TBDER);
		RTL_W32(REG_88E_HIMR, priv->pshare->InterruptMask);
	} else
#endif
	{
		RTL_W32(HIMR, RTL_R32(HIMR) & ~(HIMR_BCNDOK0));
	}
#endif

	//RTL_W16(ATIMWND, 2);
	RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_NOLINK & NETYPE_Mask) << NETYPE_SHIFT));
	}

	RESTORE_INT(flags);
}


//
// Enable AP function in virtual interface
//
void enable_vxd_ap(struct rtl8192cd_priv *priv)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	if ((priv==NULL) || !(priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) ||
		!(priv->drv_state & DRV_STATE_VXD_INIT))
		return;

	if (priv->drv_state & DRV_STATE_VXD_AP_STARTED)
		return;
	else
		priv->drv_state |= DRV_STATE_VXD_AP_STARTED;

	DEBUG_INFO("Enable vxd AP\n");

	priv->pmib->dot11RFEntry.dot11channel = GET_ROOT(priv)->pmib->dot11Bss.channel;
	//priv->pmib->dot11BssType.net_work_type = GET_ROOT_PRIV(priv)->oper_band;
	priv->pmib->dot11BssType.net_work_type = GET_ROOT(priv)->pmib->dot11BssType.net_work_type &
		GET_ROOT(priv)->pmib->dot11Bss.network;

	if (!IS_DRV_OPEN(priv))
		rtl8192cd_open(priv->dev);
	else {
		//priv->oper_band = priv->pmib->dot11BssType.net_work_type;
		validate_oper_rate(priv);
		get_oper_rate(priv);
	}

	memcpy(priv->pmib->dot11StationConfigEntry.dot11Bssid, GET_ROOT(priv)->pmib->dot11OperationEntry.hwaddr, MACADDRLEN);
	memcpy(GET_MY_HWADDR, priv->pmib->dot11StationConfigEntry.dot11Bssid, MACADDRLEN);
	memcpy(priv->pmib->dot11Bss.bssid, priv->pmib->dot11StationConfigEntry.dot11Bssid, MACADDRLEN);

	SAVE_INT_AND_CLI(flags);
	priv->ht_cap_len = 0;
	init_beacon(priv);

#if defined(CONFIG_PCI_HCI) || (defined(CONFIG_USB_HCI) && defined(CONFIG_SUPPORT_USB_INT) && defined(CONFIG_INTERRUPT_BASED_TXBCN))
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
		priv->pshare->InterruptMask |= HIMR_88E_BcnInt | HIMR_88E_TBDOK | HIMR_88E_TBDER;
		RTL_W32(REG_88E_HIMR, priv->pshare->InterruptMask);
	} else
#endif
	{
		RTL_W32(HIMR, RTL_R32(HIMR) | HIMR_BCNDOK0);
	}
#endif

	//RTL_W16(ATIMWND, 0x0030);
	RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_AP & NETYPE_Mask) << NETYPE_SHIFT));
#ifdef CONFIG_RTL_92C_SUPPORT
	if (!IS_TEST_CHIP(priv))
#endif
	{
		RTL_W8(0x422, RTL_R8(0x422) | BIT(6));
		RTL_W8(BCN_CTRL, 0); 
		RTL_W8(0x553, 1); 
		RTL_W8(BCN_CTRL, DIS_TSF_UPDATE_N| EN_BCN_FUNCTION | DIS_SUB_STATE_N | EN_TXBCN_RPT);
	}

	RESTORE_INT(flags);
}
#endif // UNIVERSAL_REPEATER

#ifdef RTK_STA_BWC
void rtl8192cd_sta_bwc_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	
	struct stat_info *pstat;
	int i;	

	// clear bandwidth control counter for every client	
	for(i=0; i<NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE)){
			pstat = &(priv->pshare->aidarray[i]->station);
			pstat->sta_bwctx_cnt = 0;
		}
	}

	mod_timer(&priv->sta_bwc_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.sta_bwc_to));
}
#endif

#ifdef GBWC
void rtl8192cd_GBWC_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct sk_buff *pskb;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (priv->pmib->gbwcEntry.GBWCMode == GBWC_MODE_DISABLE)
		return;

	priv->GBWC_consuming_Q = 1;

	// clear bandwidth control counter
	priv->GBWC_tx_count = 0;
	priv->GBWC_rx_count = 0;

	// consume Tx queue
	while(1)
	{
		pskb = (struct sk_buff *)deque(priv, &(priv->GBWC_tx_queue.head), &(priv->GBWC_tx_queue.tail),
			(unsigned long)(priv->GBWC_tx_queue.pSkb), NUM_TXPKT_QUEUE);

		if (pskb == NULL)
			break;

#ifdef ENABLE_RTL_SKB_STATS
		rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
#endif

		if (rtl8192cd_start_xmit_noM2U(pskb, pskb->dev))
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
	}

	// consume Rx queue
	while(1)
	{
		pskb = (struct sk_buff *)deque(priv, &(priv->GBWC_rx_queue.head), &(priv->GBWC_rx_queue.tail),
			(unsigned long)(priv->GBWC_rx_queue.pSkb), NUM_TXPKT_QUEUE);

		if (pskb == NULL)
			break;

		rtl_netif_rx(priv, pskb, (struct stat_info *)*(unsigned int *)&(pskb->cb[4]));
	}

	priv->GBWC_consuming_Q = 0;

	mod_timer(&priv->GBWC_timer, jiffies + GBWC_TO);
}
#endif

#ifdef SBWC
void rtl8192cd_SBWC_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct sk_buff *pskb;
	SBWC_MODE mode;
	struct stat_info	*pstat;
	struct list_head	*phead, *plist;
#ifndef SMP_SYNC    
	unsigned long	flags=0;
#endif

	SAVE_INT_AND_CLI(flags);
	SMP_LOCK(flags);

	phead = &priv->asoc_list;
	plist = phead;

	while ((plist = asoc_list_get_next(priv, plist)) != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);

		mode = SBWC_MODE_DISABLE;

		if (pstat->SBWC_tx_limit)
			mode |= SBWC_MODE_LIMIT_STA_TX;

		if (pstat->SBWC_rx_limit)
			mode |= SBWC_MODE_LIMIT_STA_RX;
		
		if (mode == SBWC_MODE_DISABLE)
			continue;

		pstat->SBWC_consuming_q = 1;

		// clear bandwidth control counter
		pstat->SBWC_tx_count = 0;
		pstat->SBWC_rx_count = 0;

		// consume Tx queue
		while(1)
		{
			pskb = (struct sk_buff *)deque(priv, &(pstat->SBWC_txq.head), &(pstat->SBWC_txq.tail),
				(unsigned long)(pstat->SBWC_txq.pSkb), NUM_TXPKT_QUEUE);

			if (!pskb)
				break;

			#ifdef ENABLE_RTL_SKB_STATS
			rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
			#endif

			if (rtl8192cd_start_xmit_noM2U(pskb, pskb->dev))
				rtl_kfree_skb(priv, pskb, _SKB_TX_);
		}

		// consume Rx queue
		while(1)
		{
			pskb = (struct sk_buff *)deque(priv, &(pstat->SBWC_rxq.head), &(pstat->SBWC_rxq.tail),
				(unsigned long)(pstat->SBWC_rxq.pSkb), NUM_TXPKT_QUEUE);

			if (!pskb)
				break;

			rtl_netif_rx(priv, pskb, (struct stat_info *)*(unsigned int *)&(pskb->cb[4]));
		}

		pstat->SBWC_consuming_q = 0;
		pstat->SBWC_mode = mode;
	}

	SMP_UNLOCK(flags);
	RESTORE_INT(flags);

	mod_timer(&priv->SBWC_timer, jiffies + SBWC_TO);
}
#endif

unsigned char fw_was_full(struct rtl8192cd_priv *priv)
{
	struct list_head *phead;
	struct list_head *plist;
	struct stat_info *pstat;
#if defined(SMP_SYNC) //&& (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	unsigned long flags = 0;
#endif
	unsigned char is_full;

	phead = &priv->asoc_list;
	if(list_empty(phead))
		return 0;

	is_full = 0;

	SMP_LOCK_ASOC_LIST(flags);

	plist = phead->next;
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;
        if(	pstat->sta_in_firmware != 1) 
		{
			is_full = 1;
			goto exit;
		}
	}

exit:
	SMP_UNLOCK_ASOC_LIST(flags);
	return is_full;
}


int realloc_RATid(struct rtl8192cd_priv *priv)
{
	struct list_head *phead;
	struct list_head *plist;
	struct stat_info *pstat =NULL, *pstat_chosen = NULL;
	unsigned int max_through_put = 0;
	unsigned int have_chosen = 0;
#if defined(SMP_SYNC)// && (defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI))
	unsigned long flags = 0;
#endif

	phead = &priv->asoc_list;
	if(list_empty(phead))
		return 0;

	SMP_LOCK_ASOC_LIST(flags);
	
	plist = phead->next;
	while (plist != phead) {
		int temp_through_put ;
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;

		if (pstat->sta_in_firmware == 1)// STA has rate adaptive
			continue;

		if (pstat->expire_to == 0) // exclude expired STA
			continue;

		temp_through_put =  pstat->tx_avarage + pstat->rx_avarage;

		if (temp_through_put >= max_through_put){
			pstat_chosen = pstat;
			max_through_put = temp_through_put;
			have_chosen = 1;
		}
	}

	SMP_UNLOCK_ASOC_LIST(flags);
	
	if (have_chosen == 0)
		return 0;


#ifdef  CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)){
		GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat_chosen, 3);
	} else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv)== VERSION_8723B){
		UpdateHalRAMask8812(priv, pstat_chosen, 3);
	} else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
	if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
		add_RATid(priv, pstat);
#endif
	} else
#endif
	{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
		if(CHIP_VER_92X_SERIES(priv))
			add_update_RATid(priv, pstat_chosen);
#endif
	}

	return 1;
}


void update_remapAid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	int array_idx;

    if(pstat->sta_in_firmware == 1)
        return; /*already exists, just return*/

    if(priv->pshare->fw_free_space) {
        /*find an empty slot*/
    	for (array_idx = 0; array_idx < NUM_STAT; array_idx++) {
    		if (priv->pshare->remapped_aidarray[array_idx] == 0)
    			break;
    	}

    	if (array_idx == NUM_STAT) {
            /*WARNING:  THIS SHOULD NOT HAPPEN*/
            printk("add AID fail!!\n");
            BUG();
    		return;
        }
        
    
        pstat->remapped_aid = array_idx + 1;
        priv->pshare->remapped_aidarray[array_idx] = pstat->aid; 
        pstat->sta_in_firmware = 1; // this value will updated in expire_timer
        priv->pshare->fw_free_space --;
    }
    else { /* free space run out, share the same aid*/
        pstat->remapped_aid = priv->pshare->fw_support_sta_num;
        pstat->sta_in_firmware = 0;
    }
}


void release_remapAid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
    int i;
    if (pstat->sta_in_firmware == 1)
    {
        for(i = 0; i < NUM_STAT; i++) {
            if(priv->pshare->remapped_aidarray[i] == pstat->aid){
                priv->pshare->remapped_aidarray[i] = 0;                
                break;
            }
        }        
        priv->pshare->fw_free_space ++;
        pstat->remapped_aid = 0;
        pstat->sta_in_firmware = -1;
        DEBUG_INFO("Remove id %d from ratr\n", pstat->aid);        
    }
}


unsigned int is_h2c_buf_occupy(struct rtl8192cd_priv *priv)
{
	 unsigned int occupied = 0;

	if (
#ifdef CONFIG_RTL_92C_SUPPORT
		(IS_TEST_CHIP(priv) && RTL_R8(0x1c0+priv->pshare->fw_q_fifo_count)) ||
#endif
		(RTL_R8(0x1cc) & BIT(priv->pshare->fw_q_fifo_count)))
		occupied++;

	return occupied;
}

#if defined(UNIVERSAL_REPEATER)
int under_apmode_repeater(struct rtl8192cd_priv *priv)
{
	int ret = 0;

	if(IS_ROOT_INTERFACE(priv))
	{
		if(IS_DRV_OPEN(GET_VXD_PRIV(priv))) 
		{
			if((priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) &&
				(GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE))
					ret = 1;
		}
	}
	else if(IS_VXD_INTERFACE(priv))
	{
		if(IS_DRV_OPEN(priv)) 
		{
			if((GET_ROOT(priv)->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) &&
				(priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE))
					ret = 1;
		}
	}

	return ret;
}
#endif

short signin_h2c_cmd(struct rtl8192cd_priv *priv, unsigned int content, unsigned short ext_content)
{
	int c=0;

#ifdef MP_TEST
	if (priv->pshare->rf_ft_var.mp_specific)
		goto SigninFAIL;
#endif

	/*
	 *	Check if h2c cmd signin buffer is occupied, 
	 *	for Power Saving related functions only
	 */
	//if ((content & 0x7f) < H2C_CMD_RSSI) {
		while (is_h2c_buf_occupy(priv)) {
		delay_us(10);
		if(++c ==30)
			goto SigninFAIL;
	}
	//}

	/*
		 * signin reg in order to fit hw requirement
		 */
		if(content & BIT(7))
			RTL_W16(0x88+(priv->pshare->fw_q_fifo_count*2), ext_content);

	RTL_W32(HMEBOX_0+(priv->pshare->fw_q_fifo_count*4), content);

	//printk("(smcc) sign in h2c %x\n", HMEBOX_0+(priv->pshare->fw_q_fifo_count*4));

#if defined(TESTCHIP_SUPPORT) && defined(CONFIG_RTL_92C_SUPPORT)
	/*
	 * set own bit
	 */
	if(IS_TEST_CHIP(priv))
	RTL_W8(0x1c0+priv->pshare->fw_q_fifo_count, 1);
#endif

	/*
	 * rollover ring buffer count
	 */
	if (++priv->pshare->fw_q_fifo_count > 3)
		priv->pshare->fw_q_fifo_count = 0;

	return 0;
	
SigninFAIL:
	return 1;
}


void set_ps_cmd(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	unsigned int content = 0;

	if(! CHIP_VER_92X_SERIES(priv))
		return;

	SAVE_INT_AND_CLI(flags);

	/*
	 * set ps state
	 */
	 if (pstat->state & WIFI_SLEEP_STATE)
	 	content |= BIT(24);

	/*
	 * set macid
	 */
	content |= REMAP_AID(pstat) << 8;

	/*
	 * set cmd id
	 */
	 content |= H2C_CMD_PS;

	signin_h2c_cmd(priv, content, 0);

	RESTORE_INT(flags);
}

#ifdef TX_SHORTCUT
__inline__ void clearTxShortCutBufSize(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	int i, j;
	for (i = 0; i < NR_NET80211_UP; ++i) {
		for (j = 0; j < TX_SC_ENTRY_NUM; ++j)
			pstat->tx_sc_ent[i][j].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
	}
#else
	int i;
    for (i=0; i<TX_SC_ENTRY_NUM; i++)
    {
#ifdef CONFIG_WLAN_HAL
        if (IS_HAL_CHIP(priv)) {
            GET_HAL_INTERFACE(priv)->SetShortCutTxBuffSizeHandler(priv, pstat->tx_sc_ent[i].hal_hw_desc, 0);
        } else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif // CONFIG_WLAN_HAL
        {//not HAL
	        pstat->tx_sc_ent[i].hwdesc1.Dword7 &= set_desc(~TX_TxBufSizeMask);
        }
    }
#endif
}

void clearAllTxShortCut(struct rtl8192cd_priv *priv)
{
	int i;
	for(i= 0; i<NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] && priv->pshare->aidarray[i]->used == TRUE) {
			clearTxShortCutBufSize(priv, &(priv->pshare->aidarray[i]->station));
		}
	}
}
#endif

#if defined(MULTI_STA_REFINE)


void TxPktBuf_AgingTimer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;
	if( priv->pshare->paused_sta_num >8) {
	{
		unsigned int Q_aid[8], Q_pkt[8], minIdx=0;
		 int i, pq=0; 
		 struct stat_info *pstat2;
		 
#if defined (CONFIG_WLAN_HAL_8814AE) || defined (CONFIG_WLAN_HAL_8822BE) || defined (CONFIG_WLAN_HAL_8197F)
			if (GET_CHIP_VER(priv) == VERSION_8814A || GET_CHIP_VER(priv) == VERSION_8822B || GET_CHIP_VER(priv) == VERSION_8197F) {
				for(i=0; i<4; i++) {
                    Q_pkt[i] = (RTL_R16(0x1400+i*2) & 0xFFF);     
                    Q_aid[i] = (RTL_R8(0x400+i*4+3)>>1) & 0x7f;       //31:25     7b
                    Q_pkt[i+4] = (RTL_R16(0x1408+i*2) & 0xFFF);
                    Q_aid[i+4] = (RTL_R8(0x468+i*4+3)>>1) & 0x7f; //31:25     7b	
                }		
			}
			else
#endif			
		{
		  for(i=0; i<4; i++) {
			unsigned int tmp1 = RTL_R32(0x400+i*4);
			unsigned int tmp2 = RTL_R32(0x468+i*4); 	
			Q_pkt[i] = ((tmp1>>8) & 0x7f); // 14:8	   7b
			Q_aid[i] = ((tmp1>>25) & 0x7f); //31:25 	7b
			Q_pkt[i+4] = ((tmp2>>8) & 0x7f); // 14:8	   7b
			Q_aid[i+4] = ((tmp2>>25) & 0x7f); //31:25	   7b							
			}	
		}
		 for(i=0; i<8; i++) {
			pstat2 = get_macIDinfo(priv, Q_aid[i]);
			if( pstat2 && pstat2->txpause_flag && Q_pkt[i]) {
				pq++;
				if( Q_pkt[i] <	Q_pkt[minIdx])
					minIdx = i;
			} 
		 }
		if( pq > MAXPAUSEDQUEUE) {
			pstat2 = get_macIDinfo(priv, Q_aid[minIdx]);
#if 0
			if(pq ==8) {
				struct stat_info *pstat3;
				unsigned minIdx2= (minIdx+1)&7;
				priv->pshare->lock_counter++;
//					panic_printk("%s, PAUSE All Queue !!! \n", __FUNCTION__ );
				for(i=0; i<8; i++) {
				   if( Q_pkt[i] <  Q_pkt[minIdx2] && (i!=minIdx))
					   minIdx2 = i; 
				}
				pstat3 = get_macIDinfo(priv, Q_aid[minIdx2]);
				if(pstat3) {					
#ifdef CONFIG_WLAN_HAL	
					if (IS_HAL_CHIP(priv)) {
						GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat3, DECREASE);
					} else 
#endif
					{
#ifdef CONFIG_RTL_8812_SUPPORT						
						UpdateHalMSRRPT8812(priv, pstat3, DECREASE);			
#endif
					}
					pstat3->txpdrop_flag = 1;
#ifdef CONFIG_WLAN_HAL						
					if (IS_HAL_CHIP(priv)) {
						GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat3));	
					} else
#endif
					{
#ifdef CONFIG_RTL_8812_SUPPORT						
						RTL8812_MACID_PAUSE(priv, 0, REMAP_AID(pstat3));
#endif
					}						
					pstat3->txpause_flag = 0;
//						panic_printk("%s, Free %d pkts of MAC ID:%d\n", __FUNCTION__, Q_pkt[minIdx2], Q_aid[minIdx2]);	
					if(priv->pshare->paused_sta_num)
						priv->pshare->paused_sta_num--;
					priv->pshare->unlock_counter2++;
					pstat3->dropPktCurr += Q_pkt[minIdx2];		
				}
			}
#endif	
			if(pstat2) {
#ifdef CONFIG_WLAN_HAL					
				if (IS_HAL_CHIP(priv)) {
					GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstat2, DECREASE);
				} else
#endif
				{
#ifdef CONFIG_RTL_8812_SUPPORT				
					UpdateHalMSRRPT8812(priv, pstat2, DECREASE);	
#endif
				}
				pstat2->txpdrop_flag = 1;
//					panic_printk("%s, Free %d pkts of MAC ID:%d\n", __FUNCTION__, Q_pkt[minIdx], Q_aid[minIdx]);
#ifdef CONFIG_WLAN_HAL	
				if (IS_HAL_CHIP(priv)) {
					GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstat2));	
				} else 
#endif				
				{
#ifdef CONFIG_RTL_8812_SUPPORT				
					RTL8812_MACID_PAUSE(priv, 0, REMAP_AID(pstat2));
#endif
				}
				pstat2->txpause_flag = 0;
				if(priv->pshare->paused_sta_num)
					priv->pshare->paused_sta_num--;
				priv->pshare->unlock_counter2++;
				pstat2->dropPktCurr += Q_pkt[minIdx];				
			}						
		}
	}

	// process aging
	{
		int idx = 0;
		struct stat_info* pstatd = findNextSTA(priv, &idx);
		while(pstatd) {
			if(pstatd && pstatd->txpause_flag && 
			   (TSF_DIFF(jiffies, pstatd->txpause_time) > RTL_MILISECONDS_TO_JIFFIES(PKTAGINGTIME))) 
			{
#ifdef CONFIG_WLAN_HAL			
				if (IS_HAL_CHIP(priv)) {
					GET_HAL_INTERFACE(priv)->UpdateHalMSRRPTHandler(priv, pstatd, DECREASE);
				} else
#endif					
				{
#ifdef CONFIG_RTL_8812_SUPPORT				
					UpdateHalMSRRPT8812(priv, pstatd, DECREASE);	
#endif
				}

				pstatd->txpdrop_flag = 1;							
#ifdef CONFIG_WLAN_HAL			
				if (IS_HAL_CHIP(priv)) {				
					GET_HAL_INTERFACE(priv)->SetMACIDSleepHandler(priv, 0, REMAP_AID(pstatd));	
				} else 
#endif
				{
#ifdef CONFIG_RTL_8812_SUPPORT				
					RTL8812_MACID_PAUSE(priv, 0, REMAP_AID(pstatd));
#endif				
				}
				pstatd->txpause_flag = 0;
				if(priv->pshare->paused_sta_num)
					priv->pshare->paused_sta_num--;
				pstatd->drop_expire ++; 
			}
			pstatd = findNextSTA(priv, &idx);
		};
	}
	}
	if(priv->pshare->rf_ft_var.msta_refine&1 ) 
		mod_timer(&priv->pshare->PktAging_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(10));	
}
#endif

#ifdef CONFIG_PCI_HCI
void add_ps_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct stat_info *pstat = NULL;
	unsigned int set_timer = 0;

#ifndef SMP_SYNC
    unsigned long flags = 0;
#endif
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->add_ps_timer))
		del_timer_sync(&priv->add_ps_timer);

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
			return;
#endif
#ifdef PCIE_POWER_SAVING_TEST //yllin
    if((priv->pwr_state >= L2) || (priv->pwr_state == L1)) {
            return;
    }
#endif


#ifdef CONFIG_32K //tingchu
            if(priv->offload_32k_flag==1) {
                printk("32k add_ps_timer return\n");
                    return;
            }
#endif


	if (!list_empty(&priv->addps_list)) {
		pstat = list_entry(priv->addps_list.next, struct stat_info, addps_list);
		if (!pstat)
	return ;

		if (!is_h2c_buf_occupy(priv)) {
			set_ps_cmd(priv, pstat);
			if (!list_empty(&pstat->addps_list)) {
				SAVE_INT_AND_CLI(flags);
				SMP_LOCK(flags);
				list_del_init(&pstat->addps_list);
				RESTORE_INT(flags);
				SMP_UNLOCK(flags);
			}

			if (!list_empty(&priv->addps_list))
				set_timer++;
		} else {
			set_timer++;
		}
	}

	if (set_timer)
		mod_timer(&priv->add_ps_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(50));	// 50 ms
}


#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
void add_update_ps(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
//#ifdef CONFIG_RTL_8812_SUPPORT
	if(! CHIP_VER_92X_SERIES(priv))
		return;
//#endif		
	if (is_h2c_buf_occupy(priv)) {
		if (list_empty(&pstat->addps_list)) {
			SAVE_INT_AND_CLI(flags);
			list_add_tail(&(pstat->addps_list), &(priv->addps_list));
			RESTORE_INT(flags);

			if (!timer_pending(&priv->add_ps_timer))
				mod_timer(&priv->add_ps_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(50));	// 50 ms
		}
	} else {
		set_ps_cmd(priv, pstat);
	}
}
#endif
#endif // CONFIG_PCI_HCI


#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
void update_intel_sta_bitmap(struct rtl8192cd_priv *priv, struct stat_info *pstat, int release)
{
	if (pstat->IOTPeer != HT_IOT_PEER_INTEL)
		return;
	
	if (release) {
		clearSTABitMap(&priv->pshare->intel_sta_bitmap, pstat->aid);
	} else {	// join
		setSTABitMap(&priv->pshare->intel_sta_bitmap, pstat->aid);
	}
}
#endif

#if defined(WIFI_11N_2040_COEXIST_EXT)

void update_40m_staMap(struct rtl8192cd_priv *priv, struct stat_info *pstat, int release)
{
	if(pstat) {
		if(release) {
			clearSTABitMap(&priv->pshare->_40m_staMap, pstat->aid);
			clearSTABitMap(&priv->pshare->_80m_staMap, pstat->aid);			
		}
#if defined(RTK_AC_SUPPORT)
		else if (pstat->vht_cap_len) {
			setSTABitMap(&priv->pshare->_80m_staMap, pstat->aid);
			setSTABitMap(&priv->pshare->_40m_staMap, pstat->aid);
		}
#endif
		else if((pstat->tx_bw == HT_CHANNEL_WIDTH_20)) {
			clearSTABitMap(&priv->pshare->_40m_staMap, pstat->aid);
			clearSTABitMap(&priv->pshare->_80m_staMap, pstat->aid);	
		} else if(pstat->tx_bw == HT_CHANNEL_WIDTH_20_40){
			setSTABitMap(&priv->pshare->_40m_staMap, pstat->aid);
			clearSTABitMap(&priv->pshare->_80m_staMap, pstat->aid);	
		} 

	}
}

void checkBandwidth(struct rtl8192cd_priv *priv)
{
    unsigned int _40m_stamap = orSTABitMap(&priv->pshare->_40m_staMap);
    unsigned int _80m_stamap = orSTABitMap(&priv->pshare->_80m_staMap);
		
    int FA_counter = priv->pshare->FA_total_cnt;

    if(!priv->pshare->rf_ft_var.bws_enable)
    return;

#ifdef DFS
    if(is_DFS_channel(priv->pmib->dot11RFEntry.dot11channel)
        #ifdef MCR_WIRELESS_EXTEND
        && !priv->pshare->cmw_link
        #endif
    )
    return;
#endif		

#ifdef MP_TEST
    if ( (OPMODE & WIFI_MP_STATE)|| priv->pshare->rf_ft_var.mp_specific)
        return ;
#endif

#ifdef WDS
    if (priv->pmib->dot11WdsInfo.wdsEnabled)
        return;
#endif

#ifdef CONFIG_RTK_MESH
    if(priv->pmib->dot1180211sInfo.mesh_enable)
        return;
#endif

    if (!(OPMODE & WIFI_AP_STATE))
        return;

#ifdef UNIVERSAL_REPEATER
    if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
        return;
#endif		

    if (timer_pending(&priv->ss_timer) || priv->ss_req_ongoing)
        return;

#if	defined(USE_OUT_SRC)
    if(IS_OUTSRC_CHIP(priv))
        FA_counter = ODMPTR->FalseAlmCnt.Cnt_all;
#endif

    if (priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
    {	    
        if (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_80) {            
            unsigned char do_switch = 0;        
            switch (priv->pshare->CurrentChannelBW) {
                case HT_CHANNEL_WIDTH_80:
                    if (_80m_stamap == 0 && _40m_stamap == 0) {
                        priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
                        #ifdef MCR_WIRELESS_EXTEND
                        if (priv->pshare->cmw_link)
                            RTL_W8(0x82d, 0x91);
                        #endif
                        do_switch = 1;
                    }
                    else if (_80m_stamap == 0 && _40m_stamap != 0) {
                        priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20_40;
                        do_switch = 1;
                    }
                    break;

                case HT_CHANNEL_WIDTH_20_40:
                    if (_80m_stamap == 0 && _40m_stamap == 0) {
                        priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
                        do_switch = 1;
                    }
                    else if (_80m_stamap != 0) {
                        priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_80;
                        do_switch = 1;
                    }
                    break;

                case HT_CHANNEL_WIDTH_20:
                    if (_80m_stamap == 0 && _40m_stamap != 0) {
                        priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20_40;
                        do_switch = 1;
                    }
                    else if (_80m_stamap != 0) {
                        priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_80;
                        do_switch = 1;
                    }
                    break;

                default:
                    panic_printk("No such bandwidth mode (%d)\n", priv->pshare->CurrentChannelBW);
                    break;
            }

            if (do_switch)
            {
                SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
                SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
#ifdef dybw_rx
				if(priv->pshare->rf_ft_var.bws_enable&0x2 && priv->assoc_num==1) {
					priv->ht_cap_len = 0;
					construct_ht_ie(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
				}
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
				update_RAMask_to_FW(priv,1);
#endif
            }
            return;  
        }        
    }

	//check assoc num
	int assoc_num = GET_ROOT(priv)->assoc_num, i;
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable){
		for (i=0; i<RTL8192CD_NUM_VWLAN; ++i)
		assoc_num += GET_ROOT(priv)->pvap_priv[i]-> assoc_num;
	}
#endif


    if ((priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40)  && (!_40m_stamap) ) {
        priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;		
        SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
        SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
        SetTxPowerLevel(priv, priv->pmib->dot11RFEntry.dot11channel);
    } 
    if( priv->pmib->dot11nConfigEntry.dot11nCoexist) {
#if defined(CONFIG_PCI_HCI)
        if((FA_counter> priv->pshare->rf_ft_var.bws_Thd)&&((RTL_R8(0xc50) & 0x7f) >= 0x32))
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
        u1Byte IGI = RTL_R8(0xc50) & 0x7f;
        if (((FA_counter> priv->pshare->rf_ft_var.bws_Thd) && (IGI >= 0x32)) || (IGI >= 0x42))
#endif
		{
            if(priv->pshare->is_40m_bw != HT_CHANNEL_WIDTH_20) {
                priv->pshare->is_40m_bw = HT_CHANNEL_WIDTH_20;
                priv->ht_cap_len = 0;	// reconstruct ie
                if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
                    construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);				
            }
        } 
#if 1//BW40 back when no sta		
        else if(assoc_num==0 && (FA_counter < priv->pshare->rf_ft_var.bws_CleanThd)) {
            if(priv->pshare->is_40m_bw != priv->pmib->dot11nConfigEntry.dot11nUse40M) {
                priv->pshare->is_40m_bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;
                priv->ht_cap_len = 0;				// reconstruct ie
                if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
                    construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);
            }
        }		
#endif		
    }		
    
    if( _40m_stamap && (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20)  &&
        ((priv->pmib->dot11nConfigEntry.dot11nUse40M != HT_CHANNEL_WIDTH_20))) {
        if(priv->pmib->dot11nConfigEntry.dot11nCoexist) {
            priv->pshare->is_40m_bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;
            priv->ht_cap_len = 0;				// reconstruct ie
            if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
                construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);				
        }
        priv->pshare->CurrentChannelBW = priv->pmib->dot11nConfigEntry.dot11nUse40M;
        SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
        SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
	}

    if((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) && (priv->pshare->rf_ft_var.bws_enable&0x2)) {
		if(priv->pshare->ctrl40m) {
			priv->ht_cap_len = 0;	// reconstruct ie
			construct_ht_ie(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)			
			update_RAMask_to_FW(priv,1);
#endif

			priv->pshare->ctrl40m=0;
		}
	}
}
#endif


#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
#ifdef CONFIG_PCI_HCI
void add_RATid_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct stat_info *pstat = NULL;
	unsigned int set_timer = 0;
	unsigned long flags = 0;

	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if (timer_pending(&priv->add_RATid_timer))
		del_timer_sync(&priv->add_RATid_timer);

#ifdef PCIE_POWER_SAVING
	if ((priv->pwr_state == L2) || (priv->pwr_state == L1))
			return;
#endif

#ifdef CONFIG_32K //tingchu
            if (priv->offload_32k_flag==1){
                printk("32k add_RATid_timer return\n");
                
                    return;
                }
#endif


	if (!list_empty(&priv->addRAtid_list)) {
		pstat = list_entry(priv->addRAtid_list.next, struct stat_info, addRAtid_list);
		if (!pstat)
			return;

		if (!is_h2c_buf_occupy(priv)) {
			add_RATid(priv, pstat);
			if (!list_empty(&pstat->addRAtid_list)) {
				SAVE_INT_AND_CLI(flags);
				SMP_LOCK(flags);
				list_del_init(&pstat->addRAtid_list);
				RESTORE_INT(flags);
				SMP_UNLOCK(flags);
			}

			if (!list_empty(&priv->addRAtid_list))
				set_timer++;
		} else {
			set_timer++;
		}
	}

	if (set_timer)
		mod_timer(&priv->add_RATid_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(50));	// 50 ms
}


void add_update_RATid(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	if (is_h2c_buf_occupy(priv)) {
		if (list_empty(&pstat->addRAtid_list)) {
			SAVE_INT_AND_CLI(flags);
			list_add_tail(&(pstat->addRAtid_list), &(priv->addRAtid_list));
			RESTORE_INT(flags);

			if (!timer_pending(&priv->add_RATid_timer))
				mod_timer(&priv->add_RATid_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(50));	// 50 ms
		}
	} else {
			add_RATid(priv, pstat);
	}
}
#endif // CONFIG_PCI_HCI
#endif

#ifdef SDIO_AP_OFFLOAD
void set_ap_ps_mode(struct rtl8192cd_priv *priv, unsigned char *data)
{
	int mode;

	if (strlen(data) == 0)
		return;

	mode = _atoi(data, 16);

	if (mode == 0x00) {
		ap_offload_activate(priv, OFFLOAD_PROHIBIT_USER);
	} else {
		ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_USER);
	}
}

void ap_offload_deactivate(struct rtl8192cd_priv *priv, int reason)
{
	_irqL irqL;
	
	if (IS_DRV_OPEN(GET_ROOT(priv))) {
		if (RTW_STS_SUSPEND == priv->pshare->pwr_state)
			ap_offload_exit(GET_ROOT(priv));
	}
	
	_enter_critical(&priv->pshare->offload_lock, &irqL);

#ifdef CONFIG_POWER_SAVE
	if (!priv->pshare->offload_prohibited) {
		del_timer(&priv->pshare->ps_timer);
		priv->pshare->ps_timer_expires = 0;
		rtw_lock_suspend(priv);
	}
#endif
	priv->pshare->offload_prohibited |= reason;
	
	_exit_critical(&priv->pshare->offload_lock, &irqL);
}

void ap_offload_activate(struct rtl8192cd_priv *priv, int reason)
{
	_irqL irqL;
	
	_enter_critical(&priv->pshare->offload_lock, &irqL);
	
	priv->pshare->offload_prohibited &= ~reason;
	
	_exit_critical(&priv->pshare->offload_lock, &irqL);
	
	if (IS_DRV_OPEN(GET_ROOT(priv))) {
#ifdef CONFIG_POWER_SAVE
		rtw_lock_suspend_timeout(priv, 2000);
#else
		mod_timer(&priv->pshare->ps_timer, jiffies + POWER_DOWN_T0);
#endif
	}
}
#endif // SDIO_AP_OFFLOAD

void send_h2c_cmd_detect_wps_gpio(struct rtl8192cd_priv *priv, unsigned int gpio_num, unsigned int enable, unsigned int high_active)
{
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	unsigned int content = 0;

	SAVE_INT_AND_CLI(flags);
	
	content = gpio_num << 16;
	
	/*
	 * enable firmware to detect wps gpio
	 */
	if (enable)
		content |= BIT(8);
	
	/*
	 * rising edge trigger
	 */
	if (high_active)
		content |= BIT(9);

	/*
	 * set cmd id
	 */
	content |= H2C_CMD_AP_WPS_CTRL;

	signin_h2c_cmd(priv, content, 0);
	printk("signin ap_wps_ctrl h2c: 0x%08X\n", content);
	
	RESTORE_INT(flags);
}

#ifdef RTK_QUE
void rtk_queue_init(struct ring_que *que)
{
	memset(que, '\0', sizeof(struct ring_que));
	que->qmax = MAX_PRE_ALLOC_SKB_NUM;
}

static int rtk_queue_tail(struct rtl8192cd_priv *priv, struct ring_que *que, struct sk_buff *skb)
{
	int next;
	unsigned long x;

	SAVE_INT_AND_CLI(x);
	SMP_LOCK_SKB(x);

	if (que->head == que->qmax)
		next = 0;
	else
		next = que->head + 1;

	if (que->qlen >= que->qmax || next == que->tail) {
		printk("%s: ring-queue full!\n", __FUNCTION__);
		RESTORE_INT(x);
		SMP_UNLOCK_SKB(x);
		return 0;
	}

	que->ring[que->head] = skb;
	que->head = next;
	que->qlen++;

	RESTORE_INT(x);
	SMP_UNLOCK_SKB(x);
	return 1;
}


__IRAM_IN_865X
static struct sk_buff *rtk_dequeue(struct rtl8192cd_priv *priv, struct ring_que *que)
{
	struct sk_buff *skb;
	unsigned long x;

	SAVE_INT_AND_CLI(x);
	SMP_LOCK_SKB(x);

	if (que->qlen <= 0 || que->tail == que->head) {
		RESTORE_INT(x);
		SMP_UNLOCK_SKB(x);
		return NULL;
	}

	skb = que->ring[que->tail];

	if (que->tail == que->qmax)
		que->tail  = 0;
	else
		que->tail++;

	que->qlen--;

	RESTORE_INT(x);
	SMP_UNLOCK_SKB(x);
	return (struct sk_buff *)skb;
}


void free_rtk_queue(struct rtl8192cd_priv *priv, struct ring_que *skb_que)
{
	struct sk_buff *skb;

	while (skb_que->qlen > 0) {
		skb = rtk_dequeue(priv, skb_que);
		if (skb == NULL)
			break;
		dev_kfree_skb_any(skb);
	}
}
#endif // RTK_QUE

#ifdef DELAY_REFILL_RX_BUF
#ifdef CONFIG_WLAN_HAL
    extern int refill_rx_ring_88XX(struct rtl8192cd_priv * priv, struct sk_buff * skb, unsigned char * data, unsigned int q_num, PHCI_RX_DMA_QUEUE_STRUCT_88XX cur_q);
#endif
extern int refill_rx_ring(struct rtl8192cd_priv *priv, struct sk_buff *skb, unsigned char *data);
#endif

#ifdef __ECOS
#ifdef DELAY_REFILL_RX_BUF
int __MIPS16 rtk_wifi_delay_refill(struct sk_buff  *pskb)
{
	struct rtl8192cd_priv *priv;	
	struct rtl8192cd_hw *phw;
	struct sk_buff *skb=(struct sk_buff *)pskb;
	int ret=0;
	if(!(pskb && pskb->priv))
		return ret;
	
	priv=(struct rtl8192cd_priv *)pskb->priv;
 	phw=GET_HW(priv);

#ifdef CONFIG_WLAN_HAL
	unsigned int					q_num;
	PHCI_RX_DMA_MANAGER_88XX		prx_dma;
	PHCI_RX_DMA_QUEUE_STRUCT_88XX	cur_q;
#endif // CONFIG_WLAN_HAL

#ifdef CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        if (!(priv->drv_state & DRV_STATE_OPEN)){
            /* return 0 means can't refill (because interface be closed or not opened yet) to rx ring but relesae to skb_poll*/            
            ret=0;
        }else{        
    	    q_num   = 0;
    	    prx_dma = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PRxDMA88XX);
    	    cur_q   = &(prx_dma->rx_queue[q_num]);
    	    ret = refill_rx_ring_88XX(priv, NULL, NULL, q_num, cur_q);
    	    GET_HAL_INTERFACE(priv)->UpdateRXBDHostIdxHandler(priv, q_num, cur_q->rxbd_ok_cnt);
    	    cur_q->rxbd_ok_cnt = 0;
        }
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{
		if (phw->cur_rx_refill != phw->cur_rx) {
		   	ret=refill_rx_ring(priv, NULL, NULL); 
		}
	}
	return ret;
}
#endif
#endif

#if !(defined(__ECOS) && defined(CONFIG_SDIO_HCI))
void refill_skb_queue(struct rtl8192cd_priv *priv)
{
	struct sk_buff *skb;
#ifdef DELAY_REFILL_RX_BUF
 	struct rtl8192cd_hw *phw=GET_HW(priv);

#ifdef CONFIG_WLAN_HAL
    unsigned int                    q_num;
    PHCI_RX_DMA_MANAGER_88XX        prx_dma;
    PHCI_RX_DMA_QUEUE_STRUCT_88XX   cur_q;

    if (IS_HAL_CHIP(priv)) {
        q_num   = 0;
        prx_dma = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PRxDMA88XX);
        cur_q   = &(prx_dma->rx_queue[q_num]);
    }
#endif // CONFIG_WLAN_HAL
#endif

#ifdef NOT_RTK_BSP
	while (skb_queue_len(&priv->pshare->skb_queue) < MAX_PRE_ALLOC_SKB_NUM) 
#else
	while (priv->pshare->skb_queue.qlen < MAX_PRE_ALLOC_SKB_NUM) 
#endif
	{

	#ifdef CONFIG_RTL8190_PRIV_SKB
			skb = dev_alloc_skb_priv(priv, RX_BUF_LEN);
	#else
			skb = dev_alloc_skb(RX_BUF_LEN);
	#endif

		if (skb == NULL) {
//			DEBUG_ERR("dev_alloc_skb() failed!\n");
			return;
		}
#ifdef DELAY_REFILL_RX_BUF
#ifdef CONFIG_WLAN_HAL
		if (IS_HAL_CHIP(priv)) {
	        if (cur_q->cur_host_idx != ((cur_q->host_idx + cur_q->rxbd_ok_cnt)%cur_q->total_rxbd_num)) {
	            refill_rx_ring_88XX(priv, skb, NULL, q_num, cur_q);
				continue;
		  	}
		} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif // CONFIG_WLAN_HAL
		{//not HAL
			if (phw->cur_rx_refill != phw->cur_rx) {
				refill_rx_ring(priv, skb, NULL); 
				continue;
			}
		}
#endif

#ifdef RTK_QUE
		rtk_queue_tail(priv, &priv->pshare->skb_queue, skb);
#else
#ifdef __ECOS
		skb_queue_tail(&priv->pshare->skb_queue, skb);
#else
		__skb_queue_tail(&priv->pshare->skb_queue, skb);
#endif
#endif
	}
#ifdef DELAY_REFILL_RX_BUF
#ifdef CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        GET_HAL_INTERFACE(priv)->UpdateRXBDHostIdxHandler(priv, q_num, cur_q->rxbd_ok_cnt);
        cur_q->rxbd_ok_cnt = 0;
	}
#endif // CONFIG_WLAN_HAL
#endif 
}

#if (!defined(__OSK__)) || (defined(__OSK__) && !defined(CONFIG_RTL6028))
__MIPS16
#endif
__IRAM_IN_865X
struct sk_buff *alloc_skb_from_queue(struct rtl8192cd_priv *priv)
{
	struct sk_buff *skb=NULL;

#ifdef NOT_RTK_BSP
	if (skb_queue_len(&priv->pshare->skb_queue) < 2)
#else
	if (priv->pshare->skb_queue.qlen == 0) 
#endif
	{
//		struct sk_buff *skb;
#ifdef CONFIG_RTL8190_PRIV_SKB
#ifdef CONCURRENT_MODE
		skb = dev_alloc_skb_priv(priv, RX_BUF_LEN);
#else
		skb = dev_alloc_skb_priv(priv, RX_BUF_LEN);
#endif
#else
		skb = dev_alloc_skb(RX_BUF_LEN);
#endif
		if (skb == NULL) {
			DEBUG_ERR("dev_alloc_skb() failed!\n");
		}

		return skb;
	}
#ifdef RTK_QUE
	skb = rtk_dequeue(priv, &priv->pshare->skb_queue);
#else
#ifdef __ECOS
	skb = skb_dequeue(&priv->pshare->skb_queue);
#else
	skb = __skb_dequeue(&priv->pshare->skb_queue);
#endif
#endif
	if (skb == NULL) {
		DEBUG_ERR("skb_dequeue() failed!\n");
	}

	return skb;
}


void free_skb_queue(struct rtl8192cd_priv *priv, struct sk_buff_head	*skb_que)
{
	struct sk_buff *skb;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	while (skb_que->qlen > 0) {
// 2009.09.08
		SAVE_INT_AND_CLI(flags);
		skb = (struct sk_buff *)__skb_dequeue(skb_que);
		RESTORE_INT(flags);
		if (skb == NULL)
			break;
		dev_kfree_skb_any(skb);
	}
}
#endif // !(__ECOS && CONFIG_SDIO_HCI)

#ifdef __OSK__
// moved from 8192cd_util.h jim 20100611
__IRAM_WIFI_PRI1
struct list_head *dequeue_frame(struct rtl8192cd_priv *priv, struct list_head *head)
{
#ifndef SMP_SYNC
	unsigned long flags=0;
#endif
	struct list_head *pnext;

	SAVE_INT_AND_CLI(flags);
	if (list_empty(head)) {
		RESTORE_INT(flags);
		return (void *)NULL;
	}

	pnext = head->next;
	list_del_init(pnext);

	RESTORE_INT(flags);

	return pnext;
}

__IRAM_WIFI_PRI2 struct sk_buff *
rtl_dev_alloc_skb(struct rtl8192cd_priv *priv,
				unsigned int length, int flag, int could_alloc_from_kerenl)
{
	struct sk_buff *skb = NULL;
#ifdef CONFIG_POOL_DEBUG
	RECORDRA;
#endif

#if defined(__ECOS) && defined(CONFIG_SDIO_HCI)
	skb = dev_alloc_skb(length);
#else
	skb = alloc_skb_from_queue(priv);

	if (skb == NULL && could_alloc_from_kerenl)
		skb = dev_alloc_skb(length);
#endif

#ifdef ENABLE_RTL_SKB_STATS
	if (NULL != skb) {
		if (flag & (_SKB_TX_ | _SKB_TX_IRQ_))
			rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
		else
			rtl_atomic_inc(&priv->rtl_rx_skb_cnt);
	}
#endif

	return skb;
}

__IRAM_WIFI_PRI2 void rtl_kfree_skb(struct rtl8192cd_priv *priv, struct sk_buff *skb, int flag)
{
#ifdef CONFIG_POOL_DEBUG
	RECORDRA;
	SETCCBDEBUGINFO(skb2ccb(skb),0,0,__FUNCTION__,__LINE__);
#endif

#ifdef ENABLE_RTL_SKB_STATS
	if (flag & (_SKB_TX_ | _SKB_TX_IRQ_))
		rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
	else
		rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif

	dev_kfree_skb_any(skb);
}

#ifdef CONFIG_ENABLE_NCBUFF
__IRAM_WIFI_PRI2 void rtl_kfree_skb_spec(struct rtl8192cd_priv *priv, struct sk_buff *skb, int flag)
{
#ifdef CONFIG_POOL_DEBUG
	RECORDRA;
	SETCCBDEBUGINFO(skb2ccb(skb),0,0,__FUNCTION__,__LINE__);
#endif

#ifdef ENABLE_RTL_SKB_STATS
	if (flag & (_SKB_TX_ | _SKB_TX_IRQ_))
		rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
	else
		rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif

	//dev_kfree_skb_any(skb);
	dev_kfree_skb_to_ncbuff(skb);
}
#endif

__IRAM_WIFI_PRI3 void rtl_cache_sync_wback(struct rtl8192cd_priv *priv, unsigned int start,
				unsigned int size, int direction)
{
	if (0 == size)
		return;	// if the size of cache sync is equal to zero, don't do sync action

	dma_cache_wback_inv((unsigned long)bus_to_virt(start-CONFIG_LUNA_SLAVE_PHYMEM_OFFSET), size);
}
#endif//#ifdef __OSK_

#ifdef FAST_RECOVERY
struct backup_info {
	struct aid_obj *sta[NUM_STAT];
	struct Dot11KeyMappingsEntry gkey;
#ifdef WDS
	struct wds_info	wds;
#endif
};

void *backup_sta(struct rtl8192cd_priv *priv)
{
	int i;
	struct backup_info *pBackup;

	pBackup = (struct backup_info *)kmalloc((sizeof(struct backup_info)), GFP_ATOMIC);
	if (pBackup == NULL) {
		printk("%s: kmalloc() failed!\n", __FUNCTION__);
		return NULL;
	}
	memset(pBackup, '\0', sizeof(struct backup_info));
	for (i=0; i<NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] && priv->pshare->aidarray[i]->used) {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (priv !=  priv->pshare->aidarray[i]->priv)
				continue;
#endif
			pBackup->sta[i] = (struct aid_obj *)kmalloc((sizeof(struct aid_obj)), GFP_ATOMIC);
			if (pBackup->sta[i] == NULL) {
				printk("%s: kmalloc(sta) failed!\n", __FUNCTION__);
				for (--i; i>=0; --i) {
					if (pBackup->sta[i]) {
#ifdef CONFIG_RTL_WAPI_SUPPORT
						free_sta_wapiInfo(priv, &pBackup->sta[i]->station);
#endif
						kfree(pBackup->sta[i]);
					}
				}
				kfree(pBackup);
				return NULL;
			}
			memcpy(pBackup->sta[i], priv->pshare->aidarray[i], sizeof(struct aid_obj));
#ifdef CONFIG_RTL_WAPI_SUPPORT
			// prevent backup station.wapiInfo from being freed during recovery preiod
			priv->pshare->aidarray[i]->station.wapiInfo = NULL;
#endif
		}
	}

#ifdef WDS
	memcpy(&pBackup->wds, &priv->pmib->dot11WdsInfo, sizeof(struct wds_info));
#endif
	memcpy(&pBackup->gkey, &priv->pmib->dot11GroupKeysTable, sizeof(struct Dot11KeyMappingsEntry));

	return (void *)pBackup;
}


void restore_backup_sta(struct rtl8192cd_priv *priv, void *pInfo)
{
	unsigned int i, offset;
	struct stat_info *pstat;
	unsigned char	key_combo[32];
	struct backup_info *pBackup=(struct backup_info *)pInfo;
#ifdef CONFIG_RTK_MESH
	unsigned char	is_11s_MP = FALSE;
	unsigned long flags;
#endif
	int retVal;

	for (i=0; i<NUM_STAT; i++) {
		if (pBackup->sta[i]) {

#ifdef CONFIG_RTK_MESH	// Restore Establish MP ONLY
			if ((1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable) && !isSTA2(pBackup->sta[i]->station)) {
				UINT8 State = pBackup->sta[i]->station.mesh_neighbor_TBL.State;

				if ((State == MP_SUPERORDINATE_LINK_UP) || (State == MP_SUBORDINATE_LINK_UP)
						|| (State == MP_SUPERORDINATE_LINK_DOWN) || (State == MP_SUBORDINATE_LINK_DOWN_E))
					is_11s_MP = TRUE;
				else	// is MP, but not establish, Give up.
				{
					kfree(pBackup->sta[i]);
					continue;
				}
			}
#endif

			pstat = alloc_stainfo(priv, pBackup->sta[i]->station.hwaddr, i);
			if (!pstat) {
				printk("%s: alloc_stainfo() failed!\n", __FUNCTION__);
				for (; i<NUM_STAT; i++) {
					if (pBackup->sta[i]) {
#ifdef CONFIG_RTL_WAPI_SUPPORT
						free_sta_wapiInfo(priv, &pBackup->sta[i]->station);
#endif
						kfree(pBackup->sta[i]);
					}
				}
				kfree(pBackup);
				return;
			}

#ifdef CONFIG_RTL_WAPI_SUPPORT
			// free new allocated wapiInfo before restore backup wapiInfo
			if (pstat->wapiInfo) free_sta_wapiInfo(priv, pstat);
#endif
			offset = (unsigned long)(&((struct stat_info *)0)->aid);
			memcpy(((unsigned char *)pstat)+offset,
				((unsigned char *)&pBackup->sta[i]->station)+offset, sizeof(struct stat_info)-offset);
			asoc_list_add(priv, pstat);

			if (pstat->sta_in_firmware == 1) {
				priv->pshare->remapped_aidarray[pstat->remapped_aid-1] = pstat->aid;
				priv->pshare->fw_free_space--;
			}

#ifdef CONFIG_RTK_MESH
			if (TRUE == is_11s_MP) {
				is_11s_MP = FALSE;
				SMP_LOCK_MESH_MP_HDR(flags);
				list_add_tail(&pstat->mesh_mp_ptr, &priv->mesh_mp_hdr);
				SMP_UNLOCK_MESH_MP_HDR(flags);
				mesh_cnt_ASSOC_PeerLink_CAP(priv, pstat, INCREASE);
			}
#endif

#ifdef WDS
			if (!(pstat->state & WIFI_WDS))
#endif
			if (pstat->expire_to > 0) 
				cnt_assoc_num(priv, pstat, INCREASE, (char *)__FUNCTION__);

			if ((pstat->expire_to > 0) 
#ifdef WDS
				|| (pstat->state & WIFI_WDS)
#endif
			) {
			check_sta_characteristic(priv, pstat, INCREASE);
			if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)
				construct_ht_ie(priv, priv->pshare->is_40m_bw, priv->pshare->offset_2nd_chan);

#ifndef USE_WEP_DEFAULT_KEY
			set_keymapping_wep(priv, pstat);
#endif
			if (!SWCRYPTO && pstat->dot11KeyMapping.keyInCam == TRUE) {
#ifdef CONFIG_RTL_HW_WAPI_SUPPORT
				if (pstat->wapiInfo && (pstat->wapiInfo->wapiType != wapiDisable)) {
					wapiStaInfo *wapiInfo = pstat->wapiInfo;
					
					retVal = CamAddOneEntry(priv, 
							pstat->hwaddr, 
							wapiInfo->wapiUCastKeyId,	/* keyid */ 
							DOT11_ENC_WAPI<<2,	/* type */
							0,	/* use default key */
							wapiInfo->wapiUCastKey[wapiInfo->wapiUCastKeyId].dataKey);
					if (retVal) {
						priv->pshare->CamEntryOccupied++;
						
						retVal = CamAddOneEntry(priv, 
								pstat->hwaddr, 
								wapiInfo->wapiUCastKeyId,	/* keyid */
								DOT11_ENC_WAPI<<2,	/* type */
								1,	/* use default key */
								wapiInfo->wapiUCastKey[wapiInfo->wapiUCastKeyId].micKey);
						if (retVal) {
							//pstat->dot11KeyMapping.keyInCam = TRUE;
							priv->pshare->CamEntryOccupied++;
						} else {
							retVal = CamDeleteOneEntry(priv, pstat->hwaddr, wapiInfo->wapiUCastKeyId, 0);
							if (retVal) {
								priv->pshare->CamEntryOccupied--;
								pstat->dot11KeyMapping.keyInCam = FALSE;
							}
						}
					} else {
						pstat->dot11KeyMapping.keyInCam = FALSE;
					}
				} else
#endif // CONFIG_RTL_HW_WAPI_SUPPORT
				if (pstat->dot11KeyMapping.dot11Privacy) {
				memcpy(key_combo,
					pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKey.skey,
					pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen);
				memcpy(&key_combo[pstat->dot11KeyMapping.dot11EncryptKey.dot11TTKeyLen],
					pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKey1.skey,
					pstat->dot11KeyMapping.dot11EncryptKey.dot11TMicKeyLen);
#ifdef USE_WEP_4_KEYS
#ifdef MULTI_MAC_CLONE
				if ((OPMODE & WIFI_STATION_STATE) && priv->pmib->ethBrExtInfo.macclone_enable) 
					retVal = CamAddOneEntry(priv, pstat->sa_addr, pstat->dot11KeyMapping.keyid,
						pstat->dot11KeyMapping.dot11Privacy<<2, 0, key_combo);
				else
#endif
				if((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) ||
					(priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)) 
				{
					if(priv->pshare->total_cam_entry - priv->pshare->CamEntryOccupied >=4) {
						int keyid=0;
						for(;keyid<4; keyid++) {
							if (CamDeleteOneEntry(priv, pstat->hwaddr, keyid, 0)) {
								priv->pshare->CamEntryOccupied--;
							}					
							if (CamAddOneEntry(priv, pstat->hwaddr, keyid,
								pstat->dot11KeyMapping.dot11Privacy<<2, 0, 
								priv->pmib->dot11DefaultKeysTable.keytype[keyid].skey)) { 					
								priv->pshare->CamEntryOccupied++;
								retVal ++;
							}						
						}				
					} 
					if( retVal ==4) {
						pstat->dot11KeyMapping.keyInCam = TRUE;
					} else {
						int keyid=0;
						for(;keyid<4; keyid++) {
							if (CamDeleteOneEntry(priv, pstat->hwaddr, keyid, 0)) 
								priv->pshare->CamEntryOccupied--;					
						}
						pstat->dot11KeyMapping.keyInCam = FALSE;
					}		
				}else {
					retVal = CamAddOneEntry(priv, pstat->hwaddr, pstat->dot11KeyMapping.keyid,
							pstat->dot11KeyMapping.dot11Privacy<<2, 0, key_combo);										
					if (retVal)
						priv->pshare->CamEntryOccupied++;
					else
						pstat->dot11KeyMapping.keyInCam = FALSE;
				}

#else
#ifdef MULTI_MAC_CLONE
				if ((OPMODE & WIFI_STATION_STATE) && priv->pmib->ethBrExtInfo.macclone_enable) 
					retVal = CamAddOneEntry(priv, pstat->sa_addr, pstat->dot11KeyMapping.keyid,
						pstat->dot11KeyMapping.dot11Privacy<<2, 0, key_combo);
				else
#endif
					retVal = CamAddOneEntry(priv, pstat->hwaddr, pstat->dot11KeyMapping.keyid,
						pstat->dot11KeyMapping.dot11Privacy<<2, 0, key_combo);
					
					if (retVal)
						priv->pshare->CamEntryOccupied++;
					else
						pstat->dot11KeyMapping.keyInCam = FALSE;
#endif					
				}
			}
			}
			// to avoid add RAtid fail
#ifdef CONFIG_WLAN_HAL
			if (IS_HAL_CHIP(priv)) {
				GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
			} else
#endif
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
			if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv)== VERSION_8723B) {
				UpdateHalRAMask8812(priv, pstat, 3);
			} else
#endif
#ifdef CONFIG_RTL_88E_SUPPORT
			if (GET_CHIP_VER(priv)==VERSION_8188E) {
#ifdef TXREPORT
				add_RATid(priv, pstat);
#endif
			} else
#endif
			{
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_92C_SUPPORT)
				add_update_RATid(priv, pstat);
#endif
			}
			kfree(pBackup->sta[i]);

			if (priv->pshare->is_40m_bw && (pstat->IOTPeer == HT_IOT_PEER_MARVELL))

			{
			    setSTABitMap(&priv->pshare->marvellMapBit, pstat->aid);

#if defined(CONFIG_RTL_8812_SUPPORT)||defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
				if((GET_CHIP_VER(priv)== VERSION_8812E)||(IS_HAL_CHIP(priv)) || (GET_CHIP_VER(priv)== VERSION_8723B)){
				}
                else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
				{//not HAL
				if (priv->pshare->Reg_RRSR_2 == 0 && priv->pshare->Reg_81b == 0){
					priv->pshare->Reg_RRSR_2 = RTL_R8(RRSR+2);
					priv->pshare->Reg_81b = RTL_R8(0x81b);
					RTL_W8(RRSR+2, priv->pshare->Reg_RRSR_2 | 0x60);
					RTL_W8(0x81b, priv->pshare->Reg_81b | 0x0E); 
				} 
			}
			}
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
			update_intel_sta_bitmap(priv, pstat, 0);
#endif
#if defined(WIFI_11N_2040_COEXIST_EXT)
			update_40m_staMap(priv, pstat, 0);
#endif
		}
	}

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
	update_RAMask_to_FW(priv, 1);
#endif
	SetTxPowerLevel(priv, priv->pmib->dot11RFEntry.dot11channel);
	memcpy(&priv->pmib->dot11GroupKeysTable, &pBackup->gkey, sizeof(struct Dot11KeyMappingsEntry));
		if (!SWCRYPTO && priv->pmib->dot11GroupKeysTable.keyInCam) {
#ifdef CONFIG_RTL_HW_WAPI_SUPPORT
		if (priv->pmib->wapiInfo.wapiType != wapiDisable)
		{
			const uint8	CAM_CONST_BCAST[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
			retVal = CamAddOneEntry(priv, 
								CAM_CONST_BCAST, 
								priv->wapiMCastKeyId<<1,		/* keyid */ 
								DOT11_ENC_WAPI<<2, 	/* type */
								0,						/* use default key */
								priv->wapiMCastKey[priv->wapiMCastKeyId].dataKey);
			if (retVal) {
				retVal = CamAddOneEntry(priv, 
								CAM_CONST_BCAST, 
								(priv->wapiMCastKeyId<<1)+1,		/* keyid */ 
								DOT11_ENC_WAPI<<2, 	/* type */
								1,						/* use default key */
								priv->wapiMCastKey[priv->wapiMCastKeyId].micKey);
				if (retVal) {
					priv->pshare->CamEntryOccupied++;
//					priv->pmib->dot11GroupKeysTable.keyInCam = TRUE;
				} else {
					retVal = CamDeleteOneEntry(priv, CAM_CONST_BCAST, 1, 0);
					if (retVal)
						priv->pmib->dot11GroupKeysTable.keyInCam = FALSE;
			}
			} else {
				priv->pmib->dot11GroupKeysTable.keyInCam = FALSE;
		}
		} else
#endif // CONFIG_RTL_HW_WAPI_SUPPORT
		{
			memcpy(key_combo,
				priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey,
				priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen);

			memcpy(&key_combo[priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen],
				priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKey1.skey,
				priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TMicKeyLen);

			retVal = CamAddOneEntry(priv, (unsigned char *)"\xff\xff\xff\xff\xff\xff", priv->pmib->dot11GroupKeysTable.keyid,
				priv->pmib->dot11GroupKeysTable.dot11Privacy<<2, 0, key_combo);
			
			if (retVal)
				priv->pshare->CamEntryOccupied++;
			else
				priv->pmib->dot11GroupKeysTable.keyInCam = FALSE;
		}
	}

#ifdef WDS
	memcpy(&priv->pmib->dot11WdsInfo, &pBackup->wds, sizeof(struct wds_info));
#endif
	kfree(pInfo);
}
#endif // FAST_RECOVERY

#ifdef CONFIG_RTL8190_PRIV_SKB
	#if defined(CONFIG_RTL8196B_GW_8M) || defined(CONFIG_RTL8196C_AP_ROOT) || defined(CONFIG_RTL8196C_CLIENT_ONLY) || defined(CONFIG_RTL_8198_AP_ROOT) || defined(__ECOS)
#ifdef __LINUX_2_6__
		#define SKB_BUF_SIZE	(MIN_RX_BUF_LEN+sizeof(struct skb_shared_info)+128+128)
		#define SKB_BUF_SIZE_1	(MIN_RX_BUF_LEN_1+sizeof(struct skb_shared_info)+128+128)
#else
		#define SKB_BUF_SIZE	(MIN_RX_BUF_LEN+sizeof(struct skb_shared_info)+128)
		#define SKB_BUF_SIZE_1	(MIN_RX_BUF_LEN_1+sizeof(struct skb_shared_info)+128)
#endif
	#else
#ifdef __LINUX_2_6__
		#define SKB_BUF_SIZE	(MIN_RX_BUF_LEN+sizeof(struct skb_shared_info)+128+128)
		#define SKB_BUF_SIZE_1	(MIN_RX_BUF_LEN_1+sizeof(struct skb_shared_info)+128+128)
#else
		#define SKB_BUF_SIZE	(MIN_RX_BUF_LEN+sizeof(struct skb_shared_info)+128)
		#define SKB_BUF_SIZE_1	(MIN_RX_BUF_LEN_1+sizeof(struct skb_shared_info)+128)
#endif
	#endif

#define MAGIC_CODE		"8190"

struct priv_skb_buf {
	unsigned char magic[4];
	unsigned int buf_pointer;
#ifdef CONCURRENT_MODE
	struct rtl8192cd_priv *root_priv;
#endif
	struct list_head	list;
	unsigned char buf[SKB_BUF_SIZE];
};

#ifdef CONCURRENT_MODE
struct priv_skb_buf_1 {
	unsigned char magic[4];
	unsigned int buf_pointer;
	struct rtl8192cd_priv *root_priv;
	struct list_head	list;
	unsigned char buf[SKB_BUF_SIZE_1];
};
#endif


#ifdef DUALBAND_ONLY
	#define REAL_MAX_SKB	(MAX_SKB_NUM/2)
	#define REAL_MAX_SKB_1	(MAX_SKB_NUM_1/2)
#else
	#define REAL_MAX_SKB	(MAX_SKB_NUM)
	#define REAL_MAX_SKB_1	(MAX_SKB_NUM_1)
#endif

#ifdef CONCURRENT_MODE
static struct priv_skb_buf skb_buf[REAL_MAX_SKB];
static struct priv_skb_buf_1 skb_buf_1[REAL_MAX_SKB_1];
static struct list_head skbbuf_list[NUM_WLAN_IFACE];
#ifdef CONFIG_WIRELESS_LAN_MODULE
#ifdef CONFIG_BAND_2G_ON_WLAN0
static int skb_free_num[NUM_WLAN_IFACE] = {REAL_MAX_SKB_1, REAL_MAX_SKB};
#else
static int skb_free_num[NUM_WLAN_IFACE] = {REAL_MAX_SKB, REAL_MAX_SKB_1};
#endif
#else
#ifdef CONFIG_BAND_2G_ON_WLAN0
int skb_free_num[NUM_WLAN_IFACE] = {REAL_MAX_SKB_1, REAL_MAX_SKB};
#else
int skb_free_num[NUM_WLAN_IFACE] = {REAL_MAX_SKB, REAL_MAX_SKB_1};
#endif
#endif

#else
static struct priv_skb_buf skb_buf[REAL_MAX_SKB];
static struct list_head skbbuf_list;
static struct rtl8192cd_priv *root_priv;
#ifdef CONFIG_WIRELESS_LAN_MODULE
static int skb_free_num = REAL_MAX_SKB;
#else
int skb_free_num = REAL_MAX_SKB;
#endif
#endif


extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);


void init_priv_skb_buf(struct rtl8192cd_priv *priv)
{
    int i, skb_num, skb_size;
#ifdef CONCURRENT_MODE
    int idx;
	struct priv_skb_buf *pskb_buf;
	struct priv_skb_buf_1 *pskb_buf_1;
#endif

#ifdef CONCURRENT_MODE
    idx = priv->pshare->wlandev_idx;

	if(idx == (0^WLANIDX)){ 
		skb_num = REAL_MAX_SKB;	/* 5G */
		pskb_buf = skb_buf;
		skb_size = SKB_BUF_SIZE;
		/* panic_printk("wlan0 skb_num=%d, skb_size=%d, \n",skb_num, skb_size); */
		memset(pskb_buf, '\0', sizeof(struct priv_skb_buf)*skb_num);

	    INIT_LIST_HEAD(&skbbuf_list[idx]);

	    for (i=0; i < skb_num ; i++)  {
	        memcpy(pskb_buf[i].magic, MAGIC_CODE, 4);
	        pskb_buf[i].root_priv = priv;
	        pskb_buf[i].buf_pointer = (unsigned int)&pskb_buf[i];
	        INIT_LIST_HEAD(&pskb_buf[i].list);
	        list_add_tail(&pskb_buf[i].list, &skbbuf_list[idx]);
	    }
	}else{
		skb_num = REAL_MAX_SKB_1;  /* 2G */
		pskb_buf_1 = skb_buf_1;
		skb_size = SKB_BUF_SIZE_1;
		/* panic_printk("wlan1 skb_num=%d, skb_size=%d, \n",skb_num, skb_size); */
	 	memset(pskb_buf_1, '\0', sizeof(struct priv_skb_buf_1)*skb_num);

	    INIT_LIST_HEAD(&skbbuf_list[idx]);

	    for (i=0; i < skb_num ; i++)  {
	        memcpy(pskb_buf_1[i].magic, MAGIC_CODE, 4);
	        pskb_buf_1[i].root_priv = priv;
	        pskb_buf_1[i].buf_pointer = (unsigned int)&pskb_buf_1[i];
	        INIT_LIST_HEAD(&pskb_buf_1[i].list);
	        list_add_tail(&pskb_buf_1[i].list, &skbbuf_list[idx]);
	    }
	}

   
#else

	skb_num = REAL_MAX_SKB;
	skb_size = SKB_BUF_SIZE;

    memset(skb_buf, '\0', sizeof(struct priv_skb_buf)*skb_num);

    INIT_LIST_HEAD(&skbbuf_list);

    for (i=0; i<skb_num; i++)  {
        memcpy(skb_buf[i].magic, MAGIC_CODE, 4);
        skb_buf[i].buf_pointer = (unsigned int)&skb_buf[i];				
        INIT_LIST_HEAD(&skb_buf[i].list);
        list_add_tail(&skb_buf[i].list, &skbbuf_list);
    }
    root_priv = priv;
#endif
	panic_printk("\n\n#######################################################\n");
    panic_printk("SKB_BUF_SIZE=%d MAX_SKB_NUM=%d\n",skb_size, skb_num);
    panic_printk("#######################################################\n\n");

}


#ifdef CONCURRENT_MODE
static __inline__ unsigned char *get_priv_skb_buf(struct rtl8192cd_priv *priv)
{
	int i;
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	unsigned char *data;

	SAVE_INT_AND_CLI(flags);

	i = priv->pshare->wlandev_idx;
	data = get_buf_from_poll(priv, &skbbuf_list[i], (unsigned int *)&skb_free_num[i]);

	RESTORE_INT(flags);		
	return data;
}

#else

static __inline__ unsigned char *get_priv_skb_buf(struct rtl8192cd_priv *priv)
{
	unsigned char *ret;
#ifndef SMP_SYNC
	unsigned long flags;
#endif

	SAVE_INT_AND_CLI(flags);

	ret = get_buf_from_poll(root_priv, &skbbuf_list, (unsigned int *)&skb_free_num);

	RESTORE_INT(flags);	
	return ret;
}

#endif

#if defined(DUALBAND_ONLY) && defined(CONFIG_RTL8190_PRIV_SKB)
extern u32 if_priv[];
void merge_pool(struct rtl8192cd_priv *priv)
{
	unsigned char *buf;
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf *)0)->buf);
	struct priv_skb_buf *priv_buf;	
	int next_idx;
	int idx = priv->pshare->wlandev_idx;

	if (idx == 0)
		next_idx = 1;
	else
		next_idx = 0;

	while (1) {
		if (skb_free_num[idx] >= REAL_MAX_SKB*2)
			break;

		buf = get_priv_skb_buf((struct rtl8192cd_priv *)if_priv[next_idx]);
		if (buf == NULL)
			break;

		priv_buf = (struct priv_skb_buf *)(((unsigned long)buf) - offset);
		priv_buf->root_priv = priv;
		release_buf_to_poll(priv, buf, &skbbuf_list[idx], (unsigned int *)&skb_free_num[idx]);
	}
}

void split_pool(struct rtl8192cd_priv *priv)
{
	unsigned char *buf;	
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf *)0)->buf);
	struct priv_skb_buf *priv_buf;	
	
	int next_idx;
	int idx = priv->pshare->wlandev_idx;

	if (idx == 0)
		next_idx = 1;
	else
		next_idx = 0;

	while (1) {		
		if (skb_free_num[idx] <= REAL_MAX_SKB)
			break;
		
		buf = get_priv_skb_buf(priv);
		if (buf == NULL)
			break;

		priv_buf = (struct priv_skb_buf *)(((unsigned long)buf) - offset);
		priv_buf->root_priv = (struct rtl8192cd_priv *)if_priv[next_idx];
		release_buf_to_poll((struct rtl8192cd_priv *)if_priv[next_idx], 
			buf, 	&skbbuf_list[next_idx], (unsigned int *)&skb_free_num[next_idx]);
	}
}
#endif

__IRAM_IN_865X
#ifdef CONCURRENT_MODE
static struct sk_buff *dev_alloc_skb_priv(struct rtl8192cd_priv *priv, unsigned int size)
{
	struct sk_buff *skb;

	unsigned char *data = get_priv_skb_buf(priv);
	if (data == NULL) {
//		_DEBUG_ERR("wlan: priv skb buffer empty!\n");
		return NULL;
	}

	skb = dev_alloc_8190_skb(data, size);
	if (skb == NULL) {
		free_rtl8190_priv_buf(data);
		return NULL;
	}
	return skb;
}
#else
static struct sk_buff *dev_alloc_skb_priv(struct rtl8192cd_priv *priv, unsigned int size)
{
	struct sk_buff *skb;

	unsigned char *data = get_priv_skb_buf(priv);
	if (data == NULL) {
//		_DEBUG_ERR("wlan: priv skb buffer empty!\n");
		return NULL;
	}

	skb = dev_alloc_8190_skb(data, size);
	if (skb == NULL) {
		free_rtl8190_priv_buf(data);
		return NULL;
	}
	return skb;
}
#endif

int is_rtl8190_priv_buf(unsigned char *head)
{
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf *)0)->buf);
	struct priv_skb_buf *priv_buf = (struct priv_skb_buf *)(((unsigned long)head) - offset);
#if 0
	if (!memcmp(priv_buf->magic, MAGIC_CODE, 4) &&
#else
	const unsigned int *magic_code = (unsigned int*) MAGIC_CODE;
	if ( (*(unsigned int*)(priv_buf->magic) == (*magic_code)) &&
#endif
			(priv_buf->buf_pointer == (unsigned int)priv_buf))
		return 1;
	else
		return 0;
}

__IRAM_IN_865X
void free_rtl8190_priv_buf(unsigned char *head)
{

#ifdef CONCURRENT_MODE
	unsigned long offset = (unsigned long)(&((struct priv_skb_buf *)0)->buf);
	struct priv_skb_buf *priv_buf = (struct priv_skb_buf *)(((unsigned long)head) - offset);
	struct rtl8192cd_priv *priv = priv_buf->root_priv;
	int i = priv->pshare->wlandev_idx;
#ifndef SMP_SYNC
	unsigned long x;
#endif	
#ifdef DELAY_REFILL_RX_BUF
	int ret;
#ifdef SMP_SYNC		
	unsigned long flags;
	int locked;
#endif
#ifdef CONFIG_WLAN_HAL
    unsigned int                    q_num;
    PHCI_RX_DMA_MANAGER_88XX        prx_dma;
    PHCI_RX_DMA_QUEUE_STRUCT_88XX   cur_q;
#endif // CONFIG_WLAN_HAL
#ifdef SMP_SYNC
	locked = 0;		
	SMP_TRY_LOCK_RECV(flags,locked);
#endif	
	SAVE_INT_AND_CLI(x);
#ifdef CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
        if (!(priv->drv_state & DRV_STATE_OPEN)){
            /* return 0 means can't refill (because interface be closed or not opened yet) to rx ring but relesae to skb_poll*/            
            ret=0;
        }else{
            q_num   = 0;
            prx_dma = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PRxDMA88XX);
            cur_q   = &(prx_dma->rx_queue[q_num]);
                        
            ret = refill_rx_ring_88XX(priv, NULL, head, q_num, cur_q);
            GET_HAL_INTERFACE(priv)->UpdateRXBDHostIdxHandler(priv, q_num, cur_q->rxbd_ok_cnt);
            cur_q->rxbd_ok_cnt = 0;
        }
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
		ret = refill_rx_ring(priv, NULL, head);
	}
	
	if (ret) {
#ifdef SMP_SYNC		
		if(locked)
			SMP_UNLOCK_RECV(flags);
#endif			
		RESTORE_INT(x);
		return;
	}
	else {
    	release_buf_to_poll(priv, head, &skbbuf_list[i], (unsigned int *)&skb_free_num[i]);
    }
#ifdef SMP_SYNC
    if(locked)
			SMP_UNLOCK_RECV(flags);
#endif			
	RESTORE_INT(x);
#else // ! DELAY_REFILL_RX_BUF
	SAVE_INT_AND_CLI(x);
	release_buf_to_poll(priv, head, &skbbuf_list[i], (unsigned int *)&skb_free_num[i]);
	RESTORE_INT(x);
#endif

#else // ! CONCURRENT_MODE

	unsigned long x;
	struct rtl8192cd_priv *priv = root_priv;
#if 0
	if (!is_rtl8190_priv_buf(head)) {
		printk("wlan: free invalid priv skb buf!\n");
		return;
	}
#endif

#ifdef DELAY_REFILL_RX_BUF
#ifdef CONFIG_WLAN_HAL
    unsigned int                    q_num;
    PHCI_RX_DMA_MANAGER_88XX        prx_dma;
    PHCI_RX_DMA_QUEUE_STRUCT_88XX   cur_q;
#endif
	int ret;

#ifdef SMP_SYNC
	unsigned long flags;
	int locked, cpuid;
	locked = 0;		
	SMP_TRY_LOCK_RECV(flags,locked);
#endif
	SAVE_INT_AND_CLI(x);
#ifdef CONFIG_WLAN_HAL
	if (IS_HAL_CHIP(priv)) {
	    // Currently, only one queue for rx...    
        if (!(priv->drv_state & DRV_STATE_OPEN)){
            /* return 0 means can't refill (because interface be closed or not opened yet) to rx ring but relesae to skb_poll*/            
            ret=0;
        }else{        
    	    q_num   = 0;
    	    prx_dma = (PHCI_RX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PRxDMA88XX);
    	    cur_q   = &(prx_dma->rx_queue[q_num]); 	
    	    ret = refill_rx_ring_88XX(priv, NULL, head, q_num, cur_q);
    	    GET_HAL_INTERFACE(priv)->UpdateRXBDHostIdxHandler(priv, q_num, cur_q->rxbd_ok_cnt);
    	    cur_q->rxbd_ok_cnt = 0;
        }
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
		ret = refill_rx_ring(priv, NULL, head);
	}
	
	if (ret) {
#ifdef SMP_SYNC
		if(locked)
			SMP_UNLOCK_RECV(flags);
#endif	
		RESTORE_INT(x);
		return;
	} else {
		release_buf_to_poll(root_priv, head, &skbbuf_list, (unsigned int *)&skb_free_num);
	}
#ifdef SMP_SYNC
		if(locked)
			SMP_UNLOCK_RECV(flags);
#endif	
	RESTORE_INT(x);
#else // ! DELAY_REFILL_RX_BUF

	SAVE_INT_AND_CLI(x);
	release_buf_to_poll(root_priv, head, &skbbuf_list, (unsigned int *)&skb_free_num);
	RESTORE_INT(x);	

#endif

#endif
}
#endif //CONFIG_RTL8190_PRIV_SKB



/*
unsigned int set_fw_reg(struct rtl8192cd_priv *priv, unsigned int cmd, unsigned int val, unsigned int with_val)
{
	static unsigned int delay_count;

	delay_count = 10;

	do {
		if (!RTL_R32(0x2c0))
			break;
		delay_us(5);
		delay_count--;
	} while (delay_count);
	delay_count = 10;

	if (with_val == 1)
		RTL_W32(0x2c4, val);

	RTL_W32(0x2c0, cmd);

	do {
		if (!RTL_R32(0x2c0))
			break;
		delay_us(5);
		delay_count--;
	} while (delay_count);

	return 0;
}


void set_fw_A2_entry(struct rtl8192cd_priv *priv, unsigned int cmd, unsigned char *addr)
{
	unsigned int delay_count = 10;

	do{
		if (!RTL_R32(0x2c0))
			break;
		delay_us(5);
		delay_count--;
	} while (delay_count);
	delay_count = 10;

	RTL_W32(0x2c4, addr[3]<<24 | addr[2]<<16 | addr[1]<<8 | addr[0]);
	RTL_W32(0x2c8, addr[5]<<8 | addr[4]);
	RTL_W32(0x2c0, cmd);

	do{
		if (!RTL_R32(0x2c0))
			break;
		delay_us(5);
		delay_count--;
	} while (delay_count);
}
*/

//#if defined(TXREPORT) || defined(SW_ANT_SWITCH) || defined(USE_OUT_SRC)
#if 1
struct stat_info* findNextSTA(struct rtl8192cd_priv *priv, int *idx)
{
	int i;
	for(i= *idx; i<NUM_STAT; i++) {
		if (priv->pshare->aidarray[i] && priv->pshare->aidarray[i]->used == TRUE) {
			*idx = (i+1);
            if (priv->pshare->aidarray[i]->station.sta_in_firmware != 1)
				continue;

			return &(priv->pshare->aidarray[i]->station);
		}
	}
	return NULL;
}
#endif

int is_DFS_channel(int channelVal)
{
    if( channelVal >= 52  && channelVal <= 140 ){    
        return 1;
    }else{
        return 0;
    }
}

int is_passive_channel(struct rtl8192cd_priv *priv , int domain, int chan)
{

#ifdef DFS    
	/*during DFS channel , do passive scan*/
    if( (chan >= 52  && chan <= 140) && !priv->pmib->dot11DFSEntry.disable_DFS){  
        return 1;
    }
#endif    	
    #if 0/*when the mib "w52_passive_scan" enabled , do passive scan in ch W52(ch 36 40 44 48)*/
	else if(((chan >= 36) && (chan <= 48)) 
        && priv->pmib->dot11StationConfigEntry.w52_passive_scan ){
		return 1;
    }
    #endif
    #if 0
	else if ((chan >= 12 && chan <= 14) && (domain == DOMAIN_GLOBAL || domain == DOMAIN_WORLD_WIDE)){
		return 1;
    }
    #endif
	return 0;
}


#if defined(TXREPORT)
void requestTxReport(struct rtl8192cd_priv *priv)
{
	int h2ccmd, counter=20;
	struct stat_info *sta;

	if( priv->pshare->sta_query_idx == -1)
		return;
	
	while(is_h2c_buf_occupy(priv)) {
		delay_ms(2);
		if(--counter==0)
			break;
	}
	if(!counter) 
		return;

	h2ccmd= AP_REQ_RPT;
	
	sta = findNextSTA(priv, &priv->pshare->sta_query_idx);
	if(sta)
		h2ccmd |= (REMAP_AID(sta)<<24);
	else {
		priv->pshare->sta_query_idx = -1;
		return;		
	}
	sta = findNextSTA(priv, &priv->pshare->sta_query_idx);
	if(sta)	{	
		h2ccmd |= (REMAP_AID(sta)<<16);
	} else {
		priv->pshare->sta_query_idx = -1;	
	}

	signin_h2c_cmd(priv, h2ccmd , 0);
	DEBUG_INFO("signin h2c:%x\n", h2ccmd);

}

/*
inital tx rate report from fw
---------------------------------------------------------
0 -> cck 1		  12 -> MCS0	44 -> 1NSS-MCS0 
1 -> cck 2        13 -> MCS1    45 -> 1NSS-MCS1 
2 -> cck 5.5      14 -> MCS2    46 -> 1NSS-MCS2 
3 -> cck 11       15 -> MCS3    47 -> 1NSS-MCS3 
------------      16 -> MCS4    48 -> 1NSS-MCS4 
4 ->  ofdm 6      17 -> MCS5    49 -> 1NSS-MCS5 
5 ->  ofdm 9      18 -> MCS6    50 -> 1NSS-MCS6 
6 ->  ofdm 12     19 -> MCS7    51 -> 1NSS-MCS7 
7 ->  ofdm 18     20 -> MCS8    52 -> 1NSS-MCS8 
8 ->  ofdm 24     21 -> MCS9    53 -> 1NSS-MCS9 
9 ->  ofdm 36     22 -> MCS10   54 -> 2NSS-MCS0 
10 -> ofdm 48     23 -> MCS11   55 -> 2NSS-MCS1 
11 -> ofdm 54     24 -> MCS12   56 -> 2NSS-MCS2 
                  25 -> MCS13   57 -> 2NSS-MCS3 
                  26 -> MCS14   58 -> 2NSS-MCS4 
                  27 -> MCS15   59 -> 2NSS-MCS5 
                                60 -> 2NSS-MCS6 
                                61 -> 2NSS-MCS7 
                                62 -> 2NSS-MCS8 
                                63 -> 2NSS-MCS9 
---------------------------------------------------------                                
*/
#ifdef FOR_DISPLAY_RATE
void get_inital_tx_rate2string(unsigned char txrate ){
	static unsigned char rateStr[16];
    if(txrate>=44 && txrate<=53){
		printk("VHT 1SS-MCS%d\n",txrate-44);
	}
	else if(txrate>=54 && txrate<=63){
		printk("VHT 2SS-MCS%d\n",txrate-54);
	}
	else if(txrate>=12 && txrate<=27){
		printk("MCS%d\n",txrate-12);
	}
	else if(txrate>=0 && txrate<=3){
		if(txrate==0)
			printk("CCK-1\n");
		else if(txrate==1)
			printk("CCK-2\n");		
		else if(txrate==2)
			printk("CCK-5.5\n");
		else if(txrate==3)
			printk("CCK-11\n");
	}
	else if(txrate>=4 && txrate<=11){
		if(txrate==4)
			printk("OFDM-6\n");
		else if(txrate==5)
			printk("OFDM-9\n");
		else if(txrate==6)
			printk("OFDM-12\n");
		else if(txrate==7)
			printk("OFDM-18\n");
		else if(txrate==8)
			printk("OFDM-24\n");
		else if(txrate==9)
			printk("OFDM-36\n");
		else if(txrate==10)
			printk("OFDM-48\n");
		else if(txrate==11)
			printk("OFDM-54\n");		

	}

}

//#define FDEBUG(fmt, args...) panic_printk("[%s %d]"fmt,__FUNCTION__,__LINE__,## args)
#endif

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)

void update_RAMask_to_FW(struct rtl8192cd_priv *priv, int forceUpdate)
{
	int idx = 0;
	struct stat_info *pstat = NULL;

	if( !IS_HAL_CHIP(priv) && (GET_CHIP_VER(priv)!= VERSION_8812E)
#ifdef CONFIG_RTL_8723B_SUPPORT
	&& (GET_CHIP_VER(priv)!= VERSION_8723B)
#endif
	) 
		return;


	if (!forceUpdate && !( priv->pshare->is_40m_bw 
#ifdef WIFI_11N_2040_COEXIST
			&& !((((GET_MIB(priv))->dot11OperationEntry.opmode) & WIFI_AP_STATE) 
			&& priv->pmib->dot11nConfigEntry.dot11nCoexist 
			&& (priv->bg_ap_timeout || orForce20_Switch20Map(priv)
			))
#endif
	))
		return;

	pstat = findNextSTA(priv, &idx);

	while(pstat) {
		if(forceUpdate) {
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
			if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B) {
				UpdateHalRAMask8812(priv, pstat, 3);
			}
			 else
#endif				
			{
#ifdef CONFIG_WLAN_HAL
				GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
#endif
			}
		} else {
				if(!pstat->tx_bw_fw && pstat->tx_bw) {
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
					if (GET_CHIP_VER(priv) == VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B){
						UpdateHalRAMask8812(priv, pstat, 3);
					}
					else
#endif
					{
#ifdef CONFIG_WLAN_HAL                                       
						GET_HAL_INTERFACE(priv)->UpdateHalRAMaskHandler(priv, pstat, 3);
#endif
					}
				}
		}
		pstat = findNextSTA(priv, &idx);

	}		

}

// TODO: Filen, check 8192E code below
void txrpt_handler_8812(struct rtl8192cd_priv *priv, struct tx_rpt *report, struct stat_info	*pstat)
{
	static unsigned char initial_rate = 0x7f;
	static unsigned char legacyRA =0 ;
	static unsigned int autoRate1=0;
#if defined(MULTI_STA_REFINE) && defined(TX_SHORTCUT)
	unsigned char ratechg=0;
#endif
	/*under auto rate case , pstat->current_tx_rate just for display but it'll be changed, 
	  so, take care! if under fixed rate case don't enter below block*/ 		

	if(!pstat)
		return;
    if(pstat->sta_in_firmware == 1)
	{
		if( should_restrict_Nrate(priv, pstat) && is_fixedMCSTxRate(priv, pstat)){
			legacyRA = 1;
		}

		autoRate1= is_auto_rate(priv, pstat);

		if(	!(legacyRA || autoRate1) )
			return;
		
		//FDEBUG("STA[%02x%02x%02x:%02x%02x%02x]auto rate ,txfail=%d , txok=%d , rate=",
		//	pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5],
		//	report->txfail, report->txok );
		//get_inital_tx_rate2string(report->initil_tx_rate&0x3f);
		
		
		initial_rate = report->initil_tx_rate ; 			
		if ((initial_rate & 0x7f) == 0x7f)
			return;
#if defined(MULTI_STA_REFINE) && defined(TX_SHORTCUT)
		if(	((pstat->current_tx_rate >= _MCS0_RATE_) && (pstat->current_tx_rate <= _MCS2_RATE_)) 
			||((pstat->current_tx_rate >= _MCS8_RATE_) && (pstat->current_tx_rate <= _MCS10_RATE_)) 
#ifdef RTK_AC_SUPPORT			
			||((pstat->current_tx_rate >= _NSS1_MCS0_RATE_) && (pstat->current_tx_rate <= _NSS1_MCS2_RATE_)) 
			||((pstat->current_tx_rate >= _NSS2_MCS0_RATE_) && (pstat->current_tx_rate <= _NSS2_MCS2_RATE_))
#endif			
		)
			ratechg=1;
#endif
        if ((initial_rate&0x7f) < 12) {
            pstat->current_tx_rate = dot11_rate_table[initial_rate&0x7f];

			pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;				
		} else {
            if((initial_rate&0x7f) >= 44){
                pstat->current_tx_rate = VHT_RATE_ID+((initial_rate&0x7f) -44);
			}else{
                pstat->current_tx_rate = HT_RATE_ID+((initial_rate&0x7f) -12);
			}
								
			if (initial_rate & BIT(7))
				pstat->ht_current_tx_info |= TX_USE_SHORT_GI;
			else
				pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;
		}
			
		priv->pshare->current_tx_rate    = pstat->current_tx_rate;
		priv->pshare->ht_current_tx_info = pstat->ht_current_tx_info;
#if defined(MULTI_STA_REFINE) && defined(TX_SHORTCUT) && defined(SW_TX_QUEUE)
		if(	((pstat->current_tx_rate >= _MCS0_RATE_) && (pstat->current_tx_rate <= _MCS2_RATE_)) 
			||((pstat->current_tx_rate >= _MCS8_RATE_) && (pstat->current_tx_rate <= _MCS10_RATE_)) 
#ifdef RTK_AC_SUPPORT			
			||((pstat->current_tx_rate >= _NSS1_MCS0_RATE_) && (pstat->current_tx_rate <= _NSS1_MCS2_RATE_)) 
			||((pstat->current_tx_rate >= _NSS2_MCS0_RATE_) && (pstat->current_tx_rate <= _NSS2_MCS2_RATE_))
#endif			
		)
			ratechg^=1;

		if(ratechg && (priv->pshare->swq_numActiveSTA > LOWAGGRESTA))
			clearTxShortCutBufSize(priv, pstat);
#endif
	}
}
#endif
#ifdef CONFIG_WLAN_HAL
void APReqTXRptHandler(
    struct rtl8192cd_priv   *priv,
    pu1Byte                  pbuf
)
{
    PAPREQTXRPT pparm = (PAPREQTXRPT)pbuf;
  	struct tx_rpt rpt1;
	unsigned char MacID = 0xff;        
    unsigned char idx = 0;
    int j;
    {
        for (j = 0; j < 2; j++) {

            MacID = pparm->txrpt[j].RPT_MACID;
            if (MacID == 0xff)
                continue;

            rpt1.macid =  MacID;

            if (rpt1.macid) { 
                rpt1.txok = le16_to_cpu(pparm->txrpt[j].RPT_TXOK);
                rpt1.txfail = le16_to_cpu(pparm->txrpt[j].RPT_TXFAIL);                  
                rpt1.initil_tx_rate = pparm->txrpt[j].RPT_InitialRate;
              
                txrpt_handler(priv, &rpt1); // add inital tx rate handle for 8812E
            }
            idx += 6;
        }
    }
}
#endif

#if (MU_BEAMFORMING_SUPPORT == 1)
void _txrpt_handler_MU(struct rtl8192cd_priv *priv, struct tx_rpt *report) 
{
	int i;
	PRT_BEAMFORMING_INFO		pBeamInfo = &(priv->pshare->BeamformingInfo);
	PRT_BEAMFORMING_ENTRY		pEntry = NULL;
	unsigned char gid_to_pos1_index[30] = {1,0,2,0,3,0,4,0,5,0,
											2,1,3,1,4,1,5,1,3,2,
											4,2,5,2,4,3,5,3,5,4};

	if(report->macid < 0x80 || report->macid > 0x80+29) {
		panic_printk("Wrong macid=%d\n", report->macid);
		return;
	}
	
	for(i=0; i<BEAMFORMEE_ENTRY_NUM; i++)
	{		
		pEntry = &(pBeamInfo->BeamformeeEntry[i]); 
		if(pEntry->mu_reg_index == gid_to_pos1_index[report->macid - 0x80] && pEntry->pSTA) {
			priv->net_stats.tx_errors += report->txfail;
			pEntry->pSTA->tx_fail += report->txfail;
			pEntry->pSTA->tx_pkts += report->txok+report->txfail;
			return;
		}
	}	

}

#endif

void _txrpt_handler(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct tx_rpt *report)
{
#ifdef DONT_COUNT_PROBE_PACKET
	if (pstat->tx_probe_rsp_pkts) {
		if (pstat->tx_probe_rsp_pkts >= report->txok) {
			pstat->tx_probe_rsp_pkts -= report->txok;
			report->txok = 0;
		} else {
			report->txok -= pstat->tx_probe_rsp_pkts;
			pstat->tx_probe_rsp_pkts = 0;
		}
	}
#endif // DONT_COUNT_PROBE_PACKET

#ifdef CONFIG_VERIWAVE_MU_CHECK
	if(pstat->isVeriwaveInValidSTA && report->txok > 200) {
		pstat->isVeriwaveInValidSTA = 0;		
	}
#endif

#if defined(TXRETRY_CNT)
	pstat->cur_tx_ok += report->txok;
	pstat->cur_tx_fail += report->txfail;
#endif
    // Debug Message in Run time for QC Driver     kkbomb
    RT_QC_TRACE(1,1,("TX Macid: %d \n",report->macid));
    RT_QC_TRACE(1,1,("TX OK: %d \n",report->txok));
    RT_QC_TRACE(1,1,("TX Fail: %d \n",report->txfail));

	if (GET_CHIP_VER(priv) != VERSION_8822B) //eric-8822 ??
	if ((0 == report->txok) && (0 == report->txfail))
		return;

	priv->net_stats.tx_errors += report->txfail;
	pstat->tx_fail += report->txfail;
	pstat->tx_pkts += report->txok+report->txfail;

	DEBUG_INFO("debug[%02X%02X%02X%02X%02X%02X]:id=%d,ok=%d,fail=%d\n", 
		pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5],
		report->macid, report->txok, report->txfail);
	
#ifdef DETECT_STA_EXISTANCE
#ifdef CONFIG_WLAN_HAL
	if(IS_HAL_CHIP(priv))
	{
		DetectSTAExistance88XX(priv, report, pstat);
	} else if(CONFIG_WLAN_NOT_HAL_EXIST)
#endif
	{//not HAL
		// Check for STA existance; added by Annie, 2010-08-10.Not support now
#if (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)|| defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT))
		if (CHIP_VER_92X_SERIES(priv) || (GET_CHIP_VER(priv)== VERSION_8812E) || (GET_CHIP_VER(priv)== VERSION_8723B))
			DetectSTAExistance(priv, report, pstat);
#endif
	}
#endif // DETECT_STA_EXISTANCE

#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) || defined(CONFIG_RTL_8723B_SUPPORT)
	if( GET_CHIP_VER(priv)== VERSION_8812E || IS_HAL_CHIP(priv) || (GET_CHIP_VER(priv)== VERSION_8723B))
		txrpt_handler_8812(priv, report, pstat);
#endif
}

void txrpt_handler(struct rtl8192cd_priv *priv, struct tx_rpt *report)
{
	struct stat_info	*pstat;	
#ifdef MBSSID
	int i;
#endif

#if(MU_BEAMFORMING_SUPPORT == 1)
	if(report->macid >= 0x80) {
		_txrpt_handler_MU(priv, report);
		return;
	}
	else
#endif
	{
		pstat = get_macidinfo(priv, report->macid);
		if(pstat) {
			_txrpt_handler(priv, pstat, report);
			return;
		}
	}

#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))	{
		pstat = get_macidinfo(GET_VXD_PRIV(priv), report->macid);
		if(pstat) {
			_txrpt_handler(GET_VXD_PRIV(priv), pstat, report);
			return;
		}
	}
#endif
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) 		{
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i])) {
				pstat = get_macidinfo(priv->pvap_priv[i], report->macid);
				if(pstat) {
					_txrpt_handler(priv->pvap_priv[i], pstat, report);
					return;
				}
			}
		}
	}
#endif
}

#ifdef TXRETRY_CNT

int is_support_TxRetryCnt(struct rtl8192cd_priv *priv)
{
	if ( GET_CHIP_VER(priv) == VERSION_8188E
		 || GET_CHIP_VER(priv) == VERSION_8812E
		 || GET_CHIP_VER(priv) == VERSION_8192E 
		 || GET_CHIP_VER(priv) == VERSION_8881A 
		 || GET_CHIP_VER(priv) == VERSION_8814A)
	return 1;
		
	return 0;

}

void txretry_handler(struct rtl8192cd_priv *priv, struct tx_retry *retry)
{

	struct stat_info	*pstat;	
	unsigned char macid_idx = 0;

	for(macid_idx=0; macid_idx<2; macid_idx++)
	{
		pstat = get_macidinfo(priv, priv->pshare->sta_query_retry_macid[macid_idx]);

		if(!pstat)
			break;

		pstat->cur_tx_retry_pkts = retry->retry_pkt_macid[macid_idx];
		pstat->cur_tx_retry_cnt = retry->retry_cnt_macid[macid_idx];

		pstat->total_tx_retry_pkts += pstat->cur_tx_retry_pkts;
		pstat->total_tx_retry_cnt += pstat->cur_tx_retry_cnt;
		priv->ext_stats.tx_retrys += pstat->cur_tx_retry_cnt;

	}

}


void C2HTxTxRetryHandler(struct rtl8192cd_priv *priv, unsigned char *CmdBuf)
{
	struct tx_retry retry;
	
	retry.stat_idx= CmdBuf[0];
	retry.retry_pkt_macid[0] = CmdBuf[1] | ((short)CmdBuf[2]<<8);
	retry.retry_cnt_macid[0] = CmdBuf[3] | ((short)CmdBuf[4]<<8);
	retry.retry_pkt_macid[1] = CmdBuf[5] | ((short)CmdBuf[6]<<8);
	retry.retry_cnt_macid[1] = CmdBuf[7] | ((short)CmdBuf[8]<<8);

#if 0
	panic_printk("%s [%d] %d %d : %d %d\n", __FUNCTION__, retry.stat_idx, 
		retry.retry_pkt_macid[0], retry.retry_cnt_macid[0], 
		retry.retry_pkt_macid[1], retry.retry_cnt_macid[1] );
#endif

	txretry_handler(priv, &retry);
}

#endif

#if defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT)
void C2H_isr(struct rtl8192cd_priv *priv)
{
	struct tx_rpt rpt1;
	int j, tmp32, idx=0x1a2;
#ifndef SMP_SYNC
	unsigned long flags;
#endif
	SAVE_INT_AND_CLI(flags);
	tmp32 = RTL_R16(0x1a0);
	if( (tmp32&0xff)==0xc2 ) {
		for(j=0; j<2; j++) {
			rpt1.macid= (0x1f) & RTL_R8(idx+4);
			if(rpt1.macid) {
#ifdef _BIG_ENDIAN_
				rpt1.txok = le16_to_cpu(RTL_R16(idx+2));
				rpt1.txfail = le16_to_cpu(RTL_R16(idx));
#else
				rpt1.txok = be16_to_cpu(RTL_R16(idx+2));
				rpt1.txfail = be16_to_cpu(RTL_R16(idx));
#endif
				txrpt_handler(priv, &rpt1);
			}			
			idx+=6;		
		}
	}
	RTL_W8( 0x1af, 0);
	requestTxReport(priv);
	RESTORE_INT(flags);
}
#endif

#endif
/*cfg p2p*/
#if !defined(__ECOS) && !defined(__OSK__)
void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
        int i;
        char tmpbuf[10];

        out[0] = '\0';

        for (i=0; i<len; i++) {
                sprintf(tmpbuf, "%02x", bin[i]);
                strcat(out, tmpbuf);
        }
}
#endif
/*cfg p2p*/
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}


int rtl_string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) _atoi(tmpBuf,16);
	}
	return 1;
}


#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
#ifdef __ECOS
INT32 rtl_isWlanPassthruFrame(UINT8 *data)
#else
INT32 rtl_isPassthruFrame(UINT8 *data)
#endif
{
	int	ret;

#ifdef __ECOS
	ret = FAILED;
#else
	ret = FAIL;
#endif
	if (passThruStatusWlan)
	{
		if (passThruStatusWlan&IP6_PASSTHRU_MASK)
		{
			if ((*((UINT16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_IPV6)) ||
				((*((UINT16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_8021Q))&&(*((UINT16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_IPV6))))
			{
				ret = SUCCESS;
			}
		}
		#if defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		if (passThruStatusWlan&PPPOE_PASSTHRU_MASK)
		{
			if (((*((UINT16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_PPP_SES))||(*((UINT16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_PPP_DISC))) ||
				((*((UINT16*)(data+(ETH_ALEN<<1)))==__constant_htons(ETH_P_8021Q))&&((*((UINT16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_PPP_SES))||(*((UINT16*)(data+(ETH_ALEN<<1)+VLAN_HLEN))==__constant_htons(ETH_P_PPP_DISC)))))
			{
				ret = SUCCESS;
			}
		}
		#endif
	}

	return ret;
}
#endif

#if 0	//defined(HS2_SUPPORT) || defined(DOT11K) || defined(CH_LOAD_CAL) || defined(RTK_SMART_ROAMING)
void rtl8192cd_cu_stop(struct rtl8192cd_priv *priv) {
    unsigned char stop_cu = 1;
    int i;
    struct rtl8192cd_priv * root_priv = GET_ROOT(priv);
    priv->cu_enable = 0;


    /*check if other interface need channel utilization calcultation*/
    if (IS_DRV_OPEN(root_priv) && root_priv->cu_enable) {
        stop_cu = 0;
    }
#ifdef MBSSID
    if (stop_cu == 1 && root_priv->pmib->miscEntry.vap_enable) {
        for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
            if (IS_DRV_OPEN(root_priv->pvap_priv[i]) && root_priv->pvap_priv[i]->cu_enable) {
                stop_cu = 0;
                break;
            }
        }
    }
#endif

#ifdef UNIVERSAL_REPEATER    
    if (stop_cu == 1 && IS_DRV_OPEN(GET_VXD_PRIV(root_priv)) &&
        GET_VXD_PRIV(root_priv)->cu_enable) {
        stop_cu = 0;
    }
#endif     
    
    if(stop_cu) {
        if (timer_pending(&priv->pshare->cu_info.cu_cntdwn_timer)) {
            del_timer_sync(&priv->pshare->cu_info.cu_cntdwn_timer);
        }
        priv->pshare->cu_info.cu_enable = 0;
    }
}


void rtl8192cd_cu_start(struct rtl8192cd_priv *priv) {
    priv->cu_enable = 1;    
    if(priv->pshare->cu_info.cu_enable == 0) {
        priv->pshare->cu_info.channel_utilization = 0;
        init_timer(&priv->pshare->cu_info.cu_cntdwn_timer);
        priv->pshare->cu_info.cu_cntdwn_timer.data = (unsigned long) priv;
        priv->pshare->cu_info.cu_cntdwn_timer.function = rtl8192cd_cu_cntdwn_timer;
        priv->pshare->cu_info.cu_cntdwn = priv->pshare->cu_info.cu_initialcnt = (priv->pmib->dot11StationConfigEntry.channel_utili_beaconIntval * priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod)/CU_Intval;
        start_bbp_ch_load(priv, 50000);
        //priv->pshare->cu_info.cu_cntdwn_timer.expires = jiffies + CU_TO;
        mod_timer(&priv->pshare->cu_info.cu_cntdwn_timer, jiffies + CU_TO);
        priv->pshare->cu_info.cu_enable = 1;
    }
}

void rtl8192cd_cu_cntdwn_timer(unsigned long task_priv)
{
    struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
    int val;

    if ((val = read_bbp_ch_load(priv)) == -1)
    {
        mod_timer(&priv->pshare->cu_info.cu_cntdwn_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(10));	
    }
    else
    {
        priv->pshare->cu_info.chbusytime += val;
        priv->pshare->cu_info.cu_cntdwn--;
        if (priv->pshare->cu_info.cu_cntdwn == 0)
        {
            priv->pshare->cu_info.channel_utilization = (priv->pshare->cu_info.chbusytime*255)/(priv->pmib->dot11StationConfigEntry.channel_utili_beaconIntval * priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod * 1024);
            priv->pshare->cu_info.chbusytime = 0;
            priv->pshare->cu_info.cu_cntdwn = priv->pshare->cu_info.cu_initialcnt;
        }
        start_bbp_ch_load(priv, 50000);
        mod_timer(&priv->pshare->cu_info.cu_cntdwn_timer, jiffies + CU_TO);
    }
}

#endif

#if defined(RTK_ATM) && !defined(HS2_SUPPORT)
void staip_snooping_byarp(struct sk_buff *pskb, struct stat_info *pstat)
{
	struct arphdr *arp = (struct arphdr *)(pskb->data + ETH_HLEN);
	unsigned char *arp_ptr = (unsigned char *)(arp + 1);
	if((arp->ar_pro == __constant_htons(ETH_P_IP)) && (arp->ar_op == htons(ARPOP_REQUEST))) {
		//find sender ip
		arp_ptr += arp->ar_hln;
		//backup sender ip
		if ((*arp_ptr == 0) && (*(arp_ptr+1) == 0) && (*(arp_ptr+2) == 0) && (*(arp_ptr+3) == 0))
			return;
		else {
			memcpy(pstat->sta_ip, arp_ptr, 4);
			//panic_printk("ARP cache ip=%d.%d.%d.%d\n", pstat->sta_ip[0],pstat->sta_ip[1],pstat->sta_ip[2],pstat->sta_ip[3]);
		}
	}
}
#endif

#ifdef HS2_SUPPORT
#ifdef __ECOS
#define ipv6_addr_copy(a1,a2) memcpy(a1, a2, sizeof(struct in6_addr))
static int ipv6_addr_equal(const struct in6_addr *a1,
				  const struct in6_addr *a2)
{
	return (((a1->s6_addr32[0] ^ a2->s6_addr32[0]) |
		 (a1->s6_addr32[1] ^ a2->s6_addr32[1]) |
		 (a1->s6_addr32[2] ^ a2->s6_addr32[2]) |
		 (a1->s6_addr32[3] ^ a2->s6_addr32[3])) == 0);
}
static int
inet_cksum(u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr;
    register u_short answer;
    register u_int sum = 0;
    u_short odd_byte = 0;
    while( nleft > 1 )  {
        sum += *w++;
        nleft -= 2;
    }
    if( nleft == 1 ) {
        *(u_char *)(&odd_byte) = *(u_char *)w;
        sum += odd_byte;
    }
    sum = (sum >> 16) + (sum & 0x0000ffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);                     /* add carry */
    answer = ~sum;                          /* truncate to 16 bits */
    return (answer);
}
#endif

unsigned int getDSCP2UP(struct rtl8192cd_priv *priv, unsigned char DSCP)
{
	int i;

	for(i=0;i<MAX_DSCP_EXCEPT;i++) {
		if(DSCP == priv->pmib->hs2Entry.QoSMAP_except[i][0]) // EXCEPTION DSCP VALUE
			return priv->pmib->hs2Entry.QoSMAP_except[i][1]; // PRIORITY
	}

	for(i=0;i<MAX_QOS_PRIORITY;i++) {
		if(DSCP >= priv->pmib->hs2Entry.QoSMAP_range[i][0] &&  // DSCP LOWER BOUND
			DSCP <= priv->pmib->hs2Entry.QoSMAP_range[i][1])   // DSCP UPPER BOUND
			return i; // PRIORITY
	}
	return 0;
}

void setQoSMapConf(struct rtl8192cd_priv *priv)
{
	int i;
	int curIndex;
	int curQoSMap = priv->pmib->hs2Entry.curQoSMap;	
	
	for(i=0;i<MAX_DSCP_EXCEPT;i++)
	{			
		priv->pmib->hs2Entry.QoSMAP_except[i][0] = -1;
		priv->pmib->hs2Entry.QoSMAP_except[i][1] = -1;
		curIndex++;
	}

	if(priv->pmib->hs2Entry.QoSMap_ielen[curQoSMap] < 16) 
	{	
		HS2_DEBUG_ERR("QoSMAPIE length is too small\n");
		return;
	} else if(priv->pmib->hs2Entry.QoSMap_ielen[curQoSMap] > 16) 
	{	
		if(priv->pmib->hs2Entry.QoSMap_ielen[curQoSMap]-16 > MAX_DSCP_EXCEPT * 2) {
			HS2_DEBUG_ERR("QoSMAP_except matrix supports %d except only, need enlarge QoSMAP_except matrix\n", MAX_DSCP_EXCEPT);
			return;
		}
		curIndex = 0;	
		// include DSCP exception
		for(i=0;i<priv->pmib->hs2Entry.QoSMap_ielen[curQoSMap]-MAX_QOS_PRIORITY*2;i+=2)
		{			
			priv->pmib->hs2Entry.QoSMAP_except[curIndex][0] = priv->pmib->hs2Entry.QoSMap_ie[curQoSMap][i]; // DSCP
			priv->pmib->hs2Entry.QoSMAP_except[curIndex][1] = priv->pmib->hs2Entry.QoSMap_ie[curQoSMap][i+1]; // UP
			curIndex++;
		}
	}
	
	curIndex = 0;
	for(i=priv->pmib->hs2Entry.QoSMap_ielen[curQoSMap]-MAX_QOS_PRIORITY*2;i<priv->pmib->hs2Entry.QoSMap_ielen[curQoSMap];i+=2)
	{			
		priv->pmib->hs2Entry.QoSMAP_range[curIndex][0] = priv->pmib->hs2Entry.QoSMap_ie[curQoSMap][i]; // DSCP
		priv->pmib->hs2Entry.QoSMAP_range[curIndex][1] = priv->pmib->hs2Entry.QoSMap_ie[curQoSMap][i+1]; // UP
		curIndex++;
	}

	for(i=0;i<MAX_DSCP_EXCEPT;i++)
		HS2_DEBUG_INFO("DSCP Except[%d]=%x %x\n",i,priv->pmib->hs2Entry.QoSMAP_except[i][0],priv->pmib->hs2Entry.QoSMAP_except[i][1]);
	for(i=0;i<MAX_QOS_PRIORITY;i++)
		HS2_DEBUG_INFO("DSCP range[%d]=%x %x\n",i,priv->pmib->hs2Entry.QoSMAP_range[i][0],priv->pmib->hs2Entry.QoSMAP_range[i][1]);
	
		
}
void calcu_sta_v6ip(struct stat_info *pstat)
{
    int idx=0;

	struct in6_addr addrp;
	addrp.s6_addr[0] = 0xfe;
	addrp.s6_addr[1] = 0x80;
	addrp.s6_addr[2] = 0x00;
	addrp.s6_addr[3] = 0x00;
	addrp.s6_addr[4] = 0x00;
	addrp.s6_addr[5] = 0x00;
	addrp.s6_addr[6] = 0x00;
	addrp.s6_addr[7] = 0x00;
	addrp.s6_addr[8] = pstat->hwaddr[0] | 0x02;
	addrp.s6_addr[9] = pstat->hwaddr[1];
	addrp.s6_addr[10] = pstat->hwaddr[2];
	addrp.s6_addr[11] = 0xff;
	addrp.s6_addr[12] = 0xfe;
	addrp.s6_addr[13] = pstat->hwaddr[3];
    addrp.s6_addr[14] = pstat->hwaddr[4];
    addrp.s6_addr[15] = pstat->hwaddr[5];

    if(pstat->v6ipCount==0){
    	ipv6_addr_copy(&pstat->sta_v6ip[pstat->v6ipCount], &addrp);

        pstat->v6ipCount=1;

        HS2DEBUG("v6ipCount[%d],sta[%02X%02X%02X:%02X%02X%02X]\n",pstat->v6ipCount
             ,pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2]
             ,pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
        HS2DEBUG("learn1 V6 IP[%02X%02X%02X%02X][%02X%02X%02X%02X][%02X%02X%02X%02X][%02X%02X%02X%02X]\n"
            ,addrp.s6_addr[0],addrp.s6_addr[1],addrp.s6_addr[2],addrp.s6_addr[3]
            ,addrp.s6_addr[4],addrp.s6_addr[5],addrp.s6_addr[6],addrp.s6_addr[7]
            ,addrp.s6_addr[8],addrp.s6_addr[9],addrp.s6_addr[10],addrp.s6_addr[11]
            ,addrp.s6_addr[12],addrp.s6_addr[13],addrp.s6_addr[14],addrp.s6_addr[15]);        
    }else{
        for(idx=0;idx<pstat->v6ipCount;idx++) {
            if (ipv6_addr_equal(&pstat->sta_v6ip[idx], &addrp)){
                return; /*has existed no need add*/ 
            }
        }

    	ipv6_addr_copy(&pstat->sta_v6ip[pstat->v6ipCount], &addrp);

        pstat->v6ipCount ++;
        
        /*didn't exist , add*/ 
        HS2DEBUG("v6ipCount[%d],sta[%02X%02X%02X:%02X%02X%02X]\n",pstat->v6ipCount
             ,pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2]
             ,pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
        HS2DEBUG("learn2 V6 IP[%02X%02X%02X%02X][%02X%02X%02X%02X][%02X%02X%02X%02X][%02X%02X%02X%02X]\n"
            ,addrp.s6_addr[0],addrp.s6_addr[1],addrp.s6_addr[2],addrp.s6_addr[3]
            ,addrp.s6_addr[4],addrp.s6_addr[5],addrp.s6_addr[6],addrp.s6_addr[7]
            ,addrp.s6_addr[8],addrp.s6_addr[9],addrp.s6_addr[10],addrp.s6_addr[11]
            ,addrp.s6_addr[12],addrp.s6_addr[13],addrp.s6_addr[14],addrp.s6_addr[15]);

        
    }



                        

}

void staip_snooping_byarp(struct sk_buff *pskb, struct stat_info *pstat)
{
	struct arphdr *arp = (struct arphdr *)(pskb->data + ETH_HLEN);
	unsigned char *arp_ptr = (unsigned char *)(arp + 1);
	if((arp->ar_pro == __constant_htons(ETH_P_IP)) && (arp->ar_op == htons(ARPOP_REQUEST))) {
		//find sender ip
		arp_ptr += arp->ar_hln;
		//backup sender ip
		if ((*arp_ptr == 0) && (*(arp_ptr+1) == 0) && (*(arp_ptr+2) == 0) && (*(arp_ptr+3) == 0))
			return;
		else {
			memcpy(pstat->sta_ip, arp_ptr, 4);
			//HS2DEBUG("ARP cache ip=%d.%d.%d.%d\n", pstat->sta_ip[0],pstat->sta_ip[1],pstat->sta_ip[2],pstat->sta_ip[3]);
		}
	}
}
#ifdef __ECOS
void stav6ip_snooping_bynsolic(struct sk_buff *pskb, struct stat_info *pstat)
{
	struct ip6_hdr *hdr = (struct ip6_hdr *)(pskb->data+ETH_HLEN);
    struct icmp6hdr *icmphdr;
	int pkt_len, type;
	if (hdr->ip6_vfc != 6)
        return;

	if (hdr->ip6_hlim != 255)
		return;
	pkt_len = ntohs(hdr->ip6_plen);
	if (pkt_len>0)
    {
        icmphdr = (struct icmp6hdr *)(pskb->data+ETH_HLEN+sizeof(*hdr));
        type = icmphdr->icmp6_type;
		if (type == ND_NEIGHBOR_SOLICIT)
		{
			if ((hdr->ip6_src.s6_addr32[0] == 0) && (hdr->ip6_src.s6_addr32[1] == 0) && (hdr->ip6_src.s6_addr32[2] == 0) && (hdr->ip6_src.s6_addr32[3] == 0)) {
				struct in6_addr *target = (struct in6_addr *) (icmphdr + 1);
				if ((target->s6_addr32[0] == 0) && (target->s6_addr32[1] == 0) && (target->s6_addr32[2] == 0) && (target->s6_addr32[3] == 0)) {
					return;
				}
				else {
					printk("rcv multicast ns duplicate addr\n");
					ipv6_addr_copy(&pstat->sta_v6ip, target);
				}
			}
			else {
				printk("rcv multicast ns\n");
				ipv6_addr_copy(&pstat->sta_v6ip, &hdr->ip6_src);
			}
		}
	}
}
#else
void stav6ip_snooping_bynsolic(struct rtl8192cd_priv *priv,struct sk_buff *pskb, struct stat_info *pstat)
{
	struct ipv6hdr *hdr = (struct ipv6hdr *)(pskb->data+ETH_HLEN);
    struct icmp6hdr *icmphdr;
	int pkt_len, type;
	int i;
	struct stat_info *pstat1;
	struct list_head *phead, *plist;
	if (hdr->version != 6)
        return;

	if (hdr->hop_limit != 255)
		return;

	pkt_len = ntohs(hdr->payload_len);
	if (pkt_len>0)
    {
        icmphdr = (struct icmp6hdr *)(pskb->data+ETH_HLEN+sizeof(*hdr));
        type = icmphdr->icmp6_type;
		if (type == NDISC_NEIGHBOUR_SOLICITATION)
		{
    		HS2DEBUG("rx neighbour solicitation from[%02X%02X%02X:%02X%02X%02X]\n",
                pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);      
		    /*20141209 , p Note ,when src ip=NULL && target IP!=NULL*/
			if ((hdr->saddr.s6_addr32[0] == 0) && (hdr->saddr.s6_addr32[1] == 0) 
                 && (hdr->saddr.s6_addr32[2] == 0) && (hdr->saddr.s6_addr32[3] == 0)) 
            {
                HS2DEBUG("src ip(v6)=NULL\n");
				struct in6_addr *target = (struct in6_addr *) (icmphdr + 1);
                
				if ((target->s6_addr32[0] == 0) && (target->s6_addr32[1] == 0) 
                    && (target->s6_addr32[2] == 0) && (target->s6_addr32[3] == 0)) 
                {
                    HS2DEBUG("tar ip(v6)=NULL\n");                
					return;
				}
				else {

                    HS2DEBUG("tar ip(v6) [%08X %08X %08X %08X]\n",target->s6_addr32[0],target->s6_addr32[1],
                        target->s6_addr32[2],target->s6_addr32[3]);

					phead = &priv->asoc_list;
				    plist = phead->next;
					
				    while (phead && (plist != phead))
				    {
				    	pstat1 = list_entry(plist, struct stat_info, asoc_list);
						plist = plist->next;
						for(i=0;i<pstat1->v6ipCount;i++) {
					    	if (ipv6_addr_equal(&pstat1->sta_v6ip[i], target)){
								return;
                            }
						}
				    }
					if (pstat->v6ipCount<10){

						ipv6_addr_copy(&pstat->sta_v6ip[pstat->v6ipCount++], target);

						HS2DEBUG("learn tar ip(v6)[%08X %08X %08X %08X],v6ipCount[%d]\n",target->s6_addr32[0],target->s6_addr32[1],
                            target->s6_addr32[2],target->s6_addr32[3],pstat->v6ipCount);
                        
					}else{
						HS2DEBUG("the number of v6ipaddress > 10\n");
					}
				}
			}
			else {

                HS2DEBUG("src ip(v6)[%08X %08X %08X %08X]\n",
                    hdr->saddr.s6_addr32[0],hdr->saddr.s6_addr32[1],hdr->saddr.s6_addr32[2],hdr->saddr.s6_addr32[3]);
                

				phead = &priv->asoc_list;
			    plist = phead->next;
				
			    while (phead && (plist != phead))
			    {
			    	pstat1 = list_entry(plist, struct stat_info, asoc_list);
					plist = plist->next;
					for(i=0;i<pstat1->v6ipCount;i++) {
				    	if (ipv6_addr_equal(&pstat1->sta_v6ip[i], &hdr->saddr))
							return;
					}
			    }
				if(pstat->v6ipCount < 10){


					ipv6_addr_copy(&pstat->sta_v6ip[pstat->v6ipCount++], &hdr->saddr);

					HS2DEBUG("learn (src) v6 ip [%08X %08X %08X %08X],v6ipCount[%d]\n",
                        hdr->saddr.s6_addr32[0],hdr->saddr.s6_addr32[1],hdr->saddr.s6_addr32[2],hdr->saddr.s6_addr32[3],pstat->v6ipCount);
                    
				}else{
					HS2DEBUG("the number of v6ipaddress > 10\n");
				}
			}
		}
	}
}
#endif
#if 0
void stav6ip_snooping_bynadvert(struct sk_buff *pskb, struct stat_info *pstat)
{
    struct ipv6hdr *hdr = (struct ipv6hdr *)(pskb->data+ETH_HLEN);
    struct icmp6hdr *icmphdr;
    int pkt_len, type;
    if (hdr->version != 6)
        return;

	if (hdr->hop_limit != 255)
		return;

    pkt_len = ntohs(hdr->payload_len);
    if (pkt_len>0)
    {
        icmphdr = (struct icmp6hdr *)(pskb->data+ETH_HLEN+sizeof(*hdr));
        type = icmphdr->icmp6_type;
        if (type == NDISC_NEIGHBOUR_ADVERTISEMENT)
        {
            if ((hdr->saddr.s6_addr32[0] == 0) && (hdr->saddr.s6_addr32[1] == 0) && (hdr->saddr.s6_addr32[2] == 0) && (hdr->saddr.s6_addr32[3] == 0)) {
				return;
			}
			else {
				printk("rcv unsolicited neighbor advert multicast\n");
                ipv6_addr_copy(&pstat->sta_v6ip, &hdr->saddr);
            }
        }
    }
}
#endif

void staip_snooping_bydhcp(struct sk_buff *pskb, struct rtl8192cd_priv *priv) //struct stat_info *pstat)
{
	#define DHCP_MAGIC 0x63825363

struct iphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	        __u8    ihl:4,
	                version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	        __u8    version:4,
	                ihl:4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
	        __u8    tos;
	        __u16   tot_len;
	        __u16   id;
	        __u16   frag_off;
	        __u8    ttl;
	        __u8    protocol;
#if 0
	        __u16   check;
	        __u32   saddr;
	        __u32   daddr;
#endif
};

struct udphdr {
	        __u16   source;
	        __u16   dest;
	        __u16   len;
	        __u16   check;
};

struct dhcpMessage {
		u_int8_t op;
		u_int8_t htype;
		u_int8_t hlen;
		u_int8_t hops;
		u_int32_t xid;
		u_int16_t secs;
		u_int16_t flags;
		u_int32_t ciaddr;
		u_int32_t yiaddr;
		u_int32_t siaddr;
		u_int32_t giaddr;
		u_int8_t chaddr[16];
		u_int8_t sname[64];
		u_int8_t file[128];
		u_int32_t cookie;
#if 0
		u_int8_t options[308]; /* 312 - cookie */
#endif
};

	struct stat_info *pstat;
	struct iphdr* iph;
	struct udphdr *udph;
	struct dhcpMessage *dhcph;
	struct list_head *phead, *plist;
	
	iph = (struct iphdr *)(pskb->data + ETH_HLEN);
	udph = (struct udphdr *)((unsigned int)iph + (iph->ihl << 2));
	dhcph = (struct dhcpMessage *)((unsigned int)udph + sizeof(struct udphdr));

    phead = &priv->asoc_list;
    plist = phead->next;
    while (phead && (plist != phead))
    {
        pstat = list_entry(plist, struct stat_info, asoc_list);
        plist = plist->next;

        if (!memcmp(pstat->hwaddr, &dhcph->chaddr[0], 6)) {
			if (dhcph->op == 2) //dhcp reply
			{
				if (dhcph->yiaddr == 0) 
					return;
				else {
					memcpy(pstat->sta_ip, &dhcph->yiaddr, 4);
					printk("dhcp give yip=%d.%d.%d.%d\n", pstat->sta_ip[0],pstat->sta_ip[1],pstat->sta_ip[2],pstat->sta_ip[3]);
				}
			}
			return;
		}
	}
}

int check_nei_advt(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
#ifdef __ECOS
	struct ip6_hdr *hdr = (struct ip6_hdr *)(skb->data+ETH_HLEN);
#else
	struct ipv6hdr *hdr = (struct ipv6hdr *)(skb->data+ETH_HLEN);
#endif
	struct icmp6hdr *icmpv6;
	unsigned int pkt_len;
    int type;
#ifdef __ECOS
	pkt_len = ntohs(hdr->ip6_plen);
#else
	pkt_len = ntohs(hdr->payload_len);
#endif
	if (pkt_len>0)
    {
        icmpv6 = (struct icmp6hdr *)(skb->data+ETH_HLEN+sizeof(*hdr));
        type = icmpv6->icmp6_type;
        //printk("pkt len=%d,type=%d\n", pkt_len, type);
#ifdef __ECOS
        if (type == ND_NEIGHBOR_ADVERT)
#else
        if (type == NDISC_NEIGHBOUR_ADVERTISEMENT)
#endif
		{
			printk("drop nei advr\n");
			return 1;	
		}
		else
			return 0;
	}
	return 0;
}

int proxy_icmpv6_ndisc(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
	struct sk_buff *newskb = NULL;
	struct in6_addr *addrp;
#ifdef __ECOS
	struct ip6_hdr *hdr = (struct ip6_hdr *)(skb->data+ETH_HLEN);
	struct ip6_hdr *replyhdr;
#else
	struct ipv6hdr *hdr = (struct ipv6hdr *)(skb->data+ETH_HLEN); // ipv6hdr len = 40 bytes
	struct ipv6hdr *replyhdr;
#endif
	struct icmp6hdr *icmpv6_nsolic;
	struct icmp6hdr *icmpv6_nadvt;	
	struct stat_info *pstat;
    struct list_head *phead, *plist;
	unsigned int pkt_len;
	int type;
	int idx=0;
	int foundTarget=0;
	//HS2DEBUG("proxy_icmpv6_ndisc\n");
#ifdef __ECOS
	if (hdr->ip6_vfc != 6)
		return 0;
	pkt_len = ntohs(hdr->ip6_plen);
#else
	if (hdr->version != 6)
		return 0;
	
	pkt_len = ntohs(hdr->payload_len);
#endif
	if (pkt_len>0)
	{
		icmpv6_nsolic = (struct icmp6hdr *)(skb->data+ETH_HLEN+sizeof(*hdr));
		type = icmpv6_nsolic->icmp6_type;
		//printk("pkt len=%d,type=%d\n", pkt_len, type);
#ifdef __ECOS
		if (type == ND_NEIGHBOR_SOLICIT)
#else
		if (type == NDISC_NEIGHBOUR_SOLICITATION)
#endif
        {

			addrp = (struct in6_addr *)(icmpv6_nsolic+1);

  		    /*20141209 , p Note ,when src ip=NULL , need not reply*/
			if ((hdr->saddr.s6_addr32[0] == 0) && (hdr->saddr.s6_addr32[1] == 0) 
                 && (hdr->saddr.s6_addr32[2] == 0) && (hdr->saddr.s6_addr32[3] == 0)) 
            {
                HS2DEBUG("(src v6 ip is NULL)\n");
                return 0;
            }

        	HS2DEBUG(" Neighbour Solicitation form [%02X%02X%02X:%02X%02X%02X]\n",
                skb->data[6],skb->data[7],skb->data[8],skb->data[9],skb->data[10],skb->data[11]);
            
        	HS2DEBUG("search (tar v6 ip)[%08X %08X %08X %08X]\n",
                addrp->s6_addr32[0],addrp->s6_addr32[1],addrp->s6_addr32[2],addrp->s6_addr32[3]);
            
			if (!memcmp(skb->data+ETH_ALEN, priv->pmib->dot11StationConfigEntry.dot11Bssid, ETH_ALEN))
	        {
		        HS2DEBUG("!!!  v6:arp req src mac=BSSID\n");
			    return 0;
	        }
			#ifdef __ECOS
			if((hdr->ip6_dst.s6_addr32[0] | hdr->ip6_dst.s6_addr32[1] |
		 	hdr->ip6_dst.s6_addr32[2] | (hdr->ip6_dst.s6_addr32[3] ^ htonl(1))) == 0)
			#else
			if (ipv6_addr_loopback(&hdr->daddr))
			#endif
			{
				HS2DEBUG("v6:loopback\n");
		        return 0;
			}

			//search target ip mapping pstat mac
			phead = &priv->asoc_list;
	        plist = phead->next;
		    while (phead && (plist != phead))
			{
	            pstat = list_entry(plist, struct stat_info, asoc_list);
		        plist = plist->next;



				foundTarget=0;
				for(idx=0;idx<pstat->v6ipCount;idx++){
					if (ipv6_addr_equal(&pstat->sta_v6ip[idx], addrp)){
						foundTarget=1;
						break;
					}
				}
		        if (foundTarget)
			    {
					#if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL_ETH_PRIV_SKB)
				    extern struct sk_buff *priv_skb_copy(struct sk_buff *skb);
	                newskb = priv_skb_copy(skb);
					#else
		            newskb = skb_copy(skb, GFP_ATOMIC);
					#endif
					//HS2DEBUG("compare ok!!\n");
					if (newskb == NULL)
	                {
		                priv->ext_stats.tx_drops++;
			            HS2DEBUG("alloc icmpv6 neighbor advertisement skb null!!\n");
				        rtl_kfree_skb(priv, skb, _SKB_TX_);
					    return 1;
					}
					else
					{
						unsigned char *opt;
						int len;
						//da
						memcpy(newskb->data, newskb->data+ETH_ALEN, 6);
						//sa
						memcpy(newskb->data+ETH_ALEN, pstat->hwaddr, 6);
#ifdef __ECOS
						replyhdr = (struct ip6_hdr *)(newskb->data+ETH_HLEN);
						ipv6_addr_copy(&replyhdr->ip6_src, &pstat->sta_v6ip);
						ipv6_addr_copy(&replyhdr->ip6_dst, &hdr->ip6_src);
						icmpv6_nadvt = (struct icmp6hdr *)(newskb->data+ETH_HLEN+sizeof(*hdr));
				        icmpv6_nadvt->icmp6_type = ND_NEIGHBOR_ADVERT;
						icmpv6_nadvt->icmp6_dataun.u_nd_advt.solicited = 1;
						icmpv6_nadvt->icmp6_dataun.u_nd_advt.override = 0;
						#else				
						replyhdr = (struct ipv6hdr *)(newskb->data+ETH_HLEN);
						ipv6_addr_copy(&replyhdr->saddr, &pstat->sta_v6ip[idx]);
						ipv6_addr_copy(&replyhdr->daddr, &hdr->saddr);

						icmpv6_nadvt = (struct icmp6hdr *)(newskb->data+ETH_HLEN+sizeof(*hdr));
				        icmpv6_nadvt->icmp6_type = NDISC_NEIGHBOUR_ADVERTISEMENT;
						icmpv6_nadvt->icmp6_solicited = 1;
						icmpv6_nadvt->icmp6_override = 0;
						#endif						
						//target ip
						opt = (unsigned char *)(newskb->data+ETH_HLEN+sizeof(*replyhdr)+sizeof(struct icmp6hdr));
						ipv6_addr_copy((struct in6_addr *)opt, &pstat->sta_v6ip[idx]);
						opt += sizeof(struct in6_addr);
						//option
						opt[0] = 2; // Type: Target link-layer addr
						opt[1] = 1; // Length: 1 (8 bytes)
						memcpy(opt+2, pstat->hwaddr, ETH_ALEN);
						icmpv6_nadvt->icmp6_cksum = 0;
						len = sizeof(struct icmp6hdr)+sizeof(struct in6_addr)+8;
						#ifdef __ECOS
							icmpv6_nadvt->icmp6_cksum = inet_cksum( (unsigned short *)&replyhdr, len);
						#else
						icmpv6_nadvt->icmp6_cksum = csum_ipv6_magic(&replyhdr->saddr, &replyhdr->daddr, len, 
							IPPROTO_ICMPV6, csum_partial(icmpv6_nadvt, len, 0));
						#endif
						
						dev_kfree_skb_any(skb);
						#ifdef __ECOS
						if (ipv6_addr_equal(&replyhdr->ip6_src, &replyhdr->ip6_dst))
						#else
						if (ipv6_addr_equal(&replyhdr->saddr, &replyhdr->daddr))
						#endif
                        {
                            HS2DEBUG("\n\n!!! v6:tip=sip!!\n\n\n");
                            dev_kfree_skb_any(newskb);
                            return 1;
                        }

						if ((pstat = get_stainfo(priv, newskb->data)) != NULL)
	                    {
	                        int i;
							HS2DEBUG("v6:da in wlan\n\n");
							newskb->cb[2] = (char)0xff;         // not do aggregation
							memcpy(newskb->cb+10,newskb->data,6);
							#if 1
							HS2DEBUG("data=");
							for(i=0;i<6;i++) {
								panic_printk("%02x ",newskb->data[i]);
							}
							panic_printk("\n");
							for(i=0;i<6;i++) {
								panic_printk("%02x ",newskb->data[i+6]);
							}
							panic_printk("\n");
							panic_printk("%02x %02x\n",newskb->data[12],newskb->data[13]); // type                            
							for(i=0;i< 8 ;i++) {
								panic_printk("%02x ",newskb->data[i+14]);
							}
							panic_printk("\n");                            
							for(i=0;i< 16 ;i++) {
								panic_printk("%02x ",newskb->data[i+14+8]);
							}
							panic_printk("\n");                            
							for(i=0;i< 16 ;i++) {
								panic_printk("%02x ",newskb->data[i+14+8+16]);
							}   
							panic_printk("\n");                            
							for(i=0;i< 6 ;i++) {
								panic_printk("%02x ",newskb->data[i+14+8+16+16]);
							}                                                        
							panic_printk("\n");                            
							#endif

							HS2DEBUG("rx Nei Solici relay Nei Adv\n\n");
							//dev_kfree_skb_any(newskb);
	                        __rtl8192cd_start_xmit(newskb, priv->dev, 1);
						}
						else
						{				
							HS2DEBUG("v6:da in lan\n");
							#if defined(__KERNEL__) || defined(__OSK__)
	                        if (newskb->dev)
							#ifdef __LINUX_2_6__
		                        newskb->protocol = eth_type_trans(newskb, newskb->dev);
			                else
							#endif
				                newskb->protocol = eth_type_trans(newskb, priv->dev);
							#endif

							#if defined(__LINUX_2_6__) && defined(RX_TASKLET) && !defined(CONFIG_RTL8672) && !(defined(__LINUX_3_10__) || defined(__LINUX_3_2__))
					        netif_receive_skb(newskb);
							#else
						    netif_rx(newskb);
							#endif		
						}
						return 1;
					}
				}
                
			}

            HS2DEBUG("no found!\n");
            
		}
	}
	return 0;
}

int proxy_arp_handle(struct rtl8192cd_priv *priv, struct sk_buff *skb)
{
	struct sk_buff *newskb = NULL;
	struct arphdr *arp = (struct arphdr *)(skb->data + ETH_HLEN);
	unsigned char *arp_ptr = (unsigned char *)(arp + 1), *psender, *ptarget, *psender_bak;
	struct stat_info *pstat;
	struct list_head *phead, *plist;
	int k;
	unsigned char *tmp = (unsigned char *)(skb->data);
	
	/*if((arp->ar_pro == __constant_htons(ETH_P_IP)) && (arp->ar_op == htons(ARPOP_REQUEST)||arp->ar_op == htons(ARPOP_REPLY)))
	{
		arp_ptr += arp->ar_hln;
		psender_bak = arp_ptr;
		//target ip
		arp_ptr += (arp->ar_hln + arp->ar_pln);
		ptarget = arp_ptr;
		if (!memcmp(psender_bak,ptarget,4))
		{
			printk("gratuitous ARP Request or Reply\n");
			return 0;
		}
	}*/
	HS2_DEBUG_TRACE(2, "Proxy ARP handle\n");
	if((arp->ar_pro == __constant_htons(ETH_P_IP)) && (arp->ar_op == htons(ARPOP_REQUEST)))
	{
		//{
		//	int j;
		//	printk("orin==>");
		//	for(j=0;j<skb->len;j++)
		//		printk("0x%02x:",*(skb->data+j));
		//	printk("\n");
		//}
		//sender ip
		arp_ptr += arp->ar_hln;
		psender_bak = arp_ptr;
		//target ip
		arp_ptr += (arp->ar_hln + arp->ar_pln);
		ptarget = arp_ptr;
	
		if (!memcmp(skb->data+ETH_ALEN, priv->pmib->dot11StationConfigEntry.dot11Bssid, ETH_ALEN))
		{
			HS2_DEBUG_TRACE(1, "arp req src mac=BSSID\n");
			return 0;
		}
#ifdef __ECOS
		if( (((unsigned int )ptarget & htonl(0xff000000)) == htonl(0x7f000000)) ||
			(((unsigned int )ptarget & htonl(0xf0000000)) == htonl(0xe0000000)))

#else
		if (ipv4_is_loopback(ptarget) || ipv4_is_multicast(ptarget))
#endif
		{
			HS2_DEBUG_TRACE(1, "loopback or muticast!!\n");
	        return 0;
		}

		//search target ip mapping pstat mac
		phead = &priv->asoc_list;
		plist = phead->next;
		while (phead && (plist != phead)) 
		{
			pstat = list_entry(plist, struct stat_info, asoc_list);
			plist = plist->next;
			HS2_DEBUG_INFO("Proxy ARP: Find Destination in Assocation List, sta_ip=%d.%d.%d.%d\n",pstat->sta_ip[0],pstat->sta_ip[1],pstat->sta_ip[2],pstat->sta_ip[3]);
			if (!memcmp(pstat->sta_ip, ptarget, 4))
			{
				panic_printk("Proxy ARP: Find Destination in Assocation List\n");
#if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL_ETH_PRIV_SKB)
				extern struct sk_buff *priv_skb_copy(struct sk_buff *skb);
				newskb = priv_skb_copy(skb);
#else
				newskb = skb_copy(skb, GFP_ATOMIC);
#endif
				if (newskb == NULL) 
				{
					priv->ext_stats.tx_drops++;
					HS2_DEBUG_ERR("alloc arp rsp skb null!!\n");
					rtl_kfree_skb(priv, skb, _SKB_TX_);
					return 1;
				}	
				else
				{
					// ======================
					// build new arp response
					// ======================
					//da
					memcpy(newskb->data, newskb->data+ETH_ALEN, 6);
					//memcpy(newskb->data, priv->pmib->dot11StationConfigEntry.dot11Bssid, 6);
					//sa
					memcpy(newskb->data+ETH_ALEN, pstat->hwaddr, 6);
					//memcpy(newskb->data+ETH_ALEN, priv->pmib->dot11StationConfigEntry.dot11Bssid, 6);
					//arp response			
					arp = (struct arphdr *)(newskb->data + ETH_HLEN);
					arp_ptr = (unsigned char *)(arp + 1);
					arp->ar_op = htons(ARPOP_REPLY);
					//sender mac and ip
					memcpy(arp_ptr, pstat->hwaddr, 6);
					arp_ptr += arp->ar_hln;
					psender = (unsigned char *)arp_ptr;
					memcpy(psender, ptarget, 4);
					//printk("sender mac and ip:%02x:%02x:%02x:%02x:%02x:%02x,%d.%d.%d.%d\n",pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5],ptarget[0],ptarget[1],ptarget[2],ptarget[3]);
					//target mac and ip
					arp_ptr += arp->ar_pln;
					memcpy(arp_ptr, newskb->data, 6);
					//memcpy(newskb->data, priv->pmib->dot11StationConfigEntry.dot11Bssid, 6);

					arp_ptr += arp->ar_hln;
					ptarget = arp_ptr;
					memcpy((unsigned char *)ptarget, (unsigned char *)psender_bak, 4);
					//printk("target mac and ip:%02x:%02x:%02x:%02x:%02x:%02x,%d.%d.%d.%d\n",newskb->data[0],newskb->data[1],newskb->data[2],newskb->data[3],newskb->data[4],newskb->data[5],ptarget[0],ptarget[1],ptarget[2],ptarget[3]);
							
					rtl_kfree_skb(priv, skb, _SKB_TX_);
					
					if (!memcmp(ptarget, psender, 4))
					{
						HS2_DEBUG_TRACE(2, "target ip = sender ip!!\n");
						dev_kfree_skb_any(newskb);
						return 1;
					}

					if ((pstat = get_stainfo(priv, newskb->data)) != NULL)
					{
						//struct sk_buff_head *pqueue;
					    //struct timer_list *ptimer;
					    //void (*timer_hook)(unsigned long task_priv);

						//if (newskb->dev)
//#ifdef __LINUX_2_6__
//							newskb->protocol = eth_type_trans(newskb, newskb->dev);
//						else
//#endif
//							newskb->protocol = eth_type_trans(newskb, priv->dev);

//						printk("enq\n");
//						pqueue = &pstat->swq.be_queue;
//			            ptimer = &pstat->swq.beq_timer;
//						timer_hook = rtl8192cd_beq_timer;

//					    skb_queue_tail(pqueue, newskb);
//					    ptimer->data = (unsigned long)pstat;
//						ptimer->function = timer_hook; //rtl8190_tmp_timer;
//			            mod_timer(ptimer, jiffies + 1);

//						 SAVE_INT_AND_CLI(x);
						//pstat = get_stainfo(priv, newskb->data);
						HS2_DEBUG_TRACE(1, "da in wlan\n");
				        //__rtl8192cd_start_xmit_out(newskb, pstat);
						newskb->cb[2] = (char)0xff;         // not do aggregation
						memcpy(newskb->cb+10,newskb->data,6);
                        __rtl8192cd_start_xmit(newskb, priv->dev, 1);
//				        RESTORE_INT(x);
	
					}
					else
					{		
						HS2_DEBUG_TRACE(1, "da in lan\n");
#if defined(__KERNEL__) || defined(__OSK__)
						if (newskb->dev)
#ifdef __LINUX_2_6__
							newskb->protocol = eth_type_trans(newskb, newskb->dev);
	                    else
#endif
		                    newskb->protocol = eth_type_trans(newskb, priv->dev);
#endif
			
#if defined(__LINUX_2_6__) && defined(RX_TASKLET) && !defined(CONFIG_RTL8672)
						netif_receive_skb(newskb);
#else
						netif_rx(newskb);
#endif
					}
					return 1;
				}			
			}
		}
	}
	
	//drop packet
	return 0;		
}


#endif
#ifdef CONFIG_RECORD_CLIENT_HOST
void client_host_snooping_bydhcp(struct sk_buff *pskb, struct rtl8192cd_priv *priv)
{
	#define DHCP_MAGIC 0x63825363

struct iphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	        __u8    ihl:4,
	                version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	        __u8    version:4,
	                ihl:4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
	        __u8    tos;
	        __u16   tot_len;
	        __u16   id;
	        __u16   frag_off;
	        __u8    ttl;
	        __u8    protocol;
#if 0
	        __u16   check;
	        __u32   saddr;
	        __u32   daddr;
#endif
};

struct udphdr {
	        __u16   source;
	        __u16   dest;
	        __u16   len;
	        __u16   check;
};

struct dhcpMessage {
		u_int8_t op;
		u_int8_t htype;
		u_int8_t hlen;
		u_int8_t hops;
		u_int32_t xid;
		u_int16_t secs;
		u_int16_t flags;
		u_int32_t ciaddr;
		u_int32_t yiaddr;
		u_int32_t siaddr;
		u_int32_t giaddr;
		u_int8_t chaddr[16];
		u_int8_t sname[64];
		u_int8_t file[128];
		u_int32_t cookie;
#if 1
		u_int8_t options[308]; /* 312 - cookie */
#endif
};

	struct stat_info *pstat;
	struct iphdr* iph;
	struct udphdr *udph;
	struct dhcpMessage *dhcph;
	struct list_head *phead, *plist;
	char find_host_name = 0;
	char find_host_ip = 0;
	iph = (struct iphdr *)(pskb->data + ETH_HLEN);
	udph = (struct udphdr *)((unsigned int)iph + (iph->ihl << 2));
	dhcph = (struct dhcpMessage *)((unsigned int)udph + sizeof(struct udphdr));

    phead = &priv->asoc_list;
    plist = phead->next;
		
    while (phead && (plist != phead))
    {
        pstat = list_entry(plist, struct stat_info, asoc_list);
        plist = plist->next;
		
        if (isEqualMACAddr(pstat->hwaddr, &dhcph->chaddr[0]) && (dhcph->op == 1)) {//dhcp request
			int i,option_len,op_val_addr;
			i = 0;
			option_len = dhcph->options[1];
			op_val_addr = 2;
			
			if(dhcph->options[i] != 53 || dhcph->options[op_val_addr] != 3)/*message request type*/
				return;
			
			i = op_val_addr + option_len;
			while(dhcph->options[i] != 255){/*DHCP end option*/
				option_len = dhcph->options[i+1];
				op_val_addr = i + 2;

				if(dhcph->options[i] == 12){/*Host name*/
					int hostnamelen = option_len>255?255:option_len;
					find_host_name = 1;
					memcpy(pstat->client_host_name, dhcph->options+op_val_addr, hostnamelen);
					pstat->client_host_name[hostnamelen] = 0;
				} else if(dhcph->options[i] == 50){/*Requested IP*/
					find_host_ip = 1;
					memcpy(pstat->client_host_ip, dhcph->options+op_val_addr, 4);
				}
				
				if(find_host_name && find_host_ip)
					return;
				
				i = op_val_addr + option_len;
			}
			
			if(find_host_ip && !find_host_name){
				memcpy(pstat->client_host_name,&dhcph->chaddr[0],6);
				pstat->client_host_name[6] = 0;
			}
			
			return;
		}					
	}
}
#endif
#ifdef USE_TXQUEUE
int init_txq_pool(struct list_head *head, unsigned char **ppool)
{
	unsigned char *ptr;
	unsigned int i;
	struct txq_node *pnode;

	INIT_LIST_HEAD(head);
	
	ptr = kmalloc(TXQUEUE_SIZE * sizeof(struct txq_node), GFP_ATOMIC);
	if (!ptr) {
		printk("ERRORL: %s failed\n", __FUNCTION__);
		*ppool = NULL;
		return -1;
	}

	pnode = (struct txq_node *)ptr;
	for (i=0; i<TXQUEUE_SIZE; i++)
	{
		pnode[i].skb = NULL;
		pnode[i].dev = NULL;
		list_add_tail(&(pnode[i].list), head);
	}

	*ppool = ptr;
	return 0;
}

void free_txq_pool(struct list_head *head, unsigned char *ppool)
{
	if (ppool)
		kfree(ppool);
	INIT_LIST_HEAD(head);
}

void append_skb_to_txq_head(struct txq_list_head *head, struct rtl8192cd_priv *priv, struct sk_buff *skb, struct net_device *dev, struct list_head *pool)
{
	struct txq_node *pnode = NULL;

	if (list_empty(pool))
	{
		DEBUG_ERR("%s: No unused node in pool, this should not happend, fix me.\n", __FUNCTION__);
		rtl_kfree_skb(priv, skb, _SKB_TX_);
		DEBUG_ERR("TX DROP: exceed the tx queue!\n");
		priv->ext_stats.tx_drops++;		
		return;
	}

	pnode = (struct txq_node *)pool->next;
	list_del(pool->next);
	pnode->skb = skb;
	pnode->dev = dev;
	
	add_txq_head(head, pnode);
}

void append_skb_to_txq_tail(struct txq_list_head *head, struct rtl8192cd_priv *priv, struct sk_buff *skb, struct net_device *dev, struct list_head *pool)
{
	struct txq_node *pnode = NULL;

	if (list_empty(pool))
	{
		DEBUG_ERR("%s: No unused node in pool, this should not happend, fix me.\n", __FUNCTION__);
		rtl_kfree_skb(priv, skb, _SKB_TX_);
		DEBUG_ERR("TX DROP: exceed the tx queue!\n");
		priv->ext_stats.tx_drops++;		
		return;
	}

	pnode = (struct txq_node *)pool->next;
	list_del(pool->next);
	pnode->skb = skb;
	pnode->dev = dev;
	
	add_txq_tail(head, pnode);
}

void remove_skb_from_txq(struct txq_list_head *head, struct sk_buff **pskb, struct net_device **pdev, struct list_head *pool)
{
	struct txq_node *pnode = NULL;

	if (is_txq_empty(head))
	{
		*pskb = NULL;
		*pdev = NULL;
		return;
	}

	pnode = deq_txq(head);
	*pskb = pnode->skb;
	*pdev = pnode->dev;
	pnode->skb = NULL;
	pnode->dev = NULL;

	list_add_tail(&pnode->list, pool);
}

#endif


#ifdef TLN_STATS
void stats_conn_rson_counts(struct rtl8192cd_priv *priv, unsigned int reason)
{
	switch (reason) {
	case _RSON_UNSPECIFIED_:
		priv->ext_wifi_stats.rson_UNSPECIFIED_1++;
		break;
	case _RSON_AUTH_NO_LONGER_VALID_:
		priv->ext_wifi_stats.rson_AUTH_INVALID_2++;
		break;
	case _RSON_DEAUTH_STA_LEAVING_:
		priv->ext_wifi_stats.rson_DEAUTH_STA_LEAVING_3++;
		break;
	case _RSON_INACTIVITY_:
		priv->ext_wifi_stats.rson_INACTIVITY_4++;
		break;
	case _RSON_UNABLE_HANDLE_:
		priv->ext_wifi_stats.rson_RESOURCE_INSUFFICIENT_5++;
		break;
	case _RSON_CLS2_:
		priv->ext_wifi_stats.rson_UNAUTH_CLS2FRAME_6++;
		break;
	case _RSON_CLS3_:
		priv->ext_wifi_stats.rson_UNAUTH_CLS3FRAME_7++;
		break;
	case _RSON_DISAOC_STA_LEAVING_:
		priv->ext_wifi_stats.rson_DISASSOC_STA_LEAVING_8++;
		break;
	case _RSON_ASOC_NOT_AUTH_:
		priv->ext_wifi_stats.rson_ASSOC_BEFORE_AUTH_9++;
		break;
	case _RSON_INVALID_IE_:
		priv->ext_wifi_stats.rson_INVALID_IE_13++;
		break;
	case _RSON_MIC_FAILURE_:
		priv->ext_wifi_stats.rson_MIC_FAILURE_14++;
		break;
	case _RSON_4WAY_HNDSHK_TIMEOUT_:
		priv->ext_wifi_stats.rson_4WAY_TIMEOUT_15++;
		break;
	case _RSON_GROUP_KEY_UPDATE_TIMEOUT_:
		priv->ext_wifi_stats.rson_GROUP_KEY_TIMEOUT_16++;
		break;
	case _RSON_DIFF_IE_:
		priv->ext_wifi_stats.rson_DIFF_IE_17++;
		break;
	case _RSON_MLTCST_CIPHER_NOT_VALID_:
		priv->ext_wifi_stats.rson_MCAST_CIPHER_INVALID_18++;
		break;
	case _RSON_UNICST_CIPHER_NOT_VALID_:
		priv->ext_wifi_stats.rson_UCAST_CIPHER_INVALID_19++;
		break;
	case _RSON_AKMP_NOT_VALID_:
		priv->ext_wifi_stats.rson_AKMP_INVALID_20++;
		break;
	case _RSON_UNSUPPORT_RSNE_VER_:
		priv->ext_wifi_stats.rson_UNSUPPORT_RSNIE_VER_21++;
		break;
	case _RSON_INVALID_RSNE_CAP_:
		priv->ext_wifi_stats.rson_RSNIE_CAP_INVALID_22++;
		break;
	case _RSON_IEEE_802DOT1X_AUTH_FAIL_:
		priv->ext_wifi_stats.rson_802_1X_AUTH_FAIL_23++;
		break;
	default:
		priv->ext_wifi_stats.rson_OUT_OF_SCOPE++;
		/*panic_printk("incorrect reason(%d) for statistics\n", reason);*/
		break;
	}

	priv->wifi_stats.rejected_sta++;
}


void stats_conn_status_counts(struct rtl8192cd_priv *priv, unsigned int status)
{
	switch (status) {
	case _STATS_SUCCESSFUL_:
		priv->wifi_stats.connected_sta++;
		break;
	case _STATS_FAILURE_:
		priv->ext_wifi_stats.status_FAILURE_1++;
		break;
	case _STATS_CAP_FAIL_:
		priv->ext_wifi_stats.status_CAP_FAIL_10++;
		break;
	case _STATS_NO_ASOC_:
		priv->ext_wifi_stats.status_NO_ASSOC_11++;
		break;
	case _STATS_OTHER_:
		priv->ext_wifi_stats.status_OTHER_12++;
		break;
	case _STATS_NO_SUPP_ALG_:
		priv->ext_wifi_stats.status_NOT_SUPPORT_ALG_13++;
		break;
	case _STATS_OUT_OF_AUTH_SEQ_:
		priv->ext_wifi_stats.status_OUT_OF_AUTH_SEQ_14++;
		break;
	case _STATS_CHALLENGE_FAIL_:
		priv->ext_wifi_stats.status_CHALLENGE_FAIL_15++;
		break;
	case _STATS_AUTH_TIMEOUT_:
		priv->ext_wifi_stats.status_AUTH_TIMEOUT_16++;
		break;
	case _STATS_UNABLE_HANDLE_STA_:
		priv->ext_wifi_stats.status_RESOURCE_INSUFFICIENT_17++;
		break;
	case _STATS_RATE_FAIL_:
		priv->ext_wifi_stats.status_RATE_FAIL_18++;
		break;
	default:
		priv->ext_wifi_stats.status_OUT_OF_SCOPE++;
		/*panic_printk("incorrect status(%d) for statistics\n", status);*/
		break;
	}

	if (status != _STATS_SUCCESSFUL_)
		priv->wifi_stats.rejected_sta++;
}
#endif


#ifdef SW_TX_QUEUE

void turbo_swq_setting(struct rtl8192cd_priv *priv) 
{
    struct stat_info    *pstat;
    int i,j;
    for(i= 0; i<NUM_STAT; i++) {
        if (priv->pshare->aidarray[i] && priv->pshare->aidarray[i]->used == TRUE) {
            pstat = &(priv->pshare->aidarray[i]->station);
            for(j=BK_QUEUE;j<HIGH_QUEUE;j++)
                pstat->swq.q_aggnum[j] = priv->pshare->rf_ft_var.swqturboaggnum;
        }
    }
}
void adjust_swq_setting(struct rtl8192cd_priv *priv, struct stat_info *pstat, int i, int mode)
{
    int thd, step; 
    int maxAggNum;
    if(pstat->swq.q_used[i]) {
#if (MU_BEAMFORMING_SUPPORT == 1)
		if(pstat->muPartner)
			pstat->swq.q_aggnum[i] = priv->pshare->rf_ft_var.qlmt;
		else
#endif			
        if(priv->pshare->swq_use_hw_timer && priv->pshare->swq_numActiveSTA > 8 && 
           (priv->up_time - priv->pshare->swq_turbo_time) < priv->pshare->rf_ft_var.swqmaxturbotime) {
            pstat->swq.q_aggnum[i] = priv->pshare->rf_ft_var.swqturboaggnum;
        }
        else if (mode == CHECK_DEC_AGGN) {
            if(priv->pshare->swq_use_hw_timer) {
                thd = SWQ_TIMEOUT_THD / pstat->swq.swq_prev_timeout[i];    
                if(thd > 100)
                thd = 100;
                if ((pstat->swq.q_TOCount[i] >= thd)&& ((pstat->swq.q_TOCount[i] % thd) == 0)) {                   
                    if (pstat->swq.q_aggnum[i] > 1) {
                        --(pstat->swq.q_aggnum[i]); 
                        pstat->swq.swq_keeptime[i] = priv->up_time + 3;
                    }
                    pstat->swq.swq_timeout_change[i] = 0;                    
                }                
            }
            else {
                if (pstat->swq.q_aggnum[i] <= 2)
                    thd = priv->pshare->rf_ft_var.timeout_thd;
                else if (pstat->swq.q_aggnum[i] <= 4)
                    thd = priv->pshare->rf_ft_var.timeout_thd2;
                else
                    thd = priv->pshare->rf_ft_var.timeout_thd3;

                if ((pstat->swq.q_TOCount[i] >= thd)&& ((pstat->swq.q_TOCount[i] % thd) == 0)) {
                    --(pstat->swq.q_aggnum[i]); 
                    if (pstat->swq.q_aggnum[i] <= 2)
                        pstat->swq.q_aggnum[i] = 2;
                    if (++pstat->swq.q_aggnumIncSlow[i] >= MAX_BACKOFF_CNT)
                        pstat->swq.q_aggnumIncSlow[i] = MAX_BACKOFF_CNT;
                    DEBUG_INFO("dec,aid:%d,cnt:%d\n", pstat->aid, pstat->swq.q_TOCount[i]);
                }
            }
        }
        else {
            if(priv->pshare->swq_use_hw_timer) {
                if(priv->pshare->swq_numActiveSTA > 8)
                    maxAggNum = priv->pshare->rf_ft_var.swqturboaggnum;
                else
                    maxAggNum = priv->pshare->rf_ft_var.swq_aggnum;

                if(pstat->swq.q_aggnum[i] < maxAggNum && 
                   pstat->swq.q_TOCount[i] < SWQ_TIMEOUT_THD_LOWER) {
                    if(pstat->swq.swq_timeout_change[i]) {
                        step = (maxAggNum - pstat->swq.q_aggnum[i] + 1)/2;
                        if(step < 2)
                            step = 2;
                        pstat->swq.q_aggnum[i] += step;
                        if (pstat->swq.q_aggnum[i] > maxAggNum) {                    
                            pstat->swq.q_aggnum[i] = maxAggNum;
                        }        
                    }
                    else {
                        if(priv->up_time > pstat->swq.swq_keeptime[i])
                            pstat->swq.q_aggnum[i]++;                        
                    }
                }			
            }
            else {
                if (pstat->swq.q_aggnum[i] <= 2)
                    thd = priv->pshare->rf_ft_var.timeout_thd-10;
                else if (pstat->swq.q_aggnum[i] <= 4)
                    thd = priv->pshare->rf_ft_var.timeout_thd2-30;
                else
                    thd = priv->pshare->rf_ft_var.timeout_thd3-50;

                if(pstat->swq.q_TOCount[i]< thd) {
                    step = 1;
                if(pstat->swq.q_TOCount[i]< thd/5)
                    step = 5;
                else if(pstat->swq.q_TOCount[i]< thd/3)
                    step = 3;

                pstat->swq.q_aggnum[i] += step; 

                if (pstat->swq.q_aggnum[i] > priv->pshare->rf_ft_var.swq_aggnum)
                    pstat->swq.q_aggnum[i] = priv->pshare->rf_ft_var.swq_aggnum;
                    //panic_printk("inc,aid:%d,cnt:%d,%d\n", pstat->aid, pstat->swq.q_TOCount[i], pstat->swq.q_aggnum[BE_QUEUE]);
                }
            }
        }
    }
}

void init_STA_SWQAggNum(struct rtl8192cd_priv *priv)
{
    struct stat_info    *pstat;
    int i,j;
    for(i= 0; i<NUM_STAT; i++) {
        if (priv->pshare->aidarray[i] && priv->pshare->aidarray[i]->used == TRUE) {
            pstat = &(priv->pshare->aidarray[i]->station);
            for(j=BK_QUEUE;j<HIGH_QUEUE;j++) {
                if(priv->pshare->swq_numActiveSTA > 8)
                    pstat->swq.q_aggnum[j] = 16;
                if(pstat->tx_avarage> (1<<16)) {
                    pstat->swq.q_aggnum[j] = priv->pshare->rf_ft_var.swq_aggnum>>1;                               
                }
                if(priv->pshare->swq_use_hw_timer) {
                    pstat->swq.swq_en[j] = 1;
                    pstat->swq.swq_prev_timeout[j] = 1;
                    pstat->swq.swq_timeout_change[j] = 0;
                    pstat->swq.swq_keeptime[j] = 0;         
                }
            }
        }
    }
}
#endif //SW_TX_QUEUE

#if defined(CONFIG_RTL_ULINKER)
int get_wlan_opmode(struct net_device *dev)
{
	int opmode = -1;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)dev->priv;

	if (netif_running(dev)) {
		if ((priv->pmib->dot11OperationEntry.opmode) & WIFI_AP_STATE)
			opmode = 0;
		else
			opmode = 1;
	}

	return opmode;
}
#endif



#ifdef RF_MIMO_SWITCH

void Do_BB_Reset(struct rtl8192cd_priv *priv)
{
	unsigned char tmp_reg2 = 0;
	tmp_reg2 = RTL_R8(0x2);
			
	tmp_reg2 &= (~BIT(0));
	RTL_W8(0x2, tmp_reg2);
	tmp_reg2 |= BIT(0);
	RTL_W8(0x2, tmp_reg2);

}

void Assert_BB_Reset(struct rtl8192cd_priv *priv)
{
	unsigned char tmp_reg2 = 0;
	tmp_reg2 = RTL_R8(0x2);

	tmp_reg2 &= (~BIT(0));
	RTL_W8(0x2, tmp_reg2);
}

void Release_BB_Reset(struct rtl8192cd_priv *priv)
{
	unsigned char tmp_reg2 = 0;
	tmp_reg2 = RTL_R8(0x2);

	tmp_reg2 |= BIT(0);
	RTL_W8(0x2, tmp_reg2);
}

void RF_MIMO_check_timer(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	int i=0, assoc_num = priv->assoc_num;
	
	if (!(priv->drv_state & DRV_STATE_OPEN))
		return;

	if(get_rf_mimo_mode(priv) != MIMO_2T2R && get_rf_mimo_mode(priv) != MIMO_3T3R)
		return;

    if(priv->auto_channel || timer_pending(&priv->ss_timer)) {
        goto end;
    }
#ifdef MCR_WIRELESS_EXTEND
	return;
#endif 
#ifdef MP_TEST
	if (((OPMODE & WIFI_MP_STATE) || priv->pshare->rf_ft_var.mp_specific))
		return;
#endif	
		if(0 
#ifdef WDS
		|| (priv->pmib->dot11WdsInfo.wdsEnabled)
#endif
#ifdef DFS
		|| is_DFS_channel(priv->pmib->dot11RFEntry.dot11channel)
#endif
#ifdef UNIVERSAL_REPEATER
		|| (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
#endif
		) {
			if(priv->pshare->rf_status == MIMO_1T1R && get_rf_mimo_mode(priv) == MIMO_2T2R){
				set_MIMO_Mode(priv, MIMO_2T2R);
			}
			return;	
		}

	if(priv->pshare->rf_ft_var.rf_mode ==0) {
#ifdef MBSSID
		if (priv->pmib->miscEntry.vap_enable){
			for (i=0; i<RTL8192CD_NUM_VWLAN; ++i)
				assoc_num += priv->pvap_priv[i]-> assoc_num;
		}
#endif
		if(assoc_num) {
			if(get_rf_mimo_mode(priv) == MIMO_3T3R && priv->pshare->rf_status == MIMO_2T2R)
				set_MIMO_Mode(priv, MIMO_3T3R);
			else if( get_rf_mimo_mode(priv) == MIMO_2T2R && priv->pshare->rf_status == MIMO_1T1R)
				set_MIMO_Mode(priv, MIMO_2T2R);
		}
		else {
#if defined(WIFI_11N_2040_COEXIST_EXT)
			if((priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_20_40)|| (priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_80)) {
				priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
				SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
				SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
			}
#endif	
			if(get_rf_mimo_mode(priv) == MIMO_3T3R && priv->pshare->rf_status != MIMO_2T2R) {
				set_MIMO_Mode(priv, MIMO_2T2R);
			}else if (get_rf_mimo_mode(priv) == MIMO_2T2R  && priv->pshare->rf_status != MIMO_1T1R)
				 set_MIMO_Mode(priv, MIMO_1T1R);
			
		}
	} else if (priv->pshare->rf_ft_var.rf_mode == 2) {
		set_MIMO_Mode(priv, MIMO_2T2R);
	} else if (priv->pshare->rf_ft_var.rf_mode == 1) {
		set_MIMO_Mode(priv, MIMO_1T1R);
	}else if (priv->pshare->rf_ft_var.rf_mode == 3) {
		set_MIMO_Mode(priv, MIMO_3T3R);
	}

 end:
	mod_timer(&priv->ps_timer, jiffies + IDLE_T0);
}

int assign_MIMO_TR_Mode(struct rtl8192cd_priv *priv, unsigned char *data)
{
#define dprintf printk
	int mode = _atoi(data, 16);
	if (strlen(data) == 0) {
		printk("tr mode.\n");
		printk("mimo 1: swith to 1T\n");
		printk("mimo 2: switch to 2T\n");
		printk("mimo 3: switch to 3T\n");
		printk("mimo 0: auto\n");
		return 0;
	}
	if (mode == 0x01)	{
		MIMO_Mode_Switch(priv, MIMO_1T1R);
		priv->pshare->rf_ft_var.rf_mode = 1;
	} else if (mode == 0x02)	 {		
		MIMO_Mode_Switch(priv, MIMO_2T2R);
		priv->pshare->rf_ft_var.rf_mode = 2;
	} else if (mode == 0x03)	 {		
		MIMO_Mode_Switch(priv, MIMO_3T3R);
		priv->pshare->rf_ft_var.rf_mode = 3;
	} else {
		priv->pshare->rf_ft_var.rf_mode = 0;
	}
	return 0;
}
#endif

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
struct list_head* asoc_list_get_next(struct rtl8192cd_priv *priv, struct list_head *plist)
{
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif
	struct list_head *phead, *plist_next;
	struct stat_info *pstat;
	
	phead = &priv->asoc_list;
	
	SMP_LOCK_ASOC_LIST(flags);
	
	plist_next = plist->next;
	if (plist_next != phead) {
		pstat = list_entry(plist_next, struct stat_info, asoc_list);
		pstat->asoc_list_refcnt++;
	}
	
	SMP_UNLOCK_ASOC_LIST(flags);
	
	if (plist != phead) {
		pstat =  list_entry(plist, struct stat_info, asoc_list);
		asoc_list_unref(priv, pstat);
	}
	
	return plist_next;
}

void asoc_list_unref(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif
	int unlink = 0;
	
	SMP_LOCK_ASOC_LIST(flags);
	
	BUG_ON(0 == pstat->asoc_list_refcnt);
	
	pstat->asoc_list_refcnt--;
	if (0 == pstat->asoc_list_refcnt) {
		list_del_init(&pstat->asoc_list);
		unlink = 1;
	}
	
	SMP_UNLOCK_ASOC_LIST(flags);
	
	if (unlink) {
#ifdef __ECOS
		cyg_flag_setbits(&pstat->asoc_unref_done, 0x1);
#else
		complete(&pstat->asoc_unref_done);
#endif
	}
}

// The function returns whether it had linked to asoc_list before removing.
int asoc_list_del(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif
	int ret = 0, wait = 0;
	
#ifdef __KERNEL__
	might_sleep();
#endif

	SMP_LOCK_ASOC_LIST(flags);
	
	if (0 != pstat->asoc_list_refcnt) {
		pstat->asoc_list_refcnt--;
		if (0 ==pstat->asoc_list_refcnt) {
//			if (!list_empty(&pstat->asoc_list)) {
				list_del_init(&pstat->asoc_list);
				ret = 1;
//			}
		} else {
			wait = 1;
		}
	}
	
	SMP_UNLOCK_ASOC_LIST(flags);
	
	if (wait) {
#ifdef __ECOS
		cyg_flag_wait(&pstat->asoc_unref_done, 0x01, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
#else
		wait_for_completion(&pstat->asoc_unref_done);
#endif
		ret = 1;
	}
	
	return ret;
}

int asoc_list_add(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
#ifdef SMP_SYNC
	unsigned long flags = 0;
#endif
	int ret = 0;
	
	SMP_LOCK_ASOC_LIST(flags);
	
	if (list_empty(&pstat->asoc_list)) {
		list_add_tail(&pstat->asoc_list, &priv->asoc_list);
		pstat->asoc_list_refcnt = 1;
		ret = 1;
	}
	
	SMP_UNLOCK_ASOC_LIST(flags);
	
	return ret;
}
#endif // CONFIG_USB_HCI || CONFIG_SDIO_HCI

#ifdef WIFI_SIMPLE_CONFIG
#define IGNORE_DISCON_TIMEOUT		3
void wsc_disconn_list_add(struct rtl8192cd_priv *priv, unsigned char *hwaddr)
{
	struct wsc_disconn_entry *entry;
	unsigned long flags;
	
	entry = kmalloc(sizeof(*entry), GFP_ATOMIC);
	if (NULL == entry)
		return;

	INIT_LIST_HEAD(&entry->list);
	memcpy(entry->addr, hwaddr, MACADDRLEN);
	entry->state = WSC_DISCON_STATE_RECV_EAP_FAIL;
	entry->expire_to = IGNORE_DISCON_TIMEOUT;
	
	spin_lock_irqsave(&priv->wsc_disconn_list_lock, flags);

	list_add_tail(&entry->list, &priv->wsc_disconn_list);

	spin_unlock_irqrestore(&priv->wsc_disconn_list_lock, flags);
}

void wsc_disconn_list_expire(struct rtl8192cd_priv *priv)
{
	struct list_head *phead, *plist;
	struct wsc_disconn_entry *entry;
	unsigned long flags;

	phead = &priv->wsc_disconn_list;
	
	if (list_empty(phead))
		return;

	spin_lock_irqsave(&priv->wsc_disconn_list_lock, flags);

	plist = phead->next;
	
	while (plist != phead) {
		entry = list_entry(plist, struct wsc_disconn_entry, list);
		plist = plist->next;
		
		entry->expire_to--;
		if(priv->pshare->rf_ft_var.fix_expire_to_zero == 1) {
			entry->expire_to = 0;
			priv->pshare->rf_ft_var.fix_expire_to_zero = 0;
		}		
		if (0 == entry->expire_to) {
			list_del(&entry->list);
			kfree(entry);
		}
	}

	spin_unlock_irqrestore(&priv->wsc_disconn_list_lock, flags);
}

void wsc_disconn_list_update(struct rtl8192cd_priv *priv, unsigned char *hwaddr)
{
	struct list_head *phead, *plist;
	struct wsc_disconn_entry *entry;
	unsigned long flags;

	phead = &priv->wsc_disconn_list;
	
	if (list_empty(phead))
		return;

	spin_lock_irqsave(&priv->wsc_disconn_list_lock, flags);

	plist = phead->next;
	
	while (plist != phead) {
		entry = list_entry(plist, struct wsc_disconn_entry, list);
		plist = plist->next;
		
		if (!memcmp(entry->addr, hwaddr, MACADDRLEN)) {
			if (WSC_DISCON_STATE_RECV_EAP_FAIL == entry->state)
				entry->state = WSC_DISCON_STATE_IGNORE;
			break;
		}
	}

	spin_unlock_irqrestore(&priv->wsc_disconn_list_lock, flags);
}

int wsc_disconn_list_check(struct rtl8192cd_priv *priv, unsigned char *hwaddr)
{
	struct list_head *phead, *plist;
	struct wsc_disconn_entry *entry;
	unsigned long flags;
	int status;

	phead = &priv->wsc_disconn_list;

	if (list_empty(phead))
		return FALSE;
	
	spin_lock_irqsave(&priv->wsc_disconn_list_lock, flags);

	plist = phead->next;
	
	while (plist != phead) {
		entry = list_entry(plist, struct wsc_disconn_entry, list);
		plist = plist->next;
		
		if (!memcmp(entry->addr, hwaddr, MACADDRLEN)) {
			list_del(&entry->list);
			spin_unlock_irqrestore(&priv->wsc_disconn_list_lock, flags);
			
			status = (WSC_DISCON_STATE_IGNORE == entry->state) ? TRUE : FALSE;
			kfree(entry);
			return status;
		}
	}

	spin_unlock_irqrestore(&priv->wsc_disconn_list_lock, flags);

	return FALSE;
}

void wsc_disconn_list_flush(struct rtl8192cd_priv *priv)
{
	struct list_head *phead, *plist;
	struct wsc_disconn_entry *entry;
	unsigned long flags;

	phead = &priv->wsc_disconn_list;

	if (list_empty(phead))
		return;

	spin_lock_irqsave(&priv->wsc_disconn_list_lock, flags);

	plist = phead->next;
	
	while (plist != phead) {
		entry = list_entry(plist, struct wsc_disconn_entry, list);
		plist = plist->next;
		kfree(entry);
	}

	spin_unlock_irqrestore(&priv->wsc_disconn_list_lock, flags);
}
#endif // WIFI_SIMPLE_CONFIG

//This function check all AP interfaces' status
// func_map: bit0~bit15 : AP interface active or not, bit 16~bit31:active connection or not
int checkAPfunc(struct rtl8192cd_priv *priv, unsigned int *func_map)
{
	struct rtl8192cd_priv *root_priv=GET_ROOT(priv);
	unsigned int idx=0;

	if(root_priv->pmib->miscEntry.func_off==0) {
		*func_map |= BIT0;

		if(root_priv->assoc_num)
			*func_map |= BIT16;
	}
#if defined(MBSSID)
	if(root_priv->pmib->miscEntry.vap_enable) {
		for (idx=0; idx<RTL8192CD_NUM_VWLAN; idx++) {
			if (IS_DRV_OPEN(root_priv->pvap_priv[idx])&& 
				(root_priv->pvap_priv[idx]->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE)) {
				if(root_priv->pvap_priv[idx]->pmib->miscEntry.func_off == 0) {
					*func_map |= (BIT0<<(idx+1));

					if(root_priv->pvap_priv[idx]->assoc_num)
						*func_map |= (BIT16<<(idx+1));
				}
			}
		}
	}
#endif

	return 0;
}

//return value: Auto site-survey level
//0(SS_LV_WSTA), STA connect to root AP/VAP
//1(SS_LV_WOSTA), No STA connected to root AP/VAP
//2(SS_LV_ROOTFUNCOFF), root AP only and func_off=1
int get_ss_level(struct rtl8192cd_priv *priv)
{
	int idx=0, ss_level=SS_LV_WSTA;
	unsigned int func_map=0;
	checkAPfunc(priv, &func_map);

	if(func_map & 0xffff0000)
		ss_level=SS_LV_WSTA;
	else if(func_map & 0xffff)
		ss_level=SS_LV_WOSTA;
	else
		ss_level=SS_LV_ROOTFUNCOFF;

	STADEBUG("[%s]checkAPfunc:%04x, ss_level:%d",priv->dev->name,func_map,ss_level);

	return ss_level;
}

void syncMulticastCipher(struct rtl8192cd_priv *priv, struct bss_desc *bss_target)
{
	int mcipher = 1;
	// set Multicast Cipher as same as AP's
	if (priv->pmib->dot11RsnIE.rsnie[0] == _RSN_IE_1_) {
		if(bss_target->t_stamp[0] & BIT(4))
			mcipher = 4;
		else if(bss_target->t_stamp[0] & BIT(2))
			mcipher = 2;									
		priv->pmib->dot11RsnIE.rsnie[11] = mcipher;
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
		priv->wpa_global_info->AuthInfoBuf[11] = mcipher;
#endif
	} else if(priv->pmib->dot11RsnIE.rsnie[0] == _RSN_IE_2_) {

#ifdef CONFIG_IEEE80211W_CLI 

		if((bss_target->t_stamp[1] & 0x600) == PMF_REQ) {
			PMFDEBUG("AP PMF capability = Required, AKMP=0x%x \n", priv->bss_support_akmp);
			priv->bss_support_pmf = MGMT_FRAME_PROTECTION_REQUIRED;
		}
		else if((bss_target->t_stamp[1] & 0x600) == PMF_CAP) {
			PMFDEBUG("AP PMF capability = MFPC, AKMP=0x%x \n", priv->bss_support_akmp);
			priv->bss_support_pmf = MGMT_FRAME_PROTECTION_OPTIONAL;
		}
		else if((bss_target->t_stamp[1] & 0x600) == PMF_NONE) {
			PMFDEBUG("AP PMF capability = NONE, AKMP=0x%x\n", priv->bss_support_akmp);
			priv->bss_support_pmf = NO_MGMT_FRAME_PROTECTION;
		}
				
		{
			WPA_GLOBAL_INFO *pGblInfo=priv->wpa_global_info;
			ConstructIE(priv, pGblInfo->AuthInfoElement.Octet,
						 &pGblInfo->AuthInfoElement.Length);
			ToDrv_SetIE(priv);
		}
		
#endif

		if(bss_target->t_stamp[0] & BIT(20))
			mcipher = 4;
		else if(bss_target->t_stamp[0] & BIT(18))
			mcipher = 2;		
		priv->pmib->dot11RsnIE.rsnie[7] = mcipher;
		
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD) || defined(RTK_NL80211)
		priv->wpa_global_info->AuthInfoBuf[7] = mcipher;		
#endif	
			
	}			
}

unsigned int isDHCPpkt(struct sk_buff *pskb)
{
#define DHCP_MAGIC 0x63825363

	struct iphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	        __u8    ihl:4,
	                version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	        __u8    version:4,
	                ihl:4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
	        __u8    tos;
	        __u16   tot_len;
	        __u16   id;
	        __u16   frag_off;
	        __u8    ttl;
	        __u8    protocol;
#if 0
	        __u16   check;
	        __u32   saddr;
	        __u32   daddr;
#endif
	};

	struct udphdr {
	        __u16   source;
	        __u16   dest;
	        __u16   len;
	        __u16   check;
	};

	struct dhcpMessage {
		u_int8_t op;
		u_int8_t htype;
		u_int8_t hlen;
		u_int8_t hops;
		u_int32_t xid;
		u_int16_t secs;
		u_int16_t flags;
		u_int32_t ciaddr;
		u_int32_t yiaddr;
		u_int32_t siaddr;
		u_int32_t giaddr;
		u_int8_t chaddr[16];
		u_int8_t sname[64];
		u_int8_t file[128];
		u_int32_t cookie;
#if 0
		u_int8_t options[308]; /* 312 - cookie */
#endif
	};

	unsigned short protocol = 0;
	struct iphdr* iph;
	struct udphdr *udph;
	struct dhcpMessage *dhcph;

	protocol = *((unsigned short *)(pskb->data + 2 * ETH_ALEN));

	if(protocol == __constant_htons(ETH_P_IP)) { /* IP */
		iph = (struct iphdr *)(pskb->data + ETH_HLEN);

		if(iph->protocol == 17) { /* UDP */
			udph = (struct udphdr *)((unsigned long)iph + (iph->ihl << 2));
			dhcph = (struct dhcpMessage *)((unsigned long)udph + sizeof(struct udphdr));

			if ((unsigned long)dhcph & 0x03) { //not 4-byte alignment
				u_int32_t cookie;
				char *pdhcphcookie;
				char *pcookie = (char *)&cookie;

				pdhcphcookie = (char *)&dhcph->cookie;
				pcookie[0] = pdhcphcookie[0];
				pcookie[1] = pdhcphcookie[1];
				pcookie[2] = pdhcphcookie[2];
				pcookie[3] = pdhcphcookie[3];
				if(cookie == htonl(DHCP_MAGIC))
					return TRUE;
			}
			else {
				if(dhcph->cookie == htonl(DHCP_MAGIC))
					return TRUE;
			}
		}
	}

	return FALSE;
}

#ifdef UNIVERSAL_REPEATER
#if defined(__OSK__) || defined (__ECOS) || defined(RTK_NL80211) || (defined(CONFIG_OPENWRT_SDK) && defined(__LINUX_3_10__))//wrt-dhcp  //TBD //eric-sync ?? __LINUX_3_10__
int send_arp_response(struct rtl8192cd_priv *priv, unsigned int *dip, unsigned int *sip, unsigned char *dmac, unsigned char *smac)
{
	return -1;
}
#else
int send_arp_response(struct rtl8192cd_priv *priv, unsigned int *dip, unsigned int *sip, unsigned char *dmac, unsigned char *smac)
{
	unsigned int ret = -1;
	struct sk_buff *arp_skb = NULL;
	struct rtl_arphdr *arp=NULL;
	unsigned char *ptr;
	int hlen, tlen;

	hlen = LL_RESERVED_SPACE(priv->dev);
	tlen = priv->dev->needed_tailroom;
	arp_skb = __alloc_skb(arp_hdr_len(priv->dev) + hlen + tlen, GFP_ATOMIC, 0, -1);

	if (arp_skb == NULL)
		goto err_out;

	skb_reserve(arp_skb, LL_RESERVED_SPACE(priv->dev));
	skb_reset_network_header(arp_skb);
	arp = (struct rtl_arphdr *) skb_put(arp_skb, arp_hdr_len(priv->dev));
	arp_skb->dev = priv->dev;
	arp_skb->protocol = htons(ETH_P_ARP);
#if 0
	//without consideration of VLAN
//#ifdef CONFIG_RTK_VLAN_SUPPORT
	arp_skb->tag = arp_tag;
#if defined(CONFIG_RTK_BRIDGE_VLAN_SUPPORT)
	arp_skb->src_info = arp_info;
#endif
#endif
	if (dev_hard_header(arp_skb, priv->dev, ETH_P_ARP, dmac, smac, arp_skb->len) < 0) {
		kfree_skb(arp_skb);
		goto err_out;
	}
	// ======================
	// build new arp response
	// ======================
	//arp response
	ptr = (unsigned char *)(arp + 1);
	arp->ar_op = htons(ARPOP_REPLY);
	arp->ar_hrd = htons(priv->dev->type);
	arp->ar_pro = htons(ETH_P_IP);
	arp->ar_hln = priv->dev->addr_len;;
	arp->ar_pln = 4;	//for ipv4
	//sender mac and ip
	memcpy(ptr, smac, 6);
	ptr += arp->ar_hln;
	memcpy(ptr, sip, 4);
	//printk("sender mac and ip:%02x:%02x:%02x:%02x:%02x:%02x,%d.%d.%d.%d\n",pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5],pstat->sta_ip[0],pstat->sta_ip[1],pstat->sta_ip[2],pstat->sta_ip[3]);
	//target mac and ip
	ptr += arp->ar_pln;
	memcpy(ptr, dmac, MACADDRLEN);
	ptr += arp->ar_hln;
	memcpy(ptr, dip, 4);
	//printk("target mac and ip:%02x:%02x:%02x:%02x:%02x:%02x,%d.%d.%d.%d\n",newskb->data[0],newskb->data[1],newskb->data[2],newskb->data[3],newskb->data[4],newskb->data[5],ptarget[0],ptarget[1],ptarget[2],ptarget[3]);

#ifdef MULTI_MAC_CLONE
	*(unsigned int *)&(arp_skb->cb[40]) = 0x86518192;
#endif

	if(arp_skb){
		int i=0;       
		arp_skb->cb[2] = (char)0xff;         // not do aggregation
		memcpy(arp_skb->cb+10,arp_skb->data,6);
		SMP_LOCK_XMIT(x);
		if(__rtl8192cd_start_xmit(arp_skb, priv->dev, 1)) {
			DEBUG_ERR("%s %d ARP response sent failed\n",__func__,__LINE__);
		} else {
			ret = 0;
			DEBUG_INFO("%s %d ARP response for %02x:%02x:%02x:%02x:%02x:%02x to %s sent\n",__func__,__LINE__,
				smac[0],smac[1],smac[2],smac[3],smac[4],smac[5],priv->dev->name);
			for(i=0;i<arp_skb->len;i++){
				DEBUG_INFO("%02x",arp_skb->data[i]);
				if(i/16 && ((i%16) == 0))
					DEBUG_INFO("\n");
			}
			DEBUG_INFO("\n");
		}
		SMP_UNLOCK_XMIT(x);
	} else {
		DEBUG_ERR("%s %d Alloc skb failed\n",__func__,__LINE__);
	}

	return ret;

err_out:
	return ret;
}
#endif

void snoop_STA_IP(struct sk_buff *pskb, struct rtl8192cd_priv *priv)
{
	#define DHCP_MAGIC 0x63825363
#ifdef MBSSID
	int i;
#endif
struct iphdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	        __u8    ihl:4,
	                version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	        __u8    version:4,
	                ihl:4;
#else
#error  "Please fix <asm/byteorder.h>"
#endif
	        __u8    tos;
	        __u16   tot_len;
	        __u16   id;
	        __u16   frag_off;
	        __u8    ttl;
	        __u8    protocol;
#if 0
	        __u16   check;
	        __u32   saddr;
	        __u32   daddr;
#endif
};

struct udphdr {
	        __u16   source;
	        __u16   dest;
	        __u16   len;
	        __u16   check;
};

struct dhcpMessage {
		u_int8_t op;
		u_int8_t htype;
		u_int8_t hlen;
		u_int8_t hops;
		u_int32_t xid;
		u_int16_t secs;
		u_int16_t flags;
		u_int32_t ciaddr;
		u_int32_t yiaddr;
		u_int32_t siaddr;
		u_int32_t giaddr;
		u_int8_t chaddr[16];
		u_int8_t sname[64];
		u_int8_t file[128];
		u_int32_t cookie;
#if 0
		u_int8_t options[308]; /* 312 - cookie */
#endif
};
	struct rtl8192cd_priv *ap_priv;
	struct iphdr* iph;
	struct udphdr *udph;
	struct dhcpMessage *dhcph;
	
	iph = (struct iphdr *)(pskb->data + ETH_HLEN);
	udph = (struct udphdr *)((unsigned long)iph + (iph->ihl << 2));
	dhcph = (struct dhcpMessage *)((unsigned long)udph + sizeof(struct udphdr));

	if(IS_VXD_INTERFACE(priv)) {
        ap_priv = GET_ROOT(priv);
    } else {
		DEBUG_INFO("Receive DHCP response but interface is not VXD\n");
        return ;
	}

	//dhcp reply only
	if(ap_priv && IS_DRV_OPEN(ap_priv) && (dhcph->op == 2)) {
        unsigned char sta_ip[4];

		memcpy(sta_ip,&dhcph->yiaddr,4);
		DEBUG_INFO("[%s]External DHCP Server give IP[%d.%d.%d.%d]\n",priv->dev->name,sta_ip[0],sta_ip[1],sta_ip[2],sta_ip[3]);
#ifdef MBSSID
		unsigned char vap_asso_sta = 0;
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(ap_priv->pvap_priv[i]) && get_stainfo(ap_priv->pvap_priv[i],&dhcph->chaddr[0]))
					vap_asso_sta ++;
			}
		}
#endif
		if((get_stainfo(ap_priv, &dhcph->chaddr[0]) 
#ifdef MBSSID
			|| vap_asso_sta > 0
#endif
			) && send_arp_response(priv,&dhcph->siaddr,&dhcph->yiaddr,pskb->data+MACADDRLEN,&dhcph->chaddr[0]) )
			DEBUG_ERR("Send ARP failed\n");

		return;
	}
}

int changePreamble(struct rtl8192cd_priv *priv, int preamble)
{
	unsigned char *p = (unsigned char *)priv->beaconbuf;
	unsigned short *bcn_cap = p+BEACON_MACHDR_LEN+_TIMESTAMP_+_BEACON_ITERVAL_;

	if(preamble)
		*bcn_cap |= cpu_to_le16(BIT(5));
	else
		*bcn_cap &= ~(cpu_to_le16(BIT(5)));

	return 0;
}

int HideAP(struct rtl8192cd_priv *priv)
{
	unsigned char *p = (unsigned char *)priv->beaconbuf;

	memset(p + BEACON_MACHDR_LEN + _TIMESTAMP_ + _BEACON_ITERVAL_ + _CAPABILITY_ + 2, 0, SSID_LEN);

	return 0;
}

int DehideAP(struct rtl8192cd_priv *priv)
{
	unsigned char *p = (unsigned char *)priv->beaconbuf;

	memcpy(p + BEACON_MACHDR_LEN + _TIMESTAMP_ + _BEACON_ITERVAL_ + _CAPABILITY_ + 2, SSID, SSID_LEN);
	*(p + BEACON_MACHDR_LEN + _TIMESTAMP_ + _BEACON_ITERVAL_ + _CAPABILITY_ + 1) = SSID_LEN;

	return 0;
}

//priv must be root interface's priv
int takeOverHidden(unsigned char active, struct rtl8192cd_priv *priv)
{
	if((OPMODE & WIFI_AP_STATE) && IS_DRV_OPEN(priv)) {
		if(priv->take_over_hidden != active) {
			if(active) {
				HideAP(priv);
				STADEBUG("Hidden take over!!\n");
			} else {
				DehideAP(priv);
				STADEBUG("Hidden take over end!!\n");
			}
		} else {
			//STADEBUG("Keep take_over_hidden:%d!!\n",priv->take_over_hidden);
		}

	    priv->take_over_hidden = active;
	} else {
		//do nothing exclude AP mode
	}

    return 0;
}

int zeroByteLength(unsigned char *p, int len)
{
	int ret=0;

	for(ret=0;ret<len;ret++) {
		if(p[ret]!=0)
            break;
	}

	return ret;
}

int isHiddenAP(unsigned char *pframe, struct rx_frinfo *pfrinfo, struct stat_info *pstat, struct rtl8192cd_priv *priv)
{
	unsigned int len=0, ret=0;
	unsigned char *p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SSID_IE_, &len, pfrinfo->pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);

	if(p) {
		if((len && (len==zeroByteLength(p+2,len))) || (len==0)) {
#ifdef MBSSID
			takeOverHidden(1,GET_VAP_PRIV(priv,CASE6_VAP_INDEX));
#endif
			ret = 1;
		} else {
#ifdef MBSSID
			takeOverHidden(0,GET_VAP_PRIV(priv,CASE6_VAP_INDEX));
#endif
		}
	} else {
		ret = 1;
		STADEBUG("SSID IE is not included, treat as hidden!!\n");
#ifdef MBSSID
		takeOverHidden(1,GET_VAP_PRIV(priv,CASE6_VAP_INDEX));
#endif
	}

    return ret;
}
#endif	//UNIVERSAL_REPEATER

/* Do defered channel scan when,
   1.There is not any station connected to active AP interafce(Root AP/VAP)
   2.Scan is not requested by wscd
   3.Scan is not requested by Station mode without any VAP
*/
int should_defer_ss(struct rtl8192cd_priv *priv)
{
	int ret=0;

	if(GET_ROOT(priv)->pmib->dot11OperationEntry.opmode & (WIFI_STATION_STATE|WIFI_ADHOC_STATE)) {
		ret = 0;
	} else {
		if(get_ss_level(priv) < SS_LV_ROOTFUNCOFF) {
			if((priv->site_survey->ss_channel == 100) && (priv->pmib->miscEntry.ss_delay) && !((priv->auto_channel == 1) && (priv->pmib->dot11RFEntry.acs_type == 2))) {
				ret=1;
				DEBUG_INFO("%s Sitesurvey defered\n",priv->dev->name);
			}
		}
	}

	if(priv->ss_req_ongoing == SSFROM_WSC)
		ret = 0;

	return ret;
}


#ifdef USE_OUT_SRC
unsigned char *Get_Adaptivity_Version(void)
{
	return ADAPTIVITY_VERSION;
}
#endif

/*cfg p2p*/
#ifdef P2P_SUPPORT
void copy_p2p_ie(struct rtl8192cd_priv *priv, unsigned char *p2p_ie_input, unsigned char mgmt_type)
{
    /*T(1) L(1) V , wps_ie[1]==L => LEN = value of (L) + sizeof(T)+sizeof(L), cy wang note*/ 
	int p2p_ie_len = (p2p_ie_input[1] + 2);

	if(p2p_ie_len > 256)
	{
		NDEBUG("P2P_IE length > 256 !! Can NOT copy !!\n");
		return;
	}

	if (mgmt_type == MGMT_BEACON) { /* Beacon */
		NDEBUG2("P2P:Set Beacon IE[%d]\n",p2p_ie_len);
		priv->p2pPtr->p2p_beacon_ie_len = p2p_ie_len;        
		memcpy((void *)priv->p2pPtr->p2p_beacon_ie, p2p_ie_input, p2p_ie_len);
	}
	else if (mgmt_type == MGMT_PROBERSP) {  /* Probe Rsp */
		NDEBUG2("P2P:Set Probe Rsp IE[%d]\n",p2p_ie_len);
		priv->p2pPtr->p2p_probe_rsp_ie_len= p2p_ie_len;
		memcpy((void *)priv->p2pPtr->p2p_probe_rsp_ie, p2p_ie_input, p2p_ie_len);        
	}
	else if (mgmt_type == MGMT_PROBEREQ) {  /* Probe Req */
		NDEBUG2("P2P:Set Probe Req IE[%d]\n",p2p_ie_len);
		priv->p2pPtr->p2p_probe_req_ie_len = p2p_ie_len;
		memcpy((void *)priv->p2pPtr->p2p_probe_req_ie, p2p_ie_input, p2p_ie_len);        
	}    
	else if (mgmt_type == MGMT_ASSOCRSP ) { /*Assoc Rsp*/
		NDEBUG2("P2P:Set Assoc Rsp IE[%d]\n",p2p_ie_len);
        priv->p2pPtr->p2p_assoc_RspIe_len = p2p_ie_len;        
		memcpy((void *)priv->p2pPtr->p2p_assoc_RspIe, p2p_ie_input, p2p_ie_len);
	}
	else if (mgmt_type == MGMT_ASSOCREQ) { /*Assoc Req*/	
		NDEBUG2("P2P:Set Assoc Req IE[%d]\n",p2p_ie_len);        
		priv->p2pPtr->p2p_assocReq_ie_len = p2p_ie_len;                
        memcpy((void *)priv->p2pPtr->p2p_assocReq_ie, p2p_ie_input, p2p_ie_len);        
	}else{
		NDEBUG2("P2P: rtk set IE unknown type!!\n");
    }
    
}
#endif

/*cfg p2p*/
void copy_wps_ie(struct rtl8192cd_priv *priv, unsigned char *wps_ie, unsigned char mgmt_type)
{

    /*T(1) L(1) V , wps_ie[1]==L => LEN = value of (L) + sizeof(T)+sizeof(L), cy wang note*/ 
	unsigned int wps_ie_len = (wps_ie[1] + 2);  

	if(wps_ie_len > 256){
		NDEBUG2("WPS_IE length > 256 !! Can NOT copy !!\n");
		return;
	}

	if(OPMODE & WIFI_AP_STATE)
		priv->pmib->wscEntry.wsc_enable = 2; //Enable WPS for AP mode
	else if(OPMODE & WIFI_STATION_STATE)
		priv->pmib->wscEntry.wsc_enable = 1;

	if (mgmt_type == MGMT_BEACON) {
		NDEBUG2("WSC:Set Beacon IE[%d]\n",wps_ie_len);
		priv->pmib->wscEntry.beacon_ielen = wps_ie_len;
		memcpy((void *)priv->pmib->wscEntry.beacon_ie, wps_ie, wps_ie_len);
	}
	else if (mgmt_type == MGMT_PROBERSP) {
		NDEBUG2("WSC:Set Probe Rsp IE[%d]\n",wps_ie_len);
		priv->pmib->wscEntry.probe_rsp_ielen = wps_ie_len;
		memcpy((void *)priv->pmib->wscEntry.probe_rsp_ie, wps_ie, wps_ie_len);
	}
	else if (mgmt_type == MGMT_PROBEREQ) {
		NDEBUG2("WSC:Set Probe Req IE[%d]\n",wps_ie_len);
		priv->pmib->wscEntry.probe_req_ielen= wps_ie_len;
		memcpy((void *)priv->pmib->wscEntry.probe_req_ie, wps_ie, wps_ie_len);
	}    
	else if ((mgmt_type == MGMT_ASSOCRSP) || (mgmt_type == MGMT_ASSOCREQ)) { //wrt-wps-clnt
		NDEBUG2("WSC:Set Assoc IE[%d]\n",wps_ie_len);
		priv->pmib->wscEntry.assoc_ielen = wps_ie_len;
		memcpy((void *)priv->pmib->wscEntry.assoc_ie, wps_ie, wps_ie_len);
	}
}

/**
 * rtk_get_wps_ie - Search WPS IE from a series of IEs
 * @in_ie: Address of IEs to search
 * @in_len: Length limit from in_ie
 * @wps_ie: If not NULL and WPS IE is found, WPS IE will be copied to the buf starting from wps_ie
 * @wps_ielen: If not NULL and WPS IE is found, will set to the length of the entire WPS IE
 *
 * Returns: The address of the WPS IE found, or NULL
 */

u8* rtk_get_wps_ie(u8 *in_ie, int in_len, u8 *wps_ie, int *wps_ielen)
{
	int cnt=0;
	u8 eid=0;
	u8 *wpsie_ptr=NULL;    

    u8 WSC_OUI[4] = {0x00, 0x50, 0xf2, 0x04};


	if(wps_ielen){
		*wps_ielen = 0;
    }

	if(!in_ie || in_len<=0)
		return wpsie_ptr;

	while(cnt<in_len)
	{
		eid = in_ie[cnt];
		if((eid==_VENDOR_SPECIFIC_IE_)&&( !memcmp(&in_ie[cnt+2], WSC_OUI, 4)))
		{
			wpsie_ptr = &in_ie[cnt];
			if(wps_ie){
				memcpy(wps_ie, &in_ie[cnt], in_ie[cnt+1]+2);
            }
			
			if(wps_ielen){
				*wps_ielen = in_ie[cnt+1]+2;
             }
			
			cnt+=in_ie[cnt+1]+2;

			break;
		}
		else
		{
			cnt+=in_ie[cnt+1]+2; //goto next	
		}		

	}	

	return wpsie_ptr;
}

/**
 * rtk_get_p2p_ie - Search P2P IE from a series of IEs
 * @in_ie: Address of IEs to search
 * @in_len: Length limit from in_ie
 * @p2p_ie: If not NULL and P2P IE is found, P2P IE will be copied to the buf starting from p2p_ie
 * @p2p_ielen: If not NULL and P2P IE is found, will set to the length of the entire P2P IE
 *
 * Returns: The address of the P2P IE found, or NULL
 */
/*cfg p2p*/
unsigned char*  rtk_get_p2p_ie(unsigned char*  in_ie, int in_len, unsigned char*  p2p_ie, int* p2p_ielen)
{
	int cnt=0;
	unsigned char* p2p_ptr=NULL;
	unsigned char eid=0;
    unsigned char P2P_OUI[4]={0x50,0x6F,0x9A,0x09};

	if(p2p_ielen){
		*p2p_ielen = 0;
    }

	if(!in_ie || in_len<=0)
		return p2p_ptr;

	while(cnt<in_len)
	{
		eid = in_ie[cnt];
		if((eid==_VENDOR_SPECIFIC_IE_)&&( !memcmp(&in_ie[cnt+2], P2P_OUI, 4)))
		{
			p2p_ptr = &in_ie[cnt];
			if(p2p_ie){
				memcpy(p2p_ie, &in_ie[cnt], in_ie[cnt+1]+2);
            }
			
			if(p2p_ielen){
				*p2p_ielen = in_ie[cnt+1]+2;
             }
			
			cnt+=in_ie[cnt+1]+2;
			break;
		}
        else
		{
			cnt+=in_ie[cnt+1]+2; //search next	
		}		

	}	

	return p2p_ptr;
}


/*cfg p2p*/
int rtk_get_wfd_ie(u8 *in_ie, int in_len, u8 *wfd_ie, int *wfd_ielen)
{
	int match;
	int cnt = 0;	
	u8 eid;
    unsigned char WFD_OUI[4]={0x50,0x6F,0x9A,0x0A};


	match=0;

	if ( in_len < 0 )
	{
		return match;
	}

	while(cnt<in_len)
	{
		eid = in_ie[cnt];
		
		if( ( eid == _VENDOR_SPECIFIC_IE_ ) && ( !memcmp( &in_ie[cnt+2], WFD_OUI, 4)) )
		{
			if ( wfd_ie != NULL ){
				memcpy( wfd_ie, &in_ie[ cnt ], in_ie[ cnt + 1 ] + 2 );			
			}
			else
			{
				if ( wfd_ielen != NULL ){
					*wfd_ielen = 0;
				}
			}
			
			if ( wfd_ielen != NULL ){
				*wfd_ielen = in_ie[ cnt + 1 ] + 2;
			}
			
			cnt += in_ie[ cnt + 1 ] + 2;

			match = 1;
			break;
		}
		else
		{
			cnt += in_ie[ cnt + 1 ] +2; //search next	
		}		
		
	}	

	if ( match == 1 )
	{
		match = cnt;
	}
	
	return match;

}
/*cfg p2p*/
/**
 * rtk_get_p2p_ie - Search P2P IE from a series of IEs
 * @in_ie: Address of IEs to search
 * @in_len: Length limit from in_ie
 * @p2p_ie: If not NULL and P2P IE is found, P2P IE will be copied to the buf starting from p2p_ie
 * @p2p_ielen: If not NULL and P2P IE is found, will set to the length of the entire P2P IE
 *
 * Returns: The address of the P2P IE found, or NULL
 */

u8* rtk_get_ie_with_oui(u8 *in_ie, int in_len, u8 ie_to_search, u8* oui_input ,int oui_len, int *report_ielen)
{
	int cnt=0;
	u8 *tar_ie_ptr=NULL;
	u8 eid=0;

	if(report_ielen){
		*report_ielen = 0;
    }

	if(!in_ie || in_len<=0)
		return tar_ie_ptr;

	while(cnt<in_len)
	{
		eid = in_ie[cnt];
		if((eid==ie_to_search)&&( !memcmp(&in_ie[cnt+2], oui_input,oui_len)))
		{
			tar_ie_ptr = (u8 *)&in_ie[cnt];

			
			if(report_ielen){
				*report_ielen = in_ie[cnt+1]+2;
             }
			
			cnt+=in_ie[cnt+1]+2;
			break;
		}
        else
		{
			cnt+=in_ie[cnt+1]+2; //search next	
		}		

	}	

	return tar_ie_ptr;
}
/*cfg p2p*/

#if defined(CONFIG_RTL_SIMPLE_CONFIG)
int rtk_sc_start_simple_config(struct rtl8192cd_priv *priv)
{
	int i=0;
	unsigned char null_mac[]={0,0,0,0,0,0};
	
#if defined(__ECOS)	
#ifdef UNIVERSAL_REPEATER	
	if(IS_VXD_INTERFACE(priv))
		sprintf(g_sc_ifname, "%s0", priv->dev->name);
	else
#endif
#endif
	strcpy(g_sc_ifname, priv->dev->name);

	rtk_sc_set_para(priv);
	if(priv->pmib->dot11StationConfigEntry.sc_debug)
		panic_printk("Start simple config now!\n");
	rtk_sc_clean_profile_value();
	
#if 0 //eric-sc
{
	if(priv->pmib->dot11StationConfigEntry.sc_debug)
	        panic_printk("\nFORCE ENABLE LDPC FOR SIMPLE CONFIG!!\n\n");
	RTL_W8(0x913, RTL_R8(0x913)|0x02);
}
#endif
	priv->simple_config_could_fix = 0;

	rtk_sc_set_value(SC_FIX_CHANNEL, 0);
	
#ifdef UNIVERSAL_REPEATER	
	if(IS_VXD_INTERFACE(priv))
	{
		if(priv->simple_config_status == 0)
			memcpy(priv->pmib->dot11Bss.bssid, null_mac, MACADDRLEN);
	}
#endif

	if(priv->simple_config_status == 0)
		priv->simple_config_status = 1;

	priv->pmib->dot11StationConfigEntry.sc_status = 1;
	priv->pmib->dot11StationConfigEntry.sc_vxd_rescan_time = CHECK_VXD_SC_TIMEOUT;
	priv->pmib->dot11StationConfigEntry.sc_fix_encrypt = 0;
	
	RTL_W32(RCR, RTL_R32(RCR) | RCR_AAP);	//Accept Destination Address packets.
	return 0;
}

int rtk_sc_start_connect_target(struct rtl8192cd_priv *priv)
{
	int i=0;
	//if(priv->pmib->dot11StationConfigEntry.sc_fix_channel >= 0)
	//	priv->pmib->dot11StationConfigEntry.sc_fix_channel = 0;

	if(priv->pmib->dot11StationConfigEntry.sc_reset_beacon_psk == 1)
	{
#if defined(INCLUDE_WPA_PSK) || defined(WIFI_HAPD)
		if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK) {
#if defined(CONFIG_RTL_8197F) //eric-dog 
			watchdog_kick(); 
#else
			watchdog_stop(priv);
#endif
			psk_init(priv);
			syncMulticastCipher(priv,&priv->pmib->dot11Bss);	
#if defined(CONFIG_RTL_8197F) //eric-dog 
			watchdog_kick(); 
#else
			watchdog_resume(priv);
#endif

		} 
#endif
		if( (priv->pmib->dot11StationConfigEntry.sc_sync_vxd_to_root == 1) && IS_VXD_INTERFACE(priv))
		{
#if defined(CONFIG_RTL_8197F) //eric-dog 
			watchdog_kick(); 
#else
			watchdog_stop(GET_ROOT(priv));
#endif
			if (GET_ROOT(priv)->pmib->dot1180211AuthEntry.dot11EnablePSK) {
				psk_init(GET_ROOT(priv));
				syncMulticastCipher(priv,&priv->pmib->dot11Bss);
				
			} 
			init_beacon(GET_ROOT(priv));
#if defined(CONFIG_RTL_8197F) //eric-dog 
			watchdog_kick(); 
#else
			watchdog_resume(GET_ROOT(priv));
#endif
		}
	}
	else
	{
#ifdef UNIVERSAL_REPEATER		
	if(IS_VXD_INTERFACE(priv))
	{
#ifdef MBSSID
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					priv->pvap_priv[i]->pmib->dot11OperationEntry.keep_rsnie = 1;
			}
		}
#endif
	}
#endif
	RTL_W32(RCR, RTL_R32(RCR) & ~RCR_AAP);	// Disable Accept Destination Address packets.
#ifdef CONFIG_RTL_8198C
	/* rtl8192cd_ss_timer-->start_clnt_lookup-->rtk_sc_check_security-->rtk_sc_start_connect_target
	 * ss_timer call SMP_LOCK, so we should call unlock before close interface	
	 */
	unsigned long flags;
	SMP_UNLOCK(flags);
#endif
		if(priv->pmib->dot11StationConfigEntry.sc_fix_channel > 0)
			priv->pmib->dot11StationConfigEntry.sc_fix_channel = 0;
	RTL_W8(TXPAUSE, 0xff);
	rtl8192cd_close(priv->dev);
#ifdef UNIVERSAL_REPEATER	
	if( (priv->pmib->dot11StationConfigEntry.sc_sync_vxd_to_root == 1) && IS_VXD_INTERFACE(priv))
	{
		GET_ROOT(priv)->pmib->dot11StationConfigEntry.sc_enabled = 0;
		rtl8192cd_close(GET_ROOT(priv)->dev);
		rtl8192cd_open(GET_ROOT(priv)->dev);
		while(!netif_running(GET_ROOT(priv)->dev))
			delay_ms(10);
	}
#endif
	rtl8192cd_open(priv->dev);
	RTL_W8(TXPAUSE, 0x00);
#ifdef CONFIG_RTL_8198C
	SMP_LOCK(flags);
#endif
	}
	
	rtk_sc_set_value(SC_PRIV_STATUS, 5);
	rtk_sc_set_value(SC_STATUS, 5);

	if(priv->pmib->dot11StationConfigEntry.sc_reset_beacon_psk == 1)
	{
	
		extern void start_clnt_auth(struct rtl8192cd_priv *priv);
	
		//if(IS_VXD_INTERFACE(priv))
			start_clnt_auth(priv);
		//else
		//	start_clnt_join(priv);
	}

	return 0;
}
#ifdef UNIVERSAL_REPEATER	
int rtk_sc_sync_vxd_to_root(struct rtl8192cd_priv * priv)
{
	if(IS_VXD_INTERFACE(priv))
	{
		if(priv->pmib->dot11StationConfigEntry.sc_sync_vxd_to_root == 1)
		{
			GET_ROOT(priv)->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm;
			GET_ROOT(priv)->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
			GET_ROOT(priv)->pmib->dot1180211AuthEntry.dot11EnablePSK = priv->pmib->dot1180211AuthEntry.dot11EnablePSK;
			GET_ROOT(priv)->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex;
			GET_ROOT(priv)->pmib->dot118021xAuthEntry.dot118021xAlgrthm = priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm;
			GET_ROOT(priv)->pmib->dot1180211AuthEntry.dot11WPA2Cipher = priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher;
			GET_ROOT(priv)->pmib->dot1180211AuthEntry.dot11WPACipher = priv->pmib->dot1180211AuthEntry.dot11WPACipher;
			strcpy(GET_ROOT(priv)->pmib->dot1180211AuthEntry.dot11PassPhrase, priv->pmib->dot1180211AuthEntry.dot11PassPhrase);
			strcpy(&(GET_ROOT(priv)->pmib->dot11DefaultKeysTable.keytype[0]), &(priv->pmib->dot11DefaultKeysTable.keytype[0]));
			strcpy(GET_ROOT(priv)->pmib->dot11StationConfigEntry.dot11DesiredSSID, priv->pmib->dot11StationConfigEntry.dot11DesiredSSID);
			strcpy(GET_ROOT(priv)->pmib->dot11StationConfigEntry.dot11SSIDtoScan, priv->pmib->dot11StationConfigEntry.dot11SSIDtoScan);
			GET_ROOT(priv)->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen= priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen;
			GET_ROOT(priv)->pmib->dot11StationConfigEntry.dot11SSIDtoScanLen= priv->pmib->dot11StationConfigEntry.dot11SSIDtoScanLen;
		}
	}

	return 0;
}
#else
int rtk_sc_sync_vxd_to_root(struct rtl8192cd_priv * priv)
{
	return 0;
}

#endif

int rtk_sc_stop_simple_config(struct rtl8192cd_priv *priv)
{
	int i=0, ack_num=0;
	int sc_security_type = priv->pmib->dot11StationConfigEntry.sc_security_type;

	if (OPMODE & WIFI_ASOC_STATE)
	{
		rtk_sc_set_value(SC_ENABLED, 0);
		rtk_sc_set_value(SC_PRIV_STATUS, 0);
		rtk_sc_set_value(SC_STATUS, 10+sc_security_type);
	}
	else
		rtk_sc_set_value(SC_STATUS, -1);

	rtk_sc_set_value(SC_FIX_CHANNEL, 0);
	return 0;
}

struct net_device * rtl_get_dev_by_wlan_name(char *name)
{
#if defined(__ECOS)
	return rtl_getDevByName(name);
#else
	return __dev_get_by_name(&init_net, name);
#endif
}

int rtk_sc_set_value(unsigned int id, unsigned int value)
{
	struct net_device *dev;
	dev=rtl_get_dev_by_wlan_name(g_sc_ifname);
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);


	switch(id)
	{
		case SC_ENABLED:
			priv->pmib->dot11StationConfigEntry.sc_enabled = value;
			break;
		case SC_DURATION_TIME:
			priv->pmib->dot11StationConfigEntry.sc_duration_time = value;
			break;
		case SC_GET_SYNC_TIME:
			priv->pmib->dot11StationConfigEntry.sc_get_sync_time = value;
			break;
		case SC_GET_PROFILE_TIME:
			priv->pmib->dot11StationConfigEntry.sc_get_profile_time = value;
			break;
		case SC_VXD_RESCAN_TIME:
			priv->pmib->dot11StationConfigEntry.sc_vxd_rescan_time = value;
			break;
		case SC_PIN_ENABLED:
			priv->pmib->dot11StationConfigEntry.sc_pin_enabled = value;
			break;
		case SC_STATUS:
			priv->pmib->dot11StationConfigEntry.sc_status = value;
			break;
		case SC_DEBUG:
			priv->pmib->dot11StationConfigEntry.sc_debug = value;
			break;	
		case SC_CHECK_LINK_TIME:
			priv->pmib->dot11StationConfigEntry.sc_check_link_time = value;
			break;			
		case SC_SYNC_VXD_TO_ROOT:
			priv->pmib->dot11StationConfigEntry.sc_sync_vxd_to_root = value;
			break;			
		case SC_ACK_ROUND:
			priv->pmib->dot11StationConfigEntry.sc_ack_round = value;
			break;			
		case SC_CONTROL_IP:
			priv->pmib->dot11StationConfigEntry.sc_control_ip = value;
			break;	
		case SC_PRIV_STATUS:
			priv->simple_config_status = value;
			break;
		case SC_CONFIG_TIME:
			priv->simple_config_time = value;
			break;
		case SC_FIX_CHANNEL:
			if(priv->pmib->dot11StationConfigEntry.sc_fix_channel >= 0){
				if ( (value > 0) && priv->simple_config_could_fix)
					priv->pmib->dot11StationConfigEntry.sc_fix_channel = value;
				else 
			priv->pmib->dot11StationConfigEntry.sc_fix_channel = value;
			}
			break;
		case SC_FROM_TO_DS:
			if(value == 2)
			{
				//if(priv->pmib->dot11StationConfigEntry.sc_debug)
				//	panic_printk("receive fromDS is 0, toDS is 1 packet\n");
				//priv->pmib->dot11StationConfigEntry.sc_get_sync_time = 120;
			}
			else if(value == 1)
			{
				//if(priv->pmib->dot11StationConfigEntry.sc_debug)
				//	panic_printk("receive fromDS is 1, toDS is 0 packet\n");
			}
#if 0
			if(priv->site_survey->ss_channel > 14 && priv->pmib->dot11StationConfigEntry.sc_fix_channel>=0)
			{
				priv->pmib->dot11StationConfigEntry.sc_fix_channel = PHY_QueryRFReg(priv,RF92CD_PATH_A,0x18,0xff,1);
				if(priv->pmib->dot11StationConfigEntry.sc_debug)
					panic_printk("Simple Config 5G switch to remote AP channel(center) %d\n", priv->pmib->dot11StationConfigEntry.sc_fix_channel );
			}
#endif
			break;
		case SC_FIX_ENCRYPT:
			priv->pmib->dot11StationConfigEntry.sc_fix_encrypt = value;
			break;
		default:
			break;
	}
	return 0;
}

int rtk_sc_get_value(unsigned int id)
{
	struct net_device *dev;
	int value=0;
	dev=rtl_get_dev_by_wlan_name(g_sc_ifname);
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);


	switch(id)
	{
		case SC_ENABLED:
			value = priv->pmib->dot11StationConfigEntry.sc_enabled;
			break;
		case SC_DURATION_TIME:
			value = priv->pmib->dot11StationConfigEntry.sc_duration_time;
			break;
		case SC_GET_SYNC_TIME:
			value = priv->pmib->dot11StationConfigEntry.sc_get_sync_time;
			break;
		case SC_GET_PROFILE_TIME:
			value = priv->pmib->dot11StationConfigEntry.sc_get_profile_time;
			break;
		case SC_VXD_RESCAN_TIME:
			value = priv->pmib->dot11StationConfigEntry.sc_vxd_rescan_time;
			break;
		case SC_PIN_ENABLED:
			value = priv->pmib->dot11StationConfigEntry.sc_pin_enabled;
			break;
		case SC_STATUS:
			value = priv->pmib->dot11StationConfigEntry.sc_status;
			break;
		case SC_DEBUG:
			value = priv->pmib->dot11StationConfigEntry.sc_debug;
			break;	
		case SC_CHECK_LINK_TIME:
			value = priv->pmib->dot11StationConfigEntry.sc_check_link_time;
			break;			
		case SC_SYNC_VXD_TO_ROOT:
			value = priv->pmib->dot11StationConfigEntry.sc_sync_vxd_to_root;
			break;			
		case SC_ACK_ROUND:
			value = priv->pmib->dot11StationConfigEntry.sc_ack_round;
			break;			
		case SC_CONTROL_IP:
			value = priv->pmib->dot11StationConfigEntry.sc_control_ip;
			break;	
		case SC_PRIV_STATUS:
			value = priv->simple_config_status;
			break;
		case SC_CONFIG_TIME:
			value = priv->simple_config_time;
			break;
		case SC_CHECK_LEVEL:
			value = priv->pmib->dot11StationConfigEntry.sc_check_level;
			break;	
		case SC_IGNORE_OVERLAP:
			value = priv->pmib->dot11StationConfigEntry.sc_ignore_overlap;
			break;	
		case SC_FIX_CHANNEL:
			value = priv->pmib->dot11StationConfigEntry.sc_fix_channel;
			break;
		case SC_FIX_ENCRYPT:
			value = priv->pmib->dot11StationConfigEntry.sc_fix_encrypt;
			break;
		case SC_CONFIG_TYPE:
			value = priv->pmib->dot11StationConfigEntry.sc_config_type;
			break;
		default:
			break;
	}
	return value;
}

int rtk_sc_set_string_value(unsigned int id, unsigned char *value)
{
	struct net_device *dev;
	int i=0;
	dev=rtl_get_dev_by_wlan_name(g_sc_ifname);
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	switch(id)
	{
		case SC_PIN:
			strcpy(priv->pmib->dot11StationConfigEntry.sc_pin, value);
			break;
		case SC_DEFAULT_PIN:		
			strcpy(priv->pmib->dot11StationConfigEntry.sc_default_pin, value);
			//DEBUG_PRINT("default pin %s\n",value);
			break;
		case SC_PASSWORD:
			strcpy(priv->pmib->dot11StationConfigEntry.sc_passwd, value);
			break;
		case SC_DEVICE_NAME:
			strcpy(priv->pmib->dot11StationConfigEntry.sc_device_name, value);
			break;
		case SC_DEVICE_TYPE:
			strcpy(priv->pmib->dot11StationConfigEntry.sc_device_type, value);
			break;
		case SC_SSID:
			strcpy(priv->pmib->dot11StationConfigEntry.dot11DesiredSSID, value);
			strcpy(priv->pmib->dot11StationConfigEntry.dot11SSIDtoScan, value);
			priv->pmib->dot11StationConfigEntry.dot11DesiredSSIDLen= strlen(value);
			priv->pmib->dot11StationConfigEntry.dot11SSIDtoScanLen= strlen(value);
			break;
		case SC_BSSID:
			for(i=0; i<6; i++)
			{
				priv->pmib->dot11StationConfigEntry.dot11DesiredBssid[i] = value[i];
				priv->pmib->dot11StationConfigEntry.dot11Bssid[i] = value[i];
			}
			break;
		default:
			panic_printk("RTL Simple Config don't support this mib setting now!\n");
	}
	return 0;
}

int rtk_sc_get_string_value(unsigned int id, unsigned char *value)
{
	struct net_device *dev;
	dev=rtl_get_dev_by_wlan_name(g_sc_ifname);
	struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);

	switch(id)
	{
		case SC_PIN:
			strcpy(value, priv->pmib->dot11StationConfigEntry.sc_pin);
			break;
		case SC_DEFAULT_PIN:
			strcpy(value, priv->pmib->dot11StationConfigEntry.sc_default_pin);
			break;
		case SC_PASSWORD:
			strcpy(value, priv->pmib->dot11StationConfigEntry.sc_passwd);
			break;
		case SC_DEVICE_NAME:
			strcpy(value, priv->pmib->dot11StationConfigEntry.sc_device_name);
			break;
		case SC_DEVICE_TYPE:
			strcpy(value, priv->pmib->dot11StationConfigEntry.sc_device_type);
			break;
		default:
			panic_printk("RTL Simple Config don't support this mib setting now!\n");
			break;
	}
	return 0;
}

int set_wep_key(struct rtl8192cd_priv *priv, int type)
{
	int i, length;
	unsigned char value;
	unsigned char wep_key[26]={0};
	if(type == 0)
		length=10;
	else if(type == 1)
		length=26;
	for(i=0; i<length; i++)
	{
		value = priv->pmib->dot11StationConfigEntry.sc_passwd[i];
		if((value >=0x30) && (value <=0x39))
			value -= 0x30;
		else if((value >= 'a')&&(value <= 'z'))
			value -= 0x57;
		else if((value >= 'A')&&(value <= 'Z'))
			value -= 0x37;
		value &= 0xf;
		if(i%2 == 0)
			wep_key[i/2] = value<<4;
		else
			wep_key[i/2] += value;
	}
	memcpy(&(GET_MIB(priv)->dot11DefaultKeysTable.keytype[0]), wep_key, length/2);
}

int rtk_sc_check_security(struct rtl8192cd_priv * priv,struct bss_desc * bss)
{
	int length=0, i=0;
	int status = rtk_sc_get_value(SC_STATUS);
	length = strlen(priv->pmib->dot11StationConfigEntry.sc_passwd);
	if ((bss->capability&BIT(4)) == 0)
	{
		GET_MIB(priv)->dot1180211AuthEntry.dot11AuthAlgrthm = 0;
		GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		GET_MIB(priv)->dot1180211AuthEntry.dot11EnablePSK = 0;
		rtk_sc_set_value(SC_CHECK_LINK_TIME, 2);
		GET_MIB(priv)->dot11StationConfigEntry.sc_security_type= 0;
		GET_MIB(priv)->dot11RsnIE.rsnielen = 0;
		GET_MIB(priv)->dot118021xAuthEntry.dot118021xAlgrthm= 0;
	}
	else if((bss->capability&BIT(4)))
	{
		if(bss->t_stamp[0] == 0)
		{
			GET_MIB(priv)->dot1180211AuthEntry.dot11EnablePSK = 0;
			if((length == 5) || (length == 10))
			{
				GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyAlgrthm = _WEP_40_PRIVACY_;
				if(length == 5)
				{
					memcpy(&(GET_MIB(priv)->dot11DefaultKeysTable.keytype[0]), priv->pmib->dot11StationConfigEntry.sc_passwd, 5);
					priv->pmib->dot11StationConfigEntry.sc_security_type= 1;
				}
				else if(length == 10)
				{
					set_wep_key(priv, 0);
					priv->pmib->dot11StationConfigEntry.sc_security_type= 2;
				}
				GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyKeyIndex = 0;
			}
			else if((length == 13) || (length == 26))
			{
				GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyAlgrthm = _WEP_104_PRIVACY_;
				if(length == 13)
				{
					memcpy(&(GET_MIB(priv)->dot11DefaultKeysTable.keytype[0]), priv->pmib->dot11StationConfigEntry.sc_passwd, 13);
					priv->pmib->dot11StationConfigEntry.sc_security_type= 3;
				}
				else
				{
					set_wep_key(priv, 1);
					priv->pmib->dot11StationConfigEntry.sc_security_type= 4;
				}
				GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyKeyIndex = 0;
			}
			GET_MIB(priv)->dot11RsnIE.rsnielen = 0;
			GET_MIB(priv)->dot118021xAuthEntry.dot118021xAlgrthm= 0;
			rtk_sc_set_value(SC_CHECK_LINK_TIME, 2);
		}
		else
		{
			GET_MIB(priv)->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
			GET_MIB(priv)->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
			strcpy((char *)GET_MIB(priv)->dot1180211AuthEntry.dot11PassPhrase, priv->pmib->dot11StationConfigEntry.sc_passwd);
			if(bss->t_stamp[0] & 0xffff0000)
			{
				GET_MIB(priv)->dot1180211AuthEntry.dot11EnablePSK = 2;
				if(((bss->t_stamp[0] & 0xf0000000) >> 28) == 0x4)
				{
					GET_MIB(priv)->dot118021xAuthEntry.dot118021xAlgrthm = 1;
				}
				if(((bss->t_stamp[0] & 0x0f000000) >> 24) == 0x5)
				{
					GET_MIB(priv)->dot1180211AuthEntry.dot11WPA2Cipher = 8;
					priv->pmib->dot11StationConfigEntry.sc_security_type= 5;
				}
				else if(((bss->t_stamp[0] & 0x0f000000) >> 24) == 0x4)
				{
					GET_MIB(priv)->dot1180211AuthEntry.dot11WPA2Cipher = 8;
					priv->pmib->dot11StationConfigEntry.sc_security_type= 5;
				}
				else if(((bss->t_stamp[0] & 0x0f000000) >> 24) == 0x1)
				{
					GET_MIB(priv)->dot1180211AuthEntry.dot11WPA2Cipher = 2;
					priv->pmib->dot11StationConfigEntry.sc_security_type= 6;
				}
			}
			else if(bss->t_stamp[0] & 0x0000ffff)
			{
				GET_MIB(priv)->dot1180211AuthEntry.dot11EnablePSK = 1;
				if(((bss->t_stamp[0] & 0x0000f000) >> 12) == 0x4)
				{
					GET_MIB(priv)->dot118021xAuthEntry.dot118021xAlgrthm = 0;
				}
				if(((bss->t_stamp[0] & 0x00000f00) >> 8) == 0x5)
				{
					GET_MIB(priv)->dot1180211AuthEntry.dot11WPACipher = 8;	
					priv->pmib->dot11StationConfigEntry.sc_security_type= 7;
				}
				else if(((bss->t_stamp[0] & 0x00000f00) >> 8) == 0x4)
				{
					GET_MIB(priv)->dot1180211AuthEntry.dot11WPACipher = 8;	
					priv->pmib->dot11StationConfigEntry.sc_security_type= 7;
				}
				else if(((bss->t_stamp[0] & 0x00000f00) >> 8) == 0x1)
				{
					GET_MIB(priv)->dot1180211AuthEntry.dot11WPACipher = 2;	
					priv->pmib->dot11StationConfigEntry.sc_security_type= 8;
				}
			}
		}
	}
	if(status == 3)
	{
		//if sc_security_type>0, status 3 jump to status 4, don't need to get security again.
		if(bss->ssidlen != 0)
		{
			rtk_sc_set_string_value(SC_SSID,bss->ssid);
		}
		rtk_sc_set_value(SC_FIX_ENCRYPT, priv->pmib->dot11StationConfigEntry.sc_security_type+10);
		
		rtk_sc_set_value(SC_PRIV_STATUS, 4);
		rtk_sc_set_value(SC_STATUS, 4);
		
		rtk_sc_sync_vxd_to_root(priv);
		rtk_sc_set_value(SC_CONFIG_TIME, 0);
		rtk_sc_start_connect_target(priv);
	}
	else if(status == 2)
	{
		if(bss->ssidlen != 0)
		{
			rtk_sc_set_string_value(SC_SSID,bss->ssid);
		}
		rtk_sc_set_value(SC_FIX_ENCRYPT, priv->pmib->dot11StationConfigEntry.sc_security_type+10);
		if(0)//(priv->pmib->dot11StationConfigEntry.sc_security_type == 0)
		{
			rtk_sc_set_value(SC_PRIV_STATUS, 4);
			rtk_sc_set_value(SC_STATUS, 4);
			
			rtk_sc_sync_vxd_to_root(priv);
			rtk_sc_set_value(SC_CONFIG_TIME, 0);
			rtk_sc_start_connect_target(priv);
		}
		//panic_printk("get the encrypt in status 2 and it is %d\n", priv->pmib->dot11StationConfigEntry.sc_security_type);
		
	}
	return 0;
}

int rtk_sc_set_passwd(struct rtl8192cd_priv * priv)
{
	int sc_security_type;
	sc_security_type = priv->pmib->dot11StationConfigEntry.sc_security_type;

	switch(sc_security_type)
	{
		case 0:
			break;
		case 1:
			memcpy(&(GET_MIB(priv)->dot11DefaultKeysTable.keytype[0]), priv->pmib->dot11StationConfigEntry.sc_passwd, 5);
			break;
		case 2:
			set_wep_key(priv, 0);
			break;
		case 3:
			memcpy(&(GET_MIB(priv)->dot11DefaultKeysTable.keytype[0]), priv->pmib->dot11StationConfigEntry.sc_passwd, 13);
			break;
		case 4:
			set_wep_key(priv, 0);
			break;
		case 5:
		case 6:
		case 7:
		case 8:	
			strcpy((char *)GET_MIB(priv)->dot1180211AuthEntry.dot11PassPhrase, priv->pmib->dot11StationConfigEntry.sc_passwd);
			break;
		default:
			break;
	}
	return 0;
}
int rtk_sc_get_magic(unsigned int *magic, unsigned int len)
{
	int i=0;
	unsigned int magic_num[32] = {60, 62, 68, 70, 76, 78, 80, 82};
	for(i=0; i<len; i++)
		magic[i] = magic_num[i];
	return 0;
}

int rtk_sc_is_channel_fixed(struct rtl8192cd_priv * priv)
{
	struct rtl8192cd_priv * priv_root = GET_ROOT(priv);

	if(priv->pmib->dot11StationConfigEntry.sc_enabled == 1 && priv->simple_config_could_fix == 1) {
		
		if(priv->pmib->dot11StationConfigEntry.sc_fix_channel > 0)
			return 1;
		else if((priv->pmib->dot11StationConfigEntry.sc_fix_channel < 0) 
				&& (priv->pmib->dot11StationConfigEntry.sc_status == 2 || priv->pmib->dot11StationConfigEntry.sc_status == 3)
				)
			return 1; 
	}

#ifdef UNIVERSAL_REPEATER
	if(IS_DRV_OPEN(GET_VXD_PRIV(priv_root))&& 
		GET_VXD_PRIV(priv_root)->pmib->dot11StationConfigEntry.sc_enabled && priv->simple_config_could_fix == 1) {
			
		if(GET_VXD_PRIV(priv_root)->pmib->dot11StationConfigEntry.sc_fix_channel > 0)
			return 1;
		else if((GET_VXD_PRIV(priv_root)->pmib->dot11StationConfigEntry.sc_fix_channel < 0) &&
			(GET_VXD_PRIV(priv_root)->pmib->dot11StationConfigEntry.sc_status == 2 
			|| GET_VXD_PRIV(priv_root)->pmib->dot11StationConfigEntry.sc_status == 3)
			)
			return 1;
	}
#endif

	return 0;
}


int rtk_sc_get_scan_offset(struct rtl8192cd_priv * priv, int channel, int bw)
{
	int scan_offset = HT_2NDCH_OFFSET_DONTCARE;


	
	if(bw) {

		if(channel >= 36) {
		
			if((channel>144) ? ((channel-1)%8) : (channel%8))
				scan_offset = HT_2NDCH_OFFSET_ABOVE;
			else
				scan_offset = HT_2NDCH_OFFSET_BELOW;
		} else {

			if(channel < 5)
				scan_offset = HT_2NDCH_OFFSET_ABOVE;
			else if(channel > 9)
				scan_offset = HT_2NDCH_OFFSET_BELOW;
			else {

				if(GET_ROOT(priv)->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_ABOVE)
					scan_offset = HT_2NDCH_OFFSET_BELOW;
				else 
					scan_offset = HT_2NDCH_OFFSET_ABOVE;
			}
		}
	}

	return scan_offset;
}


int rtk_sc_during_simple_config_scan(struct rtl8192cd_priv * priv)
{
	struct rtl8192cd_priv * priv_root = GET_ROOT(priv);

	if(priv->pmib->dot11StationConfigEntry.sc_enabled == 1) {
		
		if(priv->simple_config_status >= 1 && priv->simple_config_status <4)
			return 1;
	}

#ifdef UNIVERSAL_REPEATER
	if(IS_DRV_OPEN(GET_VXD_PRIV(priv_root))&& 
		GET_VXD_PRIV(priv_root)->pmib->dot11StationConfigEntry.sc_enabled) {
			
		if(GET_VXD_PRIV(priv_root)->simple_config_status >= 1 && GET_VXD_PRIV(priv_root)->simple_config_status <4)
			return 1;
	}
#endif

	return 0;
}


#endif

#ifdef SUPPORT_MONITOR
void start_monitor_mode(struct rtl8192cd_priv * priv)
{
	priv->is_monitor_mode = TRUE;	
	RTL_W32(RCR, RCR_APP_FCS | RCR_APP_MIC | RCR_APP_ICV | RCR_APP_PHYSTS | RCR_HTC_LOC_CTRL
				| RCR_AMF | RCR_ADF | RCR_AICV | RCR_ACRC32 | RCR_CBSSID_ADHOC | RCR_AB | RCR_AM | RCR_APM | RCR_AAP);

	if(!priv->pmib->miscEntry.chan_switch_disable) {
		init_timer(&priv->chan_switch_timer);
		priv->chan_switch_timer.data = (unsigned long) priv;
		priv->chan_switch_timer.function = rtl8192cd_chan_switch_timer;
		mod_timer(&priv->chan_switch_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.chan_switch_time));
	}
	
	panic_printk("priv->is_monitor_mode = %d \n", priv->is_monitor_mode); 
}
void stop_monitor_mode(struct rtl8192cd_priv * priv)
{
	priv->is_monitor_mode = FALSE;
	
	OPMODE_VAL(OPMODE & (~WIFI_SITE_MONITOR));
	if (timer_pending(&priv->chan_switch_timer))
		del_timer_sync(&priv->chan_switch_timer);

	panic_printk("priv->is_monitor_mode = %d,\n", priv->is_monitor_mode); 
}
#endif

#ifdef RTK_WLAN_EVENT_INDICATE
//
// B0~B5     B6        B7~
// Sta Addr  Reason  Interface Name
//
int rtk_wlan_event_indicate(char* ifname, int event, unsigned char* addr, char reason)
{
	char data[MACADDRLEN+1+IFNAMSIZ+2] = {0};
	int data_len = 0;
	int rtk_eventd_pid = get_nl_eventd_pid();
	struct sock* nl_eventd_sk = get_nl_eventd_sk();

	if((rtk_eventd_pid==0)||(nl_eventd_sk==NULL))
	{
		panic_printk("%s:%d, report wifi link status failed, pid=%d,sk=%p\n",__FUNCTION__,__LINE__, rtk_eventd_pid, nl_eventd_sk);
		return -1;
	}

	memcpy(data, addr, MACADDRLEN);
	data[MACADDRLEN] = reason;
	memcpy(data+MACADDRLEN+1, ifname, IFNAMSIZ);
	data[MACADDRLEN+1+IFNAMSIZ] = '\0';
	data_len = MACADDRLEN+1+IFNAMSIZ+2;
	rtk_eventd_netlink_send(rtk_eventd_pid, nl_eventd_sk, event, NULL, data, data_len);

	 return 0;
}
#endif


#ifdef CONFIG_IEEE80211R
void set_r0key_expire_time(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned long timeout)
{
	struct r0_key_holder *r0kh;

	if (timeout == 0)
		return;
	
	r0kh = search_r0kh(priv, sta_addr);
	if (r0kh) {
		r0kh->key_expire_to = timeout;
	}
}

void check_r0key_expire(struct rtl8192cd_priv *priv)
{
	struct r0_key_holder *r0kh;
	struct list_head    *phead, *plist;
	unsigned long flags;
	struct stat_info *pstat;	
	unsigned char tmpBuf[20];
	unsigned char sta_addr[6];

	SAVE_INT_AND_CLI(flags);

	phead = &priv->r0kh;
	plist = phead->next;
	while (plist != phead) {
		r0kh = list_entry(plist, struct r0_key_holder, list);
		plist = plist->next;

		if (r0kh->key_expire_to > 0 && --r0kh->key_expire_to == 0) {
			memcpy(sta_addr, r0kh->sta_addr, MACADDRLEN);
			
			memset(tmpBuf, 0, sizeof(tmpBuf));
			pstat = get_stainfo(priv, sta_addr);
			if (pstat) {
				sprintf(tmpBuf, "%02x%02x%02x%02x%02x%02xyes", 
						sta_addr[0], sta_addr[1], sta_addr[2], sta_addr[3], sta_addr[4], sta_addr[5]);
				del_sta(priv, tmpBuf);		
			}
			remove_r1kh(priv, sta_addr, BSSID);
			remove_r0kh(priv, sta_addr);
				
			FT_IndicateEvent(priv, NULL, DOT11_EVENT_FT_KEY_EXPIRE_IND, sta_addr);
		}
	}

	RESTORE_INT(flags);
}

int store_r0kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned char *pmk_r0, unsigned char *pmk_r0_name)
{
	struct r0_key_holder *r0kh;
	unsigned long flags;

	remove_r0kh(priv, sta_addr);
	
	r0kh = (struct r0_key_holder *)kmalloc(sizeof(struct r0_key_holder), GFP_ATOMIC);
	if (!r0kh) {
		DEBUG_ERR("FT: unable to allocate r0kh store\n");
		return -1;
	}

	memset(r0kh, 0, sizeof(struct r0_key_holder));
	memcpy(r0kh->sta_addr, sta_addr, MACADDRLEN);
	memcpy(r0kh->pmk_r0, pmk_r0, PMK_LEN);
	memcpy(r0kh->pmk_r0_name, pmk_r0_name, PMKID_LEN);
	INIT_LIST_HEAD(&(r0kh->list));

	SAVE_INT_AND_CLI(flags);
	list_add_tail(&(r0kh->list), &(priv->r0kh));
	RESTORE_INT(flags);

	return 0;
}

struct r0_key_holder *search_r0kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr)
{
	struct r0_key_holder *r0kh;
	struct list_head    *phead, *plist;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	phead = &priv->r0kh;
	plist = phead->next;
	while (plist != phead) {
		r0kh = list_entry(plist, struct r0_key_holder, list);
		plist = plist->next;
		
		if (!memcmp(r0kh->sta_addr, sta_addr, MACADDRLEN)) {
			RESTORE_INT(flags);
			return r0kh;
		}
	}

	RESTORE_INT(flags);

	return NULL;
}

void remove_r0kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr)
{
	struct r0_key_holder *r0kh;
	unsigned long flags;

	r0kh = search_r0kh(priv, sta_addr);
	if (r0kh) {
		SAVE_INT_AND_CLI(flags);
		list_del_init(&(r0kh->list));
		RESTORE_INT(flags);

		kfree(r0kh);
	}
}

void free_r0kh(struct rtl8192cd_priv *priv)
{
	struct r0_key_holder *r0kh;
	struct list_head    *phead, *plist;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	phead = &priv->r0kh;
	plist = phead->next;
	while (plist != phead) {
		r0kh = list_entry(plist, struct r0_key_holder, list);
		plist = plist->next;
		list_del_init(&(r0kh->list));
		kfree(r0kh);
	}

	RESTORE_INT(flags);
}

int store_r1kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned char *r1kh_id,
	unsigned char *r0kh_id, unsigned int r0kh_id_len, unsigned char *pmk_r1, unsigned char *pmk_r1_name,
	unsigned char *pmk_r0_name, int pairwise)
{
	struct r1_key_holder *r1kh;
	unsigned long flags;

	if (r0kh_id_len > MAX_R0KHID_LEN) {
		DEBUG_ERR("FT: R0KH-ID length is greater than MAX_ROKHID_LEN\n");
		return -1;
	}
	
	remove_r1kh(priv, sta_addr, r1kh_id);
	
	r1kh = (struct r1_key_holder *)kmalloc(sizeof(struct r1_key_holder), GFP_ATOMIC);
	if (!r1kh) {
		DEBUG_ERR("FT: unable to allocate r1kh store\n");
		return -1;
	}

	memset(r1kh, 0, sizeof(struct r1_key_holder));
	memcpy(r1kh->sta_addr, sta_addr, MACADDRLEN);
	memcpy(r1kh->r1kh_id, r1kh_id, MACADDRLEN);
	if (r0kh_id == NULL || r0kh_id_len == 0) {
		r1kh->r0kh_id_len = 0;
	}
	else {
		memcpy(r1kh->r0kh_id, r0kh_id, r0kh_id_len);
		r1kh->r0kh_id_len = r0kh_id_len;
	}
	memcpy(r1kh->pmk_r1, pmk_r1, PMK_LEN);
	memcpy(r1kh->pmk_r1_name, pmk_r1_name, PMKID_LEN);
	if (pmk_r0_name != NULL)
		memcpy(r1kh->pmk_r0_name, pmk_r0_name, PMKID_LEN);
	r1kh->pairwise = pairwise;
	INIT_LIST_HEAD(&(r1kh->list));

	SAVE_INT_AND_CLI(flags);
	list_add_tail(&(r1kh->list), &(priv->r1kh));
	RESTORE_INT(flags);
	
	return 0;
}

struct r1_key_holder *search_r1kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned char *r1kh_id)
{
	struct r1_key_holder *r1kh;
	struct list_head    *phead, *plist;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	phead = &priv->r1kh;
	plist = phead->next;
	while (plist != phead) {
		r1kh = list_entry(plist, struct r1_key_holder, list);
		plist = plist->next;
		
		if (!memcmp(r1kh->sta_addr, sta_addr, MACADDRLEN) &&
			!memcmp(r1kh->r1kh_id, r1kh_id, MACADDRLEN)) {
			RESTORE_INT(flags);
			return r1kh;
		}
	}

	RESTORE_INT(flags);
	return NULL;
}

struct r1_key_holder *search_r1kh_by_pmkid(struct rtl8192cd_priv *priv, unsigned char *pmkid, unsigned int id)
{
	struct r1_key_holder *r1kh;
	struct list_head    *phead, *plist;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	phead = &priv->r1kh;
	plist = phead->next;
	while (plist != phead) {
		r1kh = list_entry(plist, struct r1_key_holder, list);
		plist = plist->next;

		if ( (id == 0 && !memcmp(r1kh->pmk_r0_name, pmkid, PMKID_LEN)) ||
				(id == 1 && !memcmp(r1kh->pmk_r1_name, pmkid, PMKID_LEN)) ) {
			RESTORE_INT(flags);
			return r1kh;
		}
	}

	RESTORE_INT(flags);
	return NULL;
}

int derive_r1_key(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned char *r1kh_id, struct r1_key_holder *out)
{
	struct r0_key_holder *r0kh;
	unsigned char r1_key[PMK_LEN], r1_key_id[PMKID_LEN];
	struct stat_info	*pstat;

	pstat = get_stainfo(priv, sta_addr);
	if (!pstat) {
		DEBUG_ERR("get_stainfo failed\n");
		return -1;
	}
	
	r0kh = search_r0kh(priv, sta_addr);
	if (!r0kh) {
		DEBUG_ERR("Can't ger R0KH\n");
		return -1;
	}

	derive_ft_pmk_r1(r0kh, sta_addr, r1kh_id, r1_key, r1_key_id);
	
	memset(out, 0, sizeof(struct r1_key_holder));
	memcpy(out->sta_addr, sta_addr, MACADDRLEN);
	memcpy(out->r1kh_id, r1kh_id, MACADDRLEN);
	memcpy(out->r0kh_id, R0KH_ID, R0KH_ID_LEN);
	out->r0kh_id_len = R0KH_ID_LEN;
	memcpy(out->pmk_r1, r1_key, PMK_LEN);
	memcpy(out->pmk_r1_name, r1_key_id, PMKID_LEN);
	memcpy(out->pmk_r0_name, r0kh->pmk_r0_name, PMKID_LEN);
	if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
		out->pairwise = mapPairwise(pstat->wpa_sta_info->UnicastCipher);
	else
		out->pairwise = mapPairwise(pstat->wpa_sta_info->UnicastCipher_1x);
	return 0;
}

void remove_r1kh(struct rtl8192cd_priv *priv, unsigned char *sta_addr, unsigned char *r1kh_id)
{
	struct r1_key_holder *r1kh;
	unsigned long flags;

	r1kh = search_r1kh(priv, sta_addr, r1kh_id);
	if (r1kh) {
		SAVE_INT_AND_CLI(flags);
		list_del_init(&(r1kh->list));
		RESTORE_INT(flags);
		
		kfree(r1kh);
	}
}

void free_r1kh(struct rtl8192cd_priv *priv)
{
	struct r1_key_holder *r1kh;
	struct list_head    *phead, *plist;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	phead = &priv->r1kh;
	plist = phead->next;
	while (plist != phead) {
		r1kh = list_entry(plist, struct r1_key_holder, list);
		plist = plist->next;
		list_del_init(&(r1kh->list));
		kfree(r1kh);
	}

	RESTORE_INT(flags);
}

void ft_event_indicate(struct rtl8192cd_priv *priv)
{
	#ifdef LINUX_2_6_27_
	struct pid *pid;
	#endif

	if (priv->pshare->wlanft_pid > 0)
	{
#ifdef LINUX_2_6_27_
		kill_pid(_wlanft_pid, SIGUSR1, 1);
#else
		kill_proc(priv->pshare->wlanft_pid, SIGUSR1, 1);
#endif
	}
}

unsigned int mapPairwise(unsigned char enc)
{
	if (enc == DOT11_ENC_TKIP)
		return BIT(3);
	else if (enc == DOT11_ENC_CCMP)
		return BIT(4);
	else
		return BIT(0);
}


#endif

#ifdef dybw_tx
void dynamic_AC_bandwidth(struct rtl8192cd_priv *priv, struct stat_info *pstat) {
	//dynamically switch ac mode bandwidth
	if(priv->pshare->rf_ft_var.shrink_ac_bw) {
		/* for testing */
		if((priv->pshare->rf_ft_var.shrink_ac_bw>>2) <= priv->pshare->is_40m_bw)
			pstat->shrink_ac_bw = priv->pshare->rf_ft_var.shrink_ac_bw;
	} else {
		switch(pstat->tx_bw) {
			case HT_CHANNEL_WIDTH_AC_80:
				if( pstat->rssi <= priv->pshare->rf_ft_var.ac_del80m ) {
					pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_40]++;	//prepare switching to 40M
					pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_80] = pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_20]=0;

					if(pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_40] == dybw_thrd) {
						pstat->shrink_ac_bw=4;
						*(unsigned int *)pstat->bw_cnt=0;
					}
				} else
					*(unsigned int *)pstat->bw_cnt=0;
                break;
			case HT_CHANNEL_WIDTH_AC_40:
				if( pstat->rssi <= priv->pshare->rf_ft_var.ac_del40m ) {
					pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_20]++;	//prepare switching to 20M
					pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_80] = pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_40]=0;

					if(pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_20] == dybw_thrd) {
						pstat->shrink_ac_bw=2;
						*(unsigned int *)pstat->bw_cnt=0;
					}
				} else if( pstat->rssi > priv->pshare->rf_ft_var.ac_add80m ) {
					if(pstat->tx_bw_bak > HT_CHANNEL_WIDTH_AC_40) {
						pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_80]++;	//prepare switching to 80M
						pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_40]=pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_20]=0;

						if(pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_80] == dybw_thrd) {
							pstat->shrink_ac_bw=8;
							*(unsigned int *)pstat->bw_cnt=0;
						}
					} else {
						//station is not support 80MHz bandwidth, keep 40MHz 
						pstat->shrink_ac_bw=4;
						*(unsigned int *)pstat->bw_cnt=0;
					}
				} else
					*(unsigned int *)pstat->bw_cnt=0;
				break;
			case HT_CHANNEL_WIDTH_AC_20:
				if( pstat->rssi > priv->pshare->rf_ft_var.ac_add80m ) {
					if(pstat->tx_bw_bak > HT_CHANNEL_WIDTH_AC_40) {
						pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_80]++;	//prepare switching to 80M
						pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_40]=pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_20]=0;

						if(pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_80] == dybw_thrd) {
							pstat->shrink_ac_bw=8;
							*(unsigned int *)pstat->bw_cnt=0;
						}
					} else {
						//station is not support 80MHz bandwidth, keep 20MHz 
						pstat->shrink_ac_bw=2;
						*(unsigned int *)pstat->bw_cnt=0;
					}
				} else if( pstat->rssi > priv->pshare->rf_ft_var.ac_add40m ) {
					if(pstat->tx_bw_bak > HT_CHANNEL_WIDTH_AC_20) {
						pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_40]++;	//prepare switching to 40M
						pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_80]=pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_20]=0;

						if(pstat->bw_cnt[HT_CHANNEL_WIDTH_AC_40] == dybw_thrd) {
							pstat->shrink_ac_bw=4;
							*(unsigned int *)pstat->bw_cnt=0;
						}
					} else {
						//station is not support 40MHz bandwidth, keep 20MHz 
						pstat->shrink_ac_bw=2;
						*(unsigned int *)pstat->bw_cnt=0;
					}
				} else
					*(unsigned int *)pstat->bw_cnt=0;
				break;
			default:
				DEBUG_ERR("%02x:%02x:%02x:%02x:%02x:%02x is under unknown bandwidth:%d\n",
					pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],
					pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5],pstat->tx_bw);
		}
	}

	if((pstat->shrink_ac_bw>>2) <= priv->pshare->is_40m_bw) {	//prevent target bandwidth not support
		if((pstat->shrink_ac_bw != pstat->shrink_ac_bw_bak)) {	//bandwidth changed
			DEBUG_INFO("AC bandwidth condition changed! from %d to %d\n",
											pstat->shrink_ac_bw_bak,pstat->shrink_ac_bw);

            if(pstat->shrink_ac_bw>>2 <= pstat->tx_bw_bak) {
				pstat->shrink_ac_bw_bak = pstat->shrink_ac_bw;
				switch(pstat->shrink_ac_bw) {
					case 2:
						pstat->tx_bw = HT_CHANNEL_WIDTH_AC_20;
						clearSTABitMap(&priv->pshare->_40m_staMap, REMAP_AID(pstat));
						clearSTABitMap(&priv->pshare->_80m_staMap, REMAP_AID(pstat));
						break;
					case 4:
						pstat->tx_bw = HT_CHANNEL_WIDTH_AC_40;
						setSTABitMap(&priv->pshare->_40m_staMap, REMAP_AID(pstat));
						clearSTABitMap(&priv->pshare->_80m_staMap, REMAP_AID(pstat));
						break;
					case 8:
						pstat->tx_bw = HT_CHANNEL_WIDTH_AC_80;
						clearSTABitMap(&priv->pshare->_40m_staMap, REMAP_AID(pstat));
						setSTABitMap(&priv->pshare->_80m_staMap, REMAP_AID(pstat));
						break;
					default:
						pstat->tx_bw = pstat->tx_bw_bak;
						break;
				}

#ifdef dybw_rx
				/* done by checkBandwidth
				if(priv->assoc_num == 1) {
					//for only one station connected, RX bandwidth shrinked as well
					switch(pstat->shrink_ac_bw) {
						case 2:
							priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_AC_20;
							break;
						case 4:
							priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_AC_40;
							break;
						case 8:
							priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_AC_80;
							break;
						default:
							priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
							break;
					}

					SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
	                SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
				} else {
					DEBUG_INFO("AC bandwidth rollback to initial value\n");
					priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
					SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
					SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
				}

				priv->ht_cap_len = 0;
				construct_ht_ie(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
				*/
#endif
				/* done by checkBandwidth
				update_RAMask_to_FW(priv,1);
				*/
			}
		}
	}
}
#endif

#if defined(WIFI_11N_2040_COEXIST_EXT)
void dynamic_N_bandwidth(struct rtl8192cd_priv *priv, struct stat_info *pstat) {
	//dynamically switch N mode bandwidth
	if(pstat->ht_cap_len && (priv->pmib->dot11nConfigEntry.dot11nUse40M == HT_CHANNEL_WIDTH_20_40)) {
		switch(pstat->tx_bw) {
			case HT_CHANNEL_WIDTH_20_40:
				if(pstat->rssi <= priv->pshare->rf_ft_var.n_del40m) {
					pstat->bw_cnt[HT_CHANNEL_WIDTH_20]++;
					pstat->bw_cnt[HT_CHANNEL_WIDTH_20_40]=0;
				} else
					*(unsigned int *)pstat->bw_cnt=0;

				if(pstat->bw_cnt[HT_CHANNEL_WIDTH_20] == dybw_thrd) {
					*(unsigned int *)pstat->bw_cnt=0;
					pstat->tx_bw = HT_CHANNEL_WIDTH_20;
					clearSTABitMap(&priv->pshare->_40m_staMap, REMAP_AID(pstat));
					setSTABitMap(&priv->switch_20_sta, REMAP_AID(pstat));

					if(!orSTABitMap(&priv->pshare->_40m_staMap))
						priv->pshare->ctrl40m=1;
				}
				break;
			case HT_CHANNEL_WIDTH_20:
				if (pstat->rssi > priv->pshare->rf_ft_var.n_add40m) {
					if(pstat->tx_bw_bak > HT_CHANNEL_WIDTH_20) {
						pstat->bw_cnt[HT_CHANNEL_WIDTH_20]=0;
						pstat->bw_cnt[HT_CHANNEL_WIDTH_20_40]++;

	                	if (pstat->bw_cnt[HT_CHANNEL_WIDTH_20_40] == dybw_thrd) {
							*(unsigned int *)pstat->bw_cnt=0;

							if(priv->bg_ap_timeout == 0) {
								pstat->tx_bw = HT_CHANNEL_WIDTH_20_40;

								if(!priv->pshare->CurrentChannelBW && priv->pmib->dot11nConfigEntry.dot11nUse40M==1) {
									priv->pshare->ctrl40m=2;

									pstat->tx_bw = HT_CHANNEL_WIDTH_20_40;
									setSTABitMap(&priv->pshare->_40m_staMap, REMAP_AID(pstat));
									clearSTABitMap(&priv->switch_20_sta, REMAP_AID(pstat));
								}
							}
						}
					}
				} else
					*(unsigned int *)pstat->bw_cnt=0;

				break;
			default:
				DEBUG_ERR("%02x:%02x:%02x:%02x:%02x:%02x is under unknown bandwidth:%d\n",
					pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],
					pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5],pstat->tx_bw);
		}
	}
}
#endif


/*retunr current jiffies*/
unsigned long rtw_get_current_time(void){

	return jiffies;
	
}

/*retunr value is ms , (current_time-start_time) as ms*/
unsigned long rtw_get_passing_time_ms(unsigned long start_time){

	return RTL_JIFFIES_TO_MILISECONDS(jiffies-start_time);
	
}
