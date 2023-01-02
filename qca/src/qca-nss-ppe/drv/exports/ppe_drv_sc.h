/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @addtogroup ppe_drv_subsystem
 * @{
 */

#ifndef _PPE_DRV_SC_H_
#define _PPE_DRV_SC_H_

struct ppe_drv;

/*
 * ppe_drv_sc_type
 *	Service code types
 */
typedef enum ppe_drv_sc_type {
	PPE_DRV_SC_NONE,		/* Normal PPE processing */
	PPE_DRV_SC_BYPASS_ALL,		/* Bypasses all stages in PPE */
	PPE_DRV_SC_ADV_QOS_BRIDGED,	/* Adv QoS redirection for bridged flow */
	PPE_DRV_SC_LOOPBACK_QOS,	/* Bridge or IGS QoS redirection */
	PPE_DRV_SC_BNC_0,		/* QoS bounce */
	PPE_DRV_SC_BNC_CMPL_0,		/* QoS bounce complete */
	PPE_DRV_SC_ADV_QOS_ROUTED,	/* Adv QoS redirection for routed flow */
	PPE_DRV_SC_IPSEC_PPE2EIP,	/* Inline IPsec redirection from PPE TO EIP */
	PPE_DRV_SC_IPSEC_EIP2PPE,	/* Inline IPsec redirection from EIP to PPE */
	PPE_DRV_SC_PTP,			/* Service Code for PTP packets */
	PPE_DRV_SC_VLAN_FILTER_BYPASS,	/* VLAN filter bypass for bridge flows between 2 different VSIs */
	PPE_DRV_SC_L3_EXCEPT,		/* Indicate exception post tunnel/tap operation */
	PPE_DRV_SC_SPF_BYPASS,		/* Source port filtering bypass */
	PPE_DRV_SC_MAX,			/* Max service code */
} ppe_drv_sc_t;

typedef bool (*ppe_drv_sc_callback_t)(void *app_data, struct sk_buff *skb);

/*
 * ppe_drv_sc_process_skbuff()
 *	Register callback for a specific service code
 *
 * @param[IN] sc   Service code number.
 * @param[IN] skb  Socket buffer with service code.
 *
 * @return
 * true if packet is consumed by the API or false if the packet is not consumed.
 */
extern bool ppe_drv_sc_process_skbuff(uint8_t sc, struct sk_buff *skb);

/*
 * ppe_drv_sc_unregister_cb()
 *	Unregister callback for a specific service code
 *
 * @param[IN] sc   Service code number.
 *
 * @return
 * void
 */
extern void ppe_drv_sc_unregister_cb(ppe_drv_sc_t sc);

/*
 * ppe_drv_sc_register_cb()
 *	Register callback for a specific service code
 *
 * @param[IN] sc   Service code number.
 * @param[IN] cb   Callback API.
 * @param[IN] app_data   Application data to be passed to callback.
 *
 * @return
 * void
 */
extern void ppe_drv_sc_register_cb(ppe_drv_sc_t sc, ppe_drv_sc_callback_t cb, void *app_data);

/** @} */ /* end_addtogroup ppe_drv_sc_subsystem */

#endif /* _PPE_DRV_SC_H_ */

