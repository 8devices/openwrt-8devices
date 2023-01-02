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

#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/vmalloc.h>
#include <linux/debugfs.h>
#include <fal/fal_rss_hash.h>
#include <fal/fal_ip.h>
#include <fal/fal_init.h>
#include "ppe_drv.h"

/*
 * Define the filename to be used for assertions.
 */
struct ppe_drv ppe_drv_gbl;

/*
 * ppe_drv_hw_stats_sync()
 *	Sync PPE HW stats
 */
static void ppe_drv_hw_stats_sync(struct timer_list *tm)
{

}

/*
 * ppe_drv_sw_v4_stats_sync()
 *	Sync PPE SW IPv4 stats
 */
static void ppe_drv_sw_v4_stats_sync(struct work_struct *work)
{

}

/*
 * ppe_drv_sw_v6_stats_sync()
 *	Sync PPE SW IPv6 stats
 */
static void ppe_drv_sw_v6_stats_sync(struct work_struct *work)
{

}

/*
 * ppe_drv_hash_init()
 *	Initialize the PPE hash registers
 */
static bool ppe_drv_hash_init(void)
{
	fal_rss_hash_mode_t mode = { 0 };
	fal_rss_hash_config_t config = { 0 };

	mode = FAL_RSS_HASH_IPV4ONLY;
	config.hash_mask = PPE_DRV_HASH_MASK;
	config.hash_fragment_mode = false;
	config.hash_seed = PPE_DRV_HASH_SEED_DEFAULT;
	config.hash_sip_mix[0] = PPE_DRV_HASH_MIX_V4_SIP;
	config.hash_dip_mix[0] = PPE_DRV_HASH_MIX_V4_DIP;
	config.hash_protocol_mix = PPE_DRV_HASH_MIX_V4_PROTO;
	config.hash_sport_mix = PPE_DRV_HASH_MIX_V4_SPORT;
	config.hash_dport_mix = PPE_DRV_HASH_MIX_V4_DPORT;

	config.hash_fin_inner[0] = (PPE_DRV_HASH_FIN_INNER_OUTER_0 & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_outer[0] = ((PPE_DRV_HASH_FIN_INNER_OUTER_0 >> 5) & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_inner[1] = (PPE_DRV_HASH_FIN_INNER_OUTER_1 & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_outer[1] = ((PPE_DRV_HASH_FIN_INNER_OUTER_1 >> 5) & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_inner[2] = (PPE_DRV_HASH_FIN_INNER_OUTER_2 & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_outer[2] = ((PPE_DRV_HASH_FIN_INNER_OUTER_2 >> 5) & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_inner[3] = (PPE_DRV_HASH_FIN_INNER_OUTER_3 & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_outer[3] = ((PPE_DRV_HASH_FIN_INNER_OUTER_3 >> 5) & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_inner[4] = (PPE_DRV_HASH_FIN_INNER_OUTER_4 & PPE_DRV_HASH_FIN_MASK);
	config.hash_fin_outer[4] = ((PPE_DRV_HASH_FIN_INNER_OUTER_4 >> 5) & PPE_DRV_HASH_FIN_MASK);

	if (fal_rss_hash_config_set(0, mode, &config) != SW_OK) {
		ppe_drv_warn("IPv4 hash register initialization failed\n");
		return false;
	}

	mode = FAL_RSS_HASH_IPV6ONLY;
	config.hash_sip_mix[0] = PPE_DRV_HASH_SIPV6_MIX_0;
	config.hash_dip_mix[0] = PPE_DRV_HASH_DIPV6_MIX_0;
	config.hash_sip_mix[1] = PPE_DRV_HASH_SIPV6_MIX_1;
	config.hash_dip_mix[1] = PPE_DRV_HASH_DIPV6_MIX_1;
	config.hash_sip_mix[2] = PPE_DRV_HASH_SIPV6_MIX_2;
	config.hash_dip_mix[2] = PPE_DRV_HASH_DIPV6_MIX_2;
	config.hash_sip_mix[3] = PPE_DRV_HASH_SIPV6_MIX_3;
	config.hash_dip_mix[3] = PPE_DRV_HASH_DIPV6_MIX_3;

	if (fal_rss_hash_config_set(0, mode, &config) != SW_OK) {
		ppe_drv_warn("IPv6 hash register initialization failed\n");
		return false;
	}

	return true;
}

/*
 * ppe_drv_l3_route_ctrl_init()
 *	Initialize PPE global configuration
 */
static bool ppe_drv_l3_route_ctrl_init(struct ppe_drv *p)
{
	fal_ip_global_cfg_t cfg = { 0 };

	/*
	 * Global MTU and MRU cofiguration check; if flows MTU or MRU is not as
	 * expected by PPE then flow will be deaccelerated by PPE and packets will
	 * be redirected to CPU
	 */
	cfg.mru_fail_action = FAL_MAC_RDT_TO_CPU;
	cfg.mru_deacclr_en = true;
	cfg.mtu_fail_action = FAL_MAC_RDT_TO_CPU;
	cfg.mtu_deacclr_en = true;

	/*
	 * Don't deaccelerate flow based on DF bit.
	 */
	cfg.mtu_nonfrag_fail_action = FAL_MAC_RDT_TO_CPU;
	cfg.mtu_df_deacclr_en = false;

	if (fal_ip_global_ctrl_set(0, &cfg) != SW_OK) {
		ppe_drv_warn("%p: IP global control configuration failed\n", p);
		return false;
	}

	if (fal_ip_route_mismatch_action_set(0, FAL_MAC_RDT_TO_CPU) != SW_OK) {
		ppe_drv_warn("%p: IP route mismatch action configuration failed\n", p);
		return false;
	}

	return true;
}

static const struct of_device_id ppe_drv_dt_ids[] = {
	{ .compatible =  "qcom,nss-ppe" },
	{},
};
MODULE_DEVICE_TABLE(of, ppe_drv_dt_ids);

/*
 * ppe_drv_probe()
 *	probe the PPE driver
 */
static int ppe_drv_probe(struct platform_device *pdev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct device_node *np;
	fal_ppe_tbl_caps_t cap;

	np = of_node_get(pdev->dev.of_node);

	fal_ppe_capacity_get(0, &cap);

	/*
	 * Fill number of table entries
	 */
	p->l3_if_num = cap.l3_if_caps;
	p->port_num = cap.port_caps;
	p->vsi_num = cap.vsi_caps;
	p->pub_ip_num = cap.pub_ip_caps;
	p->host_num = cap.host_caps;
	p->flow_num = cap.flow_caps;
	p->pppoe_session_max = cap.pppoe_session_caps;
	p->nexthop_num = cap.nexthop_caps;
	p->sc_num = cap.service_code_caps;
	p->iface_num = p->l3_if_num + p->port_num + p->pppoe_session_max;

	if (!ppe_drv_hash_init()) {
		return -1;
	}

	if (!ppe_drv_l3_route_ctrl_init(p)) {
		return -1;
	}

	p->pub_ip = ppe_drv_pub_ip_entries_alloc();
	if (!p->pub_ip) {
		ppe_drv_warn("%p: failed to allocate pub_ip entries", p);
		goto fail;
	}

	p->l3_if = ppe_drv_l3_if_entries_alloc();
	if (!p->l3_if) {
		ppe_drv_warn("%p: failed to allocate l3_if entries", p);
		goto fail;
	}

	p->vsi = ppe_drv_vsi_entries_alloc();
	if (!p->vsi) {
		ppe_drv_warn("%p: failed to allocate vsi entries", p);
		goto fail;
	}

	p->pppoe = ppe_drv_pppoe_entries_alloc();
	if (!p->pppoe) {
		ppe_drv_warn("%p: failed to allocate PPPoe entries", p);
		goto fail;
	}

	p->port = ppe_drv_port_entries_alloc();
	if (!p->port) {
		ppe_drv_warn("%p: failed to allocate Port entries", p);
		goto fail;
	}

	p->sc = ppe_drv_sc_entries_alloc();
	if (!p->sc) {
		ppe_drv_warn("%p: failed to allocate service code entries", p);
		goto fail;
	}

	p->iface = ppe_drv_iface_entries_alloc();
	if (!p->iface) {
		ppe_drv_warn("%p: failed to allocate iface entries", p);
		goto fail;
	}

	p->nexthop = ppe_drv_nexthop_entries_alloc();
	if (!p->nexthop) {
		ppe_drv_warn("%p: failed to allocate nexthop entries", p);
		goto fail;
	}

	p->host = ppe_drv_host_entries_alloc();
	if (!p->host) {
		ppe_drv_warn("%p: failed to allocate host entries", p);
		goto fail;
	}

	p->flow = ppe_drv_flow_entries_alloc();
	if (!p->flow) {
		ppe_drv_warn("%p: failed to allocate flow entries", p);
		goto fail;
	}

	ppe_drv_exception_init();

	/*
	 * Initialize locks
	 */
	spin_lock_init(&p->lock);
	spin_lock_init(&p->stats_lock);

	/*
	 * Initialize HW stats sync timer
	 */
	timer_setup(&p->hw_flow_stats_timer, ppe_drv_hw_stats_sync, 0);

	/*
	 * Initialize SW stats sync workqueue
	 */
	INIT_WORK(&p->sw_v4_stats, ppe_drv_sw_v4_stats_sync);
	INIT_WORK(&p->sw_v6_stats, ppe_drv_sw_v6_stats_sync);

	/* Initialize list */
	INIT_LIST_HEAD(&p->conn_v4);
	INIT_LIST_HEAD(&p->conn_v6);

	p->toggled = false;

	/*
	 * Take a reference
	 */
	kref_init(&p->ref);

	return of_platform_populate(np, NULL, NULL, &pdev->dev);

fail:
	if (p->pub_ip) {
		ppe_drv_pub_ip_entries_free(p->pub_ip);
		p->pub_ip = NULL;
	}

	if (p->l3_if) {
		ppe_drv_l3_if_entries_free(p->l3_if);
		p->l3_if = NULL;
	}

	if (p->vsi) {
		ppe_drv_vsi_entries_free(p->vsi);
		p->vsi = NULL;
	}

	if (p->pppoe) {
		ppe_drv_pppoe_entries_free(p->pppoe);
		p->pppoe = NULL;
	}

	if (p->port) {
		ppe_drv_port_entries_free(p->port);
		p->port = NULL;
	}

	if (p->sc) {
		ppe_drv_sc_entries_free(p->sc);
		p->sc = NULL;
	}

	if (p->iface) {
		ppe_drv_iface_entries_free(p->iface);
		p->iface = NULL;
	}

	if (p->nexthop) {
		ppe_drv_nexthop_entries_free(p->nexthop);
		p->nexthop = NULL;
	}

	if (p->host) {
		ppe_drv_host_entries_free(p->host);
		p->host = NULL;
	}

	if (p->flow) {
		ppe_drv_flow_entries_free(p->flow);
		p->flow = NULL;
	}

	return -1;
}

/*
 * ppe_drv_remove()
 *	remove the ppe driver
 */
static int ppe_drv_remove(struct platform_device *pdev)
{
	struct ppe_drv *p = platform_get_drvdata(pdev);

	if (p->dentry) {
		debugfs_remove_recursive(p->dentry);
	}

	if (p->pub_ip) {
		ppe_drv_pub_ip_entries_free(p->pub_ip);
		p->pub_ip = NULL;
	}

	if (p->l3_if) {
		ppe_drv_l3_if_entries_free(p->l3_if);
		p->l3_if = NULL;
	}

	if (p->vsi) {
		ppe_drv_vsi_entries_free(p->vsi);
		p->vsi = NULL;
	}

	if (p->pppoe) {
		ppe_drv_pppoe_entries_free(p->pppoe);
		p->pppoe = NULL;
	}

	if (p->port) {
		ppe_drv_port_entries_free(p->port);
		p->port = NULL;
	}

	if (p->sc) {
		ppe_drv_sc_entries_free(p->sc);
		p->sc = NULL;
	}

	if (p->iface) {
		ppe_drv_iface_entries_free(p->iface);
		p->iface = NULL;
	}

	if (p->nexthop) {
		ppe_drv_nexthop_entries_free(p->nexthop);
		p->nexthop = NULL;
	}

	if (p->host) {
		ppe_drv_host_entries_free(p->host);
		p->host = NULL;
	}

	if (p->flow) {
		ppe_drv_flow_entries_free(p->flow);
		p->flow = NULL;
	}

	return 0;
}

/*
 * ppe_drv_platform
 *	platform device instance
 */
static struct platform_driver ppe_drv_platform = {
	.probe		= ppe_drv_probe,
	.remove		= ppe_drv_remove,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "nss-ppe",
		.of_match_table = of_match_ptr(ppe_drv_dt_ids),
	},
};

/*
 * ppe_drv_module_init()
 *	module init for ppe driver
 */
static int __init ppe_drv_module_init(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	if (!of_find_compatible_node(NULL, NULL, "qcom,nss-ppe")) {
		ppe_drv_info("PPE device tree node not found\n");
		return -EINVAL;
	}

	if (platform_driver_register(&ppe_drv_platform)) {
		ppe_drv_warn("unable to register the driver\n");
		return -EIO;
	}

	/*
	 * Non availability of debugfs directory is not a catastrophy
	 * We can still go ahead with other initialization
	 */
	p->dentry = debugfs_create_dir("qca-nss-ppe", NULL);

	return 0;
}
module_init(ppe_drv_module_init);

/*
 * ppe_drv_module_exit()
 *	module exit for ppe driver
 */
static void __exit ppe_drv_module_exit(void)
{
	platform_driver_unregister(&ppe_drv_platform);
}
module_exit(ppe_drv_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS PPE driver");
