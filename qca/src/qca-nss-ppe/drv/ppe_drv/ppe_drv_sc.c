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

#include <fal/fal_servcode.h>
#include "ppe_drv.h"

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_sc_dump()
 *	Dumps service code related tables.
 */
static void ppe_drv_sc_dump(ppe_drv_sc_t sc)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_servcode_config_t sc_cfg = {0};
	sw_error_t err;

	/*
	 * Get the service code table details through SSDK.
	 */
	err = fal_servcode_config_get(PPE_DRV_SWITCH_ID, sc, &sc_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: service code query failed for sc: %u", p, sc);
		return;
	}

	ppe_drv_trace("%p: dest_port_valid: %u", p, sc_cfg.dest_port_valid);
	ppe_drv_trace("%p: dest_port_id: %u", p, sc_cfg.dest_port_id);
	ppe_drv_trace("%p: bypass_bitmap[IN_SERVICE_TBL]: 0x%x", p, sc_cfg.bypass_bitmap[0]);
	ppe_drv_trace("%p: bypass_bitmap[L2_SERVICE_TBL]: 0x%x", p, sc_cfg.bypass_bitmap[1]);
	ppe_drv_trace("%p: bypass_bitmap[CNTRS]: 0x%x", p, sc_cfg.bypass_bitmap[2]);
	ppe_drv_trace("%p: bypass_bitmap[TL_SERVICE_TBL]: 0x%x", p, sc_cfg.bypass_bitmap[3]);
	ppe_drv_trace("%p: direction: %u", p, sc_cfg.direction);
	ppe_drv_trace("%p: field_update_bitmap: 0x%x", p, sc_cfg.field_update_bitmap);
	ppe_drv_trace("%p: next_service_code: %u", p, sc_cfg.next_service_code);
	ppe_drv_trace("%p: hw_services: %u", p, sc_cfg.hw_services);
	ppe_drv_trace("%p: offset_sel: %u", p, sc_cfg.offset_sel);
}
#else
static void ppe_drv_sc_dump(ppe_drv_sc_t sc)
{
}
#endif

/*
 * ppe_drv_sc_config()
 *	Configured service code related tables based on input information.
 */
static void ppe_drv_sc_config(ppe_drv_sc_t sc, ppe_drv_sc_t next_sc, uint8_t redir_port)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_servcode_config_t sc_cfg = {0};
	sw_error_t err;

	sc_cfg.direction = PPE_DRV_SC_IN_L2_DIR_DST;
	sc_cfg.dest_port_id = redir_port;
	sc_cfg.next_service_code = next_sc;
	sc_cfg.dest_port_valid = true;

	switch (sc) {
	case PPE_DRV_SC_BYPASS_ALL:
		sc_cfg.bypass_bitmap[0] = ~((1 << FAKE_MAC_HEADER_BYP)
					| (1 << SERVICE_CODE_BYP)
					| (1 << FAKE_L2_PROTO_BYP));

		sc_cfg.bypass_bitmap[1] = ~(1 << ACL_POST_ROUTING_CHECK_BYP);
		sc_cfg.dest_port_valid = false;
		break;

	case PPE_DRV_SC_ADV_QOS_BRIDGED:
	case PPE_DRV_SC_ADV_QOS_ROUTED:
	case PPE_DRV_SC_PTP:
		break;

	case PPE_DRV_SC_LOOPBACK_QOS:
		sc_cfg.field_update_bitmap = (1 << FLD_UPDATE_SRC_INFO)
					| (1 << FLD_UPDATE_HASH_FLOW_INDEX);
		break;

	case PPE_DRV_SC_IPSEC_PPE2EIP:
		sc_cfg.offset_sel = PPE_DRV_SC_IN_L2_OFF_L3;
		sc_cfg.bypass_bitmap[1] = (1 << L3_PKT_EDIT_BYP);
		sc_cfg.hw_services = PPE_DRV_EIP_HWSERVICE_IPSEC;
		sc_cfg.field_update_bitmap = (1 << FLD_UPDATE_DEST_INFO)
					| (1 << FLD_UPDATE_HASH_FLOW_INDEX)
					| (1 << FLD_UPDATE_FAKE_L2_PROT_EN);
		break;

	case PPE_DRV_SC_IPSEC_EIP2PPE:
		/*
		 * This is used as a tag while handling exception from PPE post EIP processing.
		 */
		sc_cfg.bypass_bitmap[0] = (1 << FLOW_SERVICE_CODE_BYP);
		sc_cfg.dest_port_valid = false;
		break;

	case PPE_DRV_SC_VLAN_FILTER_BYPASS:
		/*
		 * Avoid packet drop due to EG VLAN member check while accelerating bridge flows
		 * between ports with different VSIs.
		 *
		 * Avoid getting this service code in EDMA Rx descriptor, this help us to avoid
		 * an unnecessary service code callback lookup in EDMA Rx processing on all such
		 * packets.
		 *
		 * Disable source port filtering for bridge flows between two virtual ports.
		 *
		 * Avoid exception due to new MAC address learn when FDB learning is disabled.
		 */
		sc_cfg.bypass_bitmap[1] = ((1 << EG_VLAN_MEMBER_CHECK_BYP)
						| (1 << SOURCE_FLTR_BYP)
						| (1 << L2_SOURCE_SEC_BYP));
		sc_cfg.field_update_bitmap = (1 << FLD_UPDATE_SERVICE_CODE);
		sc_cfg.dest_port_valid = false;
		break;

	case PPE_DRV_SC_L3_EXCEPT:
		/*
		 * This service code is used to recognize that the packet exceptioned from PPE start
		 * from L3 header and it doesn't have a valid MAC header.
		 *
		 * We need to skip service code in flow table, so that if PPE exception the packet after
		 * flow lookup, it doesn't overwrite the service code with flow table service code (which
		 * is typically 0 for non-QoS flows).
		 */
		sc_cfg.bypass_bitmap[0] = (1 << FLOW_SERVICE_CODE_BYP);
		sc_cfg.dest_port_valid = false;
		break;

	case PPE_DRV_SC_SPF_BYPASS:
		/*
		 * Avoid packet drop due to source port filtering and avoid FDB based forwarding for
		 * packets sent to PPE, with SPF bypass service code.
		 */
		sc_cfg.bypass_bitmap[1] = ((1 << SOURCE_FLTR_BYP)
					| (1 << BRIDGING_FWD_BYP));
		break;

	default:
		ppe_drv_warn("%p: service code %u not supported", p, sc);
		return;
	}

	/*
	 * Program the service code tables through SSDK.
	 */
	err = fal_servcode_config_set(PPE_DRV_SWITCH_ID, sc, &sc_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: service code configuration failed for sc: %u", p, sc);
	}

	ppe_drv_sc_dump(sc);
}

/*
 * ppe_drv_sc_process_skbuff()
 *	Pass on skbuff to the registered callback.
 */
bool ppe_drv_sc_process_skbuff(uint8_t sc, struct sk_buff *skb)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_sc *psc;
	ppe_drv_sc_callback_t cb;
	void *app_data;
	bool ret;

	ppe_drv_assert(sc < PPE_DRV_SC_MAX, "%p: invalid service code %u", p, sc);

	spin_lock_bh(&p->lock);
	psc = &p->sc[sc];
	cb = psc->cb;
	app_data = psc->app_data;
	spin_unlock_bh(&p->lock);

	ppe_drv_trace("%p: processing skb:%p sc:%u cb:%p app:%p", p, skb, sc, cb, app_data);

	if (!cb) {
		ppe_drv_info("%p: callback not registered for SC:%u", p, sc);
		ppe_drv_stats_inc(&p->stats.sc_stats[sc].sc_cb_unregister);
		return false;
	}

	ret = cb(app_data, skb);
	if (ret) {
		ppe_drv_stats_inc(&p->stats.sc_stats[sc].sc_cb_success);
	} else {
		ppe_drv_stats_inc(&p->stats.sc_stats[sc].sc_cb_failure);
	}

	return ret;
}

/*
 * ppe_drv_sc_unregister_cb()
 *	Unregister callback for a give service code
 */
void ppe_drv_sc_unregister_cb(ppe_drv_sc_t sc)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_sc *psc;

	spin_lock_bh(&p->lock);
	psc = &p->sc[sc];
	ppe_drv_assert(psc->cb, "%p: no cb registered for sc: %u", p, sc);
	psc->cb = NULL;
	psc->app_data = NULL;
	spin_unlock_bh(&p->lock);

	ppe_drv_info("%p: unregistered cb/app_data for sc:%u", p, sc);
}

/*
 * ppe_drv_sc_register_cb()
 *	Registers a pnode for a given service code for redirection.
 */
void ppe_drv_sc_register_cb(ppe_drv_sc_t sc, ppe_drv_sc_callback_t cb, void *app_data)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_sc *psc;

	ppe_drv_assert(cb, "%p: cannot register null cb for sc %u", p, sc);

	spin_lock_bh(&p->lock);
	psc = &p->sc[sc];
	ppe_drv_assert(!psc->cb, "%p: multiple registration for sc:%u - "
				"prev cb:%p current cb:%p", p, sc, psc->cb, cb);
	psc->cb = cb;
	psc->app_data = app_data;
	spin_unlock_bh(&p->lock);

	ppe_drv_info("%p: registered cb:%p app_data:%p for sc:%u", p, cb, app_data, sc);
}

/*
 * ppe_drv_sc_free()
 *	Free sc instance if it was allocated.
 */
void ppe_drv_sc_entries_free(struct ppe_drv_sc *sc)
{
	vfree(sc);
}

/*
 * ppe_drv_sc_alloc()
 *	Allocates and initializes service code tables with static configuration.
 */
struct ppe_drv_sc *ppe_drv_sc_entries_alloc(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_sc *sc;

	sc = vzalloc(sizeof(struct ppe_drv_sc) * p->sc_num);
	if (!sc) {
		ppe_drv_warn("%p: Failed to allocate service code table entries", p);
		return NULL;
	}

	/*
	 * Initialize service codes
	 */
	ppe_drv_sc_config(PPE_DRV_SC_BYPASS_ALL, PPE_DRV_SC_NONE, PPE_DRV_PORT_CPU);
	ppe_drv_sc_config(PPE_DRV_SC_ADV_QOS_BRIDGED, PPE_DRV_SC_ADV_QOS_BRIDGED, PPE_DRV_PORT_CPU);
	ppe_drv_sc_config(PPE_DRV_SC_ADV_QOS_ROUTED, PPE_DRV_SC_ADV_QOS_ROUTED, PPE_DRV_PORT_CPU);
	ppe_drv_sc_config(PPE_DRV_SC_LOOPBACK_QOS, PPE_DRV_SC_BYPASS_ALL, PPE_DRV_PORT_CPU);
	ppe_drv_sc_config(PPE_DRV_SC_IPSEC_PPE2EIP, PPE_DRV_SC_IPSEC_EIP2PPE, PPE_DRV_PORT_EIP197);
	ppe_drv_sc_config(PPE_DRV_SC_IPSEC_EIP2PPE, PPE_DRV_SC_IPSEC_EIP2PPE, PPE_DRV_PORT_CPU);
	ppe_drv_sc_config(PPE_DRV_SC_VLAN_FILTER_BYPASS, PPE_DRV_SC_VLAN_FILTER_BYPASS, PPE_DRV_PORT_CPU);
	ppe_drv_sc_config(PPE_DRV_SC_L3_EXCEPT, PPE_DRV_SC_L3_EXCEPT, PPE_DRV_PORT_CPU);
	ppe_drv_sc_config(PPE_DRV_SC_SPF_BYPASS, PPE_DRV_SC_SPF_BYPASS, PPE_DRV_PORT_CPU);

	return sc;
}
