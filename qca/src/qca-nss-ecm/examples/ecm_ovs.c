/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation.  All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/ctype.h>

#include <ovsmgr.h>
#include "ecm_classifier_ovs_public.h"

/*
 * This is a test module for the ECM's ovs CLassifier.
 * It is extracting the VLAN information based on the flow information.
 */

/*
 * ecm_ovs_get_ovs()
 *	OVS get callback function registered with ECM.
 */
static ecm_classifier_ovs_result_t ecm_ovs_process(struct ovsmgr_dp_flow *flow, struct sk_buff *skb, struct ecm_classifier_ovs_process_response *resp)
{
	struct ovsmgr_vlan_info ovi;
	enum ovsmgr_flow_status status;
	pr_debug("ecm_ovs_process\n");

	memset((void *)&ovi, 0, sizeof(ovi));

	status = ovsmgr_flow_info_get(flow, skb, &ovi);
	switch (status) {
	case OVSMGR_FLOW_STATUS_DENY_ACCEL:
	case OVSMGR_FLOW_STATUS_UNKNOWN:
		pr_debug("%px: Deny accelerating the flow\n", flow);
		return ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL;
	case OVSMGR_FLOW_STATUS_DENY_ACCEL_EGRESS:
		pr_debug("%px: Deny accelerating the flow, egress %s is not allowed\n", flow, flow->outdev->name);
		return ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL_EGRESS;
	case OVSMGR_FLOW_STATUS_ALLOW_VLAN_ACCEL:
	case OVSMGR_FLOW_STATUS_ALLOW_VLAN_QINQ_ACCEL:
		pr_debug("%px: Accelerate, VLAN data is valid\n", flow);
		/*
		 * Outer ingress VLAN
		 */
		resp->ingress_vlan[0].h_vlan_TCI = ovi.ingress[0].h_vlan_TCI;
		resp->ingress_vlan[0].h_vlan_encapsulated_proto = ovi.ingress[0].h_vlan_encapsulated_proto;

		/*
		 * Outer egress VLAN
		 */
		resp->egress_vlan[0].h_vlan_TCI = ovi.egress[0].h_vlan_TCI;
		resp->egress_vlan[0].h_vlan_encapsulated_proto = ovi.egress[0].h_vlan_encapsulated_proto;

		if (status == OVSMGR_FLOW_STATUS_ALLOW_VLAN_QINQ_ACCEL) {
			/*
			 * Inner ingress VLAN
			 */
			resp->ingress_vlan[1].h_vlan_TCI = ovi.ingress[1].h_vlan_TCI;
			resp->ingress_vlan[1].h_vlan_encapsulated_proto = ovi.ingress[1].h_vlan_encapsulated_proto;

			/*
			 * Inner egress VLAN
			 */
			resp->egress_vlan[1].h_vlan_TCI = ovi.egress[1].h_vlan_TCI;
			resp->egress_vlan[1].h_vlan_encapsulated_proto = ovi.egress[1].h_vlan_encapsulated_proto;

			return ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL;
		}
		return ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_ACCEL;
	case OVSMGR_FLOW_STATUS_ALLOW_ACCEL:
		return ECM_CLASSIFIER_OVS_RESULT_ALLOW_ACCEL;
	}

	return ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL;
}

static struct ecm_classifier_ovs_callbacks callbacks = {
	.ovs_process = ecm_ovs_process,
};

/*
 * ecm_ovs_init()
 */
static int __init ecm_ovs_init(void)
{
	int res;

	pr_info("ECM OVS Test INIT\n");

	/*
	 * Register the callbacks with the ECM ovs classifier.
	 */
	res = ecm_classifier_ovs_register_callbacks(&callbacks);
	if (res < 0) {
		pr_warn("Failed to register callbacks for OVS classifier\n");
		return res;
	}

	return 0;
}

/*
 * ecm_ovs_exit()
 */
static void __exit ecm_ovs_exit(void)
{
	pr_info("ECM OVS Test EXIT\n");

	/*
	 * Unregister the callbacks.
	 */
	ecm_classifier_ovs_unregister_callbacks();
}

module_init(ecm_ovs_init)
module_exit(ecm_ovs_exit)

MODULE_DESCRIPTION("ECM OVS Test");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif
