/*
 *  Header file define some tx inline functions
 *
 *  $Id: 8192cd_tx.h,v 1.2 2010/01/29 09:39:16 jimmylin Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8192CD_TX_H_
#define _8192CD_TX_H_

#ifndef WLAN_HAL_INTERNAL_USED


#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_util.h"

#ifdef CONFIG_RTK_MESH
#define rtl8192cd_wlantx(p,t)	rtl8192cd_firetx(p, t)
#endif

enum _TX_QUEUE_ {
	MGNT_QUEUE		= 0,
	BK_QUEUE		= 1,
	BE_QUEUE		= 2,
	VI_QUEUE		= 3,
	VO_QUEUE		= 4,
	HIGH_QUEUE		= 5,
#if defined(CONFIG_PCI_HCI)
#if defined(CONFIG_WLAN_HAL)
	HIGH_QUEUE1		= 6,
	HIGH_QUEUE2		= 7,
	HIGH_QUEUE3		= 8,
	HIGH_QUEUE4		= 9,
	HIGH_QUEUE5		= 10,
	HIGH_QUEUE6		= 11,
	HIGH_QUEUE7		= 12,
	CMD_QUEUE		= 13,	
	BEACON_QUEUE	= 14
#else
	BEACON_QUEUE	= 6,
#endif
#elif defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	BEACON_QUEUE	= 6,
	TXCMD_QUEUE	= 7,
	HW_QUEUE_ENTRY = 8
#endif
};

#define MCAST_QNUM		HIGH_QUEUE

#if defined(CONFIG_NETDEV_MULTI_TX_QUEUE) || defined(CONFIG_SDIO_TX_FILTER_BY_PRI)
enum _NETDEV_TX_QUEUE_ {
	_NETDEV_TX_QUEUE_VO = 0,
	_NETDEV_TX_QUEUE_VI = 1,
	_NETDEV_TX_QUEUE_BE = 2,
	_NETDEV_TX_QUEUE_BK = 3,
	_NETDEV_TX_QUEUE_ALL
};
#endif

#ifndef RTL_MANUAL_EDCA
#ifdef SSID_PRIORITY_SUPPORT
#define PRI_TO_QNUM(priority, q_num, wifi_specific) { \
		if(priv->pmib->miscEntry.manual_priority) \
			priority = priv->pmib->miscEntry.manual_priority;	\
		if ((priority == 0) || (priority == 3)) \
			q_num = BE_QUEUE; \
		else if ((priority == 7) || (priority == 6)) \
			q_num = VO_QUEUE; \
		else if ((priority == 5) || (priority == 4)) \
			q_num = VI_QUEUE; \
		else  \
			q_num = BK_QUEUE; \
}
#else
#define PRI_TO_QNUM(priority, q_num, wifi_specific) { \
		if ((priority == 0) || (priority == 3)) \
			q_num = BE_QUEUE; \
		else if ((priority == 7) || (priority == 6)) \
			q_num = VO_QUEUE; \
		else if ((priority == 5) || (priority == 4)) \
			q_num = VI_QUEUE; \
		else  \
			q_num = BK_QUEUE; \
}
#endif
#endif

static __inline__ unsigned int get_mpdu_len(struct tx_insn *txcfg, unsigned int fr_len)
{
	return (txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->icv + txcfg->mic + _CRCLNG_ + fr_len);
}

#ifdef CONFIG_PCI_HCI
// the purpose if actually just to link up all the desc in the same q
static __inline__ void init_txdesc(struct rtl8192cd_priv *priv, struct tx_desc *pdesc,
				unsigned long ringaddr, unsigned int i)
{
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_RTL_8723B_SUPPORT)
	if(GET_CHIP_VER(priv)== VERSION_8812E || GET_CHIP_VER(priv) == VERSION_8723B){
		if (i == (CURRENT_NUM_TX_DESC - 1))
			(pdesc + i)->Dword12 = set_desc(ringaddr); // NextDescAddress
		else
			(pdesc + i)->Dword12 = set_desc(ringaddr + (i+1) * sizeof(struct tx_desc)); // NextDescAddress
	} else
#endif
	{
	if (i == (CURRENT_NUM_TX_DESC - 1))
		(pdesc + i)->Dword10 = set_desc(ringaddr); // NextDescAddress
	else
		(pdesc + i)->Dword10 = set_desc(ringaddr + (i+1) * sizeof(struct tx_desc)); // NextDescAddress
	}

}

#ifdef OSK_LOW_TX_DESC
static __inline__ void init_nonbe_txdesc(struct rtl8192cd_priv *priv, struct tx_desc *pdesc,
				unsigned long ringaddr, unsigned int i)
{
#ifdef CONFIG_RTL_8812_SUPPORT
	if(GET_CHIP_VER(priv)== VERSION_8812E){
		if (i == (NONBE_TXDESC - 1))
			(pdesc + i)->Dword12 = set_desc(ringaddr); // NextDescAddress
		else
			(pdesc + i)->Dword12 = set_desc(ringaddr + (i+1) * sizeof(struct tx_desc)); // NextDescAddress
	} else
#endif
	{
	if (i == (NONBE_TXDESC- 1))
		(pdesc + i)->Dword10 = set_desc(ringaddr); // NextDescAddress
	else
		(pdesc + i)->Dword10 = set_desc(ringaddr + (i+1) * sizeof(struct tx_desc)); // NextDescAddress
	}
}

#define txdesc_rollover_nonbe(ptxdesc, ptxhead)	(*ptxhead = (*ptxhead + 1) % NONBE_TXDESC)

#define txdesc_rollback_nonbe(ptxhead)			(*ptxhead = (*ptxhead == 0)? (NONBE_TXDESC - 1) : (*ptxhead - 1))

#define txdesc_rollover(ptxdesc, ptxhead)	(*ptxhead = (*ptxhead + 1) % BE_TXDESC)

#define txdesc_rollback(ptxhead)			(*ptxhead = (*ptxhead == 0)? (BE_TXDESC - 1) : (*ptxhead - 1))

#else
#define txdesc_rollover(ptxdesc, ptxhead)	(*ptxhead = (*ptxhead + 1) % CURRENT_NUM_TX_DESC)

#define txdesc_rollback(ptxhead)			(*ptxhead = (*ptxhead == 0)? (CURRENT_NUM_TX_DESC - 1) : (*ptxhead - 1))
#endif

static __inline__ void tx_poll(struct rtl8192cd_priv *priv, int q_num)
{
	unsigned char val = 0;

#ifdef CONFIG_RTL8671
#ifdef CONFIG_CPU_RLX4181
	r3k_flush_dcache_range(0,0);
#endif
#endif

	switch (q_num) {
	case MGNT_QUEUE:
		val = MGQ_POLL;
		break;
	case BK_QUEUE:
		val = BKQ_POLL;
		break;
	case BE_QUEUE:
		val = BEQ_POLL;
		break;
	case VI_QUEUE:
		val = VIQ_POLL;
		break;
	case VO_QUEUE:
		val = VOQ_POLL;
		break;
	case HIGH_QUEUE:
		val = HQ_POLL;
		break;
	default:
		break;
	}
	RTL_W8(PCIE_CTRL_REG, val);
}
#endif // CONFIG_PCI_HCI

#ifdef CONFIG_PCI_HCI
#define desc_copy(dst, src)		memcpy(dst, src, 32)
#ifdef TXDESC_INFO
#define descinfo_copy(d, s)										\
	do {														\
		struct tx_desc_info	*dst = (struct tx_desc_info	*)d;	\
		struct tx_desc_info	*src = (struct tx_desc_info	*)s;	\
		dst->type  = src->type;									\
	} while (0)
#else
#define descinfo_copy(d, s)										\
	do {														\
		struct tx_desc_info	*dst = (struct tx_desc_info	*)d;	\
		struct tx_desc_info	*src = (struct tx_desc_info	*)s;	\
		dst->type  = src->type;									\
		dst->len   = src->len;									\
		dst->rate  = src->rate;									\
	} while (0)
#endif
#endif
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
#define desc_copy(dst, src)		memcpy(dst, src, TXDESC_SIZE)

#define descinfo_copy(d, s)										\
	do {														\
		struct tx_desc_info	*dst = (struct tx_desc_info	*)d;	\
		struct tx_desc_info	*src = (struct tx_desc_info	*)s;	\
		dst->type  = src->type;									\
		dst->rate  = src->rate;									\
	} while (0)
#endif



#ifdef WDS
#define DECLARE_TXINSN(A)	struct tx_insn A; \
	do {	\
		memset(&A, 0, sizeof(struct tx_insn)); \
		A.wdsIdx  = -1; \
	} while (0)

#define DECLARE_TXCFG(P, TEMPLATE)	struct tx_insn *P = &(TEMPLATE); \
	do {	\
		memset(P, 0, sizeof(struct tx_insn)); \
		P->wdsIdx  = -1; \
	} while (0)

#else
#define DECLARE_TXINSN(A)	struct tx_insn A; \
	do {	\
		memset(&A, 0, sizeof(struct tx_insn)); \
	} while (0)

#define DECLARE_TXCFG(P, TEMPLATE)	struct tx_insn* P = &(TEMPLATE); \
	do {	\
		memset(P, 0, sizeof(struct tx_insn)); \
	} while (0)

#endif // WDS

#endif //#ifndef WLAN_HAL_INTERNAL_USED

#endif // _8192CD_TX_H_

