/*
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/inet.h>
#include <net/ipv6.h>
#include <linux/etherdevice.h>
#include <net/sch_generic.h>
#define DEBUG_LEVEL ECM_SFE_COMMON_DEBUG_LEVEL

#include <sfe_api.h>

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_tracker_datagram.h"
#include "ecm_tracker_udp.h"
#include "ecm_tracker_tcp.h"
#include "ecm_db.h"
#include "ecm_interface.h"
#include "ecm_front_end_common.h"
#include "ecm_sfe_ipv4.h"
#include "ecm_sfe_ipv6.h"
#include "ecm_sfe_common.h"

/*
 * Sysctl table
 */
static struct ctl_table_header *ecm_sfe_ctl_tbl_hdr;

static bool ecm_sfe_fast_xmit_enable = true;

/*
 * ecm_sfe_common_get_stats_bitmap()
 *	Get bit map
*/
uint32_t ecm_sfe_common_get_stats_bitmap(struct ecm_sfe_common_fe_info *fe_info, ecm_db_obj_dir_t dir)
{
	switch (dir) {
	case ECM_DB_OBJ_DIR_FROM:
		return fe_info->from_stats_bitmap;

	case ECM_DB_OBJ_DIR_TO:
		return fe_info->to_stats_bitmap;

	default:
		DEBUG_WARN("Direction not handled dir=%d for get stats bitmap\n", dir);
		break;
	}

	return 0;
}

/*
 * ecm_sfe_common_set_stats_bitmap()
 *	Set bit map
 */
void ecm_sfe_common_set_stats_bitmap(struct ecm_sfe_common_fe_info *fe_info, ecm_db_obj_dir_t dir, uint8_t bit)
{
	switch (dir) {
	case ECM_DB_OBJ_DIR_FROM:
		fe_info->from_stats_bitmap |= BIT(bit);
		break;

	case ECM_DB_OBJ_DIR_TO:
		fe_info->to_stats_bitmap |= BIT(bit);
		break;
	default:
		DEBUG_WARN("Direction not handled dir=%d for set stats bitmap\n", dir);
		break;
	}
}

/*
 * ecm_sfe_common_is_l2_iface_supported()
 *	Check if full offload can be supported in SFE engine for the given L2 interface and interface type
 */
bool ecm_sfe_common_is_l2_iface_supported(ecm_db_iface_type_t ii_type, int cur_heirarchy_index, int first_heirarchy_index)
{
	/*
	 * If extended feature is not supported, we dont need to check interface heirarchy.
	 */
	if (!sfe_is_l2_feature_enabled()) {
		DEBUG_TRACE("There is no support for extended features\n");
		return false;
	}

	switch (ii_type) {
	case ECM_DB_IFACE_TYPE_BRIDGE:
	case ECM_DB_IFACE_TYPE_OVS_BRIDGE:

		/*
		 * Below checks ensure that bridge slave interface is not a subinterce and top interface is bridge interface.
		 * This means that we support only ethX-br-lan in the herirarchy.
		 * We can remove all these checks if all l2 features are supported.
		 */

		if (cur_heirarchy_index != (ECM_DB_IFACE_HEIRARCHY_MAX - 1)) {
			DEBUG_TRACE("Top interface is not bridge, current index=%d\n", cur_heirarchy_index);
			goto fail;
		}
		return true;

	case ECM_DB_IFACE_TYPE_MACVLAN:
		return true;

	default:
		break;
	}

fail:
	return false;
}

/*
 * ecm_sfe_common_fast_xmit_check()
 *	Check the fast transmit feasibility.
 *
 * It only check device related attribute:
 */
bool ecm_sfe_common_fast_xmit_check(s32 interface_num)
{
	struct net_device *dev;
	struct netdev_queue *txq;
	int i;
	struct Qdisc *q;
#if defined(CONFIG_NET_CLS_ACT) && defined(CONFIG_NET_EGRESS)
	struct mini_Qdisc *miniq;
#endif
	/*
	 * Return failure if user has disabled SFE fast_xmit
	 */
	if (!ecm_sfe_fast_xmit_enable) {
		return false;
	}

	dev = dev_get_by_index(&init_net, interface_num);
	if (!dev) {
		DEBUG_INFO("device-ifindex[%d] is not present\n", interface_num);
		return false;
	}

	BUG_ON(!rcu_read_lock_bh_held());

	/*
	 * It assume that the qdisc attribute won't change after traffic
	 * running, if the qdisc changed, we need flush all of the rule.
	 */
	for (i = 0; i < dev->real_num_tx_queues; i++) {
		txq = netdev_get_tx_queue(dev, i);
		q = rcu_dereference_bh(txq->qdisc);
		if (q->enqueue) {
			DEBUG_INFO("Qdisc is present for device[%s]\n", dev->name);
			dev_put(dev);
			return false;
		}
	}

#if defined(CONFIG_NET_CLS_ACT) && defined(CONFIG_NET_EGRESS)
	miniq = rcu_dereference_bh(dev->miniq_egress);
	if (miniq) {
		DEBUG_INFO("Egress needed\n");
		dev_put(dev);
		return false;
	}
#endif

	dev_put(dev);

	return true;
}

/*
 * ecm_sfe_fast_xmit_enable_handler()
 *	Fast transmit sysctl node handler.
 */
int ecm_sfe_fast_xmit_enable_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;

	/*
	 * Write the variable with user input
	 */
	ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if (ret || (!write)) {
		return ret;
	}

	if ((ecm_sfe_fast_xmit_enable != 0) && (ecm_sfe_fast_xmit_enable != 1)) {
		DEBUG_WARN("Invalid input. Valid values 0/1\n");
		return -EINVAL;
	}

	return ret;
}

/*
 * ecm_sfe_ipv4_is_conn_limit_reached()
 *	Connection limit is reached or not ?
 */
bool ecm_sfe_ipv4_is_conn_limit_reached(void)
{

#if !defined(ECM_FRONT_END_CONN_LIMIT_ENABLE)
	return false;
#endif

	if (likely(!((ecm_front_end_is_feature_supported(ECM_FE_FEATURE_CONN_LIMIT)) && ecm_front_end_conn_limit))) {
		return false;
	}

	if (ecm_sfe_ipv4_accelerated_count == sfe_ipv4_max_conn_count()) {
		DEBUG_INFO("ECM DB connection limit %d reached, for SFE frontend \
			   new flows cannot be accelerated.\n",
			   ecm_sfe_ipv4_accelerated_count);
		return true;
	}

	return false;
}

#ifdef ECM_IPV6_ENABLE
/*
 * ecm_sfe_ipv6_is_conn_limit_reached()
 *	Connection limit is reached or not ?
 */
bool ecm_sfe_ipv6_is_conn_limit_reached(void)
{

#if !defined(ECM_FRONT_END_CONN_LIMIT_ENABLE)
	return false;
#endif

	if (likely(!((ecm_front_end_is_feature_supported(ECM_FE_FEATURE_CONN_LIMIT)) && ecm_front_end_conn_limit))) {
		return false;
	}

	if (ecm_sfe_ipv6_accelerated_count == sfe_ipv6_max_conn_count()) {
		DEBUG_INFO("ECM DB connection limit %d reached, for SFE frontend \
			   new flows cannot be accelerated.\n",
			   ecm_sfe_ipv6_accelerated_count);
		return true;
	}

	return false;
}

#endif

static struct ctl_table ecm_sfe_sysctl_tbl[] = {
	{
		.procname	= "sfe_fast_xmit_enable",
		.data		= &ecm_sfe_fast_xmit_enable,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= &ecm_sfe_fast_xmit_enable_handler,
	},
	{}
};

/*
 * ecm_sfe_sysctl_tbl_init()
 * 	Register sysctl for SFE
 */
int ecm_sfe_sysctl_tbl_init()
{
	ecm_sfe_ctl_tbl_hdr = register_sysctl(ECM_FRONT_END_SYSCTL_PATH, ecm_sfe_sysctl_tbl);
	if (!ecm_sfe_ctl_tbl_hdr) {
		DEBUG_WARN("Unable to register ecm_sfe_sysctl_tbl");
		return -EINVAL;
	}

	return 0;
}

/*
 * ecm_sfe_sysctl_tbl_exit()
 * 	Unregister sysctl for SFE
 */
void ecm_sfe_sysctl_tbl_exit()
{
	if (ecm_sfe_ctl_tbl_hdr) {
		unregister_sysctl_table(ecm_sfe_ctl_tbl_hdr);
	}
}

/*
 * ecm_sfe_common_init_fe_info()
 *	Initialize common fe info
 */
void ecm_sfe_common_init_fe_info(struct ecm_sfe_common_fe_info *info)
{
	info->from_stats_bitmap = 0;
	info->to_stats_bitmap = 0;
}
